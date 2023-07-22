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

    eprog_Init(RxBuf, sizeof(RxBuf), TxBuf, sizeof(TxBuf));

#ifdef RUN_TESTS
    int result = testCommands();
#endif
   
    while (1)
        ;
}

