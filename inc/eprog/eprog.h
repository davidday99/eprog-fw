#ifndef __EPROG_H__
#define __EPROG_H__

#include <stddef.h>

enum BusType {
    BUS_TYPE_PARALLEL,
    BUS_TYPE_SPI,
    BUS_TYPE_I2C,
};

enum AddressBusWidth {
    ADDRESS_BUS_WIDTH_8,
    ADDRESS_BUS_WIDTH_14,
    ADDRESS_BUS_WIDTH_15,
    ADDRESS_BUS_WIDTH_16,
};

enum SpiFrequency {
    SPI_FREQ_1MHZ,
    SPI_FREQ_2MHZ,
    SPI_FREQ_4MHZ,
    SPI_FREQ_5MHZ,
};

enum SpiMode {
    SPI_MODE_0,
    SPI_MODE_1,
    SPI_MODE_2,
    SPI_MODE_3,
};

/* General Commands */
int eprog_getInterfaceVersion(void);
int eprog_getSupportedBusTypes(void);
int eprog_getConnectedAddressLines(void);
int eprog_getSerialBufferSize(void);
int eprog_getTransmitBufferSize(void);
int eprog_getReceiveBufferSize(void);

/* Programmer Commands */
enum BusType eprog_getBusType(void);
int eprog_setBusType(enum BusType);
void eprog_SystemWait(unsigned long);

/* Parallel Commands */
enum AddressBusWidth eprog_getAddressBusWidth(void);
void eprog_setAddressBusWidth(enum AddressBusWidth);
int eprog_parallelWrite(unsigned long address, char *buf, size_t count);
int eprog_parallelRead(unsigned long address, char *buf, size_t count);

/* SPI Commands */
enum SpiFrequency eprog_getSpifrequency(void);
int eprog_setSpifrequency(enum SpiFrequency);
int eprog_setSpiMode(enum SpiMode);

#endif /* __EPROG_H__ */

