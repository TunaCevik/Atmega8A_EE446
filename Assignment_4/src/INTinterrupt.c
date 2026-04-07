#include "hardware.h"
#include "INTinterrupt.h"
#include "timer.h"

volatile uint8_t zero_cross_detected = 0; //
volatile uint16_t interrupt_interval = 1; //
volatile uint16_t frequency = 0; //
volatile uint16_t period = 0; //

// INT0 Initialization for Edge Detection (Zero Crossing)
void into_init() { //
    // PD2 is configured as input for INT0
    DDRD &= ~(1 << PD2); //
    
    // Trigger on Rising Edge (when AC crosses zero and optocoupler turns on)
    MCUCR |= (1 << ISC01) | (1 << ISC00);
    // Enable INT0 interrupt
    GICR |= (1 << INT0);
}

// Zero Crossing Detection Configuration
void init_zero_crossing() { //
    into_init();
    init_timer1();
}

ISR(INT0_vect) {
    // Only do the bare minimum here!
    interrupt_interval = TCNT1;
    TCNT1 = 0; 
    
    // Just set the flag and get out
    zero_cross_detected = 1; 
}