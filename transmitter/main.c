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

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

/*
normal manchester
0      0b01010101
1      0b01010110
2      0b01011001
3      0b01011010
4      0b01100101
5      0b01100101
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

const uint8_t m0 =0b01010101; //0x55
const uint8_t m1 =0b01010110; //0x56
const uint8_t m2 =0b01011001; //0x59
const uint8_t m3 =0b01011010; //0x5A
const uint8_t m4 =0b01100101; //0x65
const uint8_t m5 =0b01100110; //0x66
const uint8_t m6 =0b01101001; //0x69
const uint8_t m7 =0b01101010; //0x6A // start seeing a pattern here...
const uint8_t m8 =0b10010101; //0x95
const uint8_t m9 =0b10010110; //0x96
const uint8_t mA =0b10011001; //0x99
const uint8_t mB =0b10011010; //0x9A
const uint8_t mC =0b10100101; //0xA5
const uint8_t mD =0b10100110; //0xA6
const uint8_t mE =0b10101001; //0xA9
const uint8_t mF =0b10101010; //0xAA

//const uint32_t ID = ((mB<<24)|(mA<<16|(mD<<8)|(m1)); // Unique ID. 0xBAD1 Pre-convert to manchester encoding (because why do that at runtime if it is a constant anyway) 
//const uint32_t ID = 0b10011010100110011010011001010110; // 0xBAD1 , apearently shifting as above is non-const?

//pick one or add more (As there is no EEPROM, ID is set at compile time)
//const uint32_t ID = 0x569A569A; //0x1B1B
//const uint32_t ID = 0x55555555; //0x0000
//const uint32_t ID = 0xA599AAA9; //0xCAFE
//const uint32_t ID = 0x9A555555; //0xB000
//const uint32_t ID = 0x9A999AA9; //0xBABE
const uint32_t ID = 0x9AA9A966; //0xBEE5 (BEES)
//const uint32_t ID = 0x9A55559A; //0xB00B
//const uint32_t ID = 0x95959595; //0x8888
//const uint32_t ID = 0x66666666; //0x5555
//const uint32_t ID = 0x6A6A6A6A; //0x7777


void transmitmanch(uint16_t tx){ 
  
uint8_t i=0;
  do{
     if(tx&0x8000) PORTB=(1<<PORTB1); else PORTB=0; // MSB first, then.
     i++;    
     tx=tx<<1;//MSB first	
     _delay_us(180); // at 128Khz clock delay is needed. At 16Khz clock a negative delay would be nice...
    }while(i<16);

}

void transmitframe(uint8_t HinBye){
//HinBye is used as bool, 0 means "Bye", all else means "Hi"
transmitmanch((mF<<8)|mF); // Bias RX/TX / preamble. All "ones" so no matter which "one" it sees as a startbit, things will be fine once synchronized
transmitmanch((mF<<8)|mF); 
transmitmanch((mF<<8)|mF);
transmitmanch((mF<<8)|mF);
transmitmanch((mF<<8)|mF);
transmitmanch((mF<<8)|mF);
transmitmanch((mF<<8)|mF);
transmitmanch((mA<<8)|m5); // sync word, pre converted to machester 0xA5 = 0b10100101 -manch-> 0b1001 1001 0110 0110 = 0x9966
transmitmanch((ID>>16)&0xFFFF); // MSB first
transmitmanch(ID&0xFFFF);
    if(HinBye) transmitmanch((mF<<8)|mF); else transmitmanch((m0<<8)|m0); // Hi=0xFF, Bye=0x00. 
PORTB=0; // always end with the pin LOW
}


int main(void){
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
CLKPSR = 0x00; // set prescaler to 1, so 128 kHz main clock.
//EICRA=0x00; //0x00 is the default and means "low level on PB2 triggers INT0"
EIMSK = 0x01; // enable INT0
#endif


/* do the waiting after the right clock is selected! */
while((PINB&(1<<PINB2))==0); // wait untill bulk cap is charged
_delay_ms(5000); // and slightly longer, because input flips before it is full enough.

sei(); // Enable interupts after PB2 is high


while(1){
    transmitframe(1); // on powerup and every minute or so, transmit powerup msg
    //_delay_ms(60000);
    _delay_ms(10000); // so let's test with 10s...
    }
}


ISR(BADISR_vect)
{
    // just reset, but have this here so I could in theory add a handler
}

ISR(INT0_vect){ //note INT0 is on PB2 

// if PB2 is low, power failed / is going down
    do{
        transmitframe(0); // transmit goodbye
    }while((PINB&(1<<PINB2))==0); // until power goes out or returns.
}
