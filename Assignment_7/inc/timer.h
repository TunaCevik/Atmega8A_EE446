#ifndef TIMER_H
#define TIMER_H

extern volatile uint32_t system_tick;

void timer1_init_1ms(void);

// Initializes Timer2 for Fast PWM on pin PB3
void Timer2_PWM_init(void);

// Sets the PWM duty cycle (0 to 255)
void Timer2_PWM_set_duty(uint8_t duty);

#endif