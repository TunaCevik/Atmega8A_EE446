#include "pcf8574.h"

void PCF8574_Write(uint8_t data) {
    I2C_Start(); 
    I2C_Write(PCF8574_ADDRESS); // Write mode (R/W bit = 0)
    I2C_Write(data); 
    I2C_Stop(); 
}

uint8_t PCF8574_Read(void) {
    uint8_t data;
    I2C_Start(); 
    I2C_Write(PCF8574_ADDRESS | 0x01); // Read mode (R/W bit = 1)
    data = I2C_Read_NACK(); 
    I2C_Stop(); 
    return data; 
}