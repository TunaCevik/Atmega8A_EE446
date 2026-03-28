# --- Variables (The "Constants" of our circuit) ---
CC = avr-gcc
OBJCOPY = avr-objcopy

# -Iinc tells the compiler to look in the "inc" folder for .h files
# -Os optimizes for size, -Wall turns on all warnings
CFLAGS = -mmcu=atmega8 -DF_CPU=4000000UL -Iinc -Os -Wall

# --- Rules (Target : Dependencies) ---

# 1. The Default Target (What happens when you just type "make")
# To get "all", we need the final .hex file.
all: build/project.hex

# 2. Creating the Hex File (For flashing)
# Target: .hex | Dependency: .elf
# Command: Extract the machine code from the .elf and format it as Intel Hex.
build/project.hex: build/project.elf
	$(OBJCOPY) -O ihex build/project.elf build/project.hex

# 3. Linking the Object Files
# Target: .elf | Dependencies: All the .o files
# Command: Link them together using the internal ATmega8 memory map.
build/project.elf: build/main.o build/io_control.o
	$(CC) -mmcu=atmega8 build/main.o build/io_control.o -o build/project.elf

# 4. Compiling main.c
# Target: main.o | Dependencies: main.c and hardware.h
# Command: Compile (-c) the C code into an object file.
build/main.o: src/main.c inc/hardware.h
	$(CC) $(CFLAGS) -c src/main.c -o build/main.o

# 5. Compiling io_control.c
# Target: io_control.o | Dependencies: io_control.c and hardware.h
# Command: Compile (-c) the C code into an object file.
build/io_control.o: src/io_control.c inc/hardware.h
	$(CC) $(CFLAGS) -c src/io_control.c -o build/io_control.o

# 6. Clean Up
# Target: clean | Dependency: none
# Command: Delete everything inside the build folder to start fresh.
clean:
	rm -f build/*

# 7. Flash the Microcontroller
# Target: flash | Dependency: project.hex
# Command: Use avrdude with USBasp to write the hex file to the ATmega8
flash: build/project.hex
	avrdude -c usbasp -p m8 -U flash:w:build/project.hex:i