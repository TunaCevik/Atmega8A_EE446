#include "hardware.h"


extern volatile bool g_transfer_done;

extern volatile uint8_t g_received_data;

void I2C_Init(void);

bool I2C_Controller_WriteBuffer(uint8_t dev_addr, uint8_t *p_data, uint16_t size);

bool I2C_Controller_ReadByte(uint8_t dev_addr);
