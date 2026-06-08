#ifndef USART_H
#define USART_H

void USART_init(uint16_t ubrr);
void USART_transmit(unsigned char data);
void USART_send_string(const char *str);

#endif