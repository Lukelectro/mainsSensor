// MainsSensor receiver side POC. Runs on Atmega328, serial (or USB-serial) communication to something smarter, or maybe later port to ESP for MQTT conectivity.


// TODO: split in a couple usefull .h's and a clearer main
// TODO: develloped allong multiple lines of thinking, might be incomplete in serveral directions.
// TODO: build the hw and test&improve

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

enum status {NOTINUSE=0, ON, OFF, PRESUMED_OFF}; 
/* 
ON: received "ON" message from ID. 
OFF: Received OFF message from ID. 
PRESUMED_OFF: Received no message from ID for 2 minutes.
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
volatile uint8_t rec_buff, bitcnt;

void updateDevice(uint16_t ID, uint8_t MSG){
// find device in array and update it, and if it's not there, add it.
unsigned int pntr;
uint8_t foundit=0; //used as bool   
    for(pntr=0;pntr<numdevs;pntr++){
        if(ID==devices[pntr].ID){
            devices[pntr].lastseen = now;
            if(MSG==0xFF) devices[pntr].state=ON; else devices[pntr].state=OFF;
            foundit=1;
            break;
	    }
    }
    if(foundit==0){
        for(pntr=0;pntr<numdevs;pntr++){
            if(devices[pntr].state==NOTINUSE){
                devices[pntr].lastseen = now;
                if(MSG==0xFF) devices[pntr].state=ON; else devices[pntr].state=OFF;
                devices[pntr].ID=ID;            
            }        
        }
    }
}


void DisplayRefresh(void){
// Transmit over serial: numon (number of devices still ON) and ID of all devices still on / OFF/Presumed off.
// TODO: Display number of devices still on (either on a LED if numon>0, or on 7segment displays, or on a LCD)
// TODO: Display device ID's still on.
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
            itoa(numOn,buffer,16); //hex
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
            itoa(numOn,buffer,16); //hex
            uart_puts(buffer);
            uart_puts_P(",");      
            }
    }
    if(count) uart_putc('\n'); else uart_puts_P("NONE \n");
count = 0;
uart_puts_P("ID's that send goodbye's, known OFF:");
    for(unsigned int pntr=0;pntr<numdevs;pntr++){
        if(devices[pntr].state==OFF){
            itoa(numOn,buffer,16); //hex
            uart_puts(buffer);
            uart_puts_P(",");      
            }
        }
    if(count) uart_putc('\n'); else uart_puts_P("NONE \n");
}


int main(void){
static uint16_t prevnow=0, ID;
static uint8_t MSG;
static enum rec_state {SYNC,IDH,IDL,AANUIT,PROCESS} rec_st = SYNC;

uart_init( UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) ); 

sei();             // enable global interrupts (For timers, ext int0, and uart.h)

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
        case SYNC: // wait for sync byte
            if(rec_buff==0xA5){
            bitcnt=7;
            rec_st=IDH;
            }
        break;
        case IDH: // wait untill IDH is in
            PORTC|=(1<<0); // Show SYNC XXX only for debug
            if(bitcnt==0){
            ID|=rec_buff;
            ID=ID<<8;
            bitcnt=7;
            rec_st=IDL;
            }
        break;
        case IDL: // untill IDL is in
            if(bitcnt==0){
            ID|=rec_buff;
            bitcnt=7;
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
            PORTC|=(1<<4); // Show data rec'd XXX only for debug
            updateDevice(ID,MSG);
            rec_st=SYNC; // and wait for sync again.
        break;
        default:
        //err
        break;
        }

        if( (rec_st!=SYNC) && (bitcnt>7)){
        rec_st=SYNC;
        // this is an error and should not happen
        PORTC|=(1<<5); // show error
        }
    
        if(now-prevnow > 100){ //every second
        prevnow = now;
        numOn=0; // reset for recount.
            for(unsigned int i=0; i<numdevs; i++){
            // if "now" overflowed (uint16_t MAX 65536, at 100 Hz that's slightly over a 10 minutes. A device should be able to send a message multiple times in 10 minutes.
	            if( (devices[i].lastseen>now) && (devices[i].state!=NOTINUSE)) devices[i].state=PRESUMED_OFF; 
            //recount number of devices still on.        
                if( devices[i].state==ON) numOn++;
            }

        DisplayRefresh(); // somehow weergeven welke devices nog aan staan.
       // PINC|=(1<<PINC0); // toggle debugLED.
        }
          
    }
}


ISR(BADISR_vect)
{
    // just reset, but have this here so I could in theory add a handler
}

ISR(TIMER0_COMPA_vect){ // 16E6/8/100 = 20 kHz (50 us, for receiver timing.)
static uint8_t prescale = 0, prev = 0, tmp;
    if(prescale>=200){
    now++; // 20 kHz / 200 = 100 Hz
    prescale = 0;
    }
prescale++;
tmp=(PIND&1<<PIND2); // because PIND is volatile but I don't want to re-read it (sync it)
    if(tmp!=prev){ // only respond to edges
        prev=tmp;
        if(!tmp) rec_buff++; // if PIND2 is low now, it was a high-to-low transition, so a 1.
        rec_buff=rec_buff<<1; // shift in the bits.
        bitcnt--;              // and count them
    } 
}



