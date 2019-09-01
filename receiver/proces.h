#ifndef PROCES_H
#define PROCES_H

#include <stdlib.h>
#include <stdint.h>
#include "./uartlibrary/uart.h"
#include "main.h" // for the shared variables


void updateDevice(uint16_t ID, uint8_t MSG); // find device in array and update it, and if it's not there, add it.


#endif
