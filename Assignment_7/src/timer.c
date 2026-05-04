#include "hardware.h"
#include "timer.h"

// The master heartbeat of the entire system
volatile uint32_t system_tick = 0;

void timer1_init_1ms(void) {
    OCR1A = 499; // 1ms period
    // CTC Mode, Prescaler 8
    TCCR1B = (1 << WGM12) | (1 << CS11);
    // Enable Compare Match A Interrupt
    TIMSK |= (1 << OCIE1A);
}

// ISR fires exactly once every millisecond
ISR(TIMER1_COMPA_vect) {
    system_tick++;
}

void Timer2_PWM_init(void) {
    // Set PB3 (OC2) as output for the PWM signal
    DDRB |= (1 << PB3); 

    // Configure Timer2 for ATmega8A:
    // WGM21 = 1, WGM20 = 1: Fast PWM Mode
    // COM21 = 1, COM20 = 0: Non-Inverting Mode (Clear OC2 on compare match, set at BOTTOM)
    // CS22 = 1, CS21 = 0, CS20 = 0: Prescaler set to 64
    TCCR2 = (1 << WGM21) | (1 << WGM20) | (1 << COM21) | (1 << CS22);
    
    // Initialize duty cycle to 0 (Heater OFF)
    OCR2 = 0; 
}

void Timer2_PWM_set_duty(uint8_t duty) {
    // ATmega8A uses OCR2 instead of OCR2A
    OCR2 = duty;
}