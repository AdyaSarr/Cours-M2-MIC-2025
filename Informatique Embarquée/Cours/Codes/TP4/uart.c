#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include "ring_buffer.h"
#define BUFFER_SIZE 32
#define EMPTY_VALUE 0xFF
// Déclaration globale du buffer et de sa structure
volatile struct ring_buffer buffer;
volatile uint8_t buffer_data[BUFFER_SIZE];

#define FOSC 16000000UL // 16 MHz
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1

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

    //Activation des interruptions
    sei();
}

//Initiatlisation de la LED
void LED_init(){
    DDRB |= (1 << PB5);
}

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
    if (ring_buffer_pop((struct ring_buffer*)&rx_buffer, &data) == 1) {
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

int main()
{
    //Initialisation
    LED_init();
    UART_init();
    while (1) {
        // Lire un caractère 
        uint8_t caractere_recu = UART_getc();

        // Renvoyer le même caractère
        UART_putc(caractere_recu);
        //Allumer la LED
        PORTB |= (1 << PB5);
        //Attendre 5 secondes
        _delay_ms(5000);

        //etteindre la LED
        PORTB &= ~(1 << PB5);
    }
    return 0;
}
