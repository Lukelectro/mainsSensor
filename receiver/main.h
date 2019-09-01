#ifndef MAIN_H
#define MAIN_H

#define F_CPU 16000000 // 16 Mhz. 
//(extern crystal, lfuse 0xF7, hfuse 0xD9 (0x99 to enable debugwire), Efuse 0xFD)

#define UART_BAUD_RATE 115200

#define numdevs 32     // number of devices to keep an eye on

#include <stdint.h> 

enum status {NOTINUSE=0, ON, ON_1st, OFF, PRESUMED_OFF, GARBLE}; 
/* 
ON_1st: received first "ON" message from ID.
ON: received 2nd "ON" message from ID, within (PRESUMED_OFF waiting period) 2 minutes. (To filter noise seen as ID's) 
OFF: Received OFF message from ID. 
PRESUMED_OFF: Received no message from ID for 2 minutes.
GARBLE: MSG different from On or OFF.
NOTINUSE: this device-status storage spot is not in use. This is 0 so this is the default
*/

typedef struct{
uint16_t ID;
enum status state;
uint16_t lastseen; // timestamp for "MISSEDMSG"/"PRESUMED_OFF".
//str name[]? Of gewoon een bende led's op een frontpaneel en daar plakkertjes naast?
}device;

typedef struct{
uint16_t ID;
char name[16]; // name, max 16 characters 
}DNS; // Device Name Something

// externals, declared in main (globals), used in proces.c and show.c. Maybe should pass pointer instead. Leftover from before splitting into seperate files 
extern device devices[];   
extern uint16_t now;
extern uint8_t numOn;
extern DNS devnames[]; 

#endif
