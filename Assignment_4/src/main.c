#include "hardware.h"
#include "timer.h"
#include "usart.h"
#include "pwm.h"
#include "INTinterrupt.h"

int main(void) { //
    char buffer[64]; //
    
    // System Initializations
    usart_init(MYUBRR); //
    init_zero_crossing(); //
    timer2_ctc_init(); //
    
    // Hardware Pin Configurations
    DDRB |= (1 << PB3); // Set PB3 (OC2) as output
    
    // Set PD3 and PD4 as input (buttons) and enable internal pull-ups
    DDRD &= ~((1 << PD3) | (1 << PD4)); //
    PORTD |= (1 << PD3) | (1 << PD4); //

    sei(); // Enable global interrupts
    usart_send_string("Frequency and Period Measurement Started...\r\n"); //

    while (1) { //
        
        // Button Logic: PD3 Increases Frequency (Decreases OCR2)
        if (!(PIND & (1 << PD3))) { 
            _delay_ms(50); // Simple debounce
            if (!(PIND & (1 << PD3))) {
                if (ocr2_value > 0) ocr2_value--;
                OCR2 = ocr2_value;
            }
        }
        
        // Button Logic: PD4 Decreases Frequency (Increases OCR2)
        if (!(PIND & (1 << PD4))) { 
            _delay_ms(50); // Simple debounce
            if (!(PIND & (1 << PD4))) {
                if (ocr2_value < 255) ocr2_value++;
                OCR2 = ocr2_value;
            }
        }

        if (zero_cross_detected) {
            period = (interrupt_interval * 16) / 1000; 
            
            if (interrupt_interval > 0) {
                frequency = 62500UL / interrupt_interval;
            }

            // Print and reset flag
            snprintf(buffer, sizeof(buffer), "Freq: %u Hz\r\n", frequency);
            usart_send_string(buffer);
            zero_cross_detected = 0;
        }
    }
}