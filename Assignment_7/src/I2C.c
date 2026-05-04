#include "I2C.h"

typedef enum {
    I2C_IDLE,
    I2C_STATE_SB_GENERATED,
    I2C_STATE_TRANSMITTING,
    I2C_STATE_RECEIVING
} I2C_State;

volatile uint8_t g_received_data = 0; 
volatile bool    g_transfer_done = false; 

static volatile I2C_State current_state = I2C_IDLE;

static volatile uint8_t* p_tx_data = NULL;  // Pointer to the array
static volatile uint8_t  g_data_buffer = 0; // Holds the data to send
static volatile uint16_t tx_size = 0;       // Total bytes to send
static volatile uint16_t tx_index = 0;      // Current byte index

static volatile uint8_t target_addr_to_send = 0;


void I2C_Init(void) { 
    TWSR = 0x00; // Prescaler = 1
    TWBR = ((F_CPU/SCL_CLOCK) - 16) / 2;
}

ISR(TWI_vect) {
    switch (current_state) {
        case I2C_STATE_SB_GENERATED:
        // 1. Load the pre-formatted 8-bit address directly
            TWDR = target_addr_to_send; 

            // 2. Check the 0th bit to decide the next state
            if ((target_addr_to_send & 0x01) == 0) {
                // It's a Write (MT Mode)
                current_state = I2C_STATE_TRANSMITTING;
            } else {
                // It's a Read (MR Mode)
                // You might need a separate state here depending on your design, 
                // but we will route it to RECEIVING logic.
                current_state = I2C_STATE_RECEIVING; 
            }

            // 3. Clear TWINT to tell the hardware to transmit the address byte
            TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
            break;
        
            case I2C_STATE_TRANSMITTING:
            {
                uint8_t status = TWSR & 0xF8;

                // 0x18 = Address ACK'd, 0x28 = Data ACK'd
                if (status == 0x18 || status == 0x28) {
                    
                    if (tx_index < tx_size) {
                        // We still have data to send
                        TWDR = p_tx_data[tx_index++]; 
                        TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
                    } else {
                        // Buffer is empty, transmission is complete!
                        TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO) | (1 << TWIE);
                        current_state = I2C_IDLE;
                        g_transfer_done = true;
                    }
                    
                } 
                // 0x20 = Address NACK'd, 0x30 = Data NACK'd
                else if (status == 0x20 || status == 0x30) { 
                    // ERROR: Slave did not acknowledge!
                    // Abort the transmission and send a STOP condition
                    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO) | (1 << TWIE);
                    current_state = I2C_IDLE;
                    g_transfer_done = true; 
                    // Note: You might want an error flag here in the future
                }
            }
            break;

            case I2C_STATE_RECEIVING:
            {
                uint8_t status = TWSR & 0xF8;

                if (status == 0x40) {
                    /* * STATUS 0x40: SLA+R has been transmitted, and ACK received.
                     * We must tell the hardware to start clocking in the data.
                     * CRITICAL: We DO NOT set the TWEA bit here. Leaving TWEA as 0 
                     * ensures the hardware sends a NACK after receiving this 1 byte.
                     */
                    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE); 
                } 
                else if (status == 0x58) {
                    /* * STATUS 0x58: Data byte has been received, and we sent a NACK.
                     * The data is now ready for us to read.
                     */
                    g_received_data = TWDR; // 1. Save the read data
                    
                    // 2. Generate a STOP condition to end the transaction
                    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO) | (1 << TWIE);
                    
                    // 3. Reset the software state machine
                    current_state = I2C_IDLE;
                    g_transfer_done = true;
                }
                else {
                    /* * ERROR HANDLING: 
                     * e.g., Status 0x48 (SLA+R transmitted, NACK received).
                     * Abort and send a STOP condition.
                     */
                    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO) | (1 << TWIE);
                    current_state = I2C_IDLE;
                    g_transfer_done = true;
                }
            }
            break;

        default:
            break;
    }
    
}

bool I2C_Controller_WriteBuffer(uint8_t dev_addr, uint8_t *p_data, uint16_t size) {
    if (current_state != I2C_IDLE) return false;

    p_tx_data = p_data;
    tx_size = size;
    tx_index = 0;
    g_transfer_done = false;
    target_addr_to_send = (dev_addr << 1) | 0x00;
    current_state = I2C_STATE_SB_GENERATED;
    TWCR = (1<<TWSTA) | (1<<TWEN) | (1<<TWINT) | (1 << TWIE);

    return true;
}

bool I2C_Controller_ReadByte(uint8_t dev_addr) {
    if (current_state != I2C_IDLE) return false; 

    g_transfer_done = false;
    target_addr_to_send = (dev_addr << 1) | 0x01;
    current_state = I2C_STATE_SB_GENERATED;
    TWCR = (1<<TWSTA) | (1<<TWEN) | (1<<TWINT) | (1 << TWIE);
    return true;
}