#ifndef __EPROG_SERVER_H__
#define __EPROG_SERVER_H__

#include <stddef.h>

int eprog_serverInit(char *rxbuf, size_t maxRxSize, char *txbuf, size_t maxTxSize);
int eprog_serverTick(void);

#endif /* __EPROG_SERVER_H__ */

