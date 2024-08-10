#ifndef SLF3S4000B_H
#define	SLF3S4000B_H

#include <stdint.h>

void slf3s_init(void);
void read_flow_sensor_data(uint16_t *flow, uint16_t *temperature);

#endif	/* SLF3S_H */
