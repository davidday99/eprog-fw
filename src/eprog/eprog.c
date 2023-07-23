#include <stdint.h>
#include "eprog_conf.h"
#include "string.h"
#include "eprog.h"
#include "programmer.h"

const uint8_t eprog_ACK = 0x05;
const uint8_t eprog_NAK = 0x06;
static const uint16_t Version = EPROG_VERSION_NUMBER;
static const uint8_t SupportedBusTypes = EPROG_SUPPORTED_BUS_TYPES;
static const char *RxBuf;
static char *TxBuf;
static size_t RxBufSize;
static size_t TxBufSize;

static enum BusMode CurrentBusMode = BUS_MODE_NOT_SET;
static uint8_t CurrentAddressBusWidth = 0;
static uint32_t CurrentSpiFrequency = 0;
static enum SpiMode CurrentSpiMode = SPI_MODE_0; 

static uint32_t ParallelAddressHoldTime;
static uint32_t ChipEnablePulseWidthTime;

static inline int switchToParallelBusMode(void);
static inline int switchToSpiBusMode(void);

int (*Commands[])(const char *in, char *out) = {
    eprog_nop,
    eprog_getInterfaceVersion,
    eprog_getMaxRxSize,
    eprog_getMaxTxSize,
    eprog_ToggleIo,
    eprog_getSupportedBusTypes,
    eprog_setAddressBusWidth,
    eprog_setAddressHoldTime,
    eprog_setAddressPulseWidthTime,
    eprog_parallelRead,
    eprog_parallelWrite,
    eprog_setSpifrequency,
    eprog_setSpiMode,
    eprog_getSupportedSpiModes,
    eprog_SpiTransmit,
};

/*******************************************
********************************************
*             General Commands             *
********************************************
*******************************************/

size_t eprog_RunCommand(void) {
    enum eprog_Command cmd;
    size_t response_len = 0;
    memcpy(&cmd, RxBuf, sizeof(cmd));

    int (*func)(const char *in, char *out) = Commands[(uint8_t) cmd];

    response_len = func(RxBuf, TxBuf);

    return response_len;
}

int eprog_Init(char *rxbuf, size_t maxRxSize, char *txbuf, size_t maxTxSize) {
    RxBuf = rxbuf;
    TxBuf = txbuf;
    RxBufSize = maxRxSize;
    TxBufSize = maxTxSize;
    programmer_Init();
    return 1;
}

int eprog_nop(const char *in, char *out) {
    out[0] = eprog_ACK;
    return sizeof(eprog_ACK);
}

int eprog_getInterfaceVersion(const char *in, char *out) {
    out[0] = eprog_ACK;
    memcpy(&out[sizeof(eprog_ACK)], &Version, sizeof(Version));
    return sizeof(eprog_ACK) + sizeof(Version);
}

int eprog_getSupportedBusTypes(const char *in, char *out) {
    out[0] = eprog_ACK;
    memcpy(&out[sizeof(eprog_ACK)], &SupportedBusTypes, sizeof(SupportedBusTypes));
    return sizeof(eprog_ACK) + sizeof(SupportedBusTypes);
}

int eprog_getMaxRxSize(const char *in, char *out) {
    out[0] = eprog_ACK;
    memcpy(&out[sizeof(eprog_ACK)], &RxBufSize, sizeof(RxBufSize));
    return sizeof(eprog_ACK) + sizeof(RxBufSize);
}

int eprog_getMaxTxSize(const char *in, char *out) {
    out[0] = eprog_ACK;
    memcpy(&out[sizeof(eprog_ACK)], &TxBufSize, sizeof(TxBufSize));
    return sizeof(eprog_ACK) + sizeof(TxBufSize);
}

int eprog_ToggleIo(const char *in, char *out) {
    uint8_t state;
    out[0] = eprog_ACK;
    memcpy(&state, &in[sizeof(uint8_t)], sizeof(state));

    if (state == 0) {
        programmer_DisableIOPins();
        CurrentBusMode = BUS_MODE_NOT_SET;
    } else {
        programmer_Init(); 
    }

    memcpy(&out[sizeof(eprog_ACK)], &CurrentBusMode, sizeof(CurrentBusMode));

    return sizeof(eprog_ACK) + sizeof(state);
}

/*******************************************
********************************************
*             Parallel Commands            *
********************************************
*******************************************/

int eprog_setAddressBusWidth(const char *in, char *out) {
    uint8_t busWidth, maxBusWidth;
    int response_len = sizeof(eprog_ACK);

    maxBusWidth = programmer_GetAddressPinCount();
    memcpy(&busWidth, &in[sizeof(uint8_t)], sizeof(busWidth));
     
    if (busWidth <= maxBusWidth) {
        out[0] = eprog_ACK;
        CurrentAddressBusWidth = busWidth;
        memcpy(&out[sizeof(eprog_ACK)], &busWidth, sizeof(busWidth));
        response_len += sizeof(CurrentAddressBusWidth);
    } else {
        out[0] = eprog_NAK;
    }

    return response_len;
}

int eprog_setAddressHoldTime(const char *in, char *out) {
    uint32_t nsecs;
    int response_len = sizeof(eprog_ACK);
    memcpy(&nsecs, &in[sizeof(eprog_ACK)], sizeof(nsecs)); 

    if (nsecs > 0) {
        out[0] = eprog_ACK;
        ParallelAddressHoldTime = nsecs;
        memcpy(&out[sizeof(eprog_ACK)], &nsecs, sizeof(nsecs));
        response_len += sizeof(nsecs);
    } else {
        out[0] = eprog_NAK;
    }
    
    return response_len;
}

int eprog_setAddressPulseWidthTime(const char *in, char *out) {
    uint32_t nsecs;
    int response_len = sizeof(eprog_ACK);
    memcpy(&nsecs, &in[sizeof(eprog_ACK)], sizeof(nsecs)); 

    if (nsecs > 0) {
        out[0] = eprog_ACK;
        ChipEnablePulseWidthTime = nsecs;
        memcpy(&out[sizeof(eprog_ACK)], &nsecs, sizeof(nsecs));
        response_len += sizeof(nsecs);
    } else {
        out[0] = eprog_NAK;
    }
    
    return response_len;
}

int eprog_parallelRead(const char *in, char *out) {
    uint32_t address, count;
    int response_len = sizeof(eprog_ACK);
    memcpy(&address, &in[sizeof(eprog_ACK)], sizeof(address));  
    memcpy(&count, &in[sizeof(eprog_ACK) + sizeof(address)], sizeof(count));  

    if (count + sizeof(eprog_ACK) > TxBufSize || !switchToParallelBusMode()) {
        out[0] = eprog_NAK;
    } else {
        out[0] = eprog_ACK;
        char *databuf = &out[sizeof(eprog_ACK)];
        programmer_ToggleDataIOMode(0);
        programmer_ToggleOE(0);
        programmer_ToggleCE(0);
        for (size_t i = 0; i < count; i++) {
            programmer_SetAddress(CurrentAddressBusWidth, address + i);
            programmer_Delay100ns(ParallelAddressHoldTime);
            databuf[i] = programmer_GetData();
        } 
        programmer_ToggleCE(1);
        programmer_ToggleOE(1);
        response_len += count;
    }

    return response_len;
}

int eprog_parallelWrite(const char *in, char *out) {
    uint32_t address, count;
    int response_len = sizeof(eprog_ACK);
    memcpy(&address, &in[sizeof(eprog_ACK)], sizeof(address));  
    memcpy(&count, &in[sizeof(eprog_ACK) + sizeof(address)], sizeof(count));  

    if (count + sizeof(eprog_ACK) > TxBufSize || !switchToParallelBusMode()) {
        out[0] = eprog_NAK;        
    } else {
        out[0] = eprog_ACK;
        const char *databuf = &in[sizeof(eprog_ACK) + sizeof(address) + sizeof(count)];
        programmer_ToggleDataIOMode(1);
        programmer_ToggleOE(1);
        programmer_ToggleWE(0);
        for (size_t i = 0; i < count; i++) {
            programmer_SetAddress(CurrentAddressBusWidth, address + i);
            programmer_SetData(databuf[i]);
            programmer_Delay100ns(ParallelAddressHoldTime);
            programmer_ToggleCE(0);
            programmer_Delay100ns(ChipEnablePulseWidthTime);
            programmer_ToggleCE(1);
        }
        programmer_ToggleWE(1);
        programmer_ToggleDataIOMode(0);
    }

    return response_len;
}


/*******************************************
********************************************
*             SPI Commands                 *
********************************************
*******************************************/

int eprog_setSpifrequency(const char *in, char *out) {
    uint32_t freq;
    int response_len = sizeof(eprog_ACK);
    memcpy(&freq, &in[sizeof(eprog_ACK)], sizeof(freq));

    if (freq != CurrentSpiFrequency) {
        if (programmer_SetSpiClockFreq(freq)) {
            out[0] = eprog_ACK;
            CurrentSpiFrequency = freq;
            memcpy(&out[sizeof(eprog_ACK)], &freq, sizeof(freq));
            response_len += sizeof(freq);
        } else {
            out[0] = eprog_NAK;
        }
    }
    
    return response_len;
}

int eprog_setSpiMode(const char *in, char *out) {
    uint8_t mode;
    int response_len = sizeof(eprog_ACK);
    memcpy(&mode, &in[sizeof(eprog_ACK)], sizeof(mode));

    if (mode != CurrentSpiMode) {
        if (programmer_SetSpiMode(mode)) {
            out[0] = eprog_ACK;
            CurrentSpiMode = mode;
            memcpy(&out[sizeof(eprog_ACK)], &mode, sizeof(mode));
            response_len += sizeof(mode);
        } else {
            out[0] = eprog_NAK;
        }
    }

    return response_len;
}

int eprog_getSupportedSpiModes(const char *in, char *out) {
    out[0] = eprog_ACK;
    uint8_t supportedSpiModes = programmer_GetSupportedSpiModes();
    memcpy(&out[sizeof(eprog_ACK)], &supportedSpiModes, sizeof(supportedSpiModes));
    return sizeof(eprog_ACK) + sizeof(supportedSpiModes);
}

int eprog_SpiTransmit(const char *in, char *out) {
    uint32_t count;
    int response_len = sizeof(eprog_ACK);
    memcpy(&count, &in[sizeof(eprog_ACK)], sizeof(count));  

    if (count + sizeof(eprog_ACK) > TxBufSize || !switchToSpiBusMode()) {
        out[0] = eprog_NAK; 
    } else {
        if (programmer_SpiTransmit(&in[sizeof(uint8_t) + sizeof(count)], &out[sizeof(eprog_ACK)], count)) {
            out[0] = eprog_ACK;
            response_len += count;
        } else {
            out[0] = eprog_NAK; 
        }
    }

    return response_len;
}

static inline int switchToParallelBusMode(void) {
    if ((CurrentBusMode != BUS_MODE_PARALLEL) && (BUS_MODE_PARALLEL & SupportedBusTypes)) {
        programmer_InitParallel();
        return 1;
    } else {
        return 0;
    }
}

static inline int switchToSpiBusMode(void) {
    if ((CurrentBusMode != BUS_MODE_SPI) && (BUS_MODE_SPI & SupportedBusTypes)) {
        programmer_InitSpi();
        return 1;
    } else {
        return 0;
    }
}

