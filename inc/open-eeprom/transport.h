/**
 * @file
 *
 * This file specifies the interface for the functions needed
 * by the OpenEEPROM server to send and receive data. 
 * Generally, these functions will be implemented for a specific 
 * platform similarly to the functions declared in `programmer.h`
 * New programmer devices must implement all of the functions in this header file.
 */

#ifndef __TRANSPORT_H__
#define __TRANSPORT_H__

#include <stddef.h>

/**
 * @brief Initialize the transport interface.
 *
 * Any transport specific initialization, such as enabling USB or UART 
 * peripherals or some kind of TCP server, should be done here.
 */
int Transport_init(void);

/**
 * @brief Read count bytes from the transport interface.
 *
 * This function may be blocking.
 *
 * @param in buffer for storing read bytes
 * 
 * @param count number of bytes to read
 *
 * @return number of bytes read
 */ 
int Transport_getData(char *in, size_t count); 

/**
 * @brief Write count bytes to the transport interface.
 *
 * @param out buffer of data to send
 *
 * @param count number of bytes to send
 */ 
int Transport_putData(const char *out, size_t count); 

/**
 * @brief Flush all data out of the transport.
 *
 * The meaning of this function will vary by the transport type and 
 * the implementation. For example, if the transport is a UART, 
 * flushing would entail clearing out any data from the TX and 
 * RX FIFOs, if present.
 */
int Transport_flush(void);

/**
 * @brief Indicate if there is data waiting to be read from the transport.
 *
 * @return 1 if data is waiting, else 0
 */
int Transport_dataWaiting(void);

#endif /* __TRANSPORT_H__ */

