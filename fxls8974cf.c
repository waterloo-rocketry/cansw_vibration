#include "fxls8974cf.h"
#include "fxls8974cf_regs.h"
#include "i2c.h"

void fxls_init(void) {}

uint8_t fxls_get_prod_rev(void) {
    return i2c_read_reg8(FXLS_I2C_ADDR, FXLS_PROD_REV);
}

uint8_t fxls_get_whoami(void) {
    return i2c_read_reg8(FXLS_I2C_ADDR, FXLS_WHO_AM_I);
}
