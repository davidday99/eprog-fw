#ifndef __SPI_H__
#define __SPI_H__

#include <stddef.h>

int spi_transmit(const char *txbuf, char *rxbuf, size_t count);

#endif /* __SPI_H__ */

