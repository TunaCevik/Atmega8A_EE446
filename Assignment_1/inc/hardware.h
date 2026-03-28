#ifndef HARDWARE_H
#define HARDWARE_H

#include <avr/io.h>

// Port B Pins
#define RELAY1    PB6 
#define RELAY2    PB7 
#define MOSFET    PB3 
#define MOC3021   PB1 

// Port D Pins
#define TRANSISTOR PD5 
#define BUTTON1    PD3

// --- Function Prototypes ---
void IO_Config(void);
void Turn_Off_All(void);
void Turn_On_All(void);

#endif