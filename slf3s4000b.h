#ifndef SLF3S4000B_H
#define	SLF3S4000B_H

#include <stdint.h>

#define SLF3S_I2C_ADDR 0x8
#define CMD_START_CONT 0x3608
#define CMD_STOP_CONT 0x3FF9

void slf3s_init(void);
void read_flow_sensor_data(uint16_t *flow, uint16_t *temperature);

#endif	/* SLF3S4000B_H */
