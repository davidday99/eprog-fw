#include <stdint.h>
#include "eprog.h"

int target_EnableIOPins(void);
int target_DisableIOPins(void);
int target_SetAddress(enum AddressBusWidth busWidth, uint32_t address);
int target_SetData(uint32_t data);
uint32_t target_GetData();
int target_Delay100ns(uint32_t delay);
int target_EnableChip(void);
int target_DisableChip(void);

static const uint16_t Version = 0x01;
static const uint8_t SupportedBusTypes = BUS_TYPE_PARALLEL | BUS_TYPE_SPI;  // Put into a conf file.

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

uint16_t eprog_getInterfaceVersion(void) {
    return Version;
}

uint32_t eprog_getSupportedBusTypes(void) { 
    return SupportedBusTypes;
}

uint32_t eprog_getBufferSize(void);

uint8_t eprog_EnableIOPins(void) { 
    return target_EnableIOPins();
}

uint8_t eprog_DisableIOPins(void) { 
    return target_DisableIOPins();
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
    target_EnableChip();
    for (size_t i = 0; i < count; i++) {
        target_SetAddress(CurrentAddressBusWidth, address + i);
        target_Delay100ns(ParallelAddressHoldTime);
        buf[i] = target_GetData();
    } 
    target_DisableChip();
    return 1;
}

uint8_t eprog_parallelWrite(unsigned long address, char *buf, size_t count) {
    // Add parameter validation
    // Set programmer to Parallel Input Mode
    for (size_t i = 0; i < count; i++) {
        target_SetAddress(CurrentAddressBusWidth, address + i);
        target_SetData(buf[i]);
        target_Delay100ns(ParallelAddressHoldTime);
        target_EnableChip();
        target_Delay100ns(ChipEnablePulseWidthTime);
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

