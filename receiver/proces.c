#include "proces.h"
#include <string.h>
#include <util/crc16.h>

void updateDevice(uint16_t ID, uint8_t MSG, uint8_t crc){
// find device in array and update it, and if it's not there, add it.
unsigned int pntr;
uint8_t calccrc=0;
uint8_t processed=0; // is this device allready updated?  

/* first, calculated and compare CRC - if crc is ok, proces device, otherwise, skip it */
calccrc = _crc8_ccitt_update(calccrc,((ID>>8)&0xFF)); // first bit of ID
calccrc = _crc8_ccitt_update(calccrc,(ID&0xFF));      // 2nd bit of ID
calccrc = _crc8_ccitt_update(calccrc,MSG);                // msg

    if(calccrc != crc){
        return; // if crc's don't match, return withouth processing the new device
    }

/* check if this device ID is seen before allready. if it is, update its status (and set processed to 1)*/ 
    for(pntr=0;pntr<numdevs;pntr++){
        if(ID==devices[pntr].ID){
            devices[pntr].lastseen = now;
            if(0xFF==MSG) devices[pntr].state=ON; else if(0x00==MSG) devices[pntr].state=OFF;
            processed=1;
            break;
	    }
    }

/* if it is a device not seen before (processed == 0), then add it */
    enum status trytofill = NOTINUSE; /* first try to use a unused spot, if that fails, try to use "garble", "off" or "presumed_off", in that order */
    
    while(0==processed){ 
        for(pntr=0;pntr<numdevs;pntr++){
            if( devices[pntr].state==trytofill ){ // If all out of "unused", re-use "garble", "off" or "presumed_off".
                devices[pntr].lastseen = now;
                #ifdef LOG_ALL
                switch (MSG){
                    case 0xFF: 
                    devices[pntr].state=ON_1st; // only show as "ON" if it is seen at least twice
                    numOn++;
                    break;  
                    case 0x00:
                    devices[pntr].state=OFF; // if the new device is OFF, do not increase or decrease numOn (new devices being OFF is... strange)
                    break;
                    default:
                    devices[pntr].state=GARBLE; // if the new device MSG is not ON or OFF (Most likely transmission noise or receiver bug)
                    break;                    
                    }
                 devices[pntr].ID=ID;               
                 #else
                 if (0xFF==MSG){ // only if the new device is ON, add it:
                    devices[pntr].state=ON_1st; // only show as "ON" if it is seen at least twice (If it is not allready in the list)
                    numOn++;
                    devices[pntr].ID=ID;               
                 }   
                #endif
                processed = 1;
                break; // add new devices just once!
            }        
        }
        switch(trytofill){
        case NOTINUSE:
        trytofill = GARBLE;
        break;
        case GARBLE:
        trytofill = OFF;
        break;
        case OFF:
        trytofill = PRESUMED_OFF;
        break;
        default:
        // ran out of spots
        uart_putc('\n');
        uart_puts_P("Cannot store new device, ran out of storage space");        
        uart_putc('\n');
        processed = 1; // otherwise it would hang here.
        break;        
        }
    }


}

