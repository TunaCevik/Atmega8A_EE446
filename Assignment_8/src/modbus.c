#include "hardware.h"
#include "modbus.h"
#include "usart.h"

// Define state variables
volatile uint8_t  rx_buffer[BUFFER_SIZE] = {0};
volatile uint8_t  rx_index = 0;
volatile uint8_t  frame_ready = 0;
volatile uint16_t modbus_timer = 0;

// Application storage for target register
uint16_t holding_register_100 = 55; 

// === CRC16 (Modbus standard) ===
static uint16_t Modbus_CRC16(uint8_t *data, uint8_t len) {
    uint16_t crc = 0xFFFF;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }
    return crc;
}

// === Process Modbus Frames ===
void process_modbus(void) {
    if (!frame_ready) return;
    frame_ready = 0; // Clear flag

    // Minimum Modbus RTU frame size is 8 bytes
    if (rx_index < 8) {
        rx_index = 0;
        return;
    }

    uint8_t addr = rx_buffer[0];
    uint8_t func = rx_buffer[1];

    // Combine received CRC bytes (Low byte arrives first, then High byte)
    uint16_t crc_recv = rx_buffer[rx_index - 2] | (rx_buffer[rx_index - 1] << 8);
    uint16_t crc_calc = Modbus_CRC16((uint8_t *)rx_buffer, rx_index - 2);

    // Validate slave address and packet integrity
    if (addr != SLAVE_ID || crc_recv != crc_calc) {
        rx_index = 0;
        return;
    }

    uint8_t tx_buf[8];
    uint8_t len = 0;

    // === Read Holding Register (0x03) ===
    if (func == 0x03) {
        uint16_t reg_addr = (rx_buffer[2] << 8) | rx_buffer[3];
        uint16_t qty      = (rx_buffer[4] << 8) | rx_buffer[5];

        // Ensure master requested our valid register address and a count of 1 register
        if (reg_addr == REG_ADDR && qty == 1) {
            tx_buf[0] = SLAVE_ID;
            tx_buf[1] = 0x03;
            tx_buf[2] = 2; // Byte count (1 register = 2 bytes)
            tx_buf[3] = (holding_register_100 >> 8) & 0xFF; // High byte
            tx_buf[4] = holding_register_100 & 0xFF;        // Low byte

            // Compute response CRC
            uint16_t crc_response = Modbus_CRC16(tx_buf, 5);
            tx_buf[5] = crc_response & 0xFF;        // Low byte
            tx_buf[6] = (crc_response >> 8) & 0xFF; // High byte
            len = 7;

            // Transmit frame to master
            for (uint8_t i = 0; i < len; i++) {
                USART_transmit(tx_buf[i]);
            }
        }
    }
    // === Write Holding Register (0x06) ===
    else if (func == 0x06) {
        uint16_t reg_addr = (rx_buffer[2] << 8) | rx_buffer[3];
        uint16_t value    = (rx_buffer[4] << 8) | rx_buffer[5];

        if (reg_addr == REG_ADDR) {
            // Write received 16-bit value into the virtual register
            holding_register_100 = value;

            // Modbus 0x06 response echo back: identical to the request frame
            tx_buf[0] = rx_buffer[0];
            tx_buf[1] = rx_buffer[1];
            tx_buf[2] = rx_buffer[2];
            tx_buf[3] = rx_buffer[3];
            tx_buf[4] = rx_buffer[4];
            tx_buf[5] = rx_buffer[5];
            tx_buf[6] = rx_buffer[6];
            tx_buf[7] = rx_buffer[7];
            len = 8;

            // Transmit echo frame to master
            for (uint8_t i = 0; i < len; i++) {
                USART_transmit(tx_buf[i]);
            }
        }
    }

    rx_index = 0; // Reset index for the next incoming packet
}