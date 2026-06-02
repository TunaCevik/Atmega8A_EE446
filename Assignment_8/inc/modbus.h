#ifndef MODBUS_H
#define MODBUS_H

#include <stdint.h>

// Modbus Constants
#define SLAVE_ID        2
#define REG_ADDR        100
#define BUFFER_SIZE     32

// Shared variables for Interrupt Service Routines
extern volatile uint8_t  rx_buffer[BUFFER_SIZE];
extern volatile uint8_t  rx_index;
extern volatile uint8_t  frame_ready;
extern volatile uint16_t modbus_timer;

// Public Functions
void process_modbus(void);

#endif // MODBUS_H