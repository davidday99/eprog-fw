#include <stdint.h>
#include "string.h"
#include "eprog.h"

int programmer_EnableIOPins(void);
int programmer_DisableIOPins(void);
int programmer_SetAddress(enum AddressBusWidth busWidth, uint32_t address);
int programmer_SetData(uint32_t data);
uint32_t programmer_GetData();
int programmer_Delay100ns(uint32_t delay);
int programmer_EnableChip(void);
int programmer_DisableChip(void);

static const uint16_t Version = 0x01;
static const uint8_t SupportedBusTypes = BUS_TYPE_PARALLEL | BUS_TYPE_SPI;  // Put into a conf file.
static char *RxBuf;
static char *TxBuf;
static size_t RxBufSize;
static size_t TxBufSize;

static enum AddressBusWidth CurrentAddressBusWidth = ADDRESS_BUS_WIDTH_8;
static enum SpiFrequency CurrentSpiFrequency = SPI_FREQ_1MHZ;
static enum SpiMode CurrentSpiMode = SPI_MODE_0; 

static uint32_t ParallelAddressHoldTime;
static uint32_t ChipEnablePulseWidthTime;

/*******************************************
********************************************
*             General Commands             *
********************************************
*******************************************/


int eprog_Init(char *rxbuf, size_t rxsize, char *txbuf, size_t txsize) {
    if (!rxbuf || ! txbuf) {
        return 0;
    } {
        RxBuf = rxbuf;
        RxBufSize = rxsize;
        TxBuf = txbuf;
        RxBufSize = rxsize;
        TxBufSize = txsize;
        return 1;
    }
}

size_t eprog_RunCommand(void) {
    enum eprog_Command cmd;
    size_t response_len = 0;

    memcpy(&cmd, RxBuf, sizeof(cmd));
    
    switch(cmd) {
        case EPROG_CMD_GET_INTERFACE_VERSION:
            TxBuf[0] = eprog_ACK;
            memcpy(&TxBuf[sizeof(eprog_ACK)], &Version, sizeof(Version));
            break;
        default:
            TxBuf[0] = eprog_ACK;
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

uint32_t eprog_getBufferSize(void);

uint8_t eprog_EnableIOPins(void) { 
    return programmer_EnableIOPins();
}

uint8_t eprog_DisableIOPins(void) { 
    return programmer_DisableIOPins();
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
    programmer_EnableChip();
    for (size_t i = 0; i < count; i++) {
        programmer_SetAddress(CurrentAddressBusWidth, address + i);
        programmer_Delay100ns(ParallelAddressHoldTime);
        buf[i] = programmer_GetData();
    } 
    programmer_DisableChip();
    return 1;
}

uint8_t eprog_parallelWrite(unsigned long address, char *buf, size_t count) {
    // Add parameter validation
    // Set programmer to Parallel Input Mode
    for (size_t i = 0; i < count; i++) {
        programmer_SetAddress(CurrentAddressBusWidth, address + i);
        programmer_SetData(buf[i]);
        programmer_Delay100ns(ParallelAddressHoldTime);
        programmer_EnableChip();
        programmer_Delay100ns(ChipEnablePulseWidthTime);
    }
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

