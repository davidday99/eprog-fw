
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "driverlib/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/ssi.h"

#define PART_TM4C123GH6PM
#include "driverlib/pin_map.h"

#define DATA_WIDTH 8
#define ADDRESS_WIDTH 15

typedef struct _GpioPin {
    uint32_t port;
    uint8_t pin;
} GpioPin_t;

typedef struct _SpiModule {
    GpioPin_t CLK;
    GpioPin_t CS;
    GpioPin_t RX;
    GpioPin_t TX;
} SpiModule_t;

typedef struct _Programmer {
    uint32_t ports[10];
    GpioPin_t A[ADDRESS_WIDTH];
    GpioPin_t IO[DATA_WIDTH];
    GpioPin_t WEn;
    GpioPin_t OEn;
    GpioPin_t CEn;
    SpiModule_t spi;
} Programmer;

static Programmer *Prog;

int programmer_InitParallel(void) {
    for (uint32_t *port = Prog->ports; *port != 0; port++) {
        SysCtlPeripheralEnable(*port);
        while (!SysCtlPeripheralReady(*port))
            ;
    }

    GPIOPinTypeGPIOOutput(Prog->WEn.port, Prog->WEn.pin);
    GPIOPinTypeGPIOOutput(Prog->CEn.port, Prog->CEn.pin);
    GPIOPinTypeGPIOOutput(Prog->OEn.port, Prog->OEn.pin);

    GPIOPinWrite(Prog->WEn.port, Prog->WEn.pin, Prog->WEn.pin);
    GPIOPinWrite(Prog->CEn.port, Prog->CEn.pin, Prog->CEn.pin);
    GPIOPinWrite(Prog->OEn.port, Prog->OEn.pin, Prog->OEn.pin);

    for (int i = 0; i < ADDRESS_WIDTH; i++ ) {
        GPIOPinTypeGPIOOutput(Prog->A[i].port, Prog->A[i].pin);
    }    

    return 1;
}

int programmer_InitSpi(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(Prog->spi.CLK.port);
    SysCtlPeripheralEnable(Prog->spi.CS.port);
    SysCtlPeripheralEnable(Prog->spi.RX.port);
    SysCtlPeripheralEnable(Prog->spi.TX.port);

    GPIOPinConfigure(GPIO_PA2_SSI0CLK);
    GPIOPinConfigure(GPIO_PA3_SSI0FSS);
    GPIOPinConfigure(GPIO_PA4_SSI0RX);
    GPIOPinConfigure(GPIO_PA5_SSI0TX);

    GPIOPinTypeSSI(GPIO_PORTA_BASE, 
                     GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_3 | GPIO_PIN_2);

    SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, 
            SSI_MODE_MASTER, 1000000, 8);

    SSILoopbackEnable(SSI0_BASE);

    SSIEnable(SSI0_BASE);

    return 1;
}

int programmer_EnableIOPins(void);
int programmer_DisableIOPins(void);
int programmer_SetAddress(enum AddressBusWidth busWidth, uint32_t address);
int programmer_SetData(uint32_t data);
uint32_t programmer_GetData();
int programmer_Delay100ns(uint32_t delay);
int programmer_EnableChip(void);
int programmer_DisableChip(void);
int programmer_SetSpiClockFreq(enum SpiFrequency freq);
enum SpiFrequency programmer_GetSpiClockFreq(void);
int programmer_SetSpiMode(enum SpiMode mode);
enum SpiMode programmer_GetSpiMode(void);
int programmer_SpiWrite(const char *buf, size_t count);
int programmer_SpiRead(const char *txbuf, char *rxbuf, size_t count);

