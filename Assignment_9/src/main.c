#include "hardware.h"
#include "usart.h"
#include "timer.h"
#include "modbus.h"
#include "adc.h"
#include <stdlib.h>

#define ADC_REF 5.0
#define GAIN 100.0 
#define NIST_MV_MIN 0.0 
#define NIST_MV_MAX 42.919 

#define FILTER_ALPHA 0.1

// --- Function Prototypes ---
double calculateTemperaturePrecise(double mv);
void PID_Control(double current_temp, double dt);

// Start Values
double target_setpoint = 30.0;
double Kp = 1.0, Ki = 0.05, Kd = 0.5;
double integral_error = 0.0;

double previous_error = 0.0;

double global_dt = 0.0;

int main(void) {
    TIMER1_init_1ms();
    Timer2_PWM_init(); 
    USART_init(MYUBRR); 
    ADC_init();

    sei();
    USART_send_string("Starting System...\r\n");


    uint32_t last_adc_tick = 0;
    uint32_t last_pid_tick = 0;
    
    double filtered_adc = 0.0;
    bool first_adc_read = true;
    double final_temperature = -999.0;


// --- 4. Main Super-Loop ---
    while (1) {
        
        // TASK 1: Trigger ADC Conversion (Every 20ms)
        if (system_tick - last_adc_tick >= 20) {
            last_adc_tick = system_tick;
            ADC_start_conversion(); 
        }

        // TASK 2: Process ADC Data & Filter
        if (adc_ready) {
            adc_ready = 0; 
            
            if (first_adc_read) {
                filtered_adc = adc_value; 
                first_adc_read = false;
            } else {
                filtered_adc = (FILTER_ALPHA * adc_value) + ((1.0 - FILTER_ALPHA) * filtered_adc);
            }
        }

        // TASK 3: Fast Math & Execute PID Control (Every 500ms)
        if (system_tick - last_pid_tick >= 500) {
            uint32_t current_tick = system_tick;
            global_dt = (double)(current_tick - last_pid_tick) / 1000.0;
            last_pid_tick = current_tick;

            // --- SYNC 1: Read incoming PC parameters from Modbus array ---
            target_setpoint = (double)holding_register[0];
            Kp = (double)holding_register[1];
            Ki = (double)holding_register[2];
            Kd = (double)holding_register[3];

            // Perform Math
            double voltage = (filtered_adc * ADC_REF) / 1023.0; 
            double mv_input = voltage * 1000.0; 
            double mv_thermocouple = mv_input / GAIN; 
            double cold_junction_temp = 20.0; 
            
            if (mv_thermocouple >= NIST_MV_MIN && mv_thermocouple <= NIST_MV_MAX) {
                double raw_temp = calculateTemperaturePrecise(mv_thermocouple);
                final_temperature = raw_temp + cold_junction_temp;
                
                // --- SYNC 2: Write hardware state to Modbus array for PC to read ---
                holding_register[4] = (uint16_t)final_temperature;
                
                // Execute Time-Aware PID Control 
                PID_Control(final_temperature, global_dt);
                
            } else {
                Timer2_PWM_set_duty(0); // Failsafe: Turn heater off if sensor is disconnected
            }
        }

        // TASK 4: Process Modbus Frames (Runs Continuously)
        // This function from modbus.c will respond to the Python GUI
        process_modbus(); 
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


double calculateTemperaturePrecise(double mv) {
    static const double a[] = { 
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
        temp_calculated += (a[i] * mv_pow);
        mv_pow *= mv; 
    }
    
    return temp_calculated;
}
