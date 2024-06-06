#include "fxls8974cf.h"
#include "fxls8974cf_regs.h"
#include "i2c.h"
#include <stdio.h>

void fxls_init(void) {
    // Disable buffer mode
    uint8_t buf_config1 = i2c_read_reg8(FXLS_I2C_ADDR, FXLS_BUF_CONFIG1);
    buf_config1 &= ~BUF_MODE_MASK;  // Clear BUF_MODE1 and BUF_MODE0 bits
    i2c_write_reg8(FXLS_I2C_ADDR, FXLS_BUF_CONFIG1, buf_config1);

    // Enable data ready interrupt - is this even needed?
    uint8_t sens_config4 = i2c_read_reg8(FXLS_I2C_ADDR, FXLS_SENS_CONFIG4);
    sens_config4 |= DRDY_EN_MASK;  // Set DRDY_EN bit
    i2c_write_reg8(FXLS_I2C_ADDR, FXLS_SENS_CONFIG4, sens_config4);
}

uint8_t fxls_get_prod_rev(void) {
    return i2c_read_reg8(FXLS_I2C_ADDR, FXLS_PROD_REV);
}

uint8_t fxls_get_whoami(void) {
    return i2c_read_reg8(FXLS_I2C_ADDR, FXLS_WHO_AM_I);
}

int data_ready() {
    uint8_t status = i2c_read_reg8(FXLS_I2C_ADDR, INT_STATUS_REG);
    return (status & SRC_DRDY_MASK) ? 1 : 0;
}

void fxls_read_accel_data(int16_t *x, int16_t *y, int16_t *z) {
    uint8_t data[6];
    
    // Read 6 bytes of accelerometer data starting from the X-axis MSB register
    i2c_read_reg_multibyte(FXLS_I2C_ADDR, FXLS_REG_OUT_X_LSB, data, 6);
    
    *x = (int16_t)((data[1] << 8) | data[0]);
    *y = (int16_t)((data[3] << 8) | data[2]);
    *z = (int16_t)((data[5] << 8) | data[4]);
}
