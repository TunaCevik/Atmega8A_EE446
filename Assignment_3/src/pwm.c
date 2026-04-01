#include "hardware.h"
#include "pwm.h"

void pwm_init(void) {
    // 1. Set PB3 as an output pin for the PWM signal
    DDRB |= (1 << MOSFET); 

    // 2. Configure Timer2 for Fast PWM Mode
    // WGM21=1, WGM20=1 (Fast PWM)
    // COM21=1, COM20=0 (Clear OC2 on Compare Match, non-inverting)
    // CS22=0, CS21=1, CS20=0 (Prescaler 8)
    TCCR2 = (1 << WGM21) | (1 << WGM20) | (1 << COM21) | (1 << CS21);
    
    // 3. Initialize with 0% duty cycle
    OCR2 = 0;
}

void pwm_set_duty(uint8_t percentage) {
    // Safely clamp the percentage to 100
    if (percentage > 100) {
        percentage = 100;
    }
    // Scale 0-100% to 0-255 for the 8-bit Timer2 Register
    OCR2 = (uint8_t)(((uint16_t)percentage * 255) / 100);
}