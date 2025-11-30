#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>


#define PRESCALER 1024
#define OCR1A_VALUE 7812  

volatile uint8_t led_state = 0;
uint8_t pattern[] = {1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0};
uint8_t pattern_index = 0;
uint8_t pattern_length = sizeof(pattern);

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

void setup(void) {
    // Configuration LED intégrée (PB5)
    DDRB |= (1 << DDB5);
    PORTB &= ~(1 << PORTB5);
    
    // Configuration timer1
    timer1_init();
    
    // Activation interruptions globales
    sei();
    
    // Mode veille : IDLE
    set_sleep_mode(SLEEP_MODE_IDLE);
}

ISR(TIMER1_COMPA_vect) {
    // Gestion du motif de clignotement
    if (pattern_index < pattern_length) {
        if (pattern[pattern_index]) {
            PORTB |= (1 << PORTB5);  // LED ON
        } else {
            PORTB &= ~(1 << PORTB5); // LED OFF
        }
        pattern_index++;
    } else {
        pattern_index = 0;  // Retour au début du motif
    }
}

int main(void) {
    setup();
    
    // Désactivation des périphériques non utilisés
    PRR = (1 << PRTWI) | (1 << PRTIM2) | (1 << PRTIM0) | (1 << PRUSART0) | (1 << PRADC);
    
    while (1) {
        sleep_mode();  // Mise en veille
    }
}

/*#include <avr/interrupt.h>
#include <avr/io.h>
 Résistance de pull-up: pour forcer un état 1
débugger avec gdb: simavr --mcu atmega328p --freq 160000000UL --gdb program.elf

on peut aussi débugger avec des broches directement sur le microcontroleur

utiliser les registres TIMER qui incrémentent des registres jusqu'à
3 interrupts pour le compteur: TOV0, OC0A, OC0b
4 modes de foncitonnement:
- normal: counter (boucle de comptage) et une interruption
- counter0: mode à utiliser dans ce tp
- watchdog 




Comment communiquer avec mon code en c?:





int main(){
    return 0;
}*/