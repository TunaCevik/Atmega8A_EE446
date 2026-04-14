#ifndef PCF8574_H
#define PCF8574_H

#include "hardware.h"
#include "I2C.h"

#define PCF8574_ADDRESS 0x40 // Base Address (A2 A1 A0 = 000) shifted for R/W

void PCF8574_Write(uint8_t data);
uint8_t PCF8574_Read(void);

#endif