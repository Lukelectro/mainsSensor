// MainsSensor ontvangende kant. TBD on what chip this is going to run. Mosquitto connectivity might be nice but that could be done with serial communication to something smarter but less realtime. At least a display or a few LED's that show which things are still powered.

// TODO: split in a couple usefull .h's and a clearer main
// TODO: develloped allong multiple lines of thinking, might be incomplete in serveral directions.

#define F_CPU 16000000 // 16 Mhz. Default Atmega328
#define numdevs 16     // number of devices to keep an eye on


#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#define BAUD 9600
#include <util/setbaud.h> // uart baudrate helper

enum status {ON,OFF,PRESUMED_OFF,NOTINUSE}; 
/* 
ON: received "ON" message from ID. 
OFF: Received OFF message from ID. 
PRESUMED_OFF: Received no message from ID for 2 minutes.
NOTINUSE: this device-status storage spot is not in use
*/

typedef struct{
uint16_t ID;
enum status state;
uint16_t lastseen; // timestamp for "MISSEDMSG"/"PRESUMED_OFF".
//str name[]? Of gewoon een bende led's op een frontpaneel en daar plakkertjes naast?
}device;
;
device devices[numdevs]; // keep status on all possible ID's.
volatile uint16_t now=0, rec_tim=0, ID;
volatile uint8_t buffercount=0, MSG;
uint8_t numOn;

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

void CheckAllDev(void){
    numOn=0; // reset for recount.
    for(unsigned int i=0; i<numdevs; i++){
        // if now overflowed. (TODO: Make it overflow every 2 mins or modify this to make it check for 2 mins since last message)
	    if( (devices[i].lastseen>now) && devices[i].state!=NOTINUSE) devices[i].state=PRESUMED_OFF; 
        //recount number of devices still on.        
        if( devices[i].state==ON) numOn++;
    }
};

/*
uint8_t decodeReceived(uint16_t in) // machester encoded word to original byte. No error detection.
{
return (in&0x01)|(in&0x04)<<1|(in&0x10)<<2|(in&0x40)<<3|(in&0x100)<<4|(in&0x400)<<5|(in&0x1000)<<6|(in&0x4000)<<7;
}
// I now look for edges directly, so this is not needed: TODO: remove once certain about method of receiving
*/ 

void DisplayRefresh(void){
// TODO: Transmit over serial: numon (number of devices still ON) and ID of all devices still on.
// TODO: Display number of devices still on (either on a LED if numon>0, or on 7segment displays, or on a LCD)
// TODO: Display device ID's still on.
// TODO: Maybe even display device names for those ID's, but that would require a lookup table. That would need te be editable or known at compile time. Both difficult.

}


int main(void){
uint16_t prevnow=0;

TCCR0A = 1<<WGM01; // CTC mode
TCCR0B = 1<<CS01;  // clkIO/8 (16Mhz/8=2MHz)
OCR0A = 100;       // 16MHz/8/100= 20kHz --- 50 us
OCR0B = 200;       // 16Mhz/8/200=10kHz --- 100 us
TIMSK0 = (1<<OCIE0A | 1<<OCIE0B); // enable both OC A and B interrupts.
sei();             // enable global interrupts

    // TODO: proces data received in interrupt once frame is complete.
    if(buffercount>0){
    updateDevice(ID,MSG);
    buffercount--;
    }    

    if(now-prevnow > 10000){ //every second
        prevnow = now;
        CheckAllDev();
        DisplayRefresh(); // somehow weergeven welke devices nog aan staan.
    }   
}


ISR(BADISR_vect)
{
    // just reset, but have this here so I could in theory add a handler
}

ISR(INT0_vect){ //TODO: please note INT0 is on PB2. Set it for "trigger on all edges" 
static enum rec_state {NONE,SYNC,IDH, IDL,AANUIT} rec_st = NONE;
static enum bit_state {WAITINGFORSTART,BIT7_0} bit_st = WAITINGFORSTART;
static uint8_t recbyte, bit;
static uint16_t timestamp;
// TODO, receive manchester encoded data (Do clock recovery/syncrhonisation. TBD how best to do this)
// continuesly shift bits in, so a step later it can be synced.

// A: See an downgoing edge (1-0 transtion), treat it as the start bit (whose value is known to be a 1, first bit from the "sync word" 0xA5). 
// Start a timer / get a timestamp from a timer
// B: next edge must be after 300 us, but before 500 us (bit time is 400us). If a edge is too soon, ignore it. If an edge is too late, restart from A.
// repeat B: untill you have all bits.
// If the byte is a valid sync, receive ID, then status
// 

#define MINBITTIME 6 // 6*50us=300us.
#define MAXBITTIME 10 // 10*50us=500us.

    switch(bit_st){
    case WAITINGFORSTART:
        rec_st = NONE;
        if( !(PINB&(1<<PINB2))){ // als PINB2 nu laag is, was het een neergaand edge. (Rare glitches daargelaten, maar die mogen fout gaan)
            timestamp = rec_tim;// To keep track of bittime
            bit_st = BIT7_0;
            rec_st = SYNC;
            bit = 0;
        };
    break;
    case BIT7_0: // MSB first ontvangen
        
        
        if( (rec_tim-timestamp) <MINBITTIME){
             break; // wait a little longer
        }
        if( (rec_tim-timestamp) >MAXBITTIME) {
            bit_st = WAITINGFORSTART;
            //somehow show timeout-error?
            break;
        }
        
        timestamp = rec_tim; // reset timer, for next iteration            
 
        if(!(PINB&(1<<PINB2))){ // als PINB2 nu laag is, was het een neergaand edge. Dus een 1-0 transistie, dus een 1
          recbyte|=1;      // 1 inschuiven.
        }
        recbyte=recbyte<<1; // bitje opschuiven (Dus als het geen 1 was, wordt er een 0 ingeschoven).
        bit++;
        
        // only write new data if buffer has been read completely. Ensure there are no buffer overwrites during reads...

        if((bit==7) && (buffercount == 0)){ // evt kan ik ook als byte==syncword gebruiken als conditie, maar wat dan als dat door toeval is terwijl het synword nog niet af is?
                    // dan zou 'ie dus niet terug moeten naar startbit, maar gewoon moeten doorschuiven tot het wel af is. Terwijl 'ie anders door moet naar ID
            switch(rec_st){
            case SYNC:
                rec_st = IDH;
            break;
            case IDH:
                ID = recbyte<<8;
                rec_st = IDL;                                
            break;
            case IDL:
                rec_st = AANUIT;
                ID|=recbyte;
            case AANUIT:
                MSG=recbyte;
                buffercount++;//trigger verwerking van deze nieuw ontvangen data, in main.
                rec_st = NONE;
            break;
            default:
                rec_st=NONE;
                //err!
            }
        } // TODO: Is an "else" clause needed to catch buffer overflow or bitcount underflow?
      break;

      default:
        // err!
      break;  
      }
};


ISR(TIMER0_COMPA_vect){ // 16E6/8/100 = 20 kHz (50 us, for receiver timing.)
rec_tim++;
}

ISR(TIMER0_COMPB_vect){ // 16E6/8/200 = 10 kHz (100 us, for timekeeping)
now++;
}


