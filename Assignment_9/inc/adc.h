#ifndef ADC_H
#define ADC_H

extern volatile uint16_t adc_value;
extern volatile uint8_t adc_ready;

void ADC_init(void);
void ADC_start_conversion(void);

#endif