#ifndef SPI_H
#define	SPI_H

#define SPI_POLL_TIMEOUT 2000
#define SPI_MAX_TXWE_CNT 1000
#define SPI_MAX_RXRE_CNT 1000

#include <stdbool.h>
#include <stdint.h>

void spi1_init ();

void spi1_send(uint8_t data);

void spi1_send_buffer(uint8_t *data, uint16_t data_len, bool force_ss_high = 0);

void spi1_read_buffer(uint8_t *data, uint16_t data_len);

uint8_t spi1_read(void);



#endif	/* SPI_H */

