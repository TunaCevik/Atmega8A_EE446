#include "hardware.h"
#include "I2C.h"
#include "usart.h"
#include "timer.h"
#include "adc.h"
#include "lcd.h"
#include <avr/pgmspace.h>

// --- Teacher's System Constants ---
#define ADC_REF 5.0
#define GAIN 100.0 
#define NIST_MV_MIN 0.0 
#define NIST_MV_MAX 42.919 

// --- Our Advanced Architecture Constants ---
#define FILTER_ALPHA 0.1 // Exponential Moving Average weight

// --- Function Prototypes ---
double calculateTemperaturePrecise(double mv);
void Test_NIST_Polynomial(void);
void PID_Control(double current_temp);

// --- Global Variables for Control ---
double target_setpoint = 30.0; // Default setpoint set to 30 degrees C
double Kp = 1.0, Ki = 0.05, Kd = 0.5; // PID parameters close to 1
double integral_error = 0.0;
double previous_error = 0.0;

int main(void) {
    char set_buf[10], temp_buf[10], adc_buf[10], mv_buf[10];

    // 1. Initialization Phase
    timer1_init_1ms();
    Timer2_PWM_init(); // Initialize the PWM for the heater
    USART_init(MYUBRR); 
    I2C_Init(); 
    ADC_init();
    
    // Hardware Pin Configurations
    DDRB |= (1 << PB3);

    // Initialize Buttons on PD3 and PD4
    DDRD &= ~((1 << PD3) | (1 << PD4)); // Set as input
    PORTD |= (1 << PD3) | (1 << PD4);   // Enable internal pull-up resistors

    sei();
    USART_send_string("Starting System...\r\n");
    lcd_init();
    lcd_clear();

    uint32_t last_adc_tick = 0;
    uint32_t last_print_tick = 0;
    uint32_t last_button_tick = 0;
    
    double filtered_adc = 0.0;
    bool first_adc_read = true;

    Test_NIST_Polynomial();

    // 3. Main Super-Loop
    while (1) {
        
        // --- TASK 1: Button Polling (Every 100ms for basic debounce) ---
        if (system_tick - last_button_tick >= 100) {
            last_button_tick = system_tick;
            
            if (!(PIND & (1 << PD3))) { // If PD3 is pressed (active low)
                target_setpoint += 1.0; // Increase setpoint
            }
            if (!(PIND & (1 << PD4))) { // If PD4 is pressed (active low)
                target_setpoint -= 1.0; // Decrease setpoint
            }
        }

        // --- TASK 2: Trigger ADC Conversion (Every 10ms) ---
        if (system_tick - last_adc_tick >= 10) {
            last_adc_tick = system_tick;
            ADC_start_conversion(); 
        }

        // --- TASK 3: Process ADC Data & Apply Digital Filter ---
        if (adc_ready) {
            adc_ready = 0; 
            
            if (first_adc_read) {
                filtered_adc = adc_value; 
                first_adc_read = false;
            } else {
                filtered_adc = (FILTER_ALPHA * adc_value) + ((1.0 - FILTER_ALPHA) * filtered_adc);
            }
        }

        // --- TASK 4: Calculate, Control, and Print (Every 500ms) ---
        if (system_tick - last_print_tick >= 500) {
            last_print_tick = system_tick;

            double voltage = (filtered_adc * ADC_REF) / 1023.0; 
            double mv_input = voltage * 1000.0; 
            double mv_thermocouple = mv_input / GAIN; 
            double cold_junction_temp = 20.0; 
            
            double raw_temp = -999.0;
            double final_temperature = -999.0;

            if (mv_thermocouple >= NIST_MV_MIN && mv_thermocouple <= NIST_MV_MAX) {
                raw_temp = calculateTemperaturePrecise(mv_thermocouple);
                final_temperature = raw_temp + cold_junction_temp;
                
                // Execute PID Control
                PID_Control(final_temperature);
                
                // Format values into strings
                dtostrf(target_setpoint, 4, 1, set_buf);
                dtostrf(final_temperature, 4, 1, temp_buf);

                // Print to LCD
                lcd_set_cursor(0, 0);
                lcd_print("Set: ");
                lcd_print(set_buf);
                lcd_print(" C    "); 

                lcd_set_cursor(0, 1);
                lcd_print("Now: ");
                lcd_print(temp_buf);
                lcd_print(" C    "); 

            } else {
                Timer2_PWM_set_duty(0); // Failsafe: Turn heater off if out of range
                lcd_set_cursor(0, 0);
                lcd_print("SENSOR ERROR    ");
            }
        }
    }
}

// --- PID Controller Function ---
void PID_Control(double current_temp) {
    double error = target_setpoint - current_temp;
    
    // Proportional Term
    double P_out = Kp * error;
    
    // Integral Term (with basic anti-windup limit)
    integral_error += error;
    if (integral_error > 255.0) integral_error = 255.0;
    if (integral_error < -255.0) integral_error = -255.0;
    double I_out = Ki * integral_error;
    
    // Derivative Term
    double derivative = error - previous_error;
    double D_out = Kd * derivative;
    
    // Calculate Total Output
    double output = P_out + I_out + D_out;
    
    // Clamp the output to the PWM range (0 to 255)
    if (output > 255.0) {
        output = 255.0;
    } else if (output < 0.0) {
        output = 0.0;
    }
    
    // Apply PWM Duty Cycle
    Timer2_PWM_set_duty((uint8_t)output);
    
    // Save current error for the next derivative calculation
    previous_error = error;
}

// --- Assignment Function Implementations ---

double calculateTemperaturePrecise(double mv) {
    // Teacher's specific NIST J-Type Thermocouple Polynomial Coefficients
    static const double a[] PROGMEM = { 
        0.0000000E+00,
        1.9528268E+01,
       -1.2286185E+00,
        1.0752178E-01,
       -5.9086933E-03,
        1.7256713E-04,
       -2.8131513E-06,
        2.3963370E-08,
       -8.3823321E-11
    };
    
    double temp_calculated = 0.0;
    double mv_pow = 1.0; 
    
    int i;
    for (i = 0; i < 9; i++) {
        temp_calculated += (pgm_read_float(&a[i]) * mv_pow);
        mv_pow *= mv; 
    }
    
    return temp_calculated;
}

void Test_NIST_Polynomial(void) {
    // Teacher's specific test arrays
    static const double test_mv[] = {14.50, 14.60, 14.70, 14.76, 14.80, 14.90, 15.00};
    const int count = sizeof(test_mv) / sizeof(test_mv[0]);
    
    char mv_buf[10], temp_buf[10];
    
    USART_send_string("\r\n--- NIST Polynomial Test ---\r\n");
    
    int i;
    for (i = 0; i < count; i++) {
        double mv = test_mv[i];
        double temp = calculateTemperaturePrecise(mv);
        
        dtostrf(mv, 6, 2, mv_buf);
        dtostrf(temp, 8, 2, temp_buf);
        
        USART_send_string("mv: ");
        USART_send_string(mv_buf);
        USART_send_string(" -> Temp: ");
        USART_send_string(temp_buf);
        USART_send_string(" C\r\n");
    }
    USART_send_string("-----------------------------\r\n\r\n");
}