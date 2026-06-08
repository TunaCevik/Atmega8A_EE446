#ifndef TIMER_H
#define TIMER_H

extern volatile uint32_t system_tick;

void TIMER1_init_1ms(void);

void Timer2_PWM_init(void);

void Timer2_PWM_set_duty(uint8_t duty);
#endif