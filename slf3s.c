#include "slf3s.h"
#include "i2c.h"
#include <stdio.h>

#define SLF3S_I2C_ADDR 0x8
#define CMD_START_CONT 0x3608
#define CMD_STOP_CONT 0x3FF9
#define CMD_ID_1 0x367C
#define CMD_ID_2 0xE102

static inline void i2c_write_cmd_16(uint8_t addr, uint16_t cmd) {
    uint8_t data[2];
    data[0] = (cmd >> 8) & 0xff;
    data[1] = cmd & 0xff;
    bool success_data = i2c_write_data(addr, data, 2);
}

void slf3s_init(void) {
    uint8_t data[18];
    i2c_write_cmd_16(SLF3S_I2C_ADDR, CMD_ID_1);
    i2c_write_cmd_16(SLF3S_I2C_ADDR, CMD_ID_2);
    bool success_data = i2c_read_data(SLF3S_I2C_ADDR, data, 18);
    // Water measurements: LQDS_I2C_ADDR = 0x8, CMD_START_CONT = 0x3608
    i2c_write_cmd_16(SLF3S_I2C_ADDR, CMD_START_CONT);
}

void read_flow_sensor_data(uint16_t *flow, uint16_t *temperature) {
    uint16_t signaling_flags;
    uint8_t crc;
    uint8_t data[9]; // 3x3 bytes of data

    bool success_data = i2c_read_data(SLF3S_I2C_ADDR, data, 9);


    *flow = (((uint16_t)data[0] << 8) | data[1]);
    *temperature = (((uint16_t)data[3] << 8) | data[4]);
    signaling_flags = (((uint16_t)data[6] << 8) | data[7]);

    // Scale liquid flow rate and temperature values
    //    *flow = (*flow) / 32; // liquid flow rate in ml/min = sensor output / scale factor (32
    //    (ml/min)^-1) *temperature = (*temperature) / 200; // temperature in °C = sensor output /
    //    scale factor (200 °C^-1)
}
