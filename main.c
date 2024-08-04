#include <xc.h>

#include "canlib.h"
#include "common.h"
#include "fxls8974cf.h"
#include "i2c.h"
#include "mcc_fatfs/fatfs/ff.h"
#include "mcc_generated_files/system/system.h"

FATFS FatFs; /* FatFs work area needed for each volume */
FIL Fil;
UINT bw;

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
    TRISC2 = 0; // Acccelerometer I2C select pin output enable

    if (f_mount(&FatFs, "", 1) == FR_OK) { /* Mount SD */
        if (f_open(&Fil, "PAYLOAD.txt", FA_CREATE_NEW | FA_READ | FA_WRITE) == FR_OK) {
            f_write(&Fil, "BEGIN LOG\r\n", 10, &bw);
            f_close(&Fil);
        }
    } else {
        while (f_mount(&FatFs, "", 1) != FR_OK) {
            /*if (millis() - last_status_millis > STATUS_TIME_DIF_ms) {
                last_status_millis = millis();
                send_status_err();
            }

            txb_heartbeat();*/
        }
    }

    SET_ACCEL_I2C_ADDR(FXLS_I2C_ADDR);
    i2c_init(0b000); // I2C at 100 kHz

    uint32_t last_millis = 0;

    while (1) {
        if ((millis() - last_millis) < 500) {
            continue;
        }
        last_millis = millis();

        CLRWDT();

        uint8_t whoami = fxls_get_whoami();
        uint8_t prod_rev = fxls_get_prod_rev();

        adc_result_t adc_value = ADCC_GetSingleConversion(channel_ANA0);

        BLUE_LED_TOGGLE();

        can_msg_t msg;
        build_board_stat_msg(millis(), E_NOMINAL, NULL, 0, &msg);
        can_send(&msg);

        while (!can_send_rdy()) {}
        build_analog_data_msg(millis(), SENSOR_PAYLOAD_TEMP_1, adc_value, &msg);
        can_send(&msg);
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
