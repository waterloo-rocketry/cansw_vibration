#include <xc.h>

void spi_init () {

    SPI1CON0bits.EN = 1;
    


}

bool spi_write();

static void spi_send(uint8_t data);

static void spi_send_buffer(uint8_t *data, uint16_t data_len);

static void spi_read_buffer(uint8_t *data, uint16_t data_len);




#include "spi.h"



