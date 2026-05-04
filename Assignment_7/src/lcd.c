#include "lcd.h"
#include "I2C.h"
#include <util/delay.h>

// --- Helper Functions (Not exposed to main.c) ---

/* * This function waits for the I2C bus to be free, starts the transfer, 
 * and then waits for your ISR state machine to set g_transfer_done.
 */
static void lcd_i2c_transmit(uint8_t data) {
    // Must be static so it isn't destroyed while the ISR is reading it
    static uint8_t tx_buffer; 
    tx_buffer = data;

    // Wait until I2C is idle and accept the command
    while (!I2C_Controller_WriteBuffer(LCD_I2C_ADDR, &tx_buffer, 1));
    
    // Wait for the background ISR to finish sending it
    while (!g_transfer_done);
}

/*
 * Maps the data to the specific pins of the standard PCF8574 backpack
 * P0: RS, P1: RW, P2: EN, P3: Backlight, P4-P7: Data (nibble)
 */
static void lcd_send_nibble(uint8_t nibble, uint8_t rs) {
    // 1. Ensure the incoming nibble only occupies the bottom 4 bits (0x00 to 0x0F)
    nibble &= 0x0F; 
    
    /* 2. Construct the base byte:
     * - Shift the 4-bit data up to pins P4-P7: (nibble << 4)
     * - Keep Backlight ON at pin P3: (1 << 3)
     * - RW is always 0 (write mode) at pin P1, so we do nothing.
     * - Place the RS bit at pin P0: (rs << 0) or simply (rs)
     */
    uint8_t base_byte = (nibble << 4) | (1 << 3) | rs;
    
    // 3. Create the HIGH and LOW states for the Enable pin (P2)
    uint8_t data_high = base_byte | (1 << 2);  // Set EN bit to 1
    uint8_t data_low  = base_byte & ~(1 << 2); // Clear EN bit to 0

    // 4. Send High, wait for LCD to read, then pull Low
    lcd_i2c_transmit(data_high);
    _delay_us(1);  // Minimum delay for EN pulse width
    lcd_i2c_transmit(data_low);
    _delay_us(50); // Minimum delay for LCD to execute command
}

static void lcd_send_byte(uint8_t value, uint8_t rs) {
    // Send high nibble first, shifted down to bits 0-3
    lcd_send_nibble((value >> 4), rs);
    // Send low nibble
    lcd_send_nibble((value & 0x0F), rs);
}

static void lcd_command(uint8_t cmd) {
    lcd_send_byte(cmd, 0); // RS = 0 for commands
}

static void lcd_data(uint8_t data) {
    lcd_send_byte(data, 1); // RS = 1 for data
}


// --- Public API ---

void lcd_init(void) {
    // 1. Power-Up and Initial Delay
    _delay_ms(50); // Wait > 15ms

    // 2. First Function Set Command (0x30)
    lcd_send_nibble(0x03, 0);
    _delay_ms(5); // Wait > 4.1ms

    // 3. Second Function Set Command (0x30)
    lcd_send_nibble(0x03, 0);
    _delay_us(150); // Wait > 100us

    // 4. Third Function Set Command (0x30)
    lcd_send_nibble(0x03, 0);
    _delay_us(150);

    // 5. Switch to 4-bit Mode Command (0x20)
    lcd_send_nibble(0x02, 0);
    _delay_ms(1);

    // 6. Function Set Command (0x28): 4-bit, 2 lines, 5x8 font
    lcd_command(0x28);
    
    // 7. Display ON/OFF Control Command (0x0C): Display ON, Cursor OFF
    lcd_command(0x0C);
    
    // 8. Entry Mode Set Command (0x06): Increment cursor
    lcd_command(0x06);
    
    // 9. Clear Display Command (0x01)
    lcd_clear();
}

void lcd_clear(void) {
    lcd_command(LCD_CLEARDISPLAY);
    _delay_ms(2); // Clear display requires ~1.52 ms execution time
}

void lcd_set_cursor(uint8_t col, uint8_t row) {
    uint8_t row_offsets[] = { 0x00, 0x40 };
    if (row > 1) {
        row = 1; // Limit to 2 lines
    }
    // Command to set DDRAM address is 0x80 + Address
    lcd_command(0x80 | (col + row_offsets[row]));
}

void lcd_print(const char *str) {
    while (*str) {
        lcd_data((uint8_t)(*str));
        str++;
    }
}