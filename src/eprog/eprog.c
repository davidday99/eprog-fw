#include <stdint.h>
#include "string.h"
#include "eprog.h"
#include "programmer.h"

const uint8_t eprog_ACK = 0x05;
const uint8_t eprog_NAK = 0x06;
static const uint16_t Version = 0x01;
static const uint8_t SupportedBusTypes = BUS_MODE_PARALLEL | BUS_MODE_SPI;  // Put into a conf file.
static const char *RxBuf;
static char *TxBuf;
static size_t RxBufSize;
static size_t TxBufSize;

static enum BusMode CurrentBusMode = BUS_MODE_NOT_SET;
static enum AddressBusWidth CurrentAddressBusWidth = ADDRESS_BUS_WIDTH_8;
static enum SpiFrequency CurrentSpiFrequency = SPI_FREQ_1MHZ;
static enum SpiMode CurrentSpiMode = SPI_MODE_0; 

static uint32_t ParallelAddressHoldTime;
static uint32_t ChipEnablePulseWidthTime;

static inline int switchToParallelBusMode(void);
static inline int switchToSpiBusMode(void);

/*******************************************
********************************************
*             General Commands             *
********************************************
*******************************************/


int eprog_Init(const char *rxbuf, size_t rxsize, char *txbuf, size_t txsize) {
    if (!rxbuf || ! txbuf) {
        return 0;
    } {
        RxBuf = rxbuf;
        RxBufSize = rxsize;
        TxBuf = txbuf;
        RxBufSize = rxsize;
        TxBufSize = txsize;
        programmer_Init();
        return 1;
    }
}

size_t eprog_RunCommand(void) {
    enum eprog_Command cmd;
    uint32_t arg1_32b, arg2_32b;
    size_t response_len = 0;

    memcpy(&cmd, RxBuf, sizeof(cmd));

    TxBuf[0] = eprog_ACK;
    response_len++;
    
    switch(cmd) {
        case EPROG_CMD_NOP:
            break; 

        case EPROG_CMD_GET_INTERFACE_VERSION:
            memcpy(&TxBuf[sizeof(eprog_ACK)], &Version, sizeof(Version));
            response_len += sizeof(Version);
            break;

        case EPROG_CMD_GET_BUFFER_SIZE:
            memcpy(&TxBuf[sizeof(eprog_ACK)], &RxBufSize, sizeof(RxBufSize));
            response_len += sizeof(RxBufSize);
            break;

        case EPROG_CMD_TOGGLE_IO:
            memcpy(&arg1_32b, &RxBuf[sizeof(uint8_t)], sizeof(uint8_t));
            if (!eprog_ToggleIo(arg1_32b)) {
                TxBuf[0] = eprog_NAK; 
            }
            break;

        case EPROG_CMD_SET_ADDRESS_BUS_WIDTH:
            memcpy(&CurrentAddressBusWidth, &RxBuf[sizeof(uint8_t)], sizeof(CurrentAddressBusWidth));
            break;

        case EPROG_CMD_GET_ADDRESS_BUS_WIDTH:
            memcpy(&TxBuf[sizeof(eprog_ACK)], &CurrentAddressBusWidth, sizeof(CurrentAddressBusWidth));
            response_len += sizeof(CurrentAddressBusWidth);
            break;

        case EPROG_CMD_SET_ADDRESS_HOLD_TIME:
            memcpy(&ParallelAddressHoldTime, &RxBuf[sizeof(uint8_t)], sizeof(ParallelAddressHoldTime));
            break;

        case EPROG_CMD_GET_ADDRESS_HOLD_TIME:
            memcpy(&TxBuf[sizeof(eprog_ACK)], &ParallelAddressHoldTime, sizeof(ParallelAddressHoldTime));
            response_len += sizeof(ParallelAddressHoldTime);
            break;
            
        case EPROG_CMD_SET_PULSE_WIDTH_TIME:
            memcpy(&ChipEnablePulseWidthTime, &RxBuf[sizeof(uint8_t)], sizeof(ChipEnablePulseWidthTime));
            break;

        case EPROG_CMD_GET_PULSE_WIDTH_TIME:
            memcpy(&TxBuf[sizeof(eprog_ACK)], &ChipEnablePulseWidthTime, sizeof(ChipEnablePulseWidthTime));
            response_len += sizeof(ChipEnablePulseWidthTime);
            break;

        case EPROG_CMD_PARALLEL_READ:
            memcpy(&arg1_32b, &RxBuf[sizeof(eprog_ACK)], sizeof(arg1_32b));  // address
            memcpy(&arg2_32b, &RxBuf[sizeof(eprog_ACK) + sizeof(arg1_32b)], sizeof(arg1_32b));  // count
                                                                                               
            if (!switchToParallelBusMode()) {
                TxBuf[0] = eprog_NAK; 
                break;
            }

            if (arg2_32b > TxBufSize || !eprog_parallelRead(arg1_32b, &TxBuf[sizeof(uint8_t)], arg2_32b)) {
                TxBuf[0] = eprog_NAK; 
            } else {
                response_len += arg2_32b;
            }
            break;

        case EPROG_CMD_PARALLEL_WRITE:
            memcpy(&arg1_32b, &RxBuf[sizeof(eprog_ACK)], sizeof(arg1_32b));  // address
            memcpy(&arg2_32b, &RxBuf[sizeof(eprog_ACK) + sizeof(arg1_32b)], sizeof(arg1_32b));  // count
                                                                                                
            if (!switchToParallelBusMode()) {
                TxBuf[0] = eprog_NAK; 
                break;
            }
            
            if (!eprog_parallelWrite(arg1_32b, &RxBuf[sizeof(eprog_ACK) + sizeof(arg1_32b) + sizeof(arg2_32b)], arg2_32b)) {
                TxBuf[0] = eprog_NAK;
            } else {
                response_len += arg2_32b;
            }
            break;

        case EPROG_CMD_SET_SPI_CLOCK_FREQ:
            memcpy(&arg1_32b, &RxBuf[sizeof(eprog_ACK)], sizeof(arg1_32b));
            if (arg1_32b != CurrentSpiFrequency) {
                if (programmer_SetSpiClockFreq(arg1_32b)) {
                    CurrentSpiFrequency = arg1_32b;
                } else {
                    TxBuf[0] = eprog_NAK;
                }
            }
            break;

        case EPROG_CMD_GET_SPI_CLOCK_FREQ:
            memcpy(&TxBuf[sizeof(eprog_ACK)], &CurrentSpiFrequency, sizeof(CurrentSpiFrequency));
            response_len += sizeof(CurrentSpiFrequency);
            break;

        case EPROG_CMD_SET_SPI_MODE:
            memcpy(&arg1_32b, &RxBuf[sizeof(eprog_ACK)], sizeof(arg1_32b));
            if (arg1_32b != CurrentSpiMode) {
                if (programmer_SetSpiMode(arg1_32b)) {
                    CurrentSpiMode = arg1_32b;
                } else {
                    TxBuf[0] = eprog_NAK;
                }
            }
            break;

        case EPROG_CMD_GET_SPI_MODE:
            memcpy(&TxBuf[sizeof(eprog_ACK)], &CurrentSpiMode, sizeof(CurrentSpiMode));
            response_len += sizeof(CurrentSpiMode);
            break;

        case EPROG_CMD_SPI_READ:
            memcpy(&arg1_32b, &RxBuf[sizeof(eprog_ACK)], sizeof(arg1_32b));  // count

            if (!switchToSpiBusMode()) {
                TxBuf[0] = eprog_NAK; 
                break;
            }

            if (arg1_32b > TxBufSize || !programmer_SpiRead(&RxBuf[sizeof(uint8_t) + sizeof(arg1_32b)], &TxBuf[sizeof(eprog_ACK)], arg1_32b)) {
                TxBuf[0] = eprog_NAK; 
            } else {
                response_len += arg1_32b;
            }
            break;

        case EPROG_CMD_SPI_WRITE:
            memcpy(&arg1_32b, &RxBuf[sizeof(eprog_ACK)], sizeof(arg1_32b));  // count

            if (!switchToSpiBusMode()) {
                TxBuf[0] = eprog_NAK; 
                break;
            }

            if (!programmer_SpiWrite(&RxBuf[sizeof(uint8_t) + sizeof(arg1_32b)], arg1_32b)) {
                TxBuf[0] = eprog_NAK;
            } else {
                response_len += arg1_32b;
            }
            break;

        default:
            TxBuf[0] = eprog_NAK;
            break;
    }
    return response_len;
}

uint16_t eprog_getInterfaceVersion(void) {
    return Version;
}

uint32_t eprog_getSupportedBusTypes(void) { 
    return SupportedBusTypes;
}

uint32_t eprog_getBufferSize(void) {
    return RxBufSize;
}

uint8_t eprog_ToggleIo(enum IoState state) {
    switch (state) {
        case IO_STATE_DISABLED:
            programmer_DisableIOPins();
            CurrentBusMode = BUS_MODE_NOT_SET;
            break;

        case IO_STATE_ENABLED:
            programmer_Init(); 
            break;

        default:
            return 0;
    }
    return 1;
}

uint8_t eprog_EnableIOPins(void) { 
    // return programmer_EnableIOPins();
}

uint8_t eprog_DisableIOPins(void) { 
    // return programmer_DisableIOPins();
}

/*******************************************
********************************************
*             Parallel Commands            *
********************************************
*******************************************/

enum AddressBusWidth eprog_getAddressBusWidth(void) {
    return CurrentAddressBusWidth;
}

uint8_t eprog_setAddressBusWidth(enum AddressBusWidth busWidth) {
    CurrentAddressBusWidth = busWidth;
    return 1;
}

uint8_t eprog_parallelRead(unsigned long address, char *buf, size_t count) {
    // Validate parameters 
    // Set programmer to Parallel Output Mode
    programmer_ToggleDataIOMode(0);
    programmer_ToggleOE(0);
    programmer_ToggleCE(0);
    for (size_t i = 0; i < count; i++) {
        programmer_SetAddress(CurrentAddressBusWidth, address + i);
        programmer_Delay100ns(ParallelAddressHoldTime);
        buf[i] = programmer_GetData();
    } 
    programmer_ToggleCE(1);
    programmer_ToggleOE(1);
    return 1;
}

uint8_t eprog_parallelWrite(unsigned long address, const char *buf, size_t count) {
    // Add parameter validation
    // Set programmer to Parallel Input Mode
    programmer_ToggleDataIOMode(1);
    programmer_ToggleOE(1);
    programmer_ToggleWE(0);
    for (size_t i = 0; i < count; i++) {
        programmer_SetAddress(CurrentAddressBusWidth, address + i);
        programmer_SetData(buf[i]);
        programmer_Delay100ns(ParallelAddressHoldTime);
        programmer_ToggleCE(0);
        programmer_Delay100ns(ChipEnablePulseWidthTime);
        programmer_ToggleCE(1);
    }
    programmer_ToggleWE(1);
    programmer_ToggleDataIOMode(0);
    return 1;
}

/*******************************************
********************************************
*             SPI Commands                 *
********************************************
*******************************************/

enum SpiFrequency eprog_getSpifrequency(void);
uint8_t eprog_setSpifrequency(enum SpiFrequency);
uint8_t eprog_setSpiMode(enum SpiMode);
uint8_t spi_read(char *buf, size_t count);
uint8_t spi_write(const char *buf, size_t count);

static inline int switchToParallelBusMode(void) {
    if ((CurrentBusMode != BUS_MODE_PARALLEL) && (BUS_MODE_PARALLEL & SupportedBusTypes)) {
        programmer_InitParallel();
        return 1;
    } else {
        return 0;
    }
}

static inline int switchToSpiBusMode(void) {
    if ((CurrentBusMode != BUS_MODE_SPI) && (BUS_MODE_SPI & SupportedBusTypes)) {
        programmer_InitSpi();
        return 1;
    } else {
        return 0;
    }
}

