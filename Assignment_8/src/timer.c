#include "hardware.h"
#include "timer.h"
#include "modbus.h" // Include to update modbus flags

volatile uint32_t system_tick = 0;

void TIMER1_init_1ms(void) {
    OCR1A = 499; 
    TCCR1B = (1 << WGM12) | (1 << CS11);
    TIMSK |= (1 << OCIE1A);
}

// ISR fires exactly once every millisecond
ISR(TIMER1_COMPA_vect) {
    // MODIFICATION: Track silence on the bus lines.
    // If no character is received for more than 4ms, the current frame is done.
    if (++modbus_timer > 4) {
        frame_ready = 1;
        modbus_timer = 0;
    }
}