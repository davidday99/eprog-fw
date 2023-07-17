
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "driverlib/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/ssi.h"
#include "programmer.h"

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

static Programmer _Prog = {
    .ports = {
        SYSCTL_PERIPH_GPIOA,
        SYSCTL_PERIPH_GPIOB,
        SYSCTL_PERIPH_GPIOC,
        SYSCTL_PERIPH_GPIOD,
        SYSCTL_PERIPH_GPIOE,
        SYSCTL_PERIPH_GPIOF
    },
    .A = {
        {GPIO_PORTB_BASE, GPIO_PIN_5},
        {GPIO_PORTB_BASE, GPIO_PIN_0},
        {GPIO_PORTB_BASE, GPIO_PIN_1},
        {GPIO_PORTE_BASE, GPIO_PIN_4},
        {GPIO_PORTE_BASE, GPIO_PIN_5},
        {GPIO_PORTB_BASE, GPIO_PIN_4},
        {GPIO_PORTA_BASE, GPIO_PIN_5},
        {GPIO_PORTA_BASE, GPIO_PIN_6},
        {GPIO_PORTA_BASE, GPIO_PIN_7},
        {GPIO_PORTF_BASE, GPIO_PIN_1},
        {GPIO_PORTE_BASE, GPIO_PIN_3},
        {GPIO_PORTE_BASE, GPIO_PIN_2},
        {GPIO_PORTE_BASE, GPIO_PIN_1},
        {GPIO_PORTD_BASE, GPIO_PIN_3},
        {GPIO_PORTD_BASE, GPIO_PIN_2},
    },
    .IO = {
        {GPIO_PORTA_BASE, GPIO_PIN_3},
        {GPIO_PORTA_BASE, GPIO_PIN_4},
        {GPIO_PORTB_BASE, GPIO_PIN_6},
        {GPIO_PORTB_BASE, GPIO_PIN_7},
        {GPIO_PORTC_BASE, GPIO_PIN_5},
        {GPIO_PORTC_BASE, GPIO_PIN_4},
        {GPIO_PORTE_BASE, GPIO_PIN_0},
        {GPIO_PORTB_BASE, GPIO_PIN_2},

    },
    .CEn = {GPIO_PORTA_BASE, GPIO_PIN_2},
    .OEn = {GPIO_PORTD_BASE, GPIO_PIN_6},
    .WEn = {GPIO_PORTC_BASE, GPIO_PIN_7},
    .spi = {
        .CLK = {GPIO_PORTA_BASE, GPIO_PIN_2},
        .CS = {GPIO_PORTA_BASE, GPIO_PIN_3},
        .RX = {GPIO_PORTA_BASE, GPIO_PIN_4},
        .TX = {GPIO_PORTA_BASE, GPIO_PIN_5}
    }
};

static Programmer *Prog = &_Prog;

int programmer_Init(void) {
    for (uint32_t *port = Prog->ports; *port != 0; port++) {
        SysCtlPeripheralEnable(*port);
        while (!SysCtlPeripheralReady(*port))
            ;
    }
}

int programmer_InitParallel(void) {
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

int programmer_DisableIO(void) {
    return 0;
}

int programmer_ToggleDataIOMode(uint8_t mode) {
    if (mode == 0) {
        for (int i = 0; i < DATA_WIDTH; i++) {
            GPIOPinTypeGPIOInput(Prog->IO[i].port, Prog->IO[i].pin);
        }
    } else {
        for (int i = 0; i < DATA_WIDTH; i++) {
            GPIOPinTypeGPIOOutput(Prog->IO[i].port, Prog->IO[i].pin);
        }
    }
    return 1;
}

int programmer_SetAddress(uint8_t busWidth, uint32_t address) {
    for (int i = 0; i < ADDRESS_WIDTH; i++) {
        GPIOPinWrite(Prog->A[i].port, Prog->A[i].pin, address & 1 ? Prog->A[i].pin : 0);
        address >>= 1;
    }
}

int programmer_SetData(uint8_t value) {
    for (int i = 0; i < DATA_WIDTH; i++) {
        GPIOPinWrite(Prog->IO[i].port, Prog->IO[i].pin, (value & 1) ? Prog->IO[i].pin : 0);
        value >>= 1;
    }
}

int programmer_ToggleCE(uint8_t state) {
    GPIOPinWrite(Prog->CEn.port, Prog->CEn.pin, state == 0 ? 0 : Prog->CEn.pin); 
    return 1;
}

int programmer_ToggleOE(uint8_t state) {
    GPIOPinWrite(Prog->OEn.port, Prog->OEn.pin, state == 0 ? 0 : Prog->OEn.pin); 
    return 1;
}

int programmer_ToggleWE(uint8_t state) {
    GPIOPinWrite(Prog->WEn.port, Prog->WEn.pin, state == 0 ? 0 : Prog->WEn.pin); 
    return 1;
}

uint8_t programmer_GetData(void) {
    uint8_t data = 0;
    for (int i = 0; i < DATA_WIDTH; i++) {
        data |= (GPIOPinRead(Prog->IO[i].port, Prog->IO[i].pin) ? 1 : 0) << i;
    }
    return data;
}

int programmer_Delay100ns(uint32_t delay) {
    delay *= 8;
    SysCtlDelay(delay);
    return 1;
}

int programmer_EnableChip(void) {
    return 1;
}

int programmer_SetSpiClockFreq(uint32_t freq) {
    return 1;
}

uint32_t programmer_GetSpiClockFreq(void) {
    return 1;
}

int programmer_SetSpiMode(uint8_t mode) {
    return 1;
}

uint8_t programmer_GetSpiMode(void) {
    return 1;
}

int programmer_SpiWrite(const char *buf, size_t count) {
    return 1;
}

int programmer_SpiRead(const char *txbuf, char *rxbuf, size_t count) {
    return 1;
}

