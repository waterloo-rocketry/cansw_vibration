#include <xc.h>

#include "canlib.h"
#include "common.h"
#include "fxls8974cf.h"
#include "slf3s4000b.h"
#include "i2c.h"
#include "mcc_generated_files/system/system.h"

static inline void SET_ACCEL_I2C_ADDR(uint8_t addr) {
    LATC2 = (addr == 0x19); // SA0 pin of FXLS set LSB of 7-bit I2C Address
}

static void can_msg_handler(const can_msg_t *msg) {}

int main(void) {
    SYSTEM_Initialize();

    // Enable global interrupts
    INTCON0bits.GIE = 1;

    // Set up CAN TX
    TRISC1 = 0;
    RC1PPS = 0x33;

    // Set up CAN RX
    TRISC0 = 1;
    ANSELC0 = 0;
    CANRXPPS = 0b00010000;

    // Set up CAN module
    can_timing_t can_setup;
    can_generate_timing_params(_XTAL_FREQ, &can_setup);
    can_init(&can_setup, can_msg_handler);

    // Set up timer 0 for millis
    timer0_init();

    TRISA2 = 0; // Blue LED output enable
    TRISA3 = 0; // Green LED output enable
    TRISA4 = 0; // Red LED output enable
    TRISC2 = 0; // Accelerometer I2C select pin output enable

    SET_ACCEL_I2C_ADDR(FXLS_I2C_ADDR);
    i2c_init(0b000); // I2C at 100 kHz
    
    // Accelerometer interrupt pin (INT2)
    TRISB3 = 0;     // Accelerometer output from PIC
    LATB3 = 0;      // Active high 
    
    // Flow sensor interrupt pin (IRQn)
    TRISC6 = 0;     // Flow sensor output from PIC
    LATC6 = 1;      // Active low

    uint32_t last_millis = 0;

    while (1) {
        if ((millis() - last_millis) < 2000) {
            continue;
        }
        last_millis = millis();

        CLRWDT();
        
        fxls_init();
        
        
        uint8_t whoami = fxls_get_whoami();
        uint8_t prod_rev = fxls_get_prod_rev();

        adc_result_t adc_value = ADCC_GetSingleConversion(channel_ANA0);

        BLUE_LED_TOGGLE();

        can_msg_t msg;
        build_board_stat_msg(millis(), E_NOMINAL, NULL, 0, &msg);
        can_send(&msg);
        while (!can_send_rdy()) {}
        
        // Read accelerometer data
        LATB3 = 1;
        while(!data_ready()) {
            for (int i = 0; i < 5; i++){
                
            }
        }
        LATB3 = 0;
        
        // Variables to hold accelerometer data
        int16_t x = 0;
        int16_t y = 0;
        int16_t z = 0;
        
        // Read accelerometer data
        fxls_read_accel_data(&x, &y, &z);
        
//        build_analog_data_msg(millis(), SENSOR_MAG_2, x, &msg);
//        can_send(&msg);
//        while (!can_send_rdy()) {}
//        build_analog_data_msg(millis(), SENSOR_VELOCITY, y, &msg);
//        can_send(&msg);
//        while (!can_send_rdy()) {}
//        build_analog_data_msg(millis(), SENSOR_ARM_BATT_1, z, &msg);
//        can_send(&msg);
//        while (!can_send_rdy()) {}
        
        
        // Read liquid flow sensor data
        LATC6 = 0;
        int16_t flow = 0;
        int16_t temperature = 0;
        read_flow_sensor_data(int16_t *flow, int16_t *temperature);
        LATC6 = 1;
        
        build_analog_data_msg(millis(), SENSOR_MAG_2, flow, &msg);
        can_send(&msg);
        while (!can_send_rdy()) {}
        build_analog_data_msg(millis(), SENSOR_VELOCITY, temperature, &msg);
        can_send(&msg);
        while (!can_send_rdy()) {}
    }
}

static void __interrupt() interrupt_handler() {
    if (PIR5) {
        can_handle_interrupt();
    }

    // Timer0 has overflowed - update millis() function
    // This happens approximately every 500us
    if (PIE3bits.TMR0IE == 1 && PIR3bits.TMR0IF == 1) {
        timer0_handle_interrupt();
        PIR3bits.TMR0IF = 0;
    }
}