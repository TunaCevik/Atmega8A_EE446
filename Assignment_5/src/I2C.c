#include "I2C.h"

void I2C_Init(void) { 
    TWSR = 0x00; // Prescaler = 1
    TWBR = ((F_CPU/SCL_CLOCK) - 16) / 2;
}

void I2C_Start(void) { 
    TWCR = (1<<TWSTA) | (1<<TWEN) | (1<<TWINT);
    while (!(TWCR & (1<<TWINT))); 
}

void I2C_Stop(void) { 
    TWCR = (1<<TWSTO) | (1<<TWEN) | (1<<TWINT);
    while (TWCR & (1<<TWSTO)); 
}

void I2C_Write(uint8_t data) { 
    TWDR = data;
    TWCR = (1<<TWEN) | (1<<TWINT);
    while (!(TWCR & (1<<TWINT))); 
}

uint8_t I2C_Read_ACK(void) { 
    TWCR = (1<<TWEN) | (1<<TWINT) | (1<<TWEA);
    while (!(TWCR & (1<<TWINT)));
    return TWDR;
}

uint8_t I2C_Read_NACK(void) { 
    TWCR = (1<<TWEN) | (1<<TWINT);
    while (!(TWCR & (1<<TWINT)));
    return TWDR;
}