#ifndef FXLS8974CF_REGS_H
#define FXLS8974CF_REGS_H

#define FXLS_PROD_REV 0x12
#define FXLS_WHO_AM_I 0x13

#define INT_STATUS_REG 0x00
#define SRC_DRDY_MASK 0x80 // 10000000 in binary, bit 7 mask

#define FXLS_SENS_CONFIG1 0x15
#define SENS_CONFIG1_MASK 0x01

#define FXLS_SENS_CONFIG2 0x16

#define FXLS_SENS_CONFIG3 0x17

#define FXLS_INT_EN 0x20

#define FXLS_INT_PIN_SEL 0x21

#define FXLS_SENS_CONFIG4 0x18
#define INT_POL_MASK 0x01

#define FXLS_BUF_CONFIG1 0x26
#define BUF_MODE_MASK 0x60;

#define FXLS_REG_OUT_X_MSB 0x05
#define FXLS_REG_OUT_X_LSB 0x04
#define FXLS_REG_OUT_Y_MSB 0x07
#define FXLS_REG_OUT_Y_LSB 0x06
#define FXLS_REG_OUT_Z_MSB 0x09
#define FXLS_REG_OUT_Z_LSB 0x08

#endif
