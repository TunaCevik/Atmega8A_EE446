#include "pcf8574.h"
#include "I2C.h"

uint8_t PCF8574_Read(uint8_t dev_addr) {
    // 1. Tell the I2C peripheral to start reading 1 byte
    if (!I2C_Controller_ReadByte(dev_addr)) {
        // Return 0xFF (all pins high) if the I2C bus is currently busy
        return 0xFF; 
    }
    
    // 2. Wait for the Interrupt Service Routine to finish the transaction
    // In a professional system, you might add a timeout counter here 
    // to prevent an infinite loop if the I2C bus freezes.
    while (!g_transfer_done) {
        // Do nothing, just wait for the hardware to finish.
    }
    
    // 3. Return the data that the ISR saved
    return g_received_data;
}

bool PCF8574_Write(uint8_t dev_addr, uint8_t data) {
    // Note: I2C_Controller_WriteBuffer requires a pointer to an array.
    // We create a temporary array of size 1 to hold our single byte.
    uint8_t tx_buffer[1];
    tx_buffer[0] = data;
    
    // 1. Tell the I2C peripheral to start writing
    if (!I2C_Controller_WriteBuffer(dev_addr, tx_buffer, 1)) {
        return false; // Bus is busy
    }
    
    // 2. Wait for the hardware to finish transmitting
    while (!g_transfer_done) {
        // Wait
    }
    
    return true;
}