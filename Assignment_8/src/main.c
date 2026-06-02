#include "hardware.h"
#include "usart.h"
#include "timer.h"
#include "modbus.h"

int main(void) {
    // Pass configured calculation value to baud rate registers
    USART_init(MYUBRR); 
    TIMER1_init_1ms();
    sei(); // Enable global interrupts

    while (1)
    {
        process_modbus(); // Periodically check for complete frames
    }
}