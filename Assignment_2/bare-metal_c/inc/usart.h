#ifndef USART_H
#define USART_H

void usart_init(uint16_t ubrr);
void usart_transmit(unsigned char data);
void usart_send_string(const char *str);

#endif