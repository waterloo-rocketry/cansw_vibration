#include "fxls8974cf.h"
#include "fxls8974cf_regs.h"
#include "i2c.h"
#include <stdio.h>

void fxls_init(void) {
    // SENS_CONFIG1: Enable Active Mode for I2C communication
    uint8_t sens_config1 = i2c_read_reg8(FXLS_I2C_ADDR, FXLS_SENS_CONFIG1);
    sens_config1 |= SENS_CONFIG1_MASK;
    i2c_write_reg8(FXLS_I2C_ADDR, FXLS_SENS_CONFIG1, sens_config1);

    volatile uint8_t sys_mode = i2c_read_reg8(FXLS_I2C_ADDR, 0x14);

    // SENS_CONFIG2: Configure wake and sleep modes, endian format, and read mode
    uint8_t sens_config2 = i2c_read_reg8(FXLS_I2C_ADDR, FXLS_SENS_CONFIG2);
    sens_config2 &= ~(0x01 << 3); // Clear LE_BE bit
    sens_config2 &= ~(0x01 << 1); // Clear AINC_TEMP bit
    sens_config2 &= ~(0x01); // Clear F_READ bit
    sens_config2 |= (0x01 << 6); // Set WAKE_PM to high-performance mode (01b)
    sens_config2 |= (0x00 << 4); // Set SLEEP_PM to low-power mode (00b)
    i2c_write_reg8(FXLS_I2C_ADDR, FXLS_SENS_CONFIG2, sens_config2);
    //
    // BUF_CONFIG1: Disable buffer mode
    BUF_MODE_MASK 0x60;
    uint8_t buf_config1 = i2c_read_reg8(FXLS_I2C_ADDR, FXLS_BUF_CONFIG1);
    buf_config1 &= ~BUF_MODE_MASK; // Clear BUF_MODE1 and BUF_MODE0 bits
    i2c_write_reg8(FXLS_I2C_ADDR, FXLS_BUF_CONFIG1, buf_config1);
    //
    // SENS_CONFIG3: Set ODR to 100Hz: set bits 7:4 to 0101
    uint8_t sens_config3 = i2c_read_reg8(FXLS_I2C_ADDR, FXLS_SENS_CONFIG3);
    sens_config3 &= 0x0F;
    sens_config3 |= 0b01010000;
    i2c_write_reg8(FXLS_I2C_ADDR, FXLS_SENS_CONFIG3, sens_config3);
    //
    // Set the DRDY_EN bit (bit 7) in the INT_EN register to enable the data-ready interrupt.
    uint8_t int_en = i2c_read_reg8(FXLS_I2C_ADDR, FXLS_INT_EN);
    int_en |= 0x80;
    i2c_write_reg8(FXLS_I2C_ADDR, FXLS_INT_EN, int_en);
    //
    // Set the DRDY_INT2 bit (bit 7) in the INT_PIN_SEL register to route the data-ready interrupt
    // to the INT2 pin.
    uint8_t int_pin_sel = i2c_read_reg8(FXLS_I2C_ADDR, FXLS_INT_PIN_SEL);
    int_pin_sel |= 0x80;
    i2c_write_reg8(FXLS_I2C_ADDR, FXLS_INT_PIN_SEL, int_pin_sel);
    //
    // Set the INT_POL bit (bit 0) in the SENS_CONFIG4 register to configure the polarity of the
    // interrupt (default active high).
    uint8_t sens_config4 = i2c_read_reg8(FXLS_I2C_ADDR, FXLS_SENS_CONFIG4);
    sens_config4 |= INT_POL_MASK; // Set INT_POL bit to 1 (active high interrupt on INT2 pin)
    i2c_write_reg8(FXLS_I2C_ADDR, FXLS_SENS_CONFIG4, sens_config4);
}

uint8_t fxls_get_prod_rev(void) {
    i2c_init(0b000); // Initialize I2C at 100 kHz
    return i2c_read_reg8(FXLS_I2C_ADDR, FXLS_PROD_REV);
}

uint8_t fxls_get_whoami(void) {
    i2c_init(0b000); // Initialize I2C at 100 kHz
    return i2c_read_reg8(FXLS_I2C_ADDR, FXLS_WHO_AM_I);
}

// Do I need this if ISR handles interrupts from INT2?
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
    *x = (((int16_t)x_msb << 8) | x_lsb);
    *y = (((int16_t)y_msb << 8) | y_lsb);
    *z = (((int16_t)z_msb << 8) | z_lsb);

    // Convert to g units from default �4 g mode (1.95 mg/LSB)
    //    *x = (*x) * 1.95 / 1000.0;
    //    *y = (*y) * 1.95 / 1000.0;
    //    *z = (*z) * 1.95 / 1000.0;
}
