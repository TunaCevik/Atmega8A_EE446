#include "hardware.h"
#include "usart.h"
#include "modbus.h" // Include to access the shared buffer variables

void USART_init(uint16_t ubrr) {
    UBRRH = (unsigned char)(ubrr >> 8);
    UBRRL = (unsigned char)ubrr;
    
    // MODIFICATION: Added RXEN and RXCIE to catch data via interrupts
    UCSRB = (1 << TXEN) | (1 << RXEN) | (1 << RXCIE);
    UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0); // 8-bit data frame
}

void USART_transmit(unsigned char data) {
    while (!(UCSRA & (1 << UDRE))); 
    UDR = data;
}

void USART_send_string(const char *str) {
    while (*str) {
        USART_transmit(*str++);
    }
}

// MODIFICATION: USART Rx Complete Interrupt Handler
ISR(USART_RXC_vect) {
    uint8_t data = UDR; // Read out byte from buffer immediately
    if (rx_index < BUFFER_SIZE) {
        rx_buffer[rx_index++] = data;
    }
    modbus_timer = 0; // Clear silence duration countdown timer
}