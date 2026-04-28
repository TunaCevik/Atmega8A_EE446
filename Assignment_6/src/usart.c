#include "hardware.h"
#include "usart.h"

void USART_init(uint16_t ubrr) {
    UBRRH = (unsigned char)(ubrr >> 8);
    UBRRL = (unsigned char)ubrr;
    UCSRB = (1 << TXEN);
    UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);
}

void USART_transmit(unsigned char data) {
    while (!(UCSRA & (1 << UDRE))); // Wait for empty transmit buffer
    UDR = data;
}

void USART_send_string(const char *str) {
    while (*str) {
        USART_transmit(*str++);
    }
}