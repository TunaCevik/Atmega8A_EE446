#include "hardware.h"
#include "I2C.h"
#include "usart.h"
#include "timer.h"
#include "adc.h"
#include "lcd.h"

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

int main(void) {
    char volt_buf[10], temp_buf[10], adc_buf[10], mv_buf[10];
    char print_buffer[128];

    // 1. Initialization Phase
    timer1_init_1ms();
    USART_init(MYUBRR); 
    I2C_Init(); 
    ADC_init();
    
    // Enable Global Interrupts BEFORE initializing the LCD
    sei();
    USART_send_string("Starting LCD Init...\r\n");
    lcd_init();
    lcd_clear();
    USART_send_string("LCD Init SUCCESS!\r\n"); // If this prints, the function worked!

    // Local Variables for Scheduling and Filtering
    uint32_t last_adc_tick = 0;
    uint32_t last_print_tick = 0;
    
    double filtered_adc = 0.0;
    bool first_adc_read = true;

    // 2. Run the required assignment test at startup
    Test_NIST_Polynomial();

    // 3. Main Super-Loop (Non-Blocking Architecture)
    while (1) {
        
        // --- TASK 1: Trigger ADC Conversion (Every 10ms) ---
        if (system_tick - last_adc_tick >= 10) {
            last_adc_tick = system_tick;
            ADC_start_conversion(); 
        }

        // --- TASK 2: Process ADC Data & Apply Digital Filter ---
        if (adc_ready) {
            adc_ready = 0; // Clear the flag
            
            if (first_adc_read) {
                filtered_adc = adc_value; // Seed the filter
                first_adc_read = false;
            } else {
                // Exponential Moving Average (EMA) Filter (Replaces teacher's blocking loop)
                filtered_adc = (FILTER_ALPHA * adc_value) + ((1.0 - FILTER_ALPHA) * filtered_adc);
            }
        }

        // --- TASK 3: Calculate and Print (Every 500ms) ---
        if (system_tick - last_print_tick >= 500) {
            last_print_tick = system_tick;

            // Step A: Convert using teacher's math (dividing by 1023.0)
            double voltage = (filtered_adc * ADC_REF) / 1023.0; 
            double mv_input = voltage * 1000.0; 
            double mv_thermocouple = mv_input / GAIN; 
            double cold_junction_temp = 20.0; 
            
            double raw_temp = -999.0;
            double final_temperature = -999.0;

            // Step B: Validate millivolt range before applying NIST polynomial
            if (mv_thermocouple >= NIST_MV_MIN && mv_thermocouple <= NIST_MV_MAX) {
                raw_temp = calculateTemperaturePrecise(mv_thermocouple);
                final_temperature = raw_temp + cold_junction_temp;
                
                // Format values into strings
                dtostrf(filtered_adc, 4, 0, adc_buf);
                dtostrf(voltage, 5, 3, volt_buf);
                dtostrf(mv_thermocouple, 6, 3, mv_buf);
                dtostrf(final_temperature, 6, 2, temp_buf);

                USART_send_string("ADC: ");
                USART_send_string(adc_buf);
                USART_send_string(" | Volts: ");
                USART_send_string(volt_buf);
                USART_send_string(" V | Sensor: ");
                USART_send_string(mv_buf);
                USART_send_string(" mV | Temp: ");
                USART_send_string(temp_buf);
                USART_send_string(" C\r\n");

                // Print to LCD
                lcd_set_cursor(0, 0);
                lcd_print("Temp: ");
                lcd_print(temp_buf);
                lcd_print(" C   "); // Extra spaces to clear old characters

            } else {
                USART_send_string("!!! mv out of range !!!\r\n");
                
                lcd_set_cursor(0, 0);
                lcd_print("OUT OF RANGE    ");
            }
        }
    }
}

// --- Assignment Function Implementations ---

double calculateTemperaturePrecise(double mv) {
    // Teacher's specific NIST J-Type Thermocouple Polynomial Coefficients
    const double a[] = { 
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

void Test_NIST_Polynomial(void) {
    // Teacher's specific test arrays
    const double test_mv[] = {14.50, 14.60, 14.70, 14.76, 14.80, 14.90, 15.00};
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