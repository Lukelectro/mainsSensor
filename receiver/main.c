// MainsSensor receiver side POC. Runs on Atmega328, serial (or USB-serial) communication to something smarter, or maybe later port to ESP for MQTT conectivity.

// TODO: split in a couple usefull .h's and a clearer main


#define F_CPU 16000000 // 16 Mhz. 
//(extern crystal, lfuse 0xF7, hfuse 0xD9 (0x99 to enable debugwire), Efuse 0xFD)
#define numdevs 32     // number of devices to keep an eye on

/* If defined, also records new devices if their messages are garbled or OFF, 
 * otherwise only record new devices that are ON, so OFF messages only get accepted from devices that where ON previously and garbled messages get ignored 
 */
//#define LOG_ALL 


#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "./uartlibrary/uart.h"
#define UART_BAUD_RATE 115200

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
;
device devices[numdevs]; // keep status on all possible ID's.
uint16_t now=0;
uint8_t numOn;
volatile uint8_t rec_buff;
volatile int8_t bitcnt;
volatile enum bit_state {WAITFORSTARTH, WAITFORSTARTL, OTHERBITS} bit_st=WAITFORSTARTH;
volatile enum rec_state {START,WAITFORSYNC,IDH,IDL,AANUIT,PROCESS} rec_st = START;


void updateDevice(uint16_t ID, uint8_t MSG){
// find device in array and update it, and if it's not there, add it.
unsigned int pntr;
uint8_t processed=0; // is this device allready updated?  

/* first check if this device ID is seen before allready. if it is, update its status (and set processed to 1)*/ 
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


void DisplayRefresh(void){
// Transmit over serial: numon (number of devices still ON) and ID of all devices still on / OFF/Presumed off.
// Display number of devices still on (either on a LED if numon>0, or on 7segment displays, or on a LCD)
// Display device ID's still on.
// TODO: Maybe even display device names for those ID's, but that would require a lookup table. That would need te be editable or known at compile time. Both difficult.

char buffer[7];
unsigned int count=0;
uart_puts_P("NumOn: ");
itoa(numOn,buffer,10);
uart_puts(buffer);
uart_putc('\n');
uart_puts_P("ID's still ON:");
    for(unsigned int pntr=0;pntr<numdevs;pntr++){
        if(devices[pntr].state==ON){
            itoa(devices[pntr].ID,buffer,16); //hex
            uart_puts(buffer);
            uart_puts_P(",");
            count++;      
            }
    }
    if(count) uart_putc('\n'); else uart_puts_P("NONE \n");
count = 0;
uart_puts_P("ID's not seen a while, presumed OFF:");
    for(unsigned int pntr=0;pntr<numdevs;pntr++){
        if(devices[pntr].state==PRESUMED_OFF){
            itoa(devices[pntr].ID,buffer,16); //hex
            uart_puts(buffer);
            uart_puts_P(","); 
            count++;        
            }
    }
    if(count) uart_putc('\n'); else uart_puts_P("NONE \n");
count = 0;
uart_puts_P("ID's that sent goodbye's, known OFF:");
    for(unsigned int pntr=0;pntr<numdevs;pntr++){
        if(devices[pntr].state==OFF){
            itoa(devices[pntr].ID,buffer,16); //hex
            uart_puts(buffer);
            uart_puts_P(",");      
            count++;                
            }
        }
    if(count) uart_putc('\n'); else uart_puts_P("NONE \n");

#ifdef LOG_ALL /* only print if indeed logged */
count = 0;
uart_puts_P("ID's that sent garbled MSG's:"); 
    for(unsigned int pntr=0;pntr<numdevs;pntr++){
        if(devices[pntr].state==GARBLE){
            itoa(devices[pntr].ID,buffer,16); //hex
            uart_puts(buffer);
            uart_puts_P(",");      
            count++;                
            }
        }
    if(count) uart_putc('\n'); else uart_puts_P("NONE \n");
/* when logging all/showing all, also show ON_1st*/
count = 0;
uart_puts_P("ID's that so far only sent 1 ON msg (ON_1st):"); 
    for(unsigned int pntr=0;pntr<numdevs;pntr++){
        if(devices[pntr].state==ON_1st){
            itoa(devices[pntr].ID,buffer,16); //hex
            uart_puts(buffer);
            uart_puts_P(",");      
            count++;                
            }
        }
    if(count) uart_putc('\n'); else uart_puts_P("NONE \n");
#endif
}


int main(void){
static uint16_t prevnow=0, ID;
static uint8_t MSG;

uart_init( UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) ); 

sei();             // enable global interrupts (For timer, and uart.h)

TCCR0A = 1<<WGM01; // CTC mode
TCCR0B = 1<<CS01;  // clkIO/8 (16Mhz/8=2MHz)
OCR0A = 100;       // 16MHz/8/100= 20kHz --- 50 us
TIMSK0 = (1<<OCIE0A); // enable OC1A interrupt

// set pin modes for LED/7seg/display/whateveroutputischoosen
DDRC |= 0xFF; // all outputs. For LED's (Except C6 as that is reset)
DDRD &=~(1<<PORTD2); // PORTD2 input
PORTD |= (1<<PORTD2); // PORTD2 pullup

uart_puts_P("Hallo Wereld!\n"); 


    while(1){
   
    switch(rec_st){
        case START:
             bit_st = WAITFORSTARTH;
             bitcnt = 8;
             rec_st = IDH;
        break;
        case WAITFORSYNC:
            // do not reset bit_st while waiting for start bit!
        break;
        case IDH: // wait untill IDH is in
            if(bitcnt==0){
            ID=rec_buff;
            ID=ID<<8;
            bitcnt=8;
            rec_st=IDL;
            }
        break;
        case IDL: // untill IDL is in
            if(bitcnt==0){
            ID|=rec_buff;
            bitcnt=8;
            rec_st=AANUIT;
            }
        break;
        case AANUIT: // untill MSG is in
            if(bitcnt==0){
            MSG=rec_buff;
            rec_st = PROCESS;        
            }
        break;
        case PROCESS: // proces rec'd data
            updateDevice(ID,MSG);            
            rec_st=START; // and wait for sync again.
       
        break;
        default:
        //err
        break;
        }

  
        if(now-prevnow > 100){ //every second
        prevnow = now;
        numOn=0; // reset for recount.
            for(unsigned int i=0; i<numdevs; i++){
            // if a device is not seen for "a while" (18000 ticks at 100 Hz = 180s = 3 min), set its state to PRESUMED_OFF (probably missed goodbye msg). 
            //Only if device was ON previously.
	            if( ((now-devices[i].lastseen) >= 18000) && ( (devices[i].state==ON) || (devices[i].state==ON_1st) )) devices[i].state=PRESUMED_OFF;
            //recount number of devices still on.        
                if( devices[i].state==ON) numOn++;
            }
        DisplayRefresh(); // somehow weergeven welke devices nog aan staan.
        }
          
    }
}


ISR(BADISR_vect)
{
    // just reset, but have this here so I could in theory add a handler
}

ISR(TIMER0_COMPA_vect){ // 16E6/8/100 = 20 kHz (50 us, for receiver timing.)
static uint8_t prescale = 0, prev = 0, tmp; 
static uint16_t timer, timestamp; // it should also work with 8 bit timestamps (timer-timestamp >= n should be overflow safe), but it does not.

    if(prescale>=200){
    now++; // 20 kHz / 200 = 100 Hz
    prescale = 0;
    }
prescale++;
timer++; // use seperate variable that does not reset at 200 but overflows like expected.
tmp=(PIND&(1<<PIND2)); // because PIND is volatile but I only want to read it once to prevent race conditions

    if(prev!=tmp){ // detect edges
        prev=tmp;
        if(bitcnt>0){ // only receive bits if previous data has been read.
            switch (bit_st){
            case WAITFORSTARTH:
                if(tmp){ // upgoing edge
                    timestamp = timer;
                }
                else{ // downgoing edge
                    if( (timer-timestamp >= 15) && (timer-timestamp <= 26) ){ // >= 750 us and <= 1.3 ms
                         timestamp = timer; // save new timestamp
                         bit_st = WAITFORSTARTL; // high period was within margins
                        // PINC=(1<<0); // to see if the edge on C0 alligns with the falling edge in the "middle" of the sync bit. (it does)
                    } // else, if not whitin margins, well... retry
                }
            break;
            case WAITFORSTARTL:
                if(tmp){ // upgoing edge
                  if( (timer-timestamp >= 15) && (timer-timestamp <= 26) ){ // >= 750 us and <= 1.3 ms
                      timestamp = (timer-9); // save new timestamp minus an offset to make sure next edge gets seen as late enough
                      bit_st=OTHERBITS; // low period was within margins
                  } 
                  else{ // else, if not whitin margins, well... retry
                     timestamp=timer;
                     bit_st = WAITFORSTARTH;
                  }
                }
                else{ // downgoing edge
                // Should not happen, but still...
                bit_st = WAITFORSTARTH;
                }
            
            break;
            case OTHERBITS:
                // TODO: figure out how to make this work with both 0 or 1 as first bit after the allways-low end of the sync bit.
                // first OTHERBITS edge will allways be rising edge because end of syncbit is allways 0. But first OTHERBIT might be either 1 or 0. How to distinguish?
                // Or, for now, a simpler aproach would be to make the first ID bit always a 1. Still leaves 2^15 possible ID's (32768 possibilities )
            if((timer-timestamp)<=25){      // at most 17 ticks = 850us appart (Otherwise, restart)
                if((timer-timestamp)>=10){ //  at least 10*50 = 500 us appart (half a bittime is about 200+ us) (Otherwise, wait longer and continue)
                    rec_buff=rec_buff<<1; // shift in the (previous) bits before adding a new one (or a new zero)                
                    if(!tmp){
                        rec_buff|=1; // if PIND2 is low now, it was a high-to-low transition, so a 1.
                    }                
                    bitcnt--;             // and count them
                    timestamp = timer;
                 }
            }else{
                 rec_st = START;        // if edges are too far apart, wait for start bit 
            }
            break;
            default:
            bit_st=WAITFORSTARTH; 
            }
        }
    }
}


