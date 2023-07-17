#include <stdint.h>
#include <stdbool.h>
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/hw_memmap.h"
#include "string.h"
#include "eprog.h"

static char RxBuf[1024];
static char TxBuf[1024];

static void testCommands(void) {
    size_t response_len = 0;

    memcpy(RxBuf, (char[]) {EPROG_CMD_NOP}, 1);
    response_len = eprog_RunCommand();

    memcpy(RxBuf, (char[]) {EPROG_CMD_GET_INTERFACE_VERSION}, 1);
    response_len = eprog_RunCommand();

    memcpy(RxBuf, (char[]) {EPROG_CMD_GET_BUFFER_SIZE}, 1);
    response_len = eprog_RunCommand();

    // test toggle IO 

    memcpy(RxBuf, (char[]) {EPROG_CMD_SET_ADDRESS_BUS_WIDTH, 0x1}, 2);
    response_len = eprog_RunCommand();
    
    memcpy(RxBuf, (char[]) {EPROG_CMD_GET_ADDRESS_BUS_WIDTH}, 1);
    response_len = eprog_RunCommand();

    memcpy(RxBuf, (char[]) {EPROG_CMD_SET_ADDRESS_HOLD_TIME, 0x01, 0x00, 0x00, 0x00}, 5);
    response_len = eprog_RunCommand();

    memcpy(RxBuf, (char[]) {EPROG_CMD_GET_ADDRESS_HOLD_TIME}, 1);
    response_len = eprog_RunCommand();

    memcpy(RxBuf, (char[]) {EPROG_CMD_SET_PULSE_WIDTH_TIME, 0x01, 0x00, 0x00, 0x00}, 5);
    response_len = eprog_RunCommand();

    memcpy(RxBuf, (char[]) {EPROG_CMD_GET_PULSE_WIDTH_TIME}, 1);
    response_len = eprog_RunCommand();

    memcpy(RxBuf, (char[]) {EPROG_CMD_PARALLEL_READ, 0, 0, 0, 0, 0x14, 0, 0 ,0}, 9);
    response_len = eprog_RunCommand();

    memcpy(RxBuf, (char[]) {EPROG_CMD_PARALLEL_WRITE, 0, 0, 0, 0, 0x04, 0, 0 ,0, 0xab, 0xcd, 0xef, 0x12}, 13);
    response_len = eprog_RunCommand();
}

int main(void){

    eprog_Init(RxBuf, sizeof(RxBuf), TxBuf, sizeof(TxBuf));

    testCommands();
   
    while (1)
        ;
}

