#ifndef __TRANSPORT_H__
#define __TRANSPORT_H__

#include <stddef.h>

int transport_getData(char *in, size_t count); 
int transport_putData(const char *out, size_t count); 
int transport_dataWaiting(void);

#endif /* __TRANSPORT_H__ */

