#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "driverlib/ssi.h"
#include "driverlib/hw_memmap.h"

int spi_transmit(const char *txbuf, char *rxbuf, size_t count) {
    for (size_t i = 0; i < count; i++) {
        SSIDataPut(SSI0_BASE, txbuf[i]);
        while (SSIBusy(SSI0_BASE))
            ;
        SSIDataGetNonBlocking(SSI0_BASE, &rxbuf[i]);
    }
}


