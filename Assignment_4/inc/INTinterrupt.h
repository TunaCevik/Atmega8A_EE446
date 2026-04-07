#ifndef INTINTERRUPT_H
#define INTINTERRUPT_H

extern volatile uint8_t zero_cross_detected;
extern volatile uint16_t interrupt_interval;
extern volatile uint16_t frequency;
extern volatile uint16_t period;

void into_init(void);
void init_zero_crossing(void);

#endif
