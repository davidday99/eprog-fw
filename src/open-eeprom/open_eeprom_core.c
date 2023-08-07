/** 
 * @file
 * 
 * This file contains the implementations 
 * for the majority of the OpenEEPROM 
 * protocol commands. The only ones not 
 * included here are those related to 
 * transport properties like buffer size.
 * Those remaining commands are implemented in
 * `open_eeprom_server.c`.
 *
 * All of these functions operate on 
 * input and output buffers and return the length
 * in bytes of the response in the output buffer.
 *
 * The first byte of the input buffer will always be
 * the command byte encoding, and the first byte of the output 
 * should always be the response status byte (ACK or NAK).
 * Additional inputs and outputs are specified in the 
 * details for each function.
 */

#include <stdint.h>
#include "open-eeprom_conf.h"
#include "string.h"
#include "open-eeprom.h"
#include "programmer.h"

/**
 * @brief Command acknowledged, success.
 */
const uint8_t OpenEEPROM_ACK = 0x05;

/**
 * @brief Command not acknowledged, failure.
 */
const uint8_t OpenEEPROM_NAK = 0x06;

static const uint16_t Version = OPEN_EEPROM_VERSION_NUMBER;
static const uint8_t SupportedBusTypes = OPEN_EEPROM_SUPPORTED_BUS_TYPES;

static enum OpenEEPROM_BusMode CurrentBusMode = OPEN_EEPROM_BUS_MODE_NOT_SET;
static uint8_t CurrentAddressBusWidth = 0;
static uint32_t CurrentSpiFrequency = 0;
static enum OpenEEPROM_SpiMode CurrentSpiMode = OPEN_EEPROM_SPI_MODE_0; 

static uint32_t ParallelAddressHoldTime;
static uint32_t ChipEnablePulseWidthTime;

static inline int switchToParallelBusMode(void);
static inline int switchToSpiBusMode(void);

/*******************************************
********************************************
*             General Commands             *
********************************************
*******************************************/

/**
 * @brief Do nothing.
 *
 * @return 1
 */
int OpenEEPROM_nop(const char *in, char *out) {
    out[0] = OpenEEPROM_ACK;
    return sizeof(OpenEEPROM_ACK);
}

/**
 * @brief Return the interface version.
 *
 * @param out 16-bit version number
 * 
 * @return 3
 */
int OpenEEPROM_getInterfaceVersion(const char *in, char *out) {
    out[0] = OpenEEPROM_ACK;
    memcpy(&out[sizeof(OpenEEPROM_ACK)], &Version, sizeof(Version));
    return sizeof(OpenEEPROM_ACK) + sizeof(Version);
}

/**
 * @brief Return bus types the programmer supports.
 *
 * The value returned is a mask where each set bit  
 * correponds to a supported bus type. The values 
 * correspond to @ref OpenEEPROM_BusMode. 
 *
 * @param out 8-bit mask of supported bus types
 *
 * @return 2
 */
int OpenEEPROM_getSupportedBusTypes(const char *in, char *out) {
    out[0] = OpenEEPROM_ACK;
    memcpy(&out[sizeof(OpenEEPROM_ACK)], &SupportedBusTypes, sizeof(SupportedBusTypes));
    return sizeof(OpenEEPROM_ACK) + sizeof(SupportedBusTypes);
}

/**
 * @brief Enable or disable all programmer IO lines.
 *
 * Passing an argument of 0 in the input buffer will
 * disable I0; any other value will enable IO.
 *
 * Disabled IO implies all IO lines are in a high
 * impedance state. This allows the option for a
 * device connected to the programmer to be 
 * driven by an another source.
 *
 * @param in 8-bit state (0 disabled, else enabled)
 *
 * @param out 8-bit set state 
 *
 * @return 2
 */
int OpenEEPROM_toggleIO(const char *in, char *out) {
    uint8_t state;
    out[0] = OpenEEPROM_ACK;
    memcpy(&state, &in[sizeof(uint8_t)], sizeof(state));

    if (state == 0) {
        Programmer_disableIOPins();
        CurrentBusMode = OPEN_EEPROM_BUS_MODE_NOT_SET;
    } else {
        Programmer_init(); 
    }

    memcpy(&out[sizeof(OpenEEPROM_ACK)], &CurrentBusMode, sizeof(CurrentBusMode));

    return sizeof(OpenEEPROM_ACK) + sizeof(state);
}

/*******************************************
********************************************
*             Parallel Commands            *
********************************************
*******************************************/

/**
 * @brief Set the width of the Address Bus.
 *
 * The programmer needs this information to 
 * set the corrct number of address lines.
 *
 * @param in 8-bit address bus width
 *
 * @param out ACK and 8-bit set width or
 *      NAK and max width supported 
 *      by the programmer
 *
 * @return 2 
 */
int OpenEEPROM_setAddressBusWidth(const char *in, char *out) {
    uint8_t busWidth, maxBusWidth;
    int response_len = sizeof(OpenEEPROM_ACK);

    maxBusWidth = Programmer_getAddressPinCount();
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

/**
 * @brief Set the number of nanoseconds
 * to wait after setting the address lines.
 *
 * Parallel memory chips require a minimum
 * time after setting the address lines 
 * before the data lines can be reliably 
 * read from or written to the desired address.
 *
 * @param in 32-bit wait time in nanoseconds
 *
 * @param out ACK and 32-bit set wait time or
 *      NAK and minimum time supported by 
 *      programmer
 *
 * @return 5
 *
 */
int OpenEEPROM_setAddressHoldTime(const char *in, char *out) {
    uint32_t nsecs;
    int response_len = sizeof(OpenEEPROM_ACK);
    memcpy(&nsecs, &in[sizeof(OpenEEPROM_ACK)], sizeof(nsecs)); 

    // TODO: check that nsecs is greater than minimum supported by programmer
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

/**
 * @brief Set the nanosecond pulse width time.
 *
 * Parallel memory chips are enabled when
 * reading or writing and disabled after 
 * a minimum elapsed time to update the 
 * address and data lines; otherwise 
 * reads and writes would be unreliable.
 *
 * @param in 32-bit pulse width time in nanoseconds
 * @param out ACK and 32-bit set pulse width time or
 *      NAK and minimum time supported by 
 *      programmer
 *
 * @return 5
 *
 */
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

/**
 * @brief Read n bytes from a connected parallel chip.
 *
 * @param in 32-bit address followed by 32-bit read count
 * @param out ACK followed by n bytes if successful or NAK if 
 *      set address hold time is less than minimum supported
 *      by the programmer
 *
 * @return 1 + n (n is read count from input or 0)
 */
int OpenEEPROM_parallelRead(const char *in, char *out) {
    uint32_t address, count;
    int response_len = sizeof(OpenEEPROM_ACK);
    memcpy(&address, &in[sizeof(OpenEEPROM_ACK)], sizeof(address));  
    memcpy(&count, &in[sizeof(OpenEEPROM_ACK) + sizeof(address)], sizeof(count));  

    if (ParallelAddressHoldTime < Programmer_MinimumDelay || !switchToParallelBusMode()) {
        out[0] = OpenEEPROM_NAK;
    } else {
        out[0] = OpenEEPROM_ACK;
        char *databuf = &out[sizeof(OpenEEPROM_ACK)];
        Programmer_toggleDataIOMode(0);
        Programmer_toggleOE(0);
        Programmer_toggleCE(0);
        for (size_t i = 0; i < count; i++) {
            Programmer_setAddress(CurrentAddressBusWidth, address + i);
            Programmer_delay1ns(ParallelAddressHoldTime);
            databuf[i] = Programmer_getData();
        } 
        Programmer_toggleCE(1);
        Programmer_toggleOE(1);
        response_len += count;
    }

    return response_len;
}

/**
 * @brief Write n bytes to a connected parallel chip.
 *
 * @param in 32-bit address followed by 32-bit read count
 *      followed by n bytes
 *
 * @param out ACK if successful or NAK if either set address hold time 
 *      or set pulse width time are less than minimum supported
 *      by the programmer
 *
 * @return 1 
 */
int OpenEEPROM_parallelWrite(const char *in, char *out) {
    uint32_t address, count;
    int response_len = sizeof(OpenEEPROM_ACK);
    memcpy(&address, &in[sizeof(OpenEEPROM_ACK)], sizeof(address));  
    memcpy(&count, &in[sizeof(OpenEEPROM_ACK) + sizeof(address)], sizeof(count));  

    if (ParallelAddressHoldTime < Programmer_MinimumDelay || 
            ChipEnablePulseWidthTime < Programmer_MinimumDelay ||
            !switchToParallelBusMode()) {
        out[0] = OpenEEPROM_NAK;        
    } else {
        out[0] = OpenEEPROM_ACK;
        const char *databuf = &in[sizeof(OpenEEPROM_ACK) + sizeof(address) + sizeof(count)];
        Programmer_toggleDataIOMode(1);
        Programmer_toggleOE(1);
        Programmer_toggleWE(0);
        for (size_t i = 0; i < count; i++) {
            Programmer_setAddress(CurrentAddressBusWidth, address + i);
            Programmer_setData(databuf[i]);
            Programmer_delay1ns(ParallelAddressHoldTime);
            Programmer_toggleCE(0);
            Programmer_delay1ns(ChipEnablePulseWidthTime);
            Programmer_toggleCE(1);
        }
        Programmer_toggleWE(1);
        Programmer_toggleDataIOMode(0);
    }

    return response_len;
}


/*******************************************
********************************************
*             SPI Commands                 *
********************************************
*******************************************/

/**
 * @brief Set the SPI clock frequency.
 *
 * @param in 32-bit frequency in Hz
 *
 * @param out ACK and 32-bit set frequency
 *      or NAK and maximum supported frequency 
 *
 * @return 5
 */ 
int OpenEEPROM_setSpiFrequency(const char *in, char *out) {
    uint32_t freq;
    int response_len = sizeof(OpenEEPROM_ACK);
    memcpy(&freq, &in[sizeof(OpenEEPROM_ACK)], sizeof(freq));



    // TODO: return maximum supported frequency if input frequency is invalid
    if (switchToSpiBusMode() && Programmer_setSpiClockFreq(freq)) {
        out[0] = OpenEEPROM_ACK;
        CurrentSpiFrequency = freq;
        memcpy(&out[sizeof(OpenEEPROM_ACK)], &freq, sizeof(freq));
        response_len += sizeof(freq);
    } else {
        out[0] = OpenEEPROM_NAK;
    }
    
    return response_len;
}

/**
 * @brief Set the SPI mode.
 *
 * SPI can be configured for 4 different modes, 
 * depending on whether the clock is active high or low
 * and whether data is sampled on the rising edge and 
 * shifted out on the falling edge or vice-versa.
 *
 * TODO: confirm these modes are correct.
 * The modes are:
 *
 * - Mode 0: Active high clock, regular clock polarity.
 * - Mode 1: Active low clock, regular clock polarity.
 * - Mode 2: Active high clock, inverted clock polarity.
 * - Mode 3: Active low clock, inverted clock polarity.
 *
 * Most devices use mode 0; the programmer may 
 * not support all modes.
 *
 * @param in 8-bit mode
 *
 * @param out ACK and 8-bit set mode or NAK
 *      and current mode if input is invalid
 *
 * @return 2
 *
 */
int OpenEEPROM_setSpiMode(const char *in, char *out) {
    uint8_t mode;
    int response_len = sizeof(OpenEEPROM_ACK);
    memcpy(&mode, &in[sizeof(OpenEEPROM_ACK)], sizeof(mode));

    // TODO: return current mode if input is invalid
    if (switchToSpiBusMode() && Programmer_setSpiMode(mode)) {
        out[0] = OpenEEPROM_ACK;
        CurrentSpiMode = mode;
        memcpy(&out[sizeof(OpenEEPROM_ACK)], &mode, sizeof(mode));
        response_len += sizeof(mode);
    } else {
        out[0] = OpenEEPROM_NAK;
    }

    return response_len;
}

/**
 * @brief Return the SPI modes the programmer supports.
 *
 * The value returned is a mask where each set bit  
 * correponds to a supported SPI mode. The values 
 * correspond to @ref OpenEEPROM_SpiMode. 
 *
 * @param out 8-bit mask of supported SPI modes 
 *
 * @return 2
 *
 */
int OpenEEPROM_getSupportedSpiModes(const char *in, char *out) {
    out[0] = OpenEEPROM_ACK;
    uint8_t supportedSpiModes = Programmer_getSupportedSpiModes();
    memcpy(&out[sizeof(OpenEEPROM_ACK)], &supportedSpiModes, sizeof(supportedSpiModes));
    return sizeof(OpenEEPROM_ACK) + sizeof(supportedSpiModes);
}

/**
 * @brief Transmit and return n bytes over SPI.
 *
 * Transmit an array of bytes. Given the nature of SPI
 * the same number of bytes transmitted will also
 * be read back and returned.
 *
 * @param in 32-bit count of bytes to transmit 
 *      followed by n bytes
 *
 * @param out ACK followed by n bytes of data 
 *      or NAK if SPI mode isn't supported
 *
 * @return 1 + n (transmit count from input or 0)
 */
int OpenEEPROM_spiTransmit(const char *in, char *out) {
    uint32_t count;
    int response_len = sizeof(OpenEEPROM_ACK);
    memcpy(&count, &in[sizeof(OpenEEPROM_ACK)], sizeof(count));  

    if (!switchToSpiBusMode()) {
        out[0] = OpenEEPROM_NAK; 
    } else {
        if (Programmer_spiTransmit(&in[sizeof(uint8_t) + sizeof(count)], &out[sizeof(OpenEEPROM_ACK)], count)) {
            out[0] = OpenEEPROM_ACK;
            response_len += count;
        } else {
            out[0] = OpenEEPROM_NAK; 
        }
    }

    return response_len;
}

static inline int switchToParallelBusMode(void) {
    if ((CurrentBusMode != OPEN_EEPROM_BUS_MODE_PARALLEL) && 
            (OPEN_EEPROM_BUS_MODE_PARALLEL & SupportedBusTypes)) {
        Programmer_initParallel();
        return 1;
    } else {
        return 0;
    }
}

static inline int switchToSpiBusMode(void) {
    if ((CurrentBusMode != OPEN_EEPROM_BUS_MODE_SPI) && 
            (OPEN_EEPROM_BUS_MODE_SPI & SupportedBusTypes)) {
        Programmer_initSpi();
        return 1;
    } else {
        return 0;
    }
}

