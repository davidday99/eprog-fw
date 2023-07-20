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

static int testCommands(void) {
    size_t response_len = 0;
    int result = 1;

    memcpy(RxBuf, (char[]) {EPROG_CMD_NOP}, 1);
    response_len = eprog_RunCommand();
    result &= response_len == 1;
    result &= memcmp(TxBuf, (char[]) {eprog_ACK}, response_len) == 0;

    memcpy(RxBuf, (char[]) {EPROG_CMD_GET_INTERFACE_VERSION}, 1);
    response_len = eprog_RunCommand();
    result &= response_len == 3;

    memcpy(RxBuf, (char[]) {EPROG_CMD_GET_MAX_RX_SIZE}, 1);
    response_len = eprog_RunCommand();
    result &= response_len == 5;
    result &= memcmp(TxBuf, (char[]) {eprog_ACK, 0x00, 0x04, 0, 0}, response_len) == 0;

    memcpy(RxBuf, (char[]) {EPROG_CMD_GET_MAX_TX_SIZE}, 1);
    response_len = eprog_RunCommand();
    result &= response_len == 5;
    result &= memcmp(TxBuf, (char[]) {eprog_ACK, 0x00, 0x04, 0, 0}, response_len) == 0;

    memcpy(RxBuf, (char[]) {EPROG_CMD_TOGGLE_IO, 0}, 2);
    response_len = eprog_RunCommand();
    result &= response_len == 1;
    result &= memcmp(TxBuf, (char[]) {eprog_ACK}, response_len) == 0;

    memcpy(RxBuf, (char[]) {EPROG_CMD_TOGGLE_IO, 1}, 2);
    response_len = eprog_RunCommand();
    result &= response_len == 1;
    result &= memcmp(TxBuf, (char[]) {eprog_ACK}, response_len) == 0;

    memcpy(RxBuf, (char[]) {EPROG_CMD_TOGGLE_IO, 2}, 2);
    response_len = eprog_RunCommand();
    result &= response_len == 1;
    result &= memcmp(TxBuf, (char[]) {eprog_NAK}, response_len) == 0;

    memcpy(RxBuf, (char[]) {EPROG_CMD_SET_ADDRESS_BUS_WIDTH, 0x1}, 2);
    response_len = eprog_RunCommand();
    result &= response_len == 2;
    result &= memcmp(TxBuf, (char[]) {eprog_ACK, 0x01}, response_len) == 0;
    
    memcpy(RxBuf, (char[]) {EPROG_CMD_SET_ADDRESS_HOLD_TIME, 0x03, 0x00, 0x00, 0x00}, 5);
    response_len = eprog_RunCommand();
    result &= response_len == 5;
    result &= memcmp(TxBuf, (char[]) {eprog_ACK, 0x03, 0, 0, 0}, response_len) == 0;

    memcpy(RxBuf, (char[]) {EPROG_CMD_SET_PULSE_WIDTH_TIME, 0x03, 0x00, 0x00, 0x00}, 5);
    response_len = eprog_RunCommand();
    result &= response_len == 5;
    result &= memcmp(TxBuf, (char[]) {eprog_ACK, 0x03, 0, 0, 0}, response_len) == 0;

    memcpy(RxBuf, (char[]) {EPROG_CMD_PARALLEL_WRITE, 0, 0, 0, 0, 0x04, 0, 0 ,0, 0xab, 0xcd, 0xef, 0x12}, 13);
    response_len = eprog_RunCommand();
    result &= response_len == 1;
    result &= memcmp(TxBuf, (char[]) {eprog_ACK}, response_len) == 0;

    // delay some time for the write to complete
    for (int i = 0; i < 20000; i++)
        ;

    memcpy(RxBuf, (char[]) {EPROG_CMD_PARALLEL_READ, 0, 0, 0, 0, 0x4, 0, 0 ,0}, 9);
    response_len = eprog_RunCommand();
    result &= response_len == 5;
    result &= memcmp(TxBuf, (char[]) {eprog_ACK, 0xab, 0xcd, 0xef, 0x12}, response_len) == 0;

    return result;
}

int main(void){

    eprog_Init(RxBuf, sizeof(RxBuf), TxBuf, sizeof(TxBuf));

    int result = testCommands();
   
    while (1)
        ;
}

