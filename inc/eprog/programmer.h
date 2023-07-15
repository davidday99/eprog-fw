#ifndef __PROGRAMMER_H___
#define __PROGRAMMER_H___

#include <stdint.h>
#include "eprog.h"

int programmer_EnableIOPins(void);
int programmer_DisableIOPins(void);
int programmer_SetAddress(enum AddressBusWidth busWidth, uint32_t address);
int programmer_SetData(uint32_t data);
uint32_t programmer_GetData();
int programmer_Delay100ns(uint32_t delay);
int programmer_EnableChip(void);
int programmer_DisableChip(void);
int programmer_SetSpiClockFreq(enum SpiFrequency freq);
enum SpiFrequency programmer_GetSpiClockFreq(void);
int programmer_SetSpiMode(enum SpiMode mode);
enum SpiMode programmer_GetSpiMode(void);
int programmer_SpiWrite(const char *buf, size_t count);
int programmer_SpiRead(const char *txbuf, char *rxbuf, size_t count);

#endif /* __PROGRAMMER_H__ */

