// MainsSensor.
// doel: op zuinige attiny (attiny10? attiny45?) bij opstarten en elke paar minuten op 433Mhz eigen ID zenden, en bij wegvallen van de voeding een afmeldbericht sturen (Met de energie uit een elco) met datzelfde ID.

// PORTB1 data output
// PORTB2 = INT0 watch powerloss (same pin on both t45 and t10)

//#define t45
#define t10
// TODO: Please note: do not forget to edit the makefile as well when changing MCU between attiny45 and attiny10 (in 2 places), and when using t45, set its fuses correctly.

#define F_CPU 128000UL // 128 Khz internal osc. (t45 lfuse:E4, rest default. 0x64 for ckdiv8 16 kHz clock, 0x62 default 8/8=1Mhz. t10 clock can be changed at runtime, t45 clock can not be changed at runtime.)
// NOTE: It will be real slow then, so limit bitclock for programming: avrdude -p t45 -c dragon_isp -t -B 50 (400 at 16Khz)
// NOTE: Do NOT enable debugwire at this slow clock. It will make reprogramming impossible (debug won't work either so it bricks the chip, btdt)

#define HALFBITTIME 200 // us, actual value slightly larger because normal instructions take time too and add to delay. (Actual half bit time is arround 300us when set to 200)


#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/crc16.h>

// Rewritten to use CRC. Is calculated at runtime despite messages / ID being constant, this eases changing ID.
// uses crc8 with 0 as initial value and Polynomial: x^8 + x^2 + x + 1 (0xE0).


uint16_t ID_16b = 0xB100;   // Input ID here (TODO: put it at a fixed address in flash and change the .hex just before programming).
uint8_t HIcrc=0, BYEcrc=0;  // crc for hi-message and Bye-message, before conversion to manchester encoding
uint16_t MHIcrc, MBYEcrc;   // crc's after conversion to manchester encoding
uint16_t IDH, IDL; // manchester-encoded ID split in 16 bit units, so this is not done at transmit time but before.
// no decisionmaking while transmitting, it affects timing too much at this slow clocrate. So have a HIframe and a byeframe ready beforehand. Including their CRC's, and with pre-split ID. 


/*
normal manchester
0      0b01010101
1      0b01010110
2      0b01011001
3      0b01011010
4      0b01100101
5      0b01100110
6      0b01101001
7      0b01101010
8      0b10010101
9      0b10010110
A      0b10011001 
B      0b10011010
C      0b10100101
D      0b10100110
E      0b10101001
F      0b10101010
*/

uint16_t tomanch_8(uint8_t in){ //1->10 0->01 
uint16_t manch=0;
    for(uint8_t i=0;i<8;i++){
    manch=manch<<2;    // shift manch firts, so not shifting out significant bits on last iteration of the loop
        if((in&0x80) != 0){
            manch|=2; // 0b10
        }
        else
        {
            manch|=1; //0b10     
        }
        in=in<<1;    
    }
return manch;
}


void transmit(uint16_t tx){   
uint8_t i=0;
  do{
     if(tx&0x8000) PORTB=(1<<PORTB1); else PORTB=0; // MSB first, then.
     i++;    
     tx=tx<<1;//MSB first	
     _delay_us(HALFBITTIME); // at 128 KHz clock delay is needed. At 16 KHz clock a negative delay would be nice... So a 128 KHz clock it is.
    }while(i<16);
}
/*
void transmitHIframe(){

// sync bit / start condition (instead of sync word) 
PORTB=(1<<PORTB1);
_delay_us(HALFBITTIME*4);
PORTB=0;
_delay_us(HALFBITTIME*4);
  
transmit(IDH); // MSB first
transmit(IDL);
transmit(0xAAAA); // HI = 0xFF, in manchester 0b1010 1010 1010 1010 (0xAAAA)
transmit(MHIcrc);
PORTB=0; // always end with the pin LOW
}

void transmitBYEframe(){

// sync bit / start condition
PORTB=(1<<PORTB1);
_delay_us(HALFBITTIME*4);
PORTB=0;
_delay_us(HALFBITTIME*4);
  
transmit(IDH); // MSB first
transmit(IDL);
transmit(0x5555); // Bye=0x00, in manchester 0b0101 0101 0101 0101 (0x5555) 
transmit(MBYEcrc);
PORTB=0; // always end with the pin LOW
}
*/


/* Then, lets make it one function, so timing might degrade, but then for both Hi and Bye in the same way */
void transmitframe(uint8_t HiBYE){

/* sync bit / start condition */
PORTB=(1<<PORTB1);
_delay_us(HALFBITTIME*4);
PORTB=0;
_delay_us(HALFBITTIME*4);
  
transmit(IDH); // MSB first
transmit(IDL);
    if(HiBYE){
        transmit(0xAAAA); // HI = 0xFF, in manchester 0b1010 1010 1010 1010 (0xAAAA)    
        transmit(MHIcrc);
    }else
    {
        transmit(0x5555); // Bye=0x00, in manchester 0b0101 0101 0101 0101 (0x5555)     
        transmit(MBYEcrc);
    };
PORTB=0; // always end with the pin LOW
};


int main(void){
RSTFLR = 0 ; // clear reset flags (Because WDRF overrides WDE, so clear WDRF)
CCP = 0xD8; // enable write of protected registers
WDTCSR=0x00; // disable WDT

DDRB|=(1<<PORTB1); // PORTB1 output
PORTB=0; // start with the pin LOW

#ifdef t45
PRR = 0x0F; // disable powerhungry peripherals
//enable INT0
//MCUCR=0x00; //0x00 is the default and means "low level on PB2 triggers INT0"
GIMSK = (1<<6); // enable INT0
#endif

#ifdef t10
PRR = 0x03;// disable powerhungry peripherals
CCP = 0xD8; // signature for changing protected registers such as CLKMSR
CLKMSR = 0x01; // select 128Khz internal oscilator as main clock. Default prescaler is 8, so 16 Khz main clock.
CCP = 0xD8; // signature for changing protected registers such as CLKMSR
CLKPSR = 0x00; // set prescaler to 1, so 128 kHz main clock.
//EICRA=0x00; //0x00 is the default and means "low level on PB2 triggers INT0"
EIMSK = 0x01; // enable INT0
#endif


/* do the waiting after the right clock is selected! (And in the meantime, calculate the CRC's etc.)*/

// convert ID to manchester, and split into 2
IDH=tomanch_8( (ID_16b>>8)&0xFF );
IDL=tomanch_8(ID_16b&0xFF);
// calculate CRC's
HIcrc= _crc8_ccitt_update(HIcrc,((ID_16b>>8)&0xFF)); // first bit of ID
HIcrc= _crc8_ccitt_update(HIcrc,(ID_16b&0xFF));       // 2nd bit of ID
BYEcrc=HIcrc; // ID is the same for both, so coppy
HIcrc= _crc8_ccitt_update(HIcrc,0xFF);              // HI-msg
BYEcrc= _crc8_ccitt_update(BYEcrc,0x00);           // BYE-msg
//convert crc's to manchester:
MHIcrc=tomanch_8(HIcrc);
MBYEcrc=tomanch_8(BYEcrc);


while((PINB&(1<<PINB2))==0); // wait untill bulk cap is charged
_delay_ms(5000); // and slightly longer, because input flips before it is full enough.

sei(); // Enable interupts after PB2 is high

/* no longer using the 0xFF preamble, so can transmit the actual message more often, so will transmit the actual message more often .
*  In fact, receiver catches repeated messages better, because sometimes noise gets seen as a start condition and it only gets half a message if it isn't repeated
* also, timing seems to somehow get more reliable if transmitter is transmitting for longer (?!?).
*
* 1 msg = 32 bits at 200 us - in practice more like 300 us. 32*300us = 9600us , so about 10ms per message. 
* Band E, 433-434Mhz max 10 mW max 10% dutycycle, so in a minute (60 s) max 6s worth of messages. 8 messages a minute = 80ms << 6s, so that's OK with quite a margin.
* (Brochure vergunningsvrije radiotoepassingen, tabel 1, februari 2012)
*/

while(1){   // re-transmit HI message 4 times every minute / repeat untill powerdown.
    for (uint8_t i=0;i<4;i++){
       //transmitHIframe();
        transmitframe(1);
       _delay_ms(3);
    }
    _delay_ms(30000); 
    //_delay_ms(5000); // or test with 5s...
    }
}


ISR(INT0_vect, ISR_NAKED){ //note INT0 is on PB2 
// To save on RAM and STACK (and prevent collision), declare ISR as "naked", so no context save or restore gets added by the compiler.
// then, after ISR, don't reti, but reset, so all mangled variables and registers get reset.

    WDTCSR = 0x0F; // enable WDT while there still sure is power, set it to timeout after 2s (plenty of time to transmit multiple messages)

// if PB2 is low, power failed / is going down, so transmit goodbye message on bulk capacitor charge
    do{
        //transmitBYEframe();
        transmitframe(0); 
        _delay_ms(3); // give reciever a bit of time (to optionally proces an error and wait for syncbit again)
    }while((PINB&(1<<PINB2))==0); // until power goes out or returns.
    // if power returned, do a reset, by waiting for WDT timeout
    while(1);
}
