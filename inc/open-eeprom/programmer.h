#ifndef __PROGRAMMER_H___
#define __PROGRAMMER_H___

#include <stdint.h>
#include <stddef.h>

extern uint32_t Programmer_MinimumDelay;

int Programmer_init(void);
int Programmer_initParallel(void);
int Programmer_initSpi(void);
int Programmer_disableIOPins(void);
int Programmer_toggleDataIOMode(uint8_t mode);
int Programmer_getAddressPinCount(void);
int Programmer_setAddress(uint8_t busWidth, uint32_t address);
int Programmer_setData(uint8_t data);
uint8_t Programmer_getData(void);
int Programmer_toggleCE(uint8_t state);
int Programmer_toggleOE(uint8_t state);
int Programmer_toggleWE(uint8_t state);
int Programmer_delay1ns(uint32_t delay);
int Programmer_enableChip(void);
int Programmer_disableChip(void);
int Programmer_setSpiClockFreq(uint32_t freq);
uint32_t Programmer_getSpiClockFreq(void);
int Programmer_setSpiMode(uint8_t mode);
uint8_t Programmer_getSupportedSpiModes(void);
int Programmer_spiWrite(const char *buf, size_t count);
int Programmer_spiRead(const char *txbuf, char *rxbuf, size_t count);
int Programmer_spiTransmit(const char *txbuf, char *rxbuf, size_t count);

#endif /* __PROGRAMMER_H__ */

