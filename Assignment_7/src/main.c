#include "hardware.h"
#include "I2C.h"
#include "usart.h"
#include "timer.h"
#include "adc.h"
#include "lcd.h"
#include "INT0.h"
#include "pcf8574.h"
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
void PID_Control(double current_temp, double dt);

// --- Global Variables for Control ---
double target_setpoint = 30.0; // Default setpoint set to 30 degrees C
double Kp = 1.0, Ki = 0.05, Kd = 0.5; // PID parameters close to 1
double integral_error = 0.0;

double previous_error = 0.0;

double global_dt = 0.0;

int main(void) {
    // Buffers for LCD printing
    char set_buf[10], temp_buf[10];

    // 1. Initialization Phase
    timer1_init_1ms();
    Timer2_PWM_init(); 
    USART_init(MYUBRR); 
    I2C_Init(); 
    ADC_init();

    INT0_Init();
    PCF8574_Write(0xFF, PCF8574_DEFAULT_ADDR);

    bool button_p4_pressed = false;

    // Hardware Pin Configurations
    DDRB |= (1 << PB3);
    DDRD &= ~((1 << PD3) | (1 << PD4)); 
    PORTD |= (1 << PD3) | (1 << PD4);   

    sei();
    USART_send_string("Starting System...\r\n");
    lcd_init();
    lcd_clear();

    // 2. Task Timing Variables
    uint32_t last_button_tick = 0;
    uint32_t last_adc_tick = 0;
    uint32_t last_pid_tick = 0;
    uint32_t last_print_tick = 0;
    
    // 3. Shared State Variables (So different tasks can talk to each other)
    double filtered_adc = 0.0;
    bool first_adc_read = true;
    double final_temperature = -999.0;
    bool sensor_error = false;

    Test_NIST_Polynomial();

    // --- 4. Main Super-Loop ---
    while (1) {

        // --- NEW TASK: Event-Driven PCF8574 Input ---
        if (pcf_interrupt_flag == 1) {
            
            // 1. Instantly clear the flag so we don't process it twice
            pcf_interrupt_flag = 0; 
            
            // 2. Read the PCF8574 over I2C
            // CRITICAL: This read operation signals the PCF8574 to 
            // release its physical INT pin back to HIGH.
            uint8_t expander_pins = PCF8574_Read(PCF8574_DEFAULT_ADDR);
            
            // 3. Update our logical button state
            if (!(expander_pins & (1 << 4))) {
                button_p4_pressed = true;  // Button on P4 went LOW
            } else {
                button_p4_pressed = false; // Button on P4 went HIGH
            }
        }
        
        // TASK 1: Button Polling (Every 100ms)
        if (system_tick - last_button_tick >= 100) {
            last_button_tick = system_tick;
            
            if (!(PIND & (1 << PD3))) { 
                target_setpoint += 1.0; 
            }
            if (!(PIND & (1 << PD4))) { 
                target_setpoint -= 1.0; 
            }
        }

        // TASK 2: Trigger ADC Conversion (Every 20ms - Relaxed for 4MHz)
        if (system_tick - last_adc_tick >= 20) {
            last_adc_tick = system_tick;
            ADC_start_conversion(); 
        }

        // TASK 3: Process ADC Data & Filter (Runs immediately when hardware is ready)
        if (adc_ready) {
            adc_ready = 0; 
            
            if (first_adc_read) {
                filtered_adc = adc_value; 
                first_adc_read = false;
            } else {
                filtered_adc = (FILTER_ALPHA * adc_value) + ((1.0 - FILTER_ALPHA) * filtered_adc);
            }
        }

        // TASK 4: Fast Math & Execute PID Control (Every 500ms)
        if (system_tick - last_pid_tick >= 500) {
            // Calculate dynamic delta time (dt) in seconds
            uint32_t current_tick = system_tick;
            global_dt = (double)(current_tick - last_pid_tick) / 1000.0;
            last_pid_tick = current_tick;

            // Perform Math
            double voltage = (filtered_adc * ADC_REF) / 1023.0; 
            double mv_input = voltage * 1000.0; 
            double mv_thermocouple = mv_input / GAIN; 
            double cold_junction_temp = 20.0; 
            
            if (mv_thermocouple >= NIST_MV_MIN && mv_thermocouple <= NIST_MV_MAX) {
                sensor_error = false; // Sensor is good
                double raw_temp = calculateTemperaturePrecise(mv_thermocouple);
                final_temperature = raw_temp + cold_junction_temp;
                
                // Execute Time-Aware PID Control (~1ms execution time)
                PID_Control(final_temperature, global_dt);
                
            } else {
                sensor_error = true; // Flag for the LCD to read later
                Timer2_PWM_set_duty(0); // Failsafe: Turn heater off
            }
        }

        // TASK 5: Slow Human Interface (Every 1000ms)
        // This takes ~45ms, but because it only happens once a second, 
        // it won't destroy your ADC filter or your PID math.
        if (system_tick - last_print_tick >= 1000) {
            last_print_tick = system_tick;
            
            if (button_p4_pressed) {
                // --- BUTTON IS PRESSED: SHOW ERROR ---
                char dt_buf[10];
                dtostrf(global_dt, 5, 3, dt_buf); 
                
                lcd_set_cursor(0, 0);
                lcd_print("ERROR: P4 LOW   ");
                lcd_set_cursor(0, 1);
                lcd_print("dt: ");
                lcd_print(dt_buf);
                lcd_print(" sec   ");
            }
            else if (!sensor_error && final_temperature > -100.0) {
                // String conversions
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
                lcd_set_cursor(0, 0);
                lcd_print("SENSOR ERROR    ");
                lcd_set_cursor(0, 1);
                lcd_print("PWM: OFF        ");
            }
        }
    }
}

// --- Refactored PID Controller Function ---
// current_temp: The latest temperature reading
// dt: The exact time elapsed in seconds since the last calculation
void PID_Control(double current_temp, double dt) {
    
    // Safety Check: Prevent division by zero if dt is accidentally 0
    if (dt <= 0.0) return; 

    // 1. Calculate Current Error
    double error = target_setpoint - current_temp;
    
    // 2. Proportional Term
    double P_out = Kp * error;
    
    // 3. Integral Term (Area = Error * Time)
    integral_error += (error * dt);
    
    // Basic Anti-Windup 
    // Limits the accumulated error to prevent massive overshoots
    if (integral_error > 255.0) {
        integral_error = 255.0;
    } else if (integral_error < -255.0) {
        integral_error = -255.0;
    }
    
    double I_out = Ki * integral_error;
    
    // 4. Derivative Term (Slope = Change in Error / Time)
    // OPTIMIZATION: Calculate the reciprocal of dt once. 
    // Multiplying by inv_dt is faster than dividing by dt on a 4MHz AVR.
    double inv_dt = 1.0 / dt; 
    double derivative = (error - previous_error) * inv_dt;
    double D_out = Kd * derivative;
    
    // 5. Calculate Total Output
    double output = P_out + I_out + D_out;
    
    // 6. Clamp the output to the valid 8-bit PWM hardware range (0 to 255)
    if (output > 255.0) {
        output = 255.0;
    } else if (output < 0.0) {
        output = 0.0;
    }
    
    // 7. Apply PWM Duty Cycle to the physical heater
    Timer2_PWM_set_duty((uint8_t)output);
    
    // 8. Save current error for the next loop's derivative calculation
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