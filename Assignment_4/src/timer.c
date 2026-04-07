#include "hardware.h"
#include "timer.h"

// The master heartbeat of the entire system
uint8_t ocr2_value = 77; // Initial OCR2 value (for ~25% Hz output)

void init_timer1(void) {
    // Normal modeü Prescaler 64 = 
    TCCR1B = (1 << CS11) | (1 << CS10); // Start Timer1 with prescaler 64
    TCNT1 = 0; // Reset Timer1 count
}


// Timer2 Initialization for CTC PWM on PB3
void timer2_ctc_init() { //
    // WGM21 = 1 (CTC Mode)
    // COM20 = 1 (Toggle OC2 on compare match)
    // CS22, CS21, CS20 = 111 (Prescaler 1024)
    TCCR2 = (1 << WGM21) | (1 << COM20) | (1 << CS22) | (1 << CS21) | (1 << CS20);
    OCR2 = ocr2_value;
}
