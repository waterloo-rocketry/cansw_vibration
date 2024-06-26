#ifndef SPI_H
#define	SPI_H

#include <stdbool.h>
#include <stdint.h>

void spi_init ();

static void spi_send(uint8_t data);

static void spi_send_buffer(uint8_t *data, uint16_t data_len);

static void spi_read_buffer(uint8_t *data, uint16_t data_len);





#endif	/* SPI_H */

