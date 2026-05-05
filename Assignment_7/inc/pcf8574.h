#ifndef PCF8574_H
#define PCF8574_H

#include "hardware.h"

// Default 7-bit address for the PCF8574 (Pins A0, A1, A2 tied to Ground)
// If you are using a PCF8574A or have different address pins, change this!
#define PCF8574_DEFAULT_ADDR 0x20

/**
 * @brief Reads the 8-bit port state from the PCF8574.
 * @param dev_addr The 7-bit I2C address of the chip.
 * @return The 8-bit state of the pins (1 = HIGH/Unpressed, 0 = LOW/Pressed).
 */
uint8_t PCF8574_Read(uint8_t dev_addr);

/**
 * @brief Writes an 8-bit state to the PCF8574 port.
 * @param dev_addr The 7-bit I2C address of the chip.
 * @param data The 8-bit value to write to the pins.
 * @return true if successful, false if the I2C bus was busy.
 */
bool PCF8574_Write(uint8_t dev_addr, uint8_t data);

#endif // PCF8574_H