#include "hardware.h"
#include "adc.h"

volatile uint16_t adc_value = 0;
volatile uint8_t adc_ready = 0;

void adc_init(void) {
    // Configure PC1 (ADC1) as input, disable pull-up
    DDRC &= ~(1 << PC1);
    PORTC &= ~(1 << PC1);
    
    // AVCC reference, select ADC1 (MUX0)
    ADMUX = (1 << REFS0) | (1 << MUX0);
    // Enable ADC, Enable Interrupt, Prescaler 32
    ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS0);
}

void adc_start_conversion(void) {
    ADCSRA |= (1 << ADSC);
}

// ISR fires when the hardware finishes reading the voltage
ISR(ADC_vect) {
    adc_value = ADCW; // Grab the 10-bit value
    adc_ready = 1;    // Raise the flag for the main loop
}