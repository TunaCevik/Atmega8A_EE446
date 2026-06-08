#include "hardware.h"
#include "timer.h"
#include "modbus.h"

volatile uint32_t system_tick = 0;

void TIMER1_init_1ms(void) {
    OCR1A = 499; 
    TCCR1B = (1 << WGM12) | (1 << CS11);
    TIMSK |= (1 << OCIE1A);
}

ISR(TIMER1_COMPA_vect) {
    system_tick++;
    if (++modbus_timer > 4) {
        frame_ready = 1;
        modbus_timer = 0;
    }
}

void Timer2_PWM_init(void) {
    DDRB |= (1 << PB3); 

    TCCR2 = (1 << WGM21) | (1 << WGM20) | (1 << COM21) | (1 << CS22);
    
    OCR2 = 0; 
}

void Timer2_PWM_set_duty(uint8_t duty) {
    OCR2 = duty;
}