#ifndef SHOW_H
#define SHOW_H


/* If defined, also records new devices if their messages are garbled or OFF, 
 * otherwise only record new devices that are ON, so OFF messages only get accepted from devices that where ON previously and garbled messages get ignored 
 * also shows these in serial output
 */
//#define LOG_ALL 

#define reenableuart() uart_init(UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU)) /* because LCD and UART are on the same port*/

#include <avr/eeprom.h>
#include <stdlib.h>
#include <stdint.h>
#include "lcd.h"
#include "./uartlibrary/uart.h"
#include "main.h" // for shared variables

void readdevnames(void); // read devnames from EEPROM (on bootup)
void storedevnames(void); // store devnames in EEPROM (on modification) (NOTE: EEPROM on ATMEGA328p is 1024 bytes) 
// uses EEPROM update, so when data is the same, nothing gets rewritten (saves wear);

char* IDtoName(uint16_t ID); // DNS
void readnewname(DNS* names);// serial input of new device names for each ID? setname 0x8234 slartibartfast 

// Display number of devices still on, and names of 3 devices (Because more won't fit on LCD without scrolling/refresh, showing 3 keeps it simpler)
void LCD_refresh(void);

//sent over serial which devices are ON,OFF,PRESUMED_OFF etc.
void SerialRefresh(void);

#endif
