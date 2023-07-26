# Open EEPROM Server 
This repo contains a reference implementation of the Open EEPROM protocol. The code 
can be ported to new platforms by implementing the interfaces defined in `programmer.h` and `transport.h`.
These files are located in `inc/open-eeprom`.  

A client communicates with a device running an Open EEPROM server using a byte-stream oriented protocol 
(see the specification for a list of commands). Serial and TCP-based transport work most naturally with the protocol,
although there is nothing stopping you from implementing a custom transport layer, as long as from the server's 
perspective it is reading from a byte stream.

## Included Make Recipes
1. `all`: build both an ELF and a flat binary.

2. `clean`: delete build artifacts.

3. `flash`: flash the target with the binary.

## Debugging
The repo includes a script `debug.sh` for debugging the target using `arm-none-eabi-gdb`. 

## Dependencies

[OpenOCD](https://github.com/openocd-org/openocd.git) is necessary for debugging the remote target,
 and [lm4flash](https://github.com/utzig/lm4tools.git) is used to flash it. Both the debug script 
and the makefile are coupled with the `arm-none-eabi` toolchain.

