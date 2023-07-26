#ifndef __TRANSPORT_H__
#define __TRANSPORT_H__

#include <stddef.h>

int Transport_init(void);
int Transport_getData(char *in, size_t count); 
int Transport_putData(const char *out, size_t count); 
int Transport_flush(void);
int Transport_dataWaiting(void);

#endif /* __TRANSPORT_H__ */

