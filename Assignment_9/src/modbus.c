#include "hardware.h"
#include "modbus.h"
#include "usart.h"

// State variables
volatile uint8_t  rx_buffer[BUFFER_SIZE] = {0};
volatile uint8_t  rx_index = 0;
volatile uint8_t  frame_ready = 0;
volatile uint16_t modbus_timer = 0;

// Application storage for Modbus Holding Registers
// [0] = Set Value, [1] = Kp, [2] = Ki, [3] = Kd, [4] = Temperature
volatile uint16_t holding_register[NUM_REGISTERS] = {30, 1, 0, 0, 0}; 

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
    frame_ready = 0; 

    // Minimum valid frame size
    if (rx_index < 8) {
        rx_index = 0;
        return;
    }

    uint8_t addr = rx_buffer[0];
    uint8_t func = rx_buffer[1];

    uint16_t crc_recv = rx_buffer[rx_index - 2] | (rx_buffer[rx_index - 1] << 8);
    uint16_t crc_calc = Modbus_CRC16((uint8_t *)rx_buffer, rx_index - 2);

    if (addr != SLAVE_ID || crc_recv != crc_calc) {
        rx_index = 0;
        return;
    }

    uint8_t tx_buf[32]; 
    uint8_t len = 0;

    // === Read Holding Registers (0x03) ===
    if (func == 0x03) {
        uint16_t reg_addr = (rx_buffer[2] << 8) | rx_buffer[3];
        uint16_t qty      = (rx_buffer[4] << 8) | rx_buffer[5];

        if ((reg_addr + qty) <= NUM_REGISTERS && qty > 0) {
            tx_buf[0] = SLAVE_ID;
            tx_buf[1] = 0x03;
            tx_buf[2] = (uint8_t)(qty * 2); 

            uint8_t idx = 3;
            for (uint16_t i = 0; i < qty; i++) {
                tx_buf[idx++] = (holding_register[reg_addr + i] >> 8) & 0xFF; // High byte
                tx_buf[idx++] = holding_register[reg_addr + i] & 0xFF;        // Low byte
            }

            uint16_t crc_response = Modbus_CRC16(tx_buf, idx);
            tx_buf[idx++] = crc_response & 0xFF;        
            tx_buf[idx++] = (crc_response >> 8) & 0xFF; 
            len = idx;

            for (uint8_t i = 0; i < len; i++) {
                USART_transmit(tx_buf[i]);
            }
        }
    }
    // === Write Single Holding Register (0x06) ===
    else if (func == 0x06) {
        uint16_t reg_addr = (rx_buffer[2] << 8) | rx_buffer[3];
        uint16_t value    = (rx_buffer[4] << 8) | rx_buffer[5];

        if (reg_addr < NUM_REGISTERS) {
            holding_register[reg_addr] = value;

            // Echo back identical to request
            for(int i = 0; i < 8; i++) {
                tx_buf[i] = rx_buffer[i];
            }
            len = 8;

            for (uint8_t i = 0; i < len; i++) {
                USART_transmit(tx_buf[i]);
            }
        }
    }
    // === Write Multiple Registers (0x10) ===
    else if (func == 0x10) {
        uint16_t reg_addr = (rx_buffer[2] << 8) | rx_buffer[3];
        uint16_t qty      = (rx_buffer[4] << 8) | rx_buffer[5];
        uint8_t byte_count = rx_buffer[6];

        if ((reg_addr + qty) <= NUM_REGISTERS && byte_count == (qty * 2)) {
            uint8_t data_idx = 7;
            for (uint16_t i = 0; i < qty; i++) {
                holding_register[reg_addr + i] = (rx_buffer[data_idx] << 8) | rx_buffer[data_idx + 1];
                data_idx += 2;
            }

            tx_buf[0] = SLAVE_ID;
            tx_buf[1] = 0x10;
            tx_buf[2] = rx_buffer[2];
            tx_buf[3] = rx_buffer[3];
            tx_buf[4] = rx_buffer[4];
            tx_buf[5] = rx_buffer[5];

            uint16_t crc_response = Modbus_CRC16(tx_buf, 6);
            tx_buf[6] = crc_response & 0xFF;
            tx_buf[7] = (crc_response >> 8) & 0xFF;
            len = 8;

            for (uint8_t i = 0; i < len; i++) {
                USART_transmit(tx_buf[i]);
            }
        }
    }

    rx_index = 0; 
}