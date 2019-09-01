// MainsSensor receiver side POC. Runs on Atmega328, serial (or USB-serial) communication to something smarter, or maybe later port to ESP for MQTT conectivity.

// TODO: split in a couple usefull .h's and a clearer main
// TODO: ID->name mapping and a way to set these names over serial and a display to show the names on for devices still ON and an LED that's lit when there is still a nonzero amount of devices still ON.

#include "main.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include "./uartlibrary/uart.h"
#include "proces.h"
#include "show.h"

device devices[numdevs]; // keep status on all possible ID's.
DNS devnames[numdevs];   // keep all the name / ID couplings here.
uint16_t now=0;
uint8_t numOn;
volatile uint8_t buffer[4][2]; // new receive buffer for the entire message (16 bit ID + 8 bit message + 8 bit CRC = 4 bytes), 2 buffers: one being filled, other read
volatile uint8_t which=0; // which buffer is in use for reception and which one for readout? (0,1 - which indicates which one is in use for reception, readout should use the other)
uint8_t prevwhich; // for use by readout (If which has changed, new message can be read. From buffer[..][prevwhich];

volatile enum bit_state {WAITFORSTARTH, WAITFORSTARTL, OTHERBITS} bit_st=WAITFORSTARTH;
volatile enum rec_state {START,WAITFORSYNC, WAITFORMSG ,PROCESS} rec_st = START;



int main(void){
static uint16_t prevnow=0, ID;
static uint8_t MSG,crc;

readdevnames(); // read device names into RAM from EEPROM on bootup

uart_init( UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) ); 

sei();             // enable global interrupts (For timer, and uart.h)

TCCR0A = 1<<WGM01; // CTC mode
TCCR0B = (1<<CS01) | (1<<CS00); // clkIO/64= 16/64 = 1/4 MHz 250 KHz (4 us per tick)
OCR0A = 25;       // 16MHz/64/25= 10kHz --- 100 us

TCCR1B = (1<<CS01) | (1<<CS00); // clkIO/64= 16/64 = 1/4 MHz 250 KHz (4 us per tick) (Because Timer0 clears at ACR0A and a large range is needed for timing pulse validity, use timer 1)

TIMSK0 = (1<<OCIE0A); // enable OC1A interrupt

// set pin modes for LED/7seg/display/whateveroutputischoosen
DDRC &=~(1<<PORTC2); // PORTC2 input for radio RX (Moved from PORTD for LCD on PORTD)
DDRC |= (1<<PORTC3)|(1<<PORTC4); // Debug LED portc3 == LCD backlite. PORTC4 = debug.
PORTC |= (1<<PORTC2); // PORTC2 pullup
PORTC |= (1<<PORTC3); // turn backlite on

uart_puts_P("Hallo Wereld!\nEr zal wat line noise zijn omdat het LCD op dezelfde poort zit, dat is bekend en kan geen kwaad!\n\n"); 
_delay_ms(10); 

uart_release();/* LCD on same port as serial...*/
lcd_init();
lcd_cursor(false,false);
lcd_goto(0,0);
lcd_puts("Hallo Wereld! l1");
lcd_goto(1,0);
lcd_puts("line 2");
//lcd_goto(0,20); //actually 3,0, but 4x20 display is implemented as 2x40... So modified LCD.C instead
lcd_goto(2,0);
lcd_puts("line 3");
//lcd_goto(1,20); //actually 4,0, but 4x20 display is implemented as 2x40...
lcd_goto(3,0);
lcd_puts("line 4");
_delay_ms(1000);
reenableuart(); /* LCD on same port as serial...*/

    while(1){
   
    switch(rec_st){
        case START:
             bit_st = WAITFORSTARTH;
             rec_st = WAITFORMSG; 
        break;
        case WAITFORMSG:
            if(which!=prevwhich){ // if message is in, copy it into its seperate fields

                // for debug, print msg
                //uart_puts("new rec: %X,%X,%X,%X \n",buffer[0][prevwhich],buffer[1][prevwhich],buffer[2][prevwhich],buffer[3][prevwhich]);
                
                uart_puts("new rec: ");                 
                char charbuff[7];
                for(uint8_t i = 0; i<4; i++){
                itoa(buffer[i][prevwhich],charbuff,16); //hex
                uart_puts(charbuff);
                uart_puts(", ");
                }
                uart_puts("\n");                 
 
              // uart_puts =!= printf            
                // XXX

                ID = ((buffer[0][prevwhich])<<8) | buffer[1][prevwhich] ;
                MSG = buffer[2][prevwhich];
                crc = buffer[3][prevwhich];
                prevwhich=which;
                rec_st = PROCESS;
            }        

        break;
        case PROCESS: // proces rec'd data
            // TODO: actually check received CRC, instead of only just buffering it. (if(crcgood){ updatedevice else nothing} retry)
            updateDevice(ID,MSG);            
            rec_st=START; // and wait for sync again.
        break;
        default:
        //err
        break;
        }

        readnewname(devnames); // try to read new device names from serial input  

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
        SerialRefresh(); // somehow weergeven welke devices nog aan staan.
        LCD_refresh();
        }
          
    }
}


ISR(BADISR_vect)
{
    // just reset, but have this here so I could in theory add a handler
}

ISR(TIMER0_COMPA_vect){ // 16E6/64/25 = 10 kHz (100 us, for receiver timing.)
static uint8_t prescale = 0, prev = 0, tmp; 
static uint16_t timer, timestamp; // it should also work with 8 bit timestamps (timer-timestamp >= n should be overflow safe), but it does not.

    if(prescale>=100){
    now++; // 10 kHz / 100 = 100 Hz
    prescale = 0;
    }
prescale++;
timer++; // use seperate variable that does not reset at 100 but overflows like expected.
tmp=(PINC&(1<<PINC2)); // because PINC is volatile but I only want to read it once to prevent race conditions

    if(prev!=tmp){ // detect edges
        prev=tmp;
   
        switch (bit_st){
        case WAITFORSTARTH:
            if(tmp){ // upgoing edge
                timestamp = timer;
            }
            else{ // downgoing edge
                if( (timer-timestamp >= 8) && (timer-timestamp <= 13) ){ // >= 800 us and <= 1.3 ms
                     timestamp = timer; // save new timestamp
                     bit_st = WAITFORSTARTL; // high period was within margins
                } // else, if not whitin margins, well... retry
            }
        break;
        case WAITFORSTARTL:
            if(tmp){ // upgoing edge
              if( (timer-timestamp >= 8) && (timer-timestamp <= 13) ){ // >= 800 us and <= 1.3 ms
                  bit_st=OTHERBITS;   // low period was within margins
                  PCMSK1 = (1<<2);     // setup PIN change interrupt 1 to include only PORTC.2
                  PCICR = (1<<PCIE1); // enable PIN change interrupt 1
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

        break;
        default:
        bit_st=WAITFORSTARTH; 
        }
    }
}

ISR(PCINT1_vect){
// Pin Change Interrupt10 is PORTC2 (RX)
// plan is as follows / TODO:
// after detection of start condition, use edge detection interrupt (to find edges, duh) and the value of timer2 to determine if these edges are far enough appart to be valid. After decoding a full message worth of bits, disable pin change interrupt and process message / refresh display. After that, start watching for start condition again.
static uint16_t timekeeper;
static uint8_t bitptr=0;
static bool firstedge = true;
uint8_t adr;

if(firstedge){ // ignore the first edge
    timekeeper=TCNT1-113; // but not the next one // setup timer so it gets the next edge (faling edge on allways-1 start bit, all ID's start with a '1')
    firstedge=false;
}else if( TCNT1-timekeeper >  113) 
{ // if previous edge n*4us (Edges need to be at least 3/4 BITTIME apart, so 1.5*halfbittime, so about 450 us)
// select if it was an upgoing or downgoing edge. Because previous level then is the bit value (Downgoing edge = 1, upgoing edge is 0)
    adr=bitptr>>3; // to convert from bits to byte addresses, divide by 8. 
    if( (PINC&(1<<2))==0 ){ // If PINC.2 is 0 now, it was a downgoing edge, so a 1 
       buffer[adr][which] |=1; 
    } // if it wasn't a '1', it is a '0', no need to |= a '0'
    buffer[adr][which] = buffer[adr][which]  << 1; // shift in the new '1' or '0' 
    bitptr++;    
    timekeeper=TCNT1;
    PINC=(1<<4); // XXX debug (To see if it samples on the right edge. It does, but still receive wrong data, e.g: FCCE instead of FEE7...
} // if edge not far enough apart, just wait for the next one
//TODO: if toooo far apart, restart / start waiting for start condition again.


if(bitptr >= (8*4)){  // after a full 4 byte message:
    PCICR &= ~(1<<PCIE1); // disable PIN change interrupt 1,
    bitptr = 0;      // reset bitcount,
    if(which==1) which = 0; else which=1; // select which buffer to use next time.
    firstedge = true; // reset first-edge detection
    //PINC=(1<<4); // XXX debug
}

}
