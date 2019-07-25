// MainsSensor receiver side POC. Runs on Atmega328, serial (or USB-serial) communication to something smarter, or maybe later port to ESP for MQTT conectivity.


// TODO: split in a couple usefull .h's and a clearer main
// TODO: Make time-out for presumed off shorter (but not too short. Maybe 2 minutes instead of 10?)

// TODO: Might also need tx side modifications: improve immunity to noise. (Als ik er nu een 433Mhz ontvanger op aansluit, ziet het ding ID's die niet gezonden zijn. Meeste als "GARBLED" maar ook als OFF, en als ik lang genoeg wacht ook als ON). Misschien dat een timeout op de ontvangst van het syncbyte al aardig helpt? (Mag niet langer duren dan 16 bits na het startbit, normaal gesproken)
// TODO: Maybe ignore msg's "OFF" if there is no device "ON" with that ID? and other sanity checks 
// TODO: IDEA:(Only accept an "ON" after 2 msgs? And let transmitter transmit 2 messages in a row on startup? / faster on startup?)
// TODO: Maybe longer HEADER before the ID/MSG? For easy-er sync?

#define F_CPU 16000000 // 16 Mhz. 
//(extern crystal, lfuse 0xF7, hfuse 0xD9 (0x99 to enable debugwire), Efuse 0xFD)
#define numdevs 16     // number of devices to keep an eye on

#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "./uartlibrary/uart.h"
#define UART_BAUD_RATE 115200

enum status {NOTINUSE=0, ON, OFF, PRESUMED_OFF, GARBLE}; 
/* 
ON: received "ON" message from ID. 
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
uint8_t foundit=0; //used as bool to determine if a device is new or seen before.  
 
    for(pntr=0;pntr<numdevs;pntr++){
        if(ID==devices[pntr].ID){
            devices[pntr].lastseen = now;
            if(MSG==0xFF) devices[pntr].state=ON; else devices[pntr].state=OFF;
            foundit=1;
            break;
	    }
    }
    if(foundit==0){ // only if it is a new device, not seen before, add it:
        for(pntr=0;pntr<numdevs;pntr++){
            if( (devices[pntr].state==NOTINUSE) || (devices[pntr].state==GARBLE) ){ // TODO: If all out of "unused" or "garbled", re-use "off" or "presumed_off" when this msg is not garbled.
                devices[pntr].lastseen = now;
                switch (MSG){
                    case 0xFF: 
                    devices[pntr].state=ON; 
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
                break; // add new devices just once!
            }        
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
            PORTC= (1<<4); //  XXX show we are here
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
            // if a device is not seen for "a long while" (30000 ticks at 100 Hz = 300s = 5 min), set its state to PRESUMED_OFF (probably missed goodbye msg). 
            //Only if device was ON previously.
	            if( ((now-devices[i].lastseen) >= 30000) && (devices[i].state==ON) ) devices[i].state=PRESUMED_OFF;
            //recount number of devices still on.        
                if( devices[i].state==ON) numOn++;
            }
        // TODO: Something that sets devices that are OFF or PRESUMED_OFF to NOTINUSE after a while, otherwise no new devices can be added... (Unless they have a known ID or the receiver is reset... So it might not be that much of a problem, esp. for a POC).
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


