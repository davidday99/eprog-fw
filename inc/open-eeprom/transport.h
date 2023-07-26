#ifndef __TRANSPORT_H__
#define __TRANSPORT_H__

#include <stddef.h>

int transport_Init(void);
int transport_getData(char *in, size_t count); 
int transport_putData(const char *out, size_t count); 
int transport_flush(void);
int transport_dataWaiting(void);

#endif /* __TRANSPORT_H__ */

