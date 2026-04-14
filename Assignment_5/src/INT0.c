#include "INT0.h"

volatile uint8_t pcf_interrupt_flag = 0;

void INT0_Init(void) {
    // 1. Configure PD2 (INT0) as an input
    DDRD &= ~(1 << PD2);
    
    // 2. Enable internal pull-up resistor (crucial for open-drain INT pins)
    PORTD |= (1 << PD2);
    
    // 3. Configure INT0 to trigger on a FALLING edge
    MCUCR |= (1 << ISC01);
    MCUCR &= ~(1 << ISC00);
    
    // 4. Enable the INT0 interrupt specifically
    GICR |= (1 << INT0);
}

ISR(INT0_vect) {
    // Keep ISR thin: Just set the flag for the main loop to process
    pcf_interrupt_flag = 1;
}