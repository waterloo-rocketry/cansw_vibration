#include <xc.h>

#include "canlib.h"
#include "common.h"
#include "fxls8974cf.h"
#include "i2c.h"
#include "mcc_generated_files/system/system.h"

static inline void SET_ACCEL_I2C_ADDR(uint8_t addr) {
    LATC2 = (addr == 0x19); // SA0 pin of FXLS set LSB of 7-bit I2C Address
}

int main(void) {
    SYSTEM_Initialize();

    TRISA2 = 0; // Blue LED output enable
    TRISA3 = 0; // Green LED output enable
    TRISA4 = 0; // Red LED output enable
    TRISC2 = 0; // Acccelerometer I2C select pin output enable

    SET_ACCEL_I2C_ADDR(FXLS_I2C_ADDR);
    i2c_init(0b000); // I2C at 100 kHz

    while (1) {
        CLRWDT();

        uint8_t whoami = fxls_get_whoami();
        uint8_t prod_rev = fxls_get_prod_rev();

        BLUE_LED_OFF();
        for (unsigned int i = 0; i < 60000; i++) {}

        BLUE_LED_ON();
        for (unsigned int i = 0; i < 60000; i++) {}
    }
}
