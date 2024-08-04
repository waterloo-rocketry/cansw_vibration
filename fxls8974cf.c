#include "fxls8974cf.h"
#include "fxls8974cf_regs.h"
#include "rocketlib/include/i2c.h"
#include <stdio.h>

#include <xc.h>

void fxls_init(void) {
    volatile uint8_t sys_mode;
    i2c_read_reg8(FXLS_I2C_ADDR, 0x14, &sys_mode);
    
    // SENS_CONFIG2: Configure wake and sleep modes, endian format, and read mode
    uint8_t sens_config2 = 0b01000000;
    // <76> = 01, set to high performance mode 
    // <54> = 00, low power mode
    // <3> = 0, little endian
    // <2> = 0, doesn't matter 
    // <1> = 0, normal read instead of fast
    i2c_write_reg8(FXLS_I2C_ADDR, FXLS_SENS_CONFIG2, sens_config2);

    
    // BUF_CONFIG1: Disable buffer mode
    uint8_t buf_config1 = 0b10100000;
    // <7> = 1 First in last out
    // <65> = 01 Stream mode
    i2c_write_reg8(FXLS_I2C_ADDR, FXLS_BUF_CONFIG1, buf_config1);
      
    // SENS_CONFIG3: Set ODR to 100Hz: set bits 7:4 to 0101
    uint8_t sens_config3 = 0b01011111;
    // <7654> = 0101, set ODR to 100Hz
    // <3210> = 1111, not relevant  
    i2c_write_reg8(FXLS_I2C_ADDR, FXLS_SENS_CONFIG3, sens_config3);
    
    // Set the DRDY_EN bit (bit 7) in the INT_EN register to enable the data-ready interrupt.
    uint8_t int_en = 0b10000000;
    // <7> = 1, Interrupt is enabled and signaled on either the INT1 or INT2 output pins
    // <6> = 0, Interrupt is not routed to the INTx output pins (default)
    // <5> = 0, SDCD outside of thresholds interrupt disabled (default)
    // <4> = 0, SDCD within thresholds interrupt disabled (default)
    // <3> = 0, Orientation interrupt disabled (default)
    // <2> = 0, Auto-WAKE/SLEEP interrupt disabled (default)
    // <1> = 0, Boot interrupt is enabled and routed to either the INT1 or INT2 (default)
    // <0> = 0, The device does not signal the WAKE operating mode on the INTx output pin (default)
    i2c_write_reg8(FXLS_I2C_ADDR, FXLS_INT_EN, int_en);
    
    // Set the DRDY_INT2 bit (bit 7) in the INT_PIN_SEL register to route the data-ready interrupt to the INT2 pin.
    uint8_t int_pin_sel = 0b00000000;
    // <7> = data-ready interrupt signal is routed to INT2 pin
    // <6> = Output buffer interrupt routed to INT1 pin (not enabled)
    // <5> = SDCD outside of thresholds interrupt routed to INT1 pin (not enabled)
    // <4> = SDCD within thresholds interrupt routed to INT1 pin (not enabled)
    // <3> = ORIENT event interrupt routed to INT1 pin (not enabled)
    // <2> = Auto-WAKE/SLEEP interrupt routed to INT1 pin (not enabled)
    // <1> = BOOT event interrupt routed to INT1 pin
    // <0> = WAKE power state interrupt routed to INT1 pin (not enabled)
    i2c_write_reg8(FXLS_I2C_ADDR, FXLS_INT_PIN_SEL, int_pin_sel);
    
    
    // Set the INT_POL bit (bit 0) in the SENS_CONFIG4 register to configure the polarity of the interrupt (default active high).
    uint8_t sens_config4 = 0b00000001;
    // <7> = 0, each positive going trigger edge causes a single ADC acquisition to be made
    // <6> = 0, SDCD within thresholds event is not used (default)
    // <5> = 0, SDCD outside of thresholds event is not used (default)
    // <4> = 0, Orientation change condition is not used (default)
    // <3> = 0, A SRC_DRDY event is output on the INTx pin as an active high or active low signal
    // <2> = 0, INT2/EXT_TRIG pin is configured for the INT2 output function
    // <1> = 0, INTx output pin driver is push-pull type
    // <0> = 1, Active high: interrupt events are signaled with a logical '1' level
    i2c_write_reg8(FXLS_I2C_ADDR, FXLS_SENS_CONFIG4, sens_config4); 
    
    // SENS_CONFIG1: Enable Active Mode for I2C communication
    uint8_t sens_config1 = 0b00100001;
    // <7> = 0, no device reset pending, 
    // <65> = 01, self test x-axis
    // <4> = 0, proof mass displacement for the selected axis is in the positive direction
    // <3> = 0, SPI 3-wire or 4-wire, not relevant
    // <21> = 00, +-2g range
    // <0> = 1, active mode, the ACTIVE bit may only be set if INT2_FUNC = 0
    i2c_write_reg8(FXLS_I2C_ADDR, FXLS_SENS_CONFIG1, sens_config1);
}

uint8_t fxls_get_prod_rev(void) {
    uint8_t res;
    i2c_read_reg8(FXLS_I2C_ADDR, FXLS_PROD_REV, &res);
    return res;
}

uint8_t fxls_get_whoami(void) {
    uint8_t res;
    i2c_read_reg8(FXLS_I2C_ADDR, FXLS_WHO_AM_I, &res);
    return res;
}

// Do I need this if ISR handles interrupts from INT2?
int data_ready() {
    //INT_STATUS_REG = 0x00
    uint8_t status;
    i2c_read_reg8(FXLS_I2C_ADDR, INT_STATUS_REG, &status);
    // <7> -> new set of data available (0 = no)
    // <6> -> output data overflow detected (0 = no)
    // <5> -> output data buffer status (0 = none active)
    // <4> -> SDCD outside of thresholds condition (0 = none)
    // <3> -> SDCD within thresholds condition (0 = none)
    // <2> -> Orientation change event flag (0 = none)
    // <1> -> Auto-WAKE/SLEEP event status (0 = none)
    // <0> -> System boot complete (0 = in progress/not complete)
    return (status & SRC_DRDY_MASK) ? 1 : 0;
}

void fxls_read_accel_data(int16_t *x, int16_t *y, int16_t *z) {
    uint8_t x_lsb, x_msb, y_lsb, y_msb, z_lsb, z_msb;
    
    /*
    uint8_t data[6];
    uint8_t reg = 0x04;
    i2c_write_data(FXLS_I2C_ADDR, &reg, 1);
    i2c_read_data(FXLS_I2C_ADDR, data, 6);
    */

    // Read each byte of accelerometer data individually
    i2c_read_reg8(FXLS_I2C_ADDR, FXLS_REG_OUT_X_LSB, &x_lsb);
    i2c_read_reg8(FXLS_I2C_ADDR, FXLS_REG_OUT_X_MSB, &x_msb);
    i2c_read_reg8(FXLS_I2C_ADDR, FXLS_REG_OUT_Y_LSB, &y_lsb);
    i2c_read_reg8(FXLS_I2C_ADDR, FXLS_REG_OUT_Y_MSB, &y_msb);
    i2c_read_reg8(FXLS_I2C_ADDR, FXLS_REG_OUT_Z_LSB, &z_lsb);
    i2c_read_reg8(FXLS_I2C_ADDR, FXLS_REG_OUT_Z_MSB, &z_msb);
    
    uint8_t status;
    i2c_read_reg8(FXLS_I2C_ADDR, INT_STATUS_REG, &status);

    // Combine the LSB and MSB for each axis
    *x = (((int16_t)x_msb << 8) | x_lsb);
    *y = (((int16_t)y_msb << 8) | y_lsb);
    *z = (((int16_t)z_msb << 8) | z_lsb);
    
    // Convert to g units (0.98 mg/LSB)
    *x = (*x) * 0.98 / 1000.0;
    *y = (*y) * 0.98 / 1000.0;
    *z = (*z) * 0.98 / 1000.0;
}