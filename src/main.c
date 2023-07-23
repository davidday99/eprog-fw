#include <stdint.h>
#include <stdbool.h>
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/hw_memmap.h"
#include "string.h"
#include "eprog.h"

#define RUN_TESTS

static char RxBuf[1024];
static char TxBuf[1024];

int testCommands(void);

int main(void){


#ifdef RUN_TESTS
    int result = testCommands();

    while (1)
        ;
#endif

    eprog_Init(RxBuf, sizeof(RxBuf), TxBuf, sizeof(TxBuf));
   
    while (1)
        ;
}

