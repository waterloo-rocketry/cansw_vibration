#include "fxls8974cf.h"
#include "fxls8974cf_regs.h"
#include "i2c.h"
#include <stdio.h>

void fxls_init(void) {
    i2c_init(0b000); // I2C at 100 kHz SET_ACCEL_I2C_ADDR(FXLS_I2C_ADDR);

    // Disable buffer mode
    uint8_t buf_config1 = i2c_read_reg8(FXLS_I2C_ADDR, FXLS_BUF_CONFIG1);
    buf_config1 &= ~BUF_MODE_MASK;  // Clear BUF_MODE1 and BUF_MODE0 bits
    i2c_write_reg8(FXLS_I2C_ADDR, FXLS_BUF_CONFIG1, buf_config1);
    
    // Enable I2C
    uint8_t sens_config1 = i2c_read_reg8(FXLS_I2C_ADDR, FXLS_SENS_CONFIG1);
    sens_config1 |= SENS_CONFIG1_MASK;
    i2c_write_reg8(FXLS_I2C_ADDR, FXLS_SENS_CONFIG1, sens_config1);
    
    // Set ODR to 100Hz: set bits 7:4 to 0101
    uint8_t sens_config3 = i2c_read_reg8(FXLS_I2C_ADDR, FXLS_SENS_CONFIG3);
    sens_config3 &= 0x0F;
    sens_config3 |= 0b01010000;
    i2c_write_reg8(FXLS_I2C_ADDR, FXLS_SENS_CONFIG3, sens_config3);
    
    // Enable interrupts - 7th bit data ready interrupt
    uint8_t int_en = i2c_read_reg8(FXLS_I2C_ADDR, FXLS_INT_EN);
    int_en |= 0b10000000;
    i2c_write_reg8(FXLS_I2C_ADDR, FXLS_INT_EN, int_en);

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
    // INT_STATUS_REG = 0x00
    i2c_init(0b000); // I2C at 100 kHz SET_ACCEL_I2C_ADDR(FXLS_I2C_ADDR);
    uint8_t status = i2c_read_reg8(FXLS_I2C_ADDR, INT_STATUS_REG);
    return (status & SRC_DRDY_MASK) ? 1 : 0;
}

void fxls_read_accel_data(int16_t *x, int16_t *y, int16_t *z) {
    uint8_t x_lsb, x_msb, y_lsb, y_msb, z_lsb, z_msb;
    
    i2c_init(0b000); // Initialize I2C at 100 kHz

    // Read each byte of accelerometer data individually
    x_lsb = i2c_read_reg8(FXLS_I2C_ADDR, FXLS_REG_OUT_X_LSB);
    x_msb = i2c_read_reg8(FXLS_I2C_ADDR, FXLS_REG_OUT_X_MSB);
    y_lsb = i2c_read_reg8(FXLS_I2C_ADDR, FXLS_REG_OUT_Y_LSB);
    y_msb = i2c_read_reg8(FXLS_I2C_ADDR, FXLS_REG_OUT_Y_MSB);
    z_lsb = i2c_read_reg8(FXLS_I2C_ADDR, FXLS_REG_OUT_Z_LSB);
    z_msb = i2c_read_reg8(FXLS_I2C_ADDR, FXLS_REG_OUT_Z_MSB);

    // Combine the LSB and MSB for each axis
    *x = (int16_t)((x_msb << 8) | x_lsb);
    *y = (int16_t)((y_msb << 8) | y_lsb);
    *z = (int16_t)((z_msb << 8) | z_lsb);
}
