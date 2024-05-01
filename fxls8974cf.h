#ifndef FXLS8974CF_H
#define FXLS8974CF_H

#include <stdint.h>

#define FXLS_I2C_ADDR 0x18

void fxls_init(void);
uint8_t fxls_get_prod_rev(void);
uint8_t fxls_get_whoami(void);

#endif
