#ifndef HARDWARE_H
#define HARDWARE_H

#define MOSFET PB3

#define F_CPU 4000000UL 
#define TARGET_BAUD 9600UL

// 1. Calculate the ideal UBRR value
#define MYUBRR ((F_CPU / (16UL * TARGET_BAUD)) - 1)

// 2. Calculate what the ACTUAL baud rate will be on the silicon
#define ACTUAL_BAUD (F_CPU / (16UL * (MYUBRR + 1)))

// 3. Calculate the error scaled by 1000 (0.2% error = 2 per 1000)
// We use a ternary operator (? :) to act as an absolute value function |x|
#define BAUD_ERROR_PER_MILLE (((ACTUAL_BAUD > TARGET_BAUD) ? (ACTUAL_BAUD - TARGET_BAUD) : (TARGET_BAUD - ACTUAL_BAUD)) * 1000UL / TARGET_BAUD)

// 4. The Compiler Guard!
#if (BAUD_ERROR_PER_MILLE > 2)
    #error "CRITICAL: USART Baud Rate error exceeds the 0.2% tolerance!"
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <util/delay.h>

#endif