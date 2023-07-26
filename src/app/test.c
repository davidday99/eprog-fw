#include <stdint.h>
#include <stdbool.h>
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/hw_memmap.h"
#include "string.h"
#include "open-eeprom.h"
#include "open-eeprom_server.h"

static char RxBuf[1024];
static char TxBuf[1024];

int testGeneralCommands(void) {
    size_t response_len = 0;
    int result = 1;

    OpenEEPROM_serverInit(RxBuf, sizeof(RxBuf), TxBuf, sizeof(TxBuf));

    memcpy(RxBuf, (char[]) {OPEN_EEPROM_CMD_NOP}, 1);
    response_len = OpenEEPROM_runCommand();
    result &= response_len == 1;
    result &= memcmp(TxBuf, (char[]) {OpenEEPROM_ACK}, response_len) == 0;

    memcpy(RxBuf, (char[]) {OPEN_EEPROM_CMD_GET_INTERFACE_VERSION}, 1);
    response_len = OpenEEPROM_runCommand();
    result &= response_len == 3;

    memcpy(RxBuf, (char[]) {OPEN_EEPROM_CMD_GET_MAX_RX_SIZE}, 1);
    response_len = OpenEEPROM_runCommand();
    result &= response_len == 5;
    result &= memcmp(TxBuf, (char[]) {OpenEEPROM_ACK, 0x00, 0x04, 0, 0}, response_len) == 0;

    memcpy(RxBuf, (char[]) {OPEN_EEPROM_CMD_GET_MAX_TX_SIZE}, 1);
    response_len = OpenEEPROM_runCommand();
    result &= response_len == 5;
    result &= memcmp(TxBuf, (char[]) {OpenEEPROM_ACK, 0x00, 0x04, 0, 0}, response_len) == 0;

    memcpy(RxBuf, (char[]) {OPEN_EEPROM_CMD_TOGGLE_IO, 0}, 2);
    response_len = OpenEEPROM_runCommand();
    result &= response_len == 2;
    result &= memcmp(TxBuf, (char[]) {OpenEEPROM_ACK, 0}, response_len) == 0;

    memcpy(RxBuf, (char[]) {OPEN_EEPROM_CMD_TOGGLE_IO, 1}, 2);
    response_len = OpenEEPROM_runCommand();
    result &= response_len == 2;
    result &= memcmp(TxBuf, (char[]) {OpenEEPROM_ACK, 0}, response_len) == 0;

    return result;
}

int testParallel(void) {
    size_t response_len = 0;
    int result = 1;

    OpenEEPROM_serverInit(RxBuf, sizeof(RxBuf), TxBuf, sizeof(TxBuf));

    memcpy(RxBuf, (char[]) {OPEN_EEPROM_CMD_SET_ADDRESS_BUS_WIDTH, 15}, 2);
    response_len = OpenEEPROM_runCommand();
    result &= response_len == 2;
    result &= memcmp(TxBuf, (char[]) {OpenEEPROM_ACK, 15}, response_len) == 0;
    
    memcpy(RxBuf, (char[]) {OPEN_EEPROM_CMD_SET_ADDRESS_HOLD_TIME, 0x03, 0x00, 0x00, 0x00}, 5);
    response_len = OpenEEPROM_runCommand();
    result &= response_len == 5;
    result &= memcmp(TxBuf, (char[]) {OpenEEPROM_ACK, 0x03, 0, 0, 0}, response_len) == 0;

    memcpy(RxBuf, (char[]) {OPEN_EEPROM_CMD_SET_PULSE_WIDTH_TIME, 0x03, 0x00, 0x00, 0x00}, 5);
    response_len = OpenEEPROM_runCommand();
    result &= response_len == 5;
    result &= memcmp(TxBuf, (char[]) {OpenEEPROM_ACK, 0x03, 0, 0, 0}, response_len) == 0;

    memcpy(RxBuf, (char[]) {OPEN_EEPROM_CMD_PARALLEL_WRITE, 0, 0, 0, 0, 0x04, 0, 0 ,0, 0xab, 0xcd, 0xef, 0x12}, 13);
    response_len = OpenEEPROM_runCommand();
    result &= response_len == 1;
    result &= memcmp(TxBuf, (char[]) {OpenEEPROM_ACK}, response_len) == 0;

    // delay some time for the write to complete
    for (int i = 0; i < 20000; i++)
        ;

    memcpy(RxBuf, (char[]) {OPEN_EEPROM_CMD_PARALLEL_READ, 0, 0, 0, 0, 0x4, 0, 0 ,0}, 9);
    response_len = OpenEEPROM_runCommand();
    result &= response_len == 5;
    result &= memcmp(TxBuf, (char[]) {OpenEEPROM_ACK, 0xab, 0xcd, 0xef, 0x12}, response_len) == 0;

    memcpy(RxBuf, (char[]) {OPEN_EEPROM_CMD_SPI_TRANSMIT, 0x01, 0, 0, 0, 0x06}, 6);
    response_len = OpenEEPROM_runCommand();
    result &= response_len == 2;
    result &= memcmp(TxBuf, (char[]) {OpenEEPROM_ACK}, response_len) == 0;

    return result;
}

int testSpi(void) {
    size_t response_len = 0;
    int result = 1;

    OpenEEPROM_serverInit(RxBuf, sizeof(RxBuf), TxBuf, sizeof(TxBuf));

    memcpy(RxBuf, (char[]) {OPEN_EEPROM_CMD_SPI_TRANSMIT, 0x7, 0, 0, 0, 0x02, 0, 0, 0xab, 0xcd, 0xef, 0x12}, 12);
    response_len = OpenEEPROM_runCommand();
    result &= response_len == 8;
    result &= memcmp(TxBuf, (char[]) {OpenEEPROM_ACK, 0, 0, 0, 0, 0, 0, 0, 0}, response_len) == 0;

    memcpy(RxBuf, (char[]) {OPEN_EEPROM_CMD_SPI_TRANSMIT, 7, 0, 0, 0, 0x03, 0, 0, 0, 0, 0, 0}, 12);
    response_len = OpenEEPROM_runCommand();
    result &= response_len == 8;
    result &= memcmp(TxBuf, (char[]) {OpenEEPROM_ACK, 0, 0, 0, 0xab, 0xcd, 0xef, 0x12}, response_len) == 0;

    return result;
}

