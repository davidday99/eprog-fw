/**
 * @file
 *
 * This file specifies the interface for the functions needed
 * by the OpenEEPROM server to control the programmer. New programmer
 * devices must implement all of the functions in this header file.
 */

#ifndef __PROGRAMMER_H___
#define __PROGRAMMER_H___

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Minimum delay, in nanoseconds, supported by the programmer.
 *
 * This will depend on the speed of the MCU.
 */
// TODO: make const
extern uint32_t Programmer_MinimumDelay;

/**
 * @brief Initialize the programmer.
 *
 * This function should handle all necessary initializations such as 
 * setting the system clock frequency and enabling peripherals.
 */
int Programmer_init(void);

/**
 * @brief Initialize the GPIO pins 
 *      that serve parallel functions.
 *
 * This includes all GPIO lines used for the address and data buses
 * and control lines CE, OE, and WE. After this function runs,
 * all parallel read and write functionality shoud be enabled.
 *
 * This function may be called on multiple occasions aside from startup, 
 * such as whenever the programmer IO is reenabled after being disabled.
 * So it is necessary that subsequent calls be safe.
 */
int Programmer_initParallel(void);

/**
 * @brief Initialize the SPI peripheral
 *      and related GPIO pins.
 *
 * After this function runs, all SPI transmission functionality 
 * should be enabled.
 */
int Programmer_initSpi(void);

/**
 * @brief Disable all connected IO pins.
 *
 * All IO lines used by the programmer, 
 * including those for the parallel address/data/control lines
 * and the SPI peripheral should be put into a high impedance state.
 */
int Programmer_disableIOPins(void);

/**
 * @brief Toggle whether the parallel data lines
 *      are configured as inputs or outputs.
 *
 * This allows the programmer to drive the data lines for writing to 
 * an attached memory IC and for the memory IC to drive the 
 * lines for reading from it.
 *
 * @param mode 0 to configure data lines as inputs,
 *      otherwise configure data lines as outputs
 */
int Programmer_toggleDataIOMode(uint8_t mode);

/**
 * @brief Get the number of configured address pins 
 *      supported by the programmer.
 *
 * @return total width of the address bus.
 */
int Programmer_getAddressPinCount(void);

/**
 * @brief Set the value outputted onto the address bus.
 *
 * Set the address bus to the value `address` using 
 * no more than busWidth bits starting with the LSB.
 *
 * @param busWidth max bits to use from `address`
 *
 * @param address value to output on the bus corresponding to [An:A0], 
 *      where An is the MSB of the address bus.
 */
int Programmer_setAddress(uint8_t busWidth, uint32_t address);

/**
 * @brief Set the value outputted on the data bus.
 *
 * Currently only a fixed 8-bit data bus is supported.
 *
 * @param data value to output on the bus
 */
int Programmer_setData(uint8_t data);

/**
 * @brief Read the values on the data bus.
 * 
 * This function assumes that the data lines are already configured 
 * as inputs. It should simply read the values currently being inputted
 * into each of the data lines.
 *
 * @return the value being inputted into the data lines, [D7:D0]
 */
uint8_t Programmer_getData(void);

/**
 * @brief Toggle the IO line that serves as the CE control line.
 *
 * @param state 0 set the line low, else set the line high
 */
int Programmer_toggleCE(uint8_t state);

/**
 * @brief Toggle the IO line that serves as the OE control line.
 *
 * @param state 0 set the line low, else set the line high
 */
int Programmer_toggleOE(uint8_t state);

/**
 * @brief Toggle the IO line that serves as the WE control line.
 *
 * @param state 0 set the line low, else set the line high
 */
int Programmer_toggleWE(uint8_t state);

/**
 * @brief Wait for `delay` nanoseconds.
 *
 * Many MCUs cannot run fast enough to support 1ns resolution.
 * In this case, the function should round up to the lowest number
 * that is greater than `delay`.
 *
 * @param delay number of nanoseconds to delay
 */
int Programmer_delay1ns(uint32_t delay);

/**
 * @brief Set the clock frequency of the SPI peripheral. 
 *
 * @param freq desired frequency
 *
 * @return 1 if frequency is set, or 0 if desired frequency is not supported
 */
int Programmer_setSpiClockFreq(uint32_t freq);

/**
 * @brief Get the frequeny of the SPI clock.
 *
 * @return frequency in Hz
 */
uint32_t Programmer_getSpiClockFreq(void);

/**
 * @brief Set the mode of the SPI peripheral.
 * 
 * The programmer is only required to support mode 0. 
 *
 * @param mode desired SPI mode
 *
 * @return 1 if SPI mode is set, or 0 if desired mode is not supported
 */
int Programmer_setSpiMode(uint8_t mode);

/**
 * @brief Get the SPI modes the programmer supports.
 * 
 * The value returned is a mask where each set bit  
 * correponds to a supported SPI mode. The values 
 * correspond to @ref SpiMode. 
 *
 * @return 8-bit mask of supported SPI modes
 *
 */ 
uint8_t Programmer_getSupportedSpiModes(void);

/**
 * @brief Transmit count bytes over SPI and return the received bytes.
 *
 * SPI uses a "push-pull" configuration where each byte transmitted
 * results in a byte received. This function will transmit `count` number
 * of bytes from `txbuf`, and for each byte transmitted it will read the 
 * received byte and write it to `rxbuf`.
 *
 * Both txbuf and rxbuf should be at least the size of `count`.
 *
 * @param txbuf buffer of bytes to transmit
 *
 * @param rxbuf buffer for storing received bytes
 *
 * @param count number of bytes to transmit
 */
int Programmer_spiTransmit(const char *txbuf, char *rxbuf, size_t count);

#endif /* __PROGRAMMER_H__ */

