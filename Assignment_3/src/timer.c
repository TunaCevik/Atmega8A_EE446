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