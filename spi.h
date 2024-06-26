#ifndef SPI_H
#define	SPI_H

#include <stdbool.h>
#include <stdint.h>

void spi_init ();

void spi_send(uint8_t data);

void spi_send_buffer(uint8_t *data, uint16_t data_len);

void spi_read_buffer(uint8_t *data, uint16_t data_len);

uint8_t spi2_read(void);




#endif	/* SPI_H */

