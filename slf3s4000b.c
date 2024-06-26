#include "slf3s4000b.h"
#include "i2c.h"
#include <stdio.h>

void slf3s_init(void) {
    
}

void read_flow_sensor_data(int16_t *flow, int16_t *temperature) {
    uint16_t signaling_flags;
    uint8_t crc;
    int8_t data[9];  // 3x3 bytes of data
    
    // Water measurements: LQDS_I2C_ADDR = 0x8, CMD_START_CONT = 0x3608
    i2c_write_cmd_16(SLF3S_I2C_ADDR, CMD_START_CONT);
    
    i2c_read(LQDS_I2C_ADDR, data, 9);
    
    // Stop continuous measurements: CMD_STOP_CONT = 0x3FF9
    i2c_write_cmd_16(SLF3S_I2C_ADDR, CMD_STOP_CONT);

    *flow = (((int16_t)data[0] << 8) | data[1]);
    *temperature = (((int16_t)data[3] << 8) | data[4]);
    signaling_flags = (((int16_t)data[6] << 8) | data[7]);

    // Scale liquid flow rate and temperature values
    *flow = (*flow) / 32; // liquid flow rate in ml/min = sensor output / scale factor (32 (ml/min)^-1)
    *temperature = (*temperature) / 200; // temperature in °C = sensor output / scale factor (200 °C^-1)
}
