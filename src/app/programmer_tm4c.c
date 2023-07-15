
#include <stdint.h>

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

int programmer_InitParallel(Programmer *prog);
int programmer_InitSpi(Programmer *prog);
