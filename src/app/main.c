#include <stdint.h>
#include <stdbool.h>
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/hw_memmap.h"
#include "string.h"
#include "open-eeprom_server.h"
#include "transport.h"

//#define RUN_PARALLEL_TESTS

static char RxBuf[1024];
static char TxBuf[1024];

int testGeneralCommands(void);
int testParallel(void);
int testSpi(void);

int main(void){

#ifdef RUN_GENERAL_TESTS
    int result = testGeneralCommands();
#endif

#ifdef RUN_PARALLEL_TESTS
    int result = testParallel();
#endif
    
#ifdef RUN_SPI_TESTS
    int result = testSpi();
#endif

    OpenEEPROM_serverInit(RxBuf, sizeof(RxBuf), TxBuf, sizeof(TxBuf));

    while (1) {
        OpenEEPROM_serverTick();
    }
}

