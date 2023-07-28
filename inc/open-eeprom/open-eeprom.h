#ifndef __OPEN_EEPROM_H__
#define __OPEN_EEPROM_H__

#include <stdint.h>
#include <stddef.h>

enum OpenEEPROM_IOState {
    OPEN_EEPROM_IO_STATE_DISABLED,
    OPEN_EEPROM_IO_STATE_ENABLED,
};

enum OpenEEPROM_BusMode {
    OPEN_EEPROM_BUS_MODE_NOT_SET = 0,
    OPEN_EEPROM_BUS_MODE_PARALLEL = 1,
    OPEN_EEPROM_BUS_MODE_SPI = 2,
    OPEN_EEPROM_BUS_MODE_I2C = 4,
};

enum SpiMode {
    OPEN_EEPROM_SPI_MODE_0 = 1 ,
    OPEN_EEPROM_SPI_MODE_1 = 2,
    OPEN_EEPROM_SPI_MODE_2 = 4,
    OPEN_EEPROM_SPI_MODE_3 = 8,
};

enum OpenEEPROM_Command {
    OPEN_EEPROM_CMD_NOP,
    OPEN_EEPROM_CMD_SYNC,
    OPEN_EEPROM_CMD_GET_INTERFACE_VERSION,
    OPEN_EEPROM_CMD_GET_MAX_RX_SIZE,
    OPEN_EEPROM_CMD_GET_MAX_TX_SIZE,
    OPEN_EEPROM_CMD_TOGGLE_IO,
    OPEN_EEPROM_CMD_GET_SUPPORTED_BUS_TYPES,
    OPEN_EEPROM_CMD_SET_ADDRESS_BUS_WIDTH,
    OPEN_EEPROM_CMD_SET_ADDRESS_HOLD_TIME,
    OPEN_EEPROM_CMD_SET_PULSE_WIDTH_TIME,
    OPEN_EEPROM_CMD_PARALLEL_READ,
    OPEN_EEPROM_CMD_PARALLEL_WRITE,
    OPEN_EEPROM_CMD_SET_SPI_CLOCK_FREQ,
    OPEN_EEPROM_CMD_SET_SPI_MODE,
    OPEN_EEPROM_CMD_GET_SUPPORTED_SPI_MODES,
    OPEN_EEPROM_CMD_SPI_TRANSMIT,
};

extern const uint8_t OpenEEPROM_ACK;
extern const uint8_t OpenEEPROM_NAK;

/* General Commands */
size_t OpenEEPROM_runCommand(const char *in, char *out);
int OpenEEPROM_nop(const char *in, char *out);
int OpenEEPROM_sync(const char *in, char *out);
int OpenEEPROM_getInterfaceVersion(const char *in, char *out);
int OpenEEPROM_getSupportedBusTypes(const char *in, char *out);
int OpenEEPROM_getMaxRxSize(const char *in, char *out);
int OpenEEPROM_getMaxTxSize(const char *in, char *out);
int OpenEEPROM_toggleIO(const char *in, char *out);

/* Parallel Commands */
int OpenEEPROM_setAddressBusWidth(const char *in, char *out);
int OpenEEPROM_setAddressHoldTime(const char *in, char *out);
int OpenEEPROM_setAddressPulseWidthTime(const char *in, char *out);
int OpenEEPROM_parallelRead(const char *in, char *out);
int OpenEEPROM_parallelWrite(const char *in, char *out);

/* SPI Commands */
int OpenEEPROM_setSpiFrequency(const char *in, char *out);
int OpenEEPROM_setSpiMode(const char *in, char *out);
int OpenEEPROM_getSupportedSpiModes(const char *in, char *out);
int OpenEEPROM_spiTransmit(const char *in, char *out);

#endif /* __OPEN_EEPROM_H__ */

