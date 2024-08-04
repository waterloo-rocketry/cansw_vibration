#include "slf3s4000b.h"
#include "rocketlib/include/i2c.h"
#include <stdio.h>

void slf3s_init(void) {
    
}

void read_flow_sensor_data(uint16_t *flow, uint16_t *temperature) {
    uint16_t signaling_flags;
    uint8_t crc;
    uint8_t data[9];  // 3x3 bytes of data
    uint8_t start_cmd[2];
    uint8_t stop_cmd[2];
    
    start_cmd[0] = CMD_START_CONT >> 8;
    start_cmd[1] = CMD_START_CONT % 256;
    
    stop_cmd[0] = CMD_STOP_CONT >> 8;
    stop_cmd[1] = CMD_STOP_CONT % 256;
    
    
    // Water measurements: LQDS_I2C_ADDR = 0x8, CMD_START_CONT = 0x3608
    i2c_write(SLF3S_I2C_ADDR, start_cmd, 2);
    
    i2c_read(SLF3S_I2C_ADDR, data, 9);
    
    // Stop continuous measurements: CMD_STOP_CONT = 0x3FF9
    i2c_write(SLF3S_I2C_ADDR, stop_cmd, 2);

    *flow = (((uint16_t)data[0] << 8) | data[1]);
    *temperature = (((uint16_t)data[3] << 8) | data[4]);
    signaling_flags = (((uint16_t)data[6] << 8) | data[7]);

    // Scale liquid flow rate and temperature values
    *flow = (*flow) / 32; // liquid flow rate in ml/min = sensor output / scale factor (32 (ml/min)^-1)
    *temperature = (*temperature) / 200; // temperature in ?C = sensor output / scale factor (200 ?C^-1)
}