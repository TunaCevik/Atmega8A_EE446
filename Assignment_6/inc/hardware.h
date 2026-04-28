#ifndef HARDWARE_H
#define HARDWARE_H

#define F_CPU 4000000UL 
#define TARGET_BAUD 9600UL
#define SCL_CLOCK 100000UL

#define MYUBRR ((F_CPU / (16UL * TARGET_BAUD)) - 1)

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdlib.h> 


#endif