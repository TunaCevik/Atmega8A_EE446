#include "hardware.h"
#include "adc.h"

volatile uint16_t adc_value = 0;
volatile uint8_t adc_ready = 0;

void ADC_init(void) {
    // Configure PC2 (ADC2) as input, disable pull-up
    DDRC &= ~(1 << PC2);
    PORTC &= ~(1 << PC2);
    
    // AVCC reference, select ADC2 (MUX1)
    ADMUX = (1 << REFS0) | (1 << MUX1);
    // Enable ADC, Enable Interrupt, Prescaler 32
    ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS0);
}

void ADC_start_conversion(void) {
    ADCSRA |= (1 << ADSC);
}

ISR(ADC_vect) {
    adc_value = ADCW;
    adc_ready = 1;
}