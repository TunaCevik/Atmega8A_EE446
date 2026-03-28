#include "hardware.h"
#include <util/delay.h>


int main(void) {
    IO_Config();

    while (1) {
        if (!(PIND & (1 << BUTTON1))) { 
            // Logic implementation
            PORTB |= (1 << RELAY1);
            _delay_ms(1000);
            
            PORTB |= (1 << RELAY2);
            _delay_ms(1000);

            PORTB |= (1 << MOSFET);
            _delay_ms(1000);

            PORTB |= (1 << MOC3021);
            _delay_ms(1000);

            Turn_On_All();
            _delay_ms(2000); 
            Turn_Off_All();              

            // Debounce / Wait for release
            while (!(PIND & (1 << BUTTON1)));
            _delay_ms(50);
        }
    }
    return 0;
}