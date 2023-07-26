#ifndef __OPEN_EEPROM_SERVER_H__
#define __OPEN_EEPROM_SERVER_H__

#include <stddef.h>

int OpenEEPROM_serverInit(char *rxbuf, size_t maxRxSize, char *txbuf, size_t maxTxSize);
int OpenEEPROM_serverTick(void);

#endif /* __OPEN_EEPROM_SERVER_H__ */

