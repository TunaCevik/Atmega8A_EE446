# Toolchain and Folder Definitions
CC        = avr-gcc
OBJCOPY   = avr-objcopy
MCU       = atmega8
F_CPU     = 4000000UL 
BUILD_DIR = build
SRC_DIR   = src
INC_DIR   = inc

# Compiler Flags
# -mmcu: Specifies the target AVR chip
# -DF_CPU: Defines the clock speed for <util/delay.h> and USART baud rates
# -Os: Optimized for Size (Crucial for 8-bit AVR with limited memory)
CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -I$(INC_DIR) -Wall -Os

# Linker Flags
LDFLAGS = -mmcu=$(MCU) -Wl,-Map=$(BUILD_DIR)/final.map

# List of Object Files based on your src/ directory
OBJS = $(BUILD_DIR)/modbus.o \
       $(BUILD_DIR)/main.o \
       $(BUILD_DIR)/timer.o \
       $(BUILD_DIR)/usart.o 
	   

# Default Target: Generates the .hex file for the programmer
all: $(BUILD_DIR) $(BUILD_DIR)/final.hex

# Create the build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Rule for C files in the src/ directory
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Final Linking to ELF
$(BUILD_DIR)/final.elf: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

# Convert ELF to HEX (This is what you actually flash to the ATmega8A)
$(BUILD_DIR)/final.hex: $(BUILD_DIR)/final.elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

# Clean Target
clean:
	rm -rf $(BUILD_DIR)/*

flash: $(BUILD_DIR)/final.hex
	avrdude -c usbasp -p m8 -U flash:w:$(BUILD_DIR)/final.hex:i