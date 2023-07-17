#ifndef __EPROG_H__
#define __EPROG_H__

#include <stdint.h>
#include <stddef.h>

enum IoState {
    IO_STATE_DISABLED,
    IO_STATE_ENABLED,
};

enum BusMode {
    BUS_MODE_NOT_SET = 0,
    BUS_MODE_PARALLEL = 1,
    BUS_MODE_SPI = 2,
    BUS_MODE_I2C = 4,
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

enum eprog_Command {
    EPROG_CMD_NOP,
    EPROG_CMD_GET_INTERFACE_VERSION,
    EPROG_CMD_GET_BUFFER_SIZE,
    EPROG_CMD_TOGGLE_IO,
    EPROG_CMD_SET_ADDRESS_BUS_WIDTH,
    EPROG_CMD_GET_ADDRESS_BUS_WIDTH,
    EPROG_CMD_SET_ADDRESS_HOLD_TIME,
    EPROG_CMD_GET_ADDRESS_HOLD_TIME,
    EPROG_CMD_SET_PULSE_WIDTH_TIME,
    EPROG_CMD_GET_PULSE_WIDTH_TIME,
    EPROG_CMD_PARALLEL_READ,
    EPROG_CMD_PARALLEL_WRITE,
    EPROG_CMD_SET_SPI_CLOCK_FREQ,
    EPROG_CMD_GET_SPI_CLOCK_FREQ,
    EPROG_CMD_SET_SPI_MODE,
    EPROG_CMD_GET_SPI_MODE,
    EPROG_CMD_SPI_READ,
    EPROG_CMD_SPI_WRITE,
};

extern const uint8_t eprog_ACK;
extern const uint8_t eprog_NAK;

/* General Commands */
int eprog_Init(const char *rxbuf, size_t rxsize, char *txbuf, size_t txsize);
size_t eprog_RunCommand(void);
uint16_t eprog_getInterfaceVersion(void);
uint32_t eprog_getSupportedBusTypes(void);
uint32_t eprog_getBufferSize(void);
uint8_t eprog_ToggleIo(enum IoState);

/* Parallel Commands */
enum AddressBusWidth eprog_getAddressBusWidth(void);
uint8_t eprog_setAddressBusWidth(enum AddressBusWidth);
uint8_t eprog_parallelRead(unsigned long address, char *buf, size_t count);
uint8_t eprog_parallelWrite(unsigned long address, const char *buf, size_t count);

/* SPI Commands */
enum SpiFrequency eprog_getSpifrequency(void);
uint8_t eprog_setSpifrequency(enum SpiFrequency);
uint8_t eprog_getSpiMode(void);
uint8_t eprog_setSpiMode(enum SpiMode);
uint8_t spi_read(char *buf, size_t count);
uint8_t spi_write(const char *buf, size_t count);

#endif /* __EPROG_H__ */

