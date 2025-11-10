#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

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
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);

    // 8 bits de données, 1 Stop bit, Pas de Parité
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

//Initiatlisation de la LED
void LED_init(){
    DDRB |= (1 << PB5);
}

uint8_t UART_getc() {
    // Attente active tant que le bit RXC0 n'est pas à 1.
    while (!(UCSR0A & (1 << RXC0)));
    // Retourne le contenu du registre de données UDR0 (lecture efface RXC0)
    return UDR0;
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
