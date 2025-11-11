#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include "ring_buffer.h"
#include <stdio.h>
#include <string.h>
#include <avr/interrupt.h>

#define BUFFER_SIZE 32
#define EMPTY_VALUE 0xFF //est votre valeur non-bloquante (0xFF)
// Déclaration globale du buffer et de sa structure
volatile struct ring_buffer buffer;
volatile uint8_t buffer_data[BUFFER_SIZE];

#define FOSC 16000000UL // 16 MHz
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1

FILE uart_output;
FILE uart_input;

// Vecteur d'interruption pour la réception USART (RX Complete)
ISR(USART_RX_vect) {
    uint8_t data = UDR0; // Lire la donnée reçue
    
    // mettre la donnée sur le buffer
    ring_buffer_push((struct ring_buffer*)&buffer, data);
    
    // Le flag RXC0 est automatiquement effacé par la lecture de UDR0
}
uint8_t UART_getc() {
    uint8_t data = EMPTY_VALUE;
    
    // Tenter d'extraire une donnée du buffer
    // Le retour de ring_buffer_pop indique si l'extraction a réussi ou si le buffer est vide
    if (ring_buffer_pop((struct ring_buffer*)&buffer, &data) == 1) {
        // Succès: une donnée a été lue dans 'data'
        return data;
    } else {
        return EMPTY_VALUE;
    }
}

void UART_putc(uint8_t data) {
    // Attente active tant que le buffer d'émission n'est pas vide (UDRE0 = 1).
    while (!(UCSR0A & (1 << UDRE0)));
    // Place la donnée dans le registre UDR0 pour l'envoi
    UDR0 = data;
}

//Wrapper de sortie pour printf
int uart_put_wrapper(char c, FILE *stream) {
    UART_putc(c); 
    return 0;
}

//Wrapper d'entree pour get. Elle permet de retourner le caractere lu
int uart_get_wrapper(FILE *stream) {
    uint8_t data = UART_getc();
    if (data == EMPTY_VALUE) { 
        return _FDEV_EOF; 
    }
    return (int)data;
}

//Initialisation de l'UART
void USART_Init(){
    //Definir le Baud Rate 
    UBRR0H = (uint8_t)(MYUBRR>>8);
    UBRR0L = (uint8_t)MYUBRR;

    // RXEN0 (Bit 4) = 1 : Active le Récepteur
    // TXEN0 (Bit 3) = 1 : Active l'Émetteur
    // RXCIE0 (Bit 7 de UCSR0B) = 1 : Active l'interruption de réception complète
    UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);

    // 8 bits de données, 1 Stop bit, Pas de Parité
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

    //Initialisation du buffer circulaire
    ring_buffer_init((struct ring_buffer*)&buffer, (uint8_t*)buffer_data, BUFFER_SIZE);

    //Creation du flux de sortie en utilisant la fonction FDEV_SETUP_STREAM disponible sur la libc
    uart_output = FDEV_SETUP_STREAM(uart_put_wrapper, NULL, _FDEV_SETUP_WRITE);

    //Creation du flux d'entree idem 
    uart_input = FDEV_SETUP_STREAM(NULL, uart_get_wrapper, _FDEV_SETUP_READ);

    //Redirection des flux vers les entrées et sorties standards en C
    stdout = &uart_output;
    stdin = &uart_input;
    //Activation des interruptions
    sei();
}

//Initiatlisation de la LED
void LED_init(){
    DDRB |= (1 << PB5);
}


int main()
{
    //Initialisation
    LED_init();
    USART_Init();

 


    while (1) {
        // Lire un caractère
        uint8_t caractere_recu = UART_getc();

       //Uniquement si une donnee est disponible 
        if (caractere_recu != EMPTY_VALUE) { 
            printf("%c", caractere_recu); 
            
            //Allumer la LED
            PORTB |= (1 << PB5);
            
            //Attendre 5 secondes
            _delay_ms(5000);

            //Éteindre la LED
            PORTB &= ~(1 << PB5);
        }
    }
    return 0;
}
