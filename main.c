#include <stdio.h>
#include <string.h>
#include <xc.h>

#include "canlib.h"
#include "common.h"
#include "fxls8974cf.h"
#include "i2c.h"
#include "mcc_fatfs/fatfs/ff.h"
#include "mcc_generated_files/system/system.h"
#include "slf3s.h"


#define STATUS_TIME_DIFF_ms 500 // 2 Hz
#define FLOW_TIME_DIFF_ms 100 // 10 Hz
#define ACCEL_TIME_DIFF_ms 10 // 100 Hz

FATFS FatFs; /* FatFs work area needed for each volume */
FIL Fil;
UINT bw;

static char GLOBAL_FILENAME[20];

// memory pool for the CAN tx buffer
uint8_t tx_pool[400];

// File write buffer
char file_write_buf[1500];
uint16_t file_write_buf_ptr = 0;

// Sensor CAN rate limiting
uint8_t flow_poll_count = 0;
uint8_t accel_poll_count = 0;

#define FLOW_SEND_DIVISOR 5
#define ACCEL_SEND_DIVISOR 50

static inline void SET_ACCEL_I2C_ADDR(uint8_t addr) {
    LATC2 = (addr == 0x19); // SA0 pin of FXLS set LSB of 7-bit I2C Address
}

static void can_msg_handler(const can_msg_t *msg) {
    if (get_board_unique_id(msg) == BOARD_UNIQUE_ID) {
        return;
    }

    int dest_id = -1;
    switch (get_message_type(msg)) {
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
        default:
            break;
    }
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
    CANRXPPS = 0b00010000;

    // Set up CAN module
    can_timing_t can_setup;
    can_generate_timing_params(_XTAL_FREQ, &can_setup);
    can_init(&can_setup, can_msg_handler);

    // set up CAN tx buffer
    txb_init(tx_pool, sizeof(tx_pool), can_send, can_send_rdy);

    // Set up timer 0 for millis
    timer0_init();

    TRISA2 = 0; // Blue LED output enable
    TRISA3 = 0; // Green LED output enable
    TRISA4 = 0; // Red LED output enable
    TRISC2 = 0; // Acccelerometer I2C select pin output enable

    uint32_t last_status_millis = millis();
    uint32_t last_flow_millis = millis();
    uint32_t last_accel_millis = millis();

    while (f_mount(&FatFs, "", 1) != FR_OK) {
        if (millis() - last_status_millis > STATUS_TIME_DIFF_ms) {
            last_status_millis = millis();
            can_msg_t msg;
            build_board_stat_msg(millis(), E_LOGGING, NULL, 0, &msg);
            txb_enqueue(&msg);
        }

        txb_heartbeat();
    }

    // count the number of flies in the root directory of the SD card
    uint16_t root_dir_files = 0;
    FFDIR dir;
    if (f_opendir(&dir, "/") != FR_OK) {
        // error(E_SD_FAIL_FS_INIT);
    }

    FILINFO finfo;
    while (f_readdir(&dir, &finfo) == FR_OK && finfo.fname[0] != '\0') {
        root_dir_files++;
    }
    f_closedir(&dir);

    sprintf(GLOBAL_FILENAME, "PAY_%04x.txt", root_dir_files);

    if (f_open(&Fil, GLOBAL_FILENAME, FA_CREATE_NEW | FA_WRITE) == FR_OK) {
        f_write(&Fil, "BEGIN PAYLOAD LOG\n", 18, &bw);
        f_close(&Fil);
    }

    SET_ACCEL_I2C_ADDR(FXLS_I2C_ADDR);
    i2c_init(0b000); // I2C at 100 kHz

    fxls_init();
    slf3s_init();

    while (1) {
        volatile uint8_t whoiam = fxls_get_whoami() ;
        can_msg_t msg;
        CLRWDT();
        

        if ((millis() - last_status_millis) > STATUS_TIME_DIFF_ms) {
            last_status_millis = millis();

            RED_LED_TOGGLE();

            build_board_stat_msg(millis(), E_NOMINAL, NULL, 0, &msg);
            txb_enqueue(&msg);
        }

        if ((millis() - last_accel_millis) > ACCEL_TIME_DIFF_ms) {
            last_accel_millis = millis();

            GREEN_LED_TOGGLE();

            // Variables to hold accelerometer data
            uint16_t a[3];

            // Read accelerometer data
            fxls_read_accel_data(a, a + 1, a + 2);

            if (accel_poll_count == 0) {
                build_imu_data_msg(MSG_SENSOR_ACC, millis(), a, &msg);
                txb_enqueue(&msg);
            }
            accel_poll_count =
                (accel_poll_count == ACCEL_SEND_DIVISOR) ? 0 : (accel_poll_count + 1);

            char buf[25];
            int len = snprintf(buf, 25, "A%hu,%hu,%hu\n", a[0], a[1], a[2]);
            memcpy(file_write_buf + file_write_buf_ptr, buf, len);
            file_write_buf_ptr += len;
        }


        if ((millis() - last_flow_millis) > FLOW_TIME_DIFF_ms) {
            last_flow_millis = millis();

            BLUE_LED_TOGGLE();

            uint16_t flow, temp;
            read_flow_sensor_data(&flow, &temp);

            if (flow_poll_count == 0) {
                build_analog_data_msg(millis(), SENSOR_PAYLOAD_FLOW_RATE, flow, &msg);
                txb_enqueue(&msg);

                // Scale to degree C for CAN message, but keep original raw data for logging
                build_analog_data_msg(millis(), SENSOR_PAYLOAD_TEMP, temp / 200, &msg);
                txb_enqueue(&msg);
            }
            flow_poll_count = (flow_poll_count == FLOW_SEND_DIVISOR) ? 0 : (flow_poll_count + 1);

            char buf[25];
            int len = snprintf(buf, 25, "F%hu,%hu\n", flow, temp);
            memcpy(file_write_buf + file_write_buf_ptr, buf, len);
            file_write_buf_ptr += len;
        }

        txb_heartbeat();
        
        

        if (file_write_buf_ptr > 1400) {
            if (f_open(&Fil, GLOBAL_FILENAME, FA_OPEN_APPEND | FA_WRITE) == FR_OK) {
                f_write(&Fil, file_write_buf, file_write_buf_ptr, &bw);
                f_close(&Fil);
            }
            file_write_buf_ptr = 0;
        }
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
