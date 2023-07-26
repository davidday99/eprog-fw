#include "open-eeprom.h"
#include "open-eeprom_server.h"
#include "programmer.h"
#include "transport.h"
#include "string.h"

static char *RxBuf;
static char *TxBuf;
static size_t RxBufSize;
static size_t TxBufSize;

int (*Commands[])(const char *in, char *out) = {
    OpenEEPROM_nop,
    OpenEEPROM_getInterfaceVersion,
    OpenEEPROM_getMaxRxSize,
    OpenEEPROM_getMaxTxSize,
    OpenEEPROM_ToggleIo,
    OpenEEPROM_getSupportedBusTypes,
    OpenEEPROM_setAddressBusWidth,
    OpenEEPROM_setAddressHoldTime,
    OpenEEPROM_setAddressPulseWidthTime,
    OpenEEPROM_parallelRead,
    OpenEEPROM_parallelWrite,
    OpenEEPROM_setSpifrequency,
    OpenEEPROM_setSpiMode,
    OpenEEPROM_getSupportedSpiModes,
    OpenEEPROM_SpiTransmit,
};

static int OpenEEPROM_parseCommand(void);

int OpenEEPROM_serverInit(char *rxbuf, size_t maxRxSize, char *txbuf, size_t maxTxSize) {
    RxBuf = rxbuf;
    TxBuf = txbuf;
    RxBufSize = maxRxSize;
    TxBufSize = maxTxSize;
    programmer_Init();
    transport_Init();
    return 1;
}

int OpenEEPROM_serverTick(void) {
    int validCmd = 0;
    int response_len = 1;

    if (!transport_dataWaiting()) {
        return 0;
    }

    validCmd = OpenEEPROM_parseCommand();
    

    if (validCmd) {
        response_len = OpenEEPROM_RunCommand();
    } else {
        TxBuf[0] = OpenEEPROM_NAK;
    } 

    transport_putData(TxBuf, response_len);

    return validCmd;
}

size_t OpenEEPROM_RunCommand(void) {
    enum OpenEEPROM_Command cmd;
    size_t response_len = 0;
    memcpy(&cmd, RxBuf, sizeof(cmd));

    int (*func)(const char *in, char *out) = Commands[(uint8_t) cmd];

    response_len = func(RxBuf, TxBuf);

    return response_len;
}

static int OpenEEPROM_parseCommand(void) {
    unsigned int idx = 0;
    uint32_t nLen;
    int validCmd = 1;
    transport_getData(RxBuf, 1); 
    idx++;

    enum OpenEEPROM_Command cmd;
    memcpy(&cmd, RxBuf, sizeof(cmd));

    switch (cmd) {
        case OPEN_EEPROM_CMD_NOP:
        case OPEN_EEPROM_CMD_GET_INTERFACE_VERSION:
        case OPEN_EEPROM_CMD_GET_MAX_RX_SIZE:
        case OPEN_EEPROM_CMD_GET_MAX_TX_SIZE:
        case OPEN_EEPROM_CMD_GET_SUPPORTED_BUS_TYPES:
        case OPEN_EEPROM_CMD_GET_SUPPORTED_SPI_MODES:
            break;

        case OPEN_EEPROM_CMD_TOGGLE_IO:
        case OPEN_EEPROM_CMD_SET_ADDRESS_BUS_WIDTH:
        case OPEN_EEPROM_CMD_SET_SPI_MODE:
            transport_getData(&RxBuf[idx], 1);
            idx++;
            break;
        
        case OPEN_EEPROM_CMD_SET_ADDRESS_HOLD_TIME:
        case OPEN_EEPROM_CMD_SET_PULSE_WIDTH_TIME:
        case OPEN_EEPROM_CMD_SET_SPI_CLOCK_FREQ:
            transport_getData(&RxBuf[idx], 4);
            idx += 4;  
            break;

        case OPEN_EEPROM_CMD_PARALLEL_WRITE:   
            transport_getData(&RxBuf[idx], 4);
            idx += 4;
            transport_getData(&RxBuf[idx], 4);
            memcpy(&nLen, &RxBuf[idx], sizeof(nLen));
            idx += 4;
            
            // Account for the 9 bytes already inside the buffer.
            if (nLen+ 9 > RxBufSize) {
                validCmd = 0;
            } else {
                transport_getData(&RxBuf[idx], nLen);
                idx += nLen;
            }

            break;

        case OPEN_EEPROM_CMD_PARALLEL_READ:   
            transport_getData(&RxBuf[idx], 4);
            idx += 4;
            transport_getData(&RxBuf[idx], 4);
            memcpy(&nLen, &RxBuf[idx], sizeof(nLen));
            idx += 4;

            if (nLen + 9 > TxBufSize) {
                validCmd = 0;
            }
            
            break;

        case OPEN_EEPROM_CMD_SPI_TRANSMIT:
            transport_getData(&RxBuf[idx], 4);
            memcpy(&nLen, &RxBuf[idx], sizeof(nLen));
            idx += 4;
            
            /* In addition to the n bytes represented by nLen, the RxBuf will already contain
               the 1-byte command and 4-byte nLen, and the TxBuf will contain the command status ACK/NAK. */
            if (nLen + 5  > RxBufSize || nLen + 1 > TxBufSize) {
                validCmd = 0;
            } else {
                transport_getData(&RxBuf[idx], nLen);
            }

            break;

        default:
            validCmd = 0;
            break;
    }
    
    return validCmd;
}

int OpenEEPROM_getMaxRxSize(const char *in, char *out) {
    out[0] = OpenEEPROM_ACK;
    memcpy(&out[sizeof(OpenEEPROM_ACK)], &RxBufSize, sizeof(RxBufSize));
    return sizeof(OpenEEPROM_ACK) + sizeof(RxBufSize);
}

int OpenEEPROM_getMaxTxSize(const char *in, char *out) {
    out[0] = OpenEEPROM_ACK;
    memcpy(&out[sizeof(OpenEEPROM_ACK)], &TxBufSize, sizeof(TxBufSize));
    return sizeof(OpenEEPROM_ACK) + sizeof(TxBufSize);
}

