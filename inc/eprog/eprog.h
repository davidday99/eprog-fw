#ifndef __EPROG_H__
#define __EPROG_H__

#include <stdint.h>
#include <stddef.h>

enum BusType {
    BUS_TYPE_PARALLEL = 1,
    BUS_TYPE_SPI = 2,
    BUS_TYPE_I2C = 4,
};

enum AddressBusWidth {
    ADDRESS_BUS_WIDTH_8 = 0,
    ADDRESS_BUS_WIDTH_14 = 1,
    ADDRESS_BUS_WIDTH_15 = 2,
    ADDRESS_BUS_WIDTH_16 = 3,
};

enum SpiFrequency {
    SPI_FREQ_1MHZ = 0,
    SPI_FREQ_2MHZ = 1,
    SPI_FREQ_4MHZ = 2,
    SPI_FREQ_5MHZ = 3,
};

enum SpiMode {
    SPI_MODE_0 = 0 ,
    SPI_MODE_1 = 1,
    SPI_MODE_2 = 2,
    SPI_MODE_3 = 3,
};

/* General Commands */
uint16_t eprog_getInterfaceVersion(void);
uint32_t eprog_getSupportedBusTypes(void);
uint32_t eprog_getBufferSize(void);
uint8_t enable_io_pins(void);
uint8_t disable_io_pins(void);

/* Parallel Commands */
enum AddressBusWidth eprog_getAddressBusWidth(void);
uint8_t eprog_setAddressBusWidth(enum AddressBusWidth);
uint8_t eprog_parallelRead(unsigned long address, char *buf, size_t count);
uint8_t eprog_parallelWrite(unsigned long address, char *buf, size_t count);

/* SPI Commands */
enum SpiFrequency eprog_getSpifrequency(void);
uint8_t eprog_setSpifrequency(enum SpiFrequency);
uint8_t eprog_getSpiMode(void);
uint8_t eprog_setSpiMode(enum SpiMode);
uint8_t spi_read(char *buf, size_t count);
uint8_t spi_write(const char *buf, size_t count);

#endif /* __EPROG_H__ */

