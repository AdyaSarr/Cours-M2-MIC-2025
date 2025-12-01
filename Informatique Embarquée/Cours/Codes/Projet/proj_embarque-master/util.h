#if !defined(UTIL_H)
#define UTIL_H
#include <stdbool.h>

#define COMMAND_LIST_CREDENTIALS 0
#define COMMAND_MAKE_CREDENTIAL 1
#define COMMAND_GET_ASSERTION 2
#define COMMAND_RESET 3
#define STATUS_OK 0
#define STATUS_ERR_COMMAND_UNKNOWN 1
#define STATUS_ERR_CRYPTO_FAILED 2
#define STATUS_ERR_BAD_PARAMETER 3
#define STATUS_ERR_NOT_FOUND 4
#define STATUS_ERR_STORAGE_FULL 5
#define STATUS_ERR_APPROVAL 6



#define CREDENTIAL_ID_SIZE 16
#define PUBLIC_KEY_SIZE 40
#define APP_ID_SIZE 20
#define SIGNATURE_SIZE 40

//Les differents etats de l'authenticator
typedef enum {
    STATE_IDLE,//etat inital de repos
    STATE_RECEIVING,//etat de reception du premier octet MakeCredential
    STATE_WAIT_CONSENT,//etat d'attente du consentement de l'utilisateur
    STATE_PROCESS_CRYPTO,//le consentement a ete obtenu geneartion de la paire de cle
} authenticator_state_t;

typedef struct {
    uint8_t start; 
    // 20 octets pour SHA1(app_id), transférés en little-endian.
    uint8_t SHA1_app_id[APP_ID_SIZE]; 
} MakeCredential_t;


// Structure d'une entrée stockée dans l'EEPROM
typedef struct {
    bool is_used; // Flag d'utilisation (1 octet)
    uint8_t SHA1_app_id[APP_ID_SIZE]; // 20 octets
    uint8_t credential_id[CREDENTIAL_ID_SIZE]; // 16 octets
    uint8_t private_key[21]; // 21 octets (pour secp160r1)
    // Taille totale : 1 + 20 + 16 + 21 = 58 octets
} CredentialEntry_t;

typedef struct {
    uint8_t status;
    // 16 octets de credential_id
    uint8_t credential_id[CREDENTIAL_ID_SIZE]; 
    // 40 octets de clé publique
    uint8_t public_key[PUBLIC_KEY_SIZE];
} MakeCredentialResponse_t;

typedef struct {
    uint8_t start;
    // 20 octets de SHA1(app_id)
    uint8_t SHA1_app_id[APP_ID_SIZE];
    // 20 octets de clientDataHash
    uint8_t clientDataHash[APP_ID_SIZE];
} GetAssertion_t;


/**
 * @brief Cette fonction permet de lire un octet sur le buffer
 * @return retourne l'octet lu sur le buffer si cela existe sinon il retour la valeur EMPTY_VALUE
 */
uint8_t UART_getc();

/**
 * @brief Cette fonction permet d'envoyer un seul octet
 * @param data: valeur qui sera mis sur le registre UDR0 
 */
void UART_putc(uint8_t data);

/**
 * @brief Fonction d'initialisation de l'UART à 115200 bauds et le buffer
 */
void USART_Init_115200();

/**
 * @brief Fonction d'initialoisation du materiel et configuration de la LED
 */
void hardware_init();

/**
 * @brief Fonction pour envoyer un message d'erreur (STATUS_ERR_*)
 * @param error_code: le code d'erreur a transmettre
 */
void send_error_response(uint8_t error_code);

/**
 * @brief Pour cette implémentation, nous allons utiliser la broche analogique ADC0 
 * @brief comme source de bruit, ce qui est la même broche pour notre bouton (BUTTON_PIN)
 * @brief Initialisation de l'ADC pour lire la source de bruit (ADC0)
 */
void ADC_init();

/**
 * @brief Extraction de l'Entropie:
 * @brief Une fonction pour générer un octet aléatoire en prenant plusieurs échantillons rapides dans notre cas 8(pour extraire 1 bit par échantillon)
 * @brief Nous ne conservons que les bits de poids faible (LSB) car ce sont les plus susceptibles de contenir du bruit imprévisible.
 * @brief cette fonction génère un octet aléatoire en échantillonnant l'ADC plusieurs fois
 * @return un octet aleatoire
 */
uint8_t generate_random_byte();

/**
 * @brief Prototype de la fonction d'aléatoire pour micro-ecc
 * @brief Elle doit remplir le buffer p_dest avec size octets aléatoires avec l'alea extrat de l'ADC.
 * @param p_dest le buffer a remplir
 * @param size la taille
 */
int micro_ecc_random_bytes(uint8_t *p_dest, unsigned p_size);

/**
 * @brief Cette fonction permet de:
 * @brief Rechercher
 * @brief Stocker
 * @brief Remplacer
 * @brief Une entrée
 * @param sha1_app_id: le hash 
 * @param credential_id: l'idetifiant du client
 * @param private_key: la clé privée du client
 * @return STATUS_OK ou STATUS_ERR_STORAGE_FULL
 */
uint8_t write_credential(const uint8_t sha1_app_id[APP_ID_SIZE], const uint8_t credential_id[CREDENTIAL_ID_SIZE], const uint8_t private_key[21]);

/**
 * @brief Cette fonction envoie la réponse de succés MakeCredential (STATUS_OK, ID, Clé Publique)
 * @param credential_id: l'idetifiant du client
 * @param public_key: la clé privée du client
 */
void send_make_credential_response(const uint8_t credential_id[CREDENTIAL_ID_SIZE], const uint8_t public_key[PUBLIC_KEY_SIZE]);

/**
 * @brief Cette fonction permet de desactiver les peripheriques non utilisés pour economiser de l'energie
 */
void economy_energy();

/**
 * @brief Cette fonction permet de d'initialiser sur la main tout ce qui doit l'etre
 */
void init_ALL();

#endif // UTIL_H