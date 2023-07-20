#ifndef __PROGRAMMER_H___
#define __PROGRAMMER_H___

#include <stdint.h>
#include <stddef.h>

int programmer_Init(void);
int programmer_InitParallel(void);
int programmer_InitSpi(void);
int programmer_DisableIOPins(void);
int programmer_ToggleDataIOMode(uint8_t mode);
int programmer_SetAddress(uint8_t busWidth, uint32_t address);
int programmer_SetData(uint8_t data);
uint8_t programmer_GetData(void);
int programmer_ToggleCE(uint8_t state);
int programmer_ToggleOE(uint8_t state);
int programmer_ToggleWE(uint8_t state);
int programmer_Delay100ns(uint32_t delay);
int programmer_EnableChip(void);
int programmer_DisableChip(void);
int programmer_SetSpiClockFreq(uint32_t freq);
uint32_t programmer_GetSpiClockFreq(void);
int programmer_SetSpiMode(uint8_t mode);
uint8_t programmer_GetSpiMode(void);
int programmer_SpiRead(const char *txbuf, char *rxbuf, size_t count);
int programmer_SpiWrite(const char *buf, size_t count);

#endif /* __PROGRAMMER_H__ */

