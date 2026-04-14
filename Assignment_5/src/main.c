#include "hardware.h"
#include "I2C.h"
#include "pcf8574.h"
#include "INT0.h"
#include "usart.h" // Contains USART_Init and USART_Print

int main(void) {
    uint8_t input_data;

    // 1. System Initializations
    I2C_Init();
    usart_init(MYUBRR);
    INT0_Init();
    
    // 2. Local  Pin Configurations
    DDRB |= (1 << PB3) | (1 << PB6); // Set PB3 and PB6 as outputs
    PORTB &= ~((1 << PB3) | (1 << PB6)); // Ensure LEDs start in the OFF state
    
    // 3. Remote Hardware Initialization
    // Write 0xFF to set all PCF8574 pins HIGH (Input mode with weak pull-ups)
    PCF8574_Write(0xFF);
    // 4. Enable Global Interrupts
    sei();

    usart_send_string("System Ready. Waiting for PCF8574 Interrupt...\r\n");

    while (1) {
        
        // Constraint 3: PCF8574 is ONLY read in response to the INT0 interrupt
        if (pcf_interrupt_flag) {
            
            // Fetch the current pin states from the I2C expander
            input_data = PCF8574_Read();
            
            // Constraint 1: P0 button controls PB6 LED and UART print
            // Check if Bit 0 is LOW (Button Pressed)
            if (!(input_data & (1 << 0))) { 
                PORTB |= (1 << PB6); // Turn ON PB6 LED
                usart_send_string("Button0 pressed\r\n");
            } else {
                PORTB &= ~(1 << PB6); // Turn OFF PB6 LED
            }
            
            // Constraint 2: P1 button controls PB3 LED and UART print
            // Check if Bit 1 is LOW (Button Pressed)
            if (!(input_data & (1 << 1))) { 
                PORTB |= (1 << PB3); // Turn ON PB3 LED
                usart_send_string("Button1 pressed\r\n");
            } else {
                PORTB &= ~(1 << PB3); // Turn OFF PB3 LED
            }
            
            // Clear the flag to wait for the next interrupt
            pcf_interrupt_flag = 0;
            
            // Debounce delay to prevent the UART terminal from being flooded
            // with messages from a single physical button press
            _delay_ms(50); 
        }
    }
}