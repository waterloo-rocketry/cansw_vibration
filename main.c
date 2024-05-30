#include <xc.h>

#include "canlib/canlib.h"
#include "common.h"
#include "fxls8974cf.h"
#include "rocketlib/include/i2c.h"
#include "mcc_generated_files/system/system.h"
#include "slf3s4000b.h"
#include <stdbool.h>

bool flag = false;

static inline void SET_ACCEL_I2C_ADDR(uint8_t addr) {
    LATC2 = (addr == 0x19); // SA0 pin of FXLS set LSB of 7-bit I2C Address
}

// Process CAN messages - what do we do here?
static void can_msg_handler(const can_msg_t *msg) {
    switch (msg->sid) {
        // What to process?
    }
}

// Write sensor data to SD card
void sdcard_write(uint32_t sector, uint8_t* data) {
    LATA5 = 0;  // Select SD card
    // How to write, SD card datasheet?
    LATA5 = 1;  // Deselect SD card
}

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
    CANRXPPS = 0x10;

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
    
    // SD Card pins
    TRISA5 = 0;     // PIC_MICRO_SD_CS: chip select
    TRISC5 = 0;     // PIC_MICRO_SD_SDO: serial data out (MOSI))
    TRISC3 = 0;     // PIC_MICRO_SD_SCK: serial clock
    TRISC4 = 0;     // PIC_MICRO_SD_SDI: serial data in (MISO)
    
    // Flow sensor interrupt pin (IRQn)
    TRISC6 = 1;     // Flow sensor input
    ANSELCbits.ANSELC6 = 0;
    IOCCNbits.IOCCN6 = 1;
    uint8_t LQDS_I2C_ADDR = 0x8
    
    

    SET_ACCEL_I2C_ADDR(FXLS_I2C_ADDR);
    i2c_init(0b000); // I2C at 100 kHz

    uint32_t last_millis = 0;

    while (1) {
        if ((millis() - last_millis) < 1000) {
            continue;
        }
        last_millis = millis();

        CLRWDT();   // Watchdog timer 
        
        // Accelerometer - where are these values used?
        uint8_t whoami = fxls_get_whoami();
        uint8_t prod_rev = fxls_get_prod_rev();
        
        // Converts analog signal to digital
        adc_result_t adc_value = ADCC_GetSingleConversion(channel_ANA0);

        BLUE_LED_TOGGLE();
        
        // Instance of CAN message struct
        can_msg_t msg;
        
        // Board status message
        build_board_stat_msg(millis(), E_NOMINAL, NULL, 0, &msg);
        can_send(&msg);
        

        while (!can_send_rdy()) {}
        
        // Sensor data message - external thermistor
        build_analog_data_msg(millis(), SENSOR_PAYLOAD_TEMP_1, adc_value, &msg);
        can_send(&msg);
        
        while (!can_send_rdy()) {}
        
        if (flag) {
            flag = false;
            
            // Read data from SDA line
            uint16_t sensor_data = i2c_read_reg16(LQDS_I2C_ADDR, )
            
            // Build flow sensor data message - placeholder for now
            build_analog_data_msg(millis(), SENSOR_VELOCITY, 21, &msg);
            can_send(&msg);

            while (!can_send_rdy()) {}
        }
                
        
        // What sector of the SD card?
        uint32_t sector = 0;
        
        // Write sensor data to SD card
        sdcard_write(sector, msg.data);
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
    
    // Interrupt from flow sensor - falling edge detected
    if (IOCCFbits.IOCCF6 == 1) {
        IOCCFbits.IOCCF6 = 0;   // Clear interrupt flag
        flag = true;
    }
}