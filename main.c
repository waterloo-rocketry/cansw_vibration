#include <stdio.h>
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

// memory pool for the CAN tx buffer
uint8_t tx_pool[400];

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

    if (f_mount(&FatFs, "", 1) == FR_OK) {
        if (f_open(&Fil, "PAYLOAD.txt", FA_CREATE_NEW | FA_WRITE) == FR_OK) {
            f_write(&Fil, "BEGIN LOG\r\n", 10, &bw);
            f_close(&Fil);
        }
    } else {
        while (f_mount(&FatFs, "", 1) != FR_OK) {
            if (millis() - last_status_millis > STATUS_TIME_DIFF_ms) {
                last_status_millis = millis();
                can_msg_t msg;
                build_board_stat_msg(millis(), E_LOGGING, NULL, 0, &msg);
                txb_enqueue(&msg);
            }

            txb_heartbeat();
        }
    }

    SET_ACCEL_I2C_ADDR(FXLS_I2C_ADDR);
    i2c_init(0b000); // I2C at 100 kHz

    fxls_init();
    slf3s_init();

    while (1) {
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

            build_imu_data_msg(MSG_SENSOR_ACC, millis(), a, &msg);
            txb_enqueue(&msg);

            char buf[25];
            size_t len = snprintf(buf, 25, "A%hu,%hu,%hu\n", a[0], a[1], a[2]);
            if (f_open(&Fil, "PAYLOAD.txt", FA_OPEN_APPEND | FA_WRITE) == FR_OK) {
                f_write(&Fil, buf, len, &bw);
                f_close(&Fil);
            }
        }

        if ((millis() - last_flow_millis) > FLOW_TIME_DIFF_ms) {
            last_flow_millis = millis();

            BLUE_LED_TOGGLE();

            uint16_t flow, temp;
            read_flow_sensor_data(&flow, &temp);

            build_analog_data_msg(millis(), SENSOR_PAYLOAD_FLOW_RATE, flow, &msg);
            txb_enqueue(&msg);

            build_analog_data_msg(millis(), SENSOR_PAYLOAD_TEMP, temp, &msg);
            txb_enqueue(&msg);

            char buf[25];
            size_t len = snprintf(buf, 25, "F%hu,%hu\n", flow, temp);
            if (f_open(&Fil, "PAYLOAD.txt", FA_OPEN_APPEND | FA_WRITE) == FR_OK) {
                f_write(&Fil, buf, len, &bw);
                f_close(&Fil);
            }
        }

        txb_heartbeat();
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
