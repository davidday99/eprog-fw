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

enum SpiMode {
    SPI_MODE_0 = 1 ,
    SPI_MODE_1 = 2,
    SPI_MODE_2 = 4,
    SPI_MODE_3 = 8,
};

enum eprog_Command {
    EPROG_CMD_NOP,
    EPROG_CMD_GET_INTERFACE_VERSION,
    EPROG_CMD_GET_MAX_RX_SIZE,
    EPROG_CMD_GET_MAX_TX_SIZE,
    EPROG_CMD_TOGGLE_IO,
    EPROG_CMD_GET_SUPPORTED_BUS_TYPES,
    EPROG_CMD_SET_ADDRESS_BUS_WIDTH,
    EPROG_CMD_SET_ADDRESS_HOLD_TIME,
    EPROG_CMD_SET_PULSE_WIDTH_TIME,
    EPROG_CMD_PARALLEL_READ,
    EPROG_CMD_PARALLEL_WRITE,
    EPROG_CMD_SET_SPI_CLOCK_FREQ,
    EPROG_CMD_SET_SPI_MODE,
    EPROG_CMD_GET_SUPPORTED_SPI_MODE,
    EPROG_CMD_SPI_TRANSMIT,
};

extern const uint8_t eprog_ACK;
extern const uint8_t eprog_NAK;

/* General Commands */
int eprog_Init(char *rxbuf, size_t maxRxSize, char *txbuf, size_t maxTxSize);
size_t eprog_RunCommand(void);
int eprog_nop(const char *in, char *out);
int eprog_getInterfaceVersion(const char *in, char *out);
int eprog_getSupportedBusTypes(const char *in, char *out);
int eprog_getMaxRxSize(const char *in, char *out);
int eprog_getMaxTxSize(const char *in, char *out);
int eprog_ToggleIo(const char *in, char *out);

/* Parallel Commands */
int eprog_setAddressBusWidth(const char *in, char *out);
int eprog_setAddressHoldTime(const char *in, char *out);
int eprog_setAddressPulseWidthTime(const char *in, char *out);
int eprog_parallelRead(const char *in, char *out);
int eprog_parallelWrite(const char *in, char *out);

/* SPI Commands */
int eprog_setSpifrequency(const char *in, char *out);
int eprog_setSpiMode(const char *in, char *out);
int eprog_SpiTransmit(const char *in, char *out);

#endif /* __EPROG_H__ */

