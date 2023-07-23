#include "eprog.h"
#include "eprog_server.h"
#include "string.h"

static char *RxBuf;
static char *TxBuf;
static size_t RxBufSize;
static size_t TxBufSize;

int transport_getData(char *in, size_t count); 

int eprog_parseCommand(void) {
    unsigned int idx = 0;
    uint32_t nLen;
    int validCmd = 1;
    transport_getData(RxBuf, 1); 
    idx++;

    enum eprog_Command cmd;
    size_t response_len = 0;
    memcpy(&cmd, RxBuf, sizeof(cmd));

    switch (cmd) {
        case EPROG_CMD_NOP:
        case EPROG_CMD_GET_INTERFACE_VERSION:
        case EPROG_CMD_GET_MAX_RX_SIZE:
        case EPROG_CMD_GET_MAX_TX_SIZE:
        case EPROG_CMD_GET_SUPPORTED_BUS_TYPES:
        case EPROG_CMD_GET_SUPPORTED_SPI_MODES:
            break;

        case EPROG_CMD_TOGGLE_IO:
        case EPROG_CMD_SET_ADDRESS_BUS_WIDTH:
        case EPROG_CMD_SET_SPI_MODE:
            transport_getData(&RxBuf[idx], 1);
            idx++;
            break;
        
        case EPROG_CMD_SET_ADDRESS_HOLD_TIME:
        case EPROG_CMD_SET_PULSE_WIDTH_TIME:
        case EPROG_CMD_SET_SPI_CLOCK_FREQ:
            transport_getData(&RxBuf[idx], 4);
            idx += 4;  
            break;

        case EPROG_CMD_PARALLEL_WRITE:   
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

        case EPROG_CMD_PARALLEL_READ:   
            transport_getData(&RxBuf[idx], 4);
            idx += 4;
            transport_getData(&RxBuf[idx], 4);
            memcpy(&nLen, &RxBuf[idx], sizeof(nLen));
            idx += 4;

            if (nLen + 9 > TxBufSize) {
                validCmd = 0;
            }
            
            break;

        case EPROG_CMD_SPI_TRANSMIT:
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


