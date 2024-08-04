#include <xc.h>
#include <stdio.h>

#include "canlib/canlib.h"
#include "common.h"
#include "fxls8974cf.h"
#include "slf3s4000b.h"
#include "rocketlib/include/i2c.h"

#include "mcc_generated_files/mcc.h"
#include "adcc.h"
#include "mcc_generated_files/fatfs/ff.h"

#define STATUS_TIME_DIF_ms 500
#define ACCEL_TIME_DIF_ms 500
#define FLOW_TIME_DIF_ms 500
#define MESSAGE_TIME_DIF_ms 500

static inline void SET_ACCEL_I2C_ADDR(uint8_t addr) {
    //should this be high? always low right now
    LATC2 = (addr == 0x19); // SA0 pin of FXLS set LSB of 7-bit I2C Address
}

FATFS FatFs; /* FatFs work area needed for each volume */
FIL Fil;
UINT bw;

// memory pool for the CAN tx buffer
uint8_t tx_pool[150];
volatile bool seen_can_message = false;

static void can_msg_handler(const can_msg_t *msg);
static void send_status_ok(void);
static void send_status_err(void);

void main(void) {
    // Initialize the device and adc
    SYSTEM_Initialize();
    ADCC_Initialize();

    // Enable global interrupts
    INTCON0bits.GIE = 1;

    // Set up timer 0 for millis
    timer0_init();
    uint32_t last_status_millis = millis();
    uint32_t last_accel_millis = millis();
    uint32_t last_flow_millis = millis();
    uint32_t last_message_millis = millis();

    // Set up CAN TX
    TRISC1 = 0;
    RC1PPS = 0x33;

    // Set up CAN RX
    TRISC0 = 1;
    ANSELC0 = 0;
    CANRXPPS = 0b00010000;

    // set up CAN module
    can_timing_t can_setup;
    can_generate_timing_params(_XTAL_FREQ, &can_setup);
    can_init(&can_setup, can_msg_handler);
    // set up CAN tx buffer
    txb_init(tx_pool, sizeof (tx_pool), can_send, can_send_rdy);
    //set up can message
    can_msg_t msg;

    //init flow sensor and accel
    I2C1SDAPPS = 0b01010;
    I2C1SCLPPS = 0b01001;
    RB2PPS = 0b100010;
    RB1PPS = 0b100001;
    SET_ACCEL_I2C_ADDR(FXLS_I2C_ADDR);
    i2c_init(0b000); // I2C at 100 kHz

    // Flow sensor interrupt pin (IRQn)
    TRISC6 = 0; // Flow sensor output from PIC
    LATC6 = 1; // Active low

    uint8_t whoami = fxls_get_whoami();
    uint8_t prod_rev = fxls_get_prod_rev();

    // Accelerometer interrupt pin (INT2)
    TRISB3 = 0; // Accelerometer output from PIC
    LATB3 = 0; // Active high

    TRISA2 = 0; // Blue LED output enable
    TRISA3 = 0; // Green LED output enable
    TRISA4 = 0; // Red LED output enable
    TRISC2 = 0; // Accelerometer I2C select pin output enable

    if (f_mount(&FatFs, "", 1) == FR_OK) { /* Mount SD */
        if (f_open(&Fil, "Data.txt", FA_CREATE_NEW | FA_READ | FA_WRITE) == FR_OK) {
            f_write(&Fil, "BEGIN LOG\r\n", 10, &bw);
            f_close(&Fil);
        }
    } else {
        while (f_mount(&FatFs, "", 1) != FR_OK) {
            if (millis() - last_status_millis > STATUS_TIME_DIF_ms) {
                last_status_millis = millis();
                send_status_err();
            }

            txb_heartbeat();
        }
    }


    while (1) {
        CLRWDT();

        adc_result_t adc_value = ADCC_GetSingleConversion(channel_ANA0);

        if (millis() - last_status_millis > STATUS_TIME_DIF_ms) {
            last_status_millis = millis();
            send_status_ok();
        }


        // Read accelerometer data
        //BROCKEN
        if (millis() - last_accel_millis > ACCEL_TIME_DIF_ms) {
            fxls_init();

            LATB3 = 1;
            while (!data_ready()) {
                uint8_t sys_mode;
                i2c_read_reg8(FXLS_I2C_ADDR, SYS_MODE_REG, &sys_mode);
                // <7> -> Buffer gate error
                // <65432> -> Number of OOR periods that have elapsed since the BUF _GATE_ERR flag was asserted
                // <10> -> Current system operating mode (00 standby, 01 wake, 10 sleep, 11 ext_trig)

                uint8_t buf_status;
                i2c_read_reg8(FXLS_I2C_ADDR, BUF_STATUS_REG, &buf_status);
                // <7> -> not important should be 0
                // <6> -> Buffer overflow, 1 = occurred
                // <543210> -> num of accel data triplets in data buffer

                uint8_t status;
                i2c_read_reg8(FXLS_I2C_ADDR, INT_STATUS_REG, &status);
                // <7> -> new set of data available (0 = no)
                // <6> -> output data overflow detected (0 = no)
                // <5> -> output data buffer status (0 = none active)
                // <4> -> SDCD outside of thresholds condition (0 = none)
                // <3> -> SDCD within thresholds condition (0 = none)
                // <2> -> Orientation change event flag (0 = none)
                // <1> -> Auto-WAKE/SLEEP event status (0 = none)
                // <0> -> System boot complete (0 = in progress/not complete)\
                
                uint8_t who;
                i2c_read_reg8(FXLS_I2C_ADDR, FXLS_WHO_AM_I, &who);
                
                uint8_t rev;
                i2c_read_reg8(FXLS_I2C_ADDR, FXLS_PROD_REV, &rev);
                }
            LATB3 = 0;

            // Variables to hold accelerometer data
            uint16_t x = 0;
            uint16_t y = 0;
            uint16_t z = 0;

            // Read accelerometer data
            fxls_read_accel_data(&x, &y, &z);

            build_analog_data_msg(millis(), SENSOR_MAG_2, x, &msg);

            build_analog_data_msg(millis(), SENSOR_VELOCITY, y, &msg);

            build_analog_data_msg(millis(), SENSOR_ARM_BATT_1, z, &msg);


        }




        // Read liquid flow sensor data
        //BROCKEN
        /*
        if (millis() - last_flow_millis > FLOW_TIME_DIF_ms) {
            LATC6 = 0;
            uint16_t flow = 0;
            uint16_t temperature = 0;
            read_flow_sensor_data(&flow, &temperature);
            LATC6 = 1;

            build_analog_data_msg(millis(), SENSOR_MAG_2, flow, &msg);
            txb_enqueue(&msg);
            build_analog_data_msg(millis(), SENSOR_VELOCITY, temperature, &msg);
            txb_enqueue(&msg);

            if (f_open(&Fil, "Data.txt", FA_OPEN_APPEND | FA_READ | FA_WRITE) == FR_OK) {
                f_write(&Fil, "Hello world!\r\n", 14, &bw);
            }
            f_close(&Fil);
            
        } */

        txb_heartbeat();
    }
}

static void can_msg_handler(const can_msg_t *msg) {
    seen_can_message = true;
    uint16_t msg_type = get_message_type(msg);
    int dest_id = -1;
    int cmd_type = -1;
    // ignore messages that were sent from this board
    if (get_board_unique_id(msg) == BOARD_UNIQUE_ID) {
        return;
    }

    switch (msg_type) {
        case MSG_LEDS_ON:
            BLUE_LED_ON();
            GREEN_LED_ON();
            RED_LED_ON();
            break;

        case MSG_LEDS_OFF:
            BLUE_LED_OFF();
            GREEN_LED_OFF();
            RED_LED_OFF();
            break;

        case MSG_RESET_CMD:
            dest_id = get_reset_board_id(msg);
            if (dest_id == BOARD_UNIQUE_ID || dest_id == 0) {
                RESET();
            }
            break;

            // all the other ones - do nothing
        default:
            break;
    }
}

// Send a CAN message with nominal status

static void send_status_ok(void) {
    can_msg_t board_stat_msg;
    build_board_stat_msg(millis(), E_NOMINAL, NULL, 0, &board_stat_msg);

    //not working
    txb_enqueue(&board_stat_msg);

    uint32_t time = millis();
    uint8_t timestamp[3];

    timestamp[0] = (time >> 16) & 0xff;
    timestamp[1] = (time >> 8) & 0xff;
    timestamp[2] = (time >> 0) & 0xff;

    char text[3];
    sprintf(text, "%lud\r\n", time);

    if (f_open(&Fil, "Data.txt", FA_OPEN_APPEND | FA_READ | FA_WRITE) == FR_OK) {
        //logging time with a bunch of random shit in front of it but probably good enough
        f_write(&Fil, text, sizeof (text), &bw);
        f_write(&Fil, "\n", 1, &bw);
        f_close(&Fil);

        BLUE_LED_TOGGLE();
    }
}

static void send_status_err(void) {
    can_msg_t board_stat_msg;
    build_board_stat_msg(millis(), E_MISSING_CRITICAL_BOARD, NULL, 0, &board_stat_msg);
    txb_enqueue(&board_stat_msg);

    RED_LED_TOGGLE();
}

static void __interrupt() interrupt_handler(void) {
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