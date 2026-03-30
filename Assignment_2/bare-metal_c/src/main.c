#include "hardware.h"
#include "timer.h"
#include "adc.h"
#include "usart.h"

int main(void) {
    char buffer[10];
    uint8_t percentage;
    uint32_t last_adc_task_time = 0;

    // 1. Initialize all hardware modules
    usart_init(MYUBRR);
    adc_init();
    timer1_init_1ms();

    // 2. Turn on the main breaker
    sei(); 

    // 3. The Infinite Superloop
    while(1) {
        
        // --- SCHEDULER: Trigger ADC every 1000 milliseconds ---
        if ((system_tick - last_adc_task_time) >= 1000) {
            last_adc_task_time = system_tick; // Immediately update timestamp
            adc_start_conversion();           // Tell ADC hardware to start
        }

        // --- EVENT HANDLER: When ADC finishes ---
        if (adc_ready == 1) {
            adc_ready = 0; // Lower the flag immediately

            // Calculate percentage safely
            percentage = (uint8_t)(((uint32_t)adc_value * 100) / 1023);

            // Transmit data
            usart_send_string("LDR Light Intensity: ");
            itoa(percentage, buffer, 10);
            usart_send_string(buffer);
            usart_send_string(" %\r\n");
        }
        
    }
    return 0;
}