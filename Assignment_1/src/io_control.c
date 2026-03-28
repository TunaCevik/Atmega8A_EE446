#include "hardware.h"

void IO_Config(void) {
    // Set outputs
    DDRB |= (1 << RELAY1) | (1 << RELAY2) | (1 << MOSFET) | (1 << MOC3021);
    DDRD |= (1 << TRANSISTOR);

    // Set input and enable pull-up
    DDRD &= ~(1 << BUTTON1);
    PORTD |= (1 << BUTTON1);
}

void Turn_Off_All(void) {
    PORTB &= ~((1 << RELAY1) | (1 << RELAY2) | (1 << MOSFET) | (1 << MOC3021));
    PORTD &= ~(1 << TRANSISTOR);
}

void Turn_On_All(void) {
    PORTB |= (1 << RELAY1) | (1 << RELAY2) | (1 << MOSFET) | (1 << MOC3021);
    PORTD |= (1 << TRANSISTOR);
}