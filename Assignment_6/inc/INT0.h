#ifndef INT0_H
#define INT0_H

#include "hardware.h"

// Extern declares the variable exists, but memory is allocated in INT0.c
extern volatile uint8_t pcf_interrupt_flag;

void INT0_Init(void);

#endif