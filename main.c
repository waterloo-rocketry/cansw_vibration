#include <xc.h>

#include "canlib/canlib.h"
#include "common.h"
#include "fxls8974cf.h"
#include "rocketlib/include/i2c.h"
#include "mcc_generated_files/system/system.h"
#include "slf3s4000b.h"
#include <stdbool.h>

#define LQDS_I2C_ADDR 0x8
#define CMD_START_CONT 0x3608
#define MAX_BUS_DEAD_TIME_ms 1000
#define MAX_LOOP_TIME_DIFF_ms 250

bool SLF3S_flag = false;
bool FXLS_flag = false;
volatile bool seen_can_message = false;

// Instance of CAN message struct
can_msg_t msg;

static inline void SET_ACCEL_I2C_ADDR(uint8_t addr) {
    LATC2 = (addr == 0x19); // SA0 pin of FXLS set LSB of 7-bit I2C Address
}

// What do we do here?
static void can_msg_handler(const can_msg_t *msg) {
    seen_can_message = true;
    uint16_t msg_type = get_message_type(msg);
    
    // ignore messages that were sent from this board
    if (get_board_unique_id(msg) == BOARD_UNIQUE_ID) {
        return;
    }
    
    switch (msg_type) {
        // Add cases here for board messages?
    }
}

//uint8_t calculate_crc(uint8_t *data, size_t length) {
//    uint8_t crc = 0xFF;  // Initialization value
//    for (size_t i = 0; i < length; i++) {
//        crc ^= data[i];  // XOR with the input byte
//        for (uint8_t j = 0; j < 8; j++) {
//            if (crc & 0x80) {
//                crc = (crc << 1) ^ 0x31;  // Polynomial used for CRC-8
//            } else {
//                crc <<= 1;
//            }
//        }
//    }
//    return crc;
//}

uint8_t calculate_crc(uint8_t data1, uint8_t data2) {
    uint8_t crc = 0xFF; // Initialization value
    uint8_t polynomial = 0x31; // Polynomial x8 + x5 + x4 + 1

    uint8_t data[] = { data1, data2 };
    for (int i = 0; i < 2; i++) {
        crc ^= data[i]; // XOR-in the next input byte

        for (int j = 0; j < 8; j++) { // Process each bit
            if (crc & 0x80) {
                crc = (crc << 1) ^ polynomial;
            } else {
                crc <<= 1;
            }
        }
    }

    return crc; // No final XOR
}

void read_flow_sensor_data() {
    uint16_t flow;
    uint16_t temperature;
    uint16_t signaling_flags;
    uint8_t crc;
    uint8_t data[9];  // 3x3 bytes of data
    
    // Water measurements: LQDS_I2C_ADDR = 0x8, CMD_START_CONT = 0x3608
    i2c_write_cmd(LQDS_I2C_ADDR, CMD_START_CONT);
    
    while (1) {
        if (i2c_read(LQDS_I2C_ADDR, data, 9)) {
            break;  // Error while reading I2C data
        }
        
        flow = (data[0] << 8) | data[1];
        temperature = (data[3] << 8) | data[4];
        signaling_flags = (data[6] << 8) | data[7];
        
        // Scale liquid flow rate and temperature values
        flow = flow / 32; // liquid flow rate in ml/min = sensor output / scale factor (32 (ml/min)^-1)
        temperature = temperature / 200; // temperature in °C = sensor output / scale factor (200 °C^-1)

        // Build flow sensor data messages
        //can_msg_t msg;
        build_analog_data_msg(millis(), SENSOR_PAYLOAD_TEMP_2, flow, &msg);
        can_send(&msg);

        while (!can_send_rdy()) {}

        build_analog_data_msg(millis(), SENSOR_PAYLOAD_TEMP_2, temperature, &msg);
        can_send(&msg);

        while (!can_send_rdy()) {}
    }
}

// Write sensor data to SD card
void sdcard_write(uint32_t sector, uint8_t* data) {
    LATA5 = 0;  // Select SD card
    //
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
    
    // Initialize accelerometer
    fxls_init();
    
    // loop timer
    uint32_t last_millis = millis();
    uint32_t sensor_last_millis = millis();
    uint32_t last_message_millis = millis();


    TRISA2 = 0; // Blue LED output enable
    TRISA3 = 0; // Green LED output enable
    TRISA4 = 0; // Red LED output enable
    TRISC2 = 0; // Accelerometer I2C select pin output enable
    
    // SD Card pins
    TRISA5 = 0;     // PIC_MICRO_SD_CS: chip select
    TRISC5 = 0;     // PIC_MICRO_SD_SDO: serial data out (MOSI)
    TRISC3 = 0;     // PIC_MICRO_SD_SCK: serial clock
    TRISC4 = 0;     // PIC_MICRO_SD_SDI: serial data in (MISO)
    
    // Flow sensor interrupt pin (IRQn)
    TRISC6 = 1;     // Flow sensor input
    ANSELCbits.ANSELC6 = 0;   
    
    // Accelerometer interrupt pin (INT2)
    TRISB3 = 1;     // Flow sensor input
    ANSELBbits.ANSELB3 = 0; 

    SET_ACCEL_I2C_ADDR(FXLS_I2C_ADDR);
    i2c_init(0b000); // I2C at 100 kHz
    
    IOCBPbits.IOCBP3 = 1;    // Accelerometer active high interrupt (RB3: INT2)
    IOCCNbits.IOCCN6 = 1;    // Flow sensor active low interrupt (RC6: IRQn)

    
//    uint32_t last_millis = 0;    // old timer
    bool heartbeat = false;        // Where is this used?

    while (1) {
        CLRWDT();   // Watchdog timer
        
        if (seen_can_message) {
            seen_can_message = false;
            last_message_millis = millis();
        }
        
        if (millis() - last_message_millis > MAX_BUS_DEAD_TIME_ms) {
            // We've got too long without seeing a valid CAN message (including
            // one of ours)
            RESET();
        }
        
        // Old loop delay
        if ((millis() - last_millis) < 1000) {
            continue;
        }
        
        last_millis = millis();
        
        // Board status message
        build_board_stat_msg(millis(), E_NOMINAL, NULL, 0, &msg);
        can_send(&msg);
        while (!can_send_rdy()) {}
            
        // Accelerometer
        uint8_t whoami = fxls_get_whoami();
        uint8_t prod_rev = fxls_get_prod_rev();
        
        // Ensure any previous interrupts are cleared by reading the INT_STATUS register - is this needed?
        uint8_t int_status = i2c_read_reg8(FXLS_I2C_ADDR, INT_STATUS_REG);  

        if (FXLS_flag) {
            FXLS_flag = false;
            
            // Variables to hold accelerometer data
            int16_t x, y, z;
        
            // Read accelerometer data
            fxls_read_accel_data(&x, &y, &z);
            
            // Accelerometer x-axis data
            build_analog_data_msg(millis(), SENSOR_MAG_1, x, &msg);
            can_send(&msg);
            while (!can_send_rdy()) {}

            // Accelerometer y-axis data
            build_analog_data_msg(millis(), SENSOR_MAG_2, y, &msg);
            can_send(&msg);
            while (!can_send_rdy()) {}

            // Accelerometer z-axis data
            build_analog_data_msg(millis(), SENSOR_VELOCITY, z, &msg);
            can_send(&msg);
            while (!can_send_rdy()) {}
        }
        
        int test_data_ready = data_ready();
                
        // Converts analog signal to digital
        adc_result_t adc_value = ADCC_GetSingleConversion(channel_ANA0);

        BLUE_LED_TOGGLE();  // heartbeat?
        
//        // External thermistor
//        build_analog_data_msg(millis(), SENSOR_PAYLOAD_TEMP_1, adc_value, &msg);
//        can_send(&msg);
//        while (!can_send_rdy()) {}
        
        // Liquid flow sensor 
        if (SLF3S_flag) {
            SLF3S_flag = false;

            // Read liquid flow sensor data
            read_flow_sensor_data();
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
    
    // Interrupt from accelerometer - rising edge detected (active high)
    if (IOCBFbits.IOCBF3 == 1) {
        IOCBFbits.IOCBF3 = 0;   // Clear interrupt flag
        FXLS_flag = true;
    }
    
    // Interrupt from flow sensor - falling edge detected (active low)
    if (IOCCFbits.IOCCF6 == 1) {
        IOCCFbits.IOCCF6 = 0;   // Clear interrupt flag
        SLF3S_flag = true;
    }
}