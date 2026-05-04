#ifndef LCD_H
#define LCD_H

#include "hardware.h"

// Default I2C address for PCF8574 is often 0x27. 
// If your LCD doesn't work, try 0x3F.
#define LCD_I2C_ADDR 0x27 

// LCD Commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02

// LCD control bits
#define LCD_RS 0
#define LCD_RW 1
#define LCD_EN 2
#define LCD_BACKLIGHT 3 

// Function Prototypes
void lcd_init(void);
void lcd_clear(void);
void lcd_set_cursor(uint8_t col, uint8_t row);
void lcd_print(const char *str);

#endif