#include "hardware.h"
#include "timer.h"
#include "adc.h"
#include "usart.h"
#include "pwm.h"

int main(void) {
    char buffer[10];
    uint8_t light_percent = 0;
    uint8_t pwm_duty_percent = 0;
    
    uint32_t last_adc_task = 0;
    uint32_t last_usart_task = 0;

    // 1. Initialize all hardware modules
    usart_init(MYUBRR);
    adc_init();
    timer1_init_1ms();
    pwm_init(); // Initialize the DC Chopper driver

    // 2. Enable global interrupts
    sei(); 
    
    usart_send_string("System Started\r\n");

    // 3. The Event-Driven Superloop
    while(1) {
        
        // --- EVENT 1: Trigger ADC every 10 ms ---
        if ((system_tick - last_adc_task) >= 100) {
            last_adc_task = system_tick; 
            adc_start_conversion(); 
        }

        // --- EVENT 2: Process ADC Data when ready ---
        if (adc_ready == 1) {
            adc_ready = 0; // Lower the flag

            // Scale ADC value (0-1023) to Percentage (0-100)
            light_percent = (uint8_t)(((uint32_t)adc_value * 100) / 767);

            // Calculate INVERSE proportion for the DC Chopper
            // If light is 100%, PWM must be 0%. If light is 0%, PWM must be 100%.
            pwm_duty_percent = 100 - light_percent;
            
            // Update the hardware
            pwm_set_duty(pwm_duty_percent);
        }
        
        // --- EVENT 3: Transmit data periodically (every 2000 ms) ---
        if ((system_tick - last_usart_task) >= 500) {
            last_usart_task = system_tick;
            
            usart_send_string("Light: ");
            itoa(light_percent, buffer, 10);
            usart_send_string(buffer);
            usart_send_string("% | PWM Output: ");
            
            itoa(pwm_duty_percent, buffer, 10);
            usart_send_string(buffer);
            usart_send_string("%\r\n");
        }
        
    }
    return 0;
}