#ifndef TIMER_H
#define TIMER_H

extern uint8_t ocr2_value; // Global variable to hold the current OCR2 value for PWM

void init_timer1(void);
void timer2_ctc_init(void);

#endif