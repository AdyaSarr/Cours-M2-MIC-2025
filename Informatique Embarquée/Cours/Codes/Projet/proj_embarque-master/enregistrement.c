#define uECC_PLATFORM uECC_avr // Définit la plateforme comme AVR (valeur 7 dans uECC.h)
#define uECC_VLI_NATIVE_LITTLE_ENDIAN 1 // Optimisation pour architecture AVR (little-endian)
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include "ring_buffer.h" // Fichier externe
#include "util.h"        // Fichier d'inclusion fourni par l'utilisateur
#include <avr/sleep.h>
#include <avr/eeprom.h>
#include <string.h>
#include "../micro-ecc-master/uECC.h"



// --- Constantes Timer 1 (pour 500 ms) ---
// F_CPU / (Prescaler * Intervalle_ms) = OCR1A + 1
// 16,000,000 / (1024 * 500) = 31.25. 
// Pour 500ms, nous cherchons le nombre de cycles : (16,000,000 / 1024) * 0.5s = 7812.5
#define PRESCALER_1024_DIVIDER 1024
#define TIMER1_INTERVAL_MS 500
#define OCR1A_VALUE 7812

// --- Constantes UART et Buffer ---
#define FOSC 16000000UL // 16 MHz
#define BAUD 115200 
#define MYUBRR (FOSC/16/BAUD)-1

#define BUFFER_SIZE 32
#define EMPTY_VALUE 0xFF // Valeur pour indiquer qu'aucune donnée n'est disponible(le prof avait proposer une autre methode: oublier)

// ---- Déclaration globale du buffer---
volatile struct ring_buffer buffer;
volatile uint8_t buffer_data[BUFFER_SIZE];

// --- Définition des Périphériques ---
#define LED_PIN PB5 // Définit la broche logique de la LED : Bit 5 du PORT B
#define BUTTON_PIN PC0 // Définit la broche logique du bouton : Bit 0 du PORT C (Broche A0 de l'Arduino).


// --- Variables de Gestion du Temps (Timer 0) ---
volatile uint32_t timer_500ms_ticks = 0;
volatile authenticator_state_t current_state = STATE_IDLE;
uint32_t consent_timer_start_500ms_ticks = 0;
uint32_t led_blink_timer_ms = 0;


//La taille maximum d'entree que l'on peut stocker sur la memoire EEPROM du microcontroleur et la definittion de l'emplacement du tab de structure en EEPROM
#define MAX_CREDENTIALS 17
CredentialEntry_t EEMEM CREDENTIALS_STORAGE[MAX_CREDENTIALS];

//Taille des parametres des cles
#define PUBLIC_KEY_SIZE 40




// ============================================================
// Fonctions UART et Interruption
// ============================================================

//S'execute automatiquement et interrompt le main des d'un octet est entierement recu sur la liason serie (UART)
ISR(USART_RX_vect) {
    uint8_t data = UDR0;//lit l'octet recu sur le regsitre de données UART(UDR0)
    ring_buffer_push((struct ring_buffer*)&buffer, data);//ajoute l'octet lu sur le buffer circulair
}

// --- Timer 1 ISR (pour 500 ms) ---
ISR(TIMER1_COMPA_vect) {
    if (current_state == STATE_WAIT_CONSENT) {
        // Clignotement : Inverser l'état de la LED toutes les 500 ms
        PORTB ^= (1 << LED_PIN); 
    }
    // Incrémenter le compteur de cycles (500ms/cycle)
    timer_500ms_ticks++; 
}

void timer1_init(void) {
    // Mode CTC avec OCR1A
    TCCR1A = 0;
    TCCR1B = (1 << WGM12);
    
    // Prescaler 1024
    TCCR1B |= (1 << CS12) | (1 << CS10);
    
    // Valeur de comparaison pour 500ms
    OCR1A = OCR1A_VALUE;
    
    // Activation interruption comparaison A
    TIMSK1 = (1 << OCIE1A);
}

//Lecture d'un octet sur le buffer
uint8_t UART_getc() {
    uint8_t data = EMPTY_VALUE;
    if (ring_buffer_pop((struct ring_buffer*)&buffer, &data) == 1) {
        return data;
    } else {
        return EMPTY_VALUE;
    }
}
//fonction qui envoie un seul octet
void UART_putc(uint8_t data) {
    while (!(UCSR0A & (1 << UDRE0))); // Attente du buffer vide
    UDR0 = data;
}

// Initialisation de l'UART à 115200 bauds
void USART_Init_115200(){
    UBRR0H = (uint8_t)(MYUBRR>>8);//(H, L)Chargé la valeur calculée pour le baud 115200
    UBRR0L = (uint8_t)MYUBRR;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
    ring_buffer_init((struct ring_buffer*)&buffer, (uint8_t*)buffer_data, BUFFER_SIZE);//initialisation du buffer
}

// Initialisation du materiel:
// Configuration de la LED (la Sortie)
void hardware_init(){
    DDRB |= (1 << LED_PIN);//pour la led
    // Configuration du Bouton (Entrée avec une resistance de Pull-up)
    DDRC &= ~(1 << BUTTON_PIN); 
    PORTC |= (1 << BUTTON_PIN);
}

// Fonction utilitaire pour envoyer un message d'erreur (STATUS_ERR_*)
void send_error_response(uint8_t error_code) {
    UART_putc(error_code); 
}


// =================================================================================
// Génération et Stockage des Clés
// =================================================================================

/**
 * Pour cette implémentation, nous allons utiliser la broche analogique ADC0 
 * comme source de bruit, ce qui est la même broche que votre bouton (BUTTON_PIN)
 * @brief Initialisation de l'ADC pour lire la source de bruit (ADC0)
 */
void ADC_init() {
    ADMUX = (1 << REFS0);
    // Comme PC0 est notre bouton, on s'assure que cette broche peut être lue.
    ADMUX |= 0; // ADC0 = 0000
    // Activation de  l'ADC (ADEN=1)
    // Prescaler 128 : (ADPS2=1, ADPS1=1, ADPS0=1)
    // Ceci donne F_ADC = 16MHz / 128 = 125 kHz (vitesse standard pour 10 bits de résolution)
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

/**
 * Extraction de l'Entropie:
 * Une fonction pour générer un octet aléatoire en prenant plusieurs échantillons rapides
 * Nous ne conservons que les bits de poids faible (LSB) car ce sont les plus susceptibles de contenir du bruit imprévisible.
 * @brief cette fonction génère un octet aléatoire en échantillonnant l'ADC plusieurs fois
 */
uint8_t generate_random_byte() {
    uint8_t random_byte = 0;
    
    // échantillonne l'ADC 8 fois pour extraire 1 bit par échantillon
    for (int i = 0; i < 8; i++) {
        // ON demarre la conversion (ADSC=1)
        ADCSRA |= (1 << ADSC); 
        // Attend que la conversion soit terminée (ADIF = 1)
        while (ADCSRA & (1 << ADSC));

        // On deplace le bit le plus à droite (le plus aléatoire) dans random_byte
        // On utilise l'un des bits de poids faible de ADCL.
        // ADCL contient les 8 bits de poids faible.
        random_byte = (random_byte << 1) | (ADCL & 0x01); 
        // Retarder un peu pour que la source de bruit évolue
        _delay_us(10); 
    }
    return random_byte;
}

/**
 * @brief Prototype de la fonction d'aléatoire pour micro-ecc
 * @brief Elle doit remplir le buffer p_dest avec size octets aléatoires.
 * @param p_dest le buffer a remplir
 * @param size la taille
 */
int micro_ecc_random_bytes(uint8_t *p_dest, unsigned p_size) {
    for (unsigned i = 0; i < p_size; i++) {
        // Remplir le buffer avec l'aléatoire extrait de l'ADC
        p_dest[i] = generate_random_byte();
    }
    
    // C'est ici que l'on pourrait ajouter l'événement bouton pour le "seeding"
    // Exemple : si le consentement vient d'être donné, mélanger l'aléatoire
    // avec un hachage de TCNT1 ou autre source de gigue.
    
    return 1; // 1 pour succès, 0 pour échec
}

// Recherche, stocke ou remplace une entrée. Retourne STATUS_OK ou STATUS_ERR_STORAGE_FULL.
uint8_t write_credential(const uint8_t sha1_app_id[APP_ID_SIZE], const uint8_t credential_id[CREDENTIAL_ID_SIZE], const uint8_t private_key[21]) {
    int empty_slot = -1;
    for (int i = 0; i < MAX_CREDENTIALS; i++) {
        CredentialEntry_t current_entry;
        // Lecture de l'entrée actuelle depuis l'EEPROM
        eeprom_read_block(&current_entry, &CREDENTIALS_STORAGE[i], sizeof(CredentialEntry_t));

        if (current_entry.is_used) {
            // Vérifier si SHA1(app_id) correspond (Remplacement)
            if (memcmp(current_entry.SHA1_app_id, sha1_app_id, APP_ID_SIZE) == 0) {
                empty_slot = i; // On utilise ce slot pour la mise à jour
                break;
            }
        } else {
            // Si le slot est libre, le mémoriser
            if (empty_slot == -1) {
                empty_slot = i;
            }
        }
    }

    //Traitement du résultat de la recherche
    if (empty_slot == -1) {
        // La mémoire est pleine et le RP n'a pas été trouvé.
        return STATUS_ERR_STORAGE_FULL;
    } else {
        // Créer la nouvelle entrée
        CredentialEntry_t new_entry;
        new_entry.is_used = true;
        memcpy(new_entry.SHA1_app_id, sha1_app_id, APP_ID_SIZE);
        memcpy(new_entry.credential_id, credential_id, CREDENTIAL_ID_SIZE);
        memcpy(new_entry.private_key, private_key, 21);
        
        // Écrire la structure complète dans le slot trouvé
        eeprom_write_block(&new_entry, &CREDENTIALS_STORAGE[empty_slot], sizeof(CredentialEntry_t));
        
        return STATUS_OK;
    }
}

// Envoie la réponse de succès MakeCredential (STATUS_OK, ID, Clé Publique)
void send_make_credential_response(const uint8_t credential_id[CREDENTIAL_ID_SIZE], const uint8_t public_key[PUBLIC_KEY_SIZE]) {
    
    //Envoyer le statut OK (0x00)
    UART_putc(STATUS_OK);
    
    //Envoyer le credential_id (16 octets, little-endian)
    for (int i = 0; i < CREDENTIAL_ID_SIZE; i++) {
        UART_putc(credential_id[i]);
    }

    //Envoyer la clé publique (40 octets, little-endian)
    for (int i = 0; i < PUBLIC_KEY_SIZE; i++) {
        UART_putc(public_key[i]);
    }
}

// =================================================================================
// Initialisation et Attente de la Requête
// =================================================================================
int main()
{
    //Initialisation
    hardware_init(); 
    USART_Init_115200(); 
    timer1_init();
    ADC_init();


    uECC_set_rng(micro_ecc_random_bytes);
    // Configuration du mode veille
    set_sleep_mode(SLEEP_MODE_IDLE);

    // Activation des interruptions globales
    sei();

    // Déclaration de la structure pour stocker la requête
    MakeCredential_t request;
    uint8_t* target_ptr = (uint8_t*)&request;
    const uint8_t REQUEST_TOTAL_SIZE = sizeof(MakeCredential_t);
    
    current_state = STATE_IDLE; 

    // Désactivation des périphériques non utilisés (économie d'énergie)
    PRR = (1 << PRTWI) | (1 << PRTIM2) | (1 << PRTIM0) | (1 << PRADC) | (1 << PRSPI); 

    uint8_t bytes_received = 0;
    while (1) {
        
        // -------------------------------------------------------------------------
        // Gestion de la Réception
        // -------------------------------------------------------------------------
        uint8_t data = UART_getc();
        if (data != EMPTY_VALUE) {
            
            if (current_state == STATE_IDLE || current_state == STATE_RECEIVING) {
                
                if (bytes_received == 0) {
                    if (data == COMMAND_MAKE_CREDENTIAL) {
                        current_state = STATE_RECEIVING;
                        target_ptr[bytes_received++] = data;
                    } 
                } else {
                    target_ptr[bytes_received++] = data;

                    if (bytes_received == REQUEST_TOTAL_SIZE) {
                        bytes_received = 0;
                        
                        // --- Attendre le Consentement ---
                        current_state = STATE_WAIT_CONSENT;
                        consent_timer_start_500ms_ticks = timer_500ms_ticks;
                    }
                    
                    if (bytes_received >= REQUEST_TOTAL_SIZE) {
                         send_error_response(STATUS_ERR_BAD_PARAMETER);
                         current_state = STATE_IDLE;
                         bytes_received = 0;
                    }
                }
            }
        }

        // -------------------------------------------------------------------------
        //  Gestion du Consentement
        // -------------------------------------------------------------------------
        if (current_state == STATE_WAIT_CONSENT) {

            // Gestion du Timeout: 10 secondes = 20 cycles de 500 ms
            if (timer_500ms_ticks - consent_timer_start_500ms_ticks >= 20) {
                
                PORTB &= ~(1 << LED_PIN); // Éteindre la LED
                send_error_response(STATUS_ERR_APPROVAL);
                current_state = STATE_IDLE;
            }

            // Lecture du Bouton (Consentement)
            else if (!(PINC & (1 << BUTTON_PIN))) {
                
                PORTB &= ~(1 << LED_PIN);// Arrêter le clignotement
                current_state = STATE_PROCESS_CRYPTO;
            }
        }

        // -------------------------------------------------------------------------
        // Traitement Cryptographique
        // -------------------------------------------------------------------------
        if (current_state == STATE_PROCESS_CRYPTO) {
    
            uint8_t private_key[21]; 
            uint8_t public_key[PUBLIC_KEY_SIZE];
            uint8_t credential_id[CREDENTIAL_ID_SIZE];
            uint8_t result_status = STATUS_OK;

            // --- Génération de la Paire de Clés ECDSA (secp160r1) ---
            // Clé privée (21B) et Clé publique (40B). Utilise la fonction RNG définie.
            if (uECC_make_key(public_key, private_key, uECC_secp160r1()) != 1) { 
                result_status = STATUS_ERR_CRYPTO_FAILED;
            }
            
            // ---Génération du Credential ID (16 octets) ---
            // Utilise la fonction RNG pour garantir l'unicité de l'ID.
            if (result_status == STATUS_OK) {
                if (micro_ecc_random_bytes(credential_id, CREDENTIAL_ID_SIZE) != 1) {
                    result_status = STATUS_ERR_CRYPTO_FAILED; 
                }
            }
            
            // ---Stockage en EEPROM ---
            if (result_status == STATUS_OK) {
                // Enregistre l'empreinte RP, l'ID et la clé privée
                result_status = write_credential(request.SHA1_app_id, credential_id, private_key);
            }

            // ---Réponse ---
            if (result_status == STATUS_OK) {
                send_make_credential_response(credential_id, public_key);
            } else {
                send_error_response(result_status);
            }

            current_state = STATE_IDLE; 
        }

        // Mise en Veille : Réveillé par l'interruption du Timer 1 ou la réception UART
        sleep_mode(); 
    }
    return 0;
}