#include <stdint.h>
#include "open-eeprom_conf.h"
#include "string.h"
#include "open-eeprom.h"
#include "programmer.h"

const uint8_t OpenEEPROM_ACK = 0x05;
const uint8_t OpenEEPROM_NAK = 0x06;
static const uint16_t Version = EPROG_VERSION_NUMBER;
static const uint8_t SupportedBusTypes = EPROG_SUPPORTED_BUS_TYPES;

static enum BusMode CurrentBusMode = BUS_MODE_NOT_SET;
static uint8_t CurrentAddressBusWidth = 0;
static uint32_t CurrentSpiFrequency = 0;
static enum SpiMode CurrentSpiMode = SPI_MODE_0; 

static uint32_t ParallelAddressHoldTime;
static uint32_t ChipEnablePulseWidthTime;

static inline int switchToParallelBusMode(void);
static inline int switchToSpiBusMode(void);

/*******************************************
********************************************
*             General Commands             *
********************************************
*******************************************/

int OpenEEPROM_nop(const char *in, char *out) {
    out[0] = OpenEEPROM_ACK;
    return sizeof(OpenEEPROM_ACK);
}

int OpenEEPROM_getInterfaceVersion(const char *in, char *out) {
    out[0] = OpenEEPROM_ACK;
    memcpy(&out[sizeof(OpenEEPROM_ACK)], &Version, sizeof(Version));
    return sizeof(OpenEEPROM_ACK) + sizeof(Version);
}

int OpenEEPROM_getSupportedBusTypes(const char *in, char *out) {
    out[0] = OpenEEPROM_ACK;
    memcpy(&out[sizeof(OpenEEPROM_ACK)], &SupportedBusTypes, sizeof(SupportedBusTypes));
    return sizeof(OpenEEPROM_ACK) + sizeof(SupportedBusTypes);
}

int OpenEEPROM_ToggleIo(const char *in, char *out) {
    uint8_t state;
    out[0] = OpenEEPROM_ACK;
    memcpy(&state, &in[sizeof(uint8_t)], sizeof(state));

    if (state == 0) {
        programmer_DisableIOPins();
        CurrentBusMode = BUS_MODE_NOT_SET;
    } else {
        programmer_Init(); 
    }

    memcpy(&out[sizeof(OpenEEPROM_ACK)], &CurrentBusMode, sizeof(CurrentBusMode));

    return sizeof(OpenEEPROM_ACK) + sizeof(state);
}

/*******************************************
********************************************
*             Parallel Commands            *
********************************************
*******************************************/

int OpenEEPROM_setAddressBusWidth(const char *in, char *out) {
    uint8_t busWidth, maxBusWidth;
    int response_len = sizeof(OpenEEPROM_ACK);

    maxBusWidth = programmer_GetAddressPinCount();
    memcpy(&busWidth, &in[sizeof(uint8_t)], sizeof(busWidth));
     
    if (busWidth <= maxBusWidth) {
        out[0] = OpenEEPROM_ACK;
        CurrentAddressBusWidth = busWidth;
        memcpy(&out[sizeof(OpenEEPROM_ACK)], &busWidth, sizeof(busWidth));
        response_len += sizeof(CurrentAddressBusWidth);
    } else {
        out[0] = OpenEEPROM_NAK;
    }

    return response_len;
}

int OpenEEPROM_setAddressHoldTime(const char *in, char *out) {
    uint32_t nsecs;
    int response_len = sizeof(OpenEEPROM_ACK);
    memcpy(&nsecs, &in[sizeof(OpenEEPROM_ACK)], sizeof(nsecs)); 

    if (nsecs > 0) {
        out[0] = OpenEEPROM_ACK;
        ParallelAddressHoldTime = nsecs;
        memcpy(&out[sizeof(OpenEEPROM_ACK)], &nsecs, sizeof(nsecs));
        response_len += sizeof(nsecs);
    } else {
        out[0] = OpenEEPROM_NAK;
    }
    
    return response_len;
}

int OpenEEPROM_setAddressPulseWidthTime(const char *in, char *out) {
    uint32_t nsecs;
    int response_len = sizeof(OpenEEPROM_ACK);
    memcpy(&nsecs, &in[sizeof(OpenEEPROM_ACK)], sizeof(nsecs)); 

    if (nsecs > 0) {
        out[0] = OpenEEPROM_ACK;
        ChipEnablePulseWidthTime = nsecs;
        memcpy(&out[sizeof(OpenEEPROM_ACK)], &nsecs, sizeof(nsecs));
        response_len += sizeof(nsecs);
    } else {
        out[0] = OpenEEPROM_NAK;
    }
    
    return response_len;
}

int OpenEEPROM_parallelRead(const char *in, char *out) {
    uint32_t address, count;
    int response_len = sizeof(OpenEEPROM_ACK);
    memcpy(&address, &in[sizeof(OpenEEPROM_ACK)], sizeof(address));  
    memcpy(&count, &in[sizeof(OpenEEPROM_ACK) + sizeof(address)], sizeof(count));  

    if (!switchToParallelBusMode()) {
        out[0] = OpenEEPROM_NAK;
    } else {
        out[0] = OpenEEPROM_ACK;
        char *databuf = &out[sizeof(OpenEEPROM_ACK)];
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

int OpenEEPROM_parallelWrite(const char *in, char *out) {
    uint32_t address, count;
    int response_len = sizeof(OpenEEPROM_ACK);
    memcpy(&address, &in[sizeof(OpenEEPROM_ACK)], sizeof(address));  
    memcpy(&count, &in[sizeof(OpenEEPROM_ACK) + sizeof(address)], sizeof(count));  

    if (!switchToParallelBusMode()) {
        out[0] = OpenEEPROM_NAK;        
    } else {
        out[0] = OpenEEPROM_ACK;
        const char *databuf = &in[sizeof(OpenEEPROM_ACK) + sizeof(address) + sizeof(count)];
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

int OpenEEPROM_setSpifrequency(const char *in, char *out) {
    uint32_t freq;
    int response_len = sizeof(OpenEEPROM_ACK);
    memcpy(&freq, &in[sizeof(OpenEEPROM_ACK)], sizeof(freq));

    if (freq != CurrentSpiFrequency) {
        if (programmer_SetSpiClockFreq(freq)) {
            out[0] = OpenEEPROM_ACK;
            CurrentSpiFrequency = freq;
            memcpy(&out[sizeof(OpenEEPROM_ACK)], &freq, sizeof(freq));
            response_len += sizeof(freq);
        } else {
            out[0] = OpenEEPROM_NAK;
        }
    }
    
    return response_len;
}

int OpenEEPROM_setSpiMode(const char *in, char *out) {
    uint8_t mode;
    int response_len = sizeof(OpenEEPROM_ACK);
    memcpy(&mode, &in[sizeof(OpenEEPROM_ACK)], sizeof(mode));

    if (mode != CurrentSpiMode) {
        if (programmer_SetSpiMode(mode)) {
            out[0] = OpenEEPROM_ACK;
            CurrentSpiMode = mode;
            memcpy(&out[sizeof(OpenEEPROM_ACK)], &mode, sizeof(mode));
            response_len += sizeof(mode);
        } else {
            out[0] = OpenEEPROM_NAK;
        }
    }

    return response_len;
}

int OpenEEPROM_getSupportedSpiModes(const char *in, char *out) {
    out[0] = OpenEEPROM_ACK;
    uint8_t supportedSpiModes = programmer_GetSupportedSpiModes();
    memcpy(&out[sizeof(OpenEEPROM_ACK)], &supportedSpiModes, sizeof(supportedSpiModes));
    return sizeof(OpenEEPROM_ACK) + sizeof(supportedSpiModes);
}

int OpenEEPROM_SpiTransmit(const char *in, char *out) {
    uint32_t count;
    int response_len = sizeof(OpenEEPROM_ACK);
    memcpy(&count, &in[sizeof(OpenEEPROM_ACK)], sizeof(count));  

    if (!switchToSpiBusMode()) {
        out[0] = OpenEEPROM_NAK; 
    } else {
        if (programmer_SpiTransmit(&in[sizeof(uint8_t) + sizeof(count)], &out[sizeof(OpenEEPROM_ACK)], count)) {
            out[0] = OpenEEPROM_ACK;
            response_len += count;
        } else {
            out[0] = OpenEEPROM_NAK; 
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

