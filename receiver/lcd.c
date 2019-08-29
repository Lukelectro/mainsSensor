#include "lcd.h"
#include <avr/io.h>

/* 
   The following 2 functions are not called directly but
   via the macro's _lcd_delay_us_small or _lcd_delay_us which 
   calculate the parameter t using the delay in us and the 
   microcontroller clock frequence F_CPU in Hz.

   _lcd_delay_us_small can be used for us <=47 (for worst case F_CPU == 16000000)
   _lcd_delay_us can be used for us <=16128 (for worst case F_CPU == 16000000)

   These functions and macro's are written by Jesse.
*/
#ifdef JesseDelay
/* One byte delay loop, one loop costs 3 cycles. */
void _lcd_delay_8(uint8_t t) {
	asm volatile (	"\n"
					"L_dl0%=:	subi %0,1" 		"\n\t"
					"			brcc L_dl0%=" 	"\n\t"
					:: "r" (t));
}

/* Two byte delay loop, one loop costs 4 cycles. */
void _lcd_delay_16(uint16_t t) {
	asm volatile (	"\n"
					"L_dl1%=:	subi %A0,1" 		"\n\t"
					"			sbci %B0,0" 		"\n\t"
					"			brcc L_dl1%=" 	"\n\t"
					:: "r" (t));
}
#else
#define F_CPU 16e6
#include <util/delay.h>
#define _lcd_delay_us_small _delay_us
#define _lcd_delay_us _delay_us
#endif

// LCD is connected on PORTD in 4bit mode (Was PORTA in original library)
// Databus is connected to high nibble
// PA4 == DB4
// PA5 == DB5
// PA6 == DB6
// PA7 == DB7
// Controlbus is connected to low nibble
// configuration can be changed by the following defines
// PA0 is Not Connected and is NOT AFFECTED by this LCD software
#define _LCD_E PD3
#define _LCD_RW PD2
#define _LCD_RS PD1

uint8_t lcd_read_command(void) {
	uint8_t command;
	DDRD|=1<<_LCD_E|1<<_LCD_RW|1<<_LCD_RS; // control bus output
	DDRD&=~0xF0; // databus input
	PORTD|=1<<_LCD_RW|0xF0; // R/!W= 1 (Read) and Pullup inputs 
	PORTD&=~(1<<_LCD_RS); // RS=0
	_lcd_delay_us_small(2);
	PORTD|=1<<_LCD_E; // E=1
	_lcd_delay_us_small(1);
	command=PIND&0xF0; // read high nibble
	PORTD&=~(1<<_LCD_E); // E=0
	_lcd_delay_us_small(2);
	PORTD|=1<<_LCD_E; // E=1
	_lcd_delay_us_small(1);
	command|=PIND>>4; // read low nibble
	PORTD&=~(1<<_LCD_E); // E=0
	return command;
}

uint8_t lcd_read_data(void) {
	uint8_t data;
	DDRD|=1<<_LCD_E|1<<_LCD_RW|1<<_LCD_RS;
	DDRD&=~0xF0;
	PORTD|=1<<_LCD_RW|1<<_LCD_RS|0xF0; // R/!W= 1 (Read) RS=1 and Pullup inputs 
	_lcd_delay_us_small(2);
	PORTD|=1<<_LCD_E;
	_lcd_delay_us_small(1);
	data=PIND&0xF0;
	PORTD&=~(1<<_LCD_E);
	_lcd_delay_us_small(2);
	PORTD|=1<<_LCD_E;
	_lcd_delay_us_small(1);
	data|=PIND>>4;
	PORTD&=~(1<<_LCD_E);
	return data;
}

static void wait(void) {
	while (lcd_read_command()&0x80);
}
	
static void pos_pulse_E(void) {
	_lcd_delay_us_small(2);
	PORTD|=1<<_LCD_E; // E=1
	_lcd_delay_us_small(1);
	PORTD&=~(1<<_LCD_E); // E=0
}

void lcd_write_command(uint8_t command) {
	wait();
	DDRD|=1<<_LCD_E|1<<_LCD_RW|1<<_LCD_RS|0xF0; // controlbus and databus output
	PORTD&=~(1<<_LCD_E|1<<_LCD_RW|1<<_LCD_RS|0xF0);
	PORTD|=command&0xF0; // all control signals low RS=0 R/!W=0
	pos_pulse_E();// write high nibble
	PORTD&=~(1<<_LCD_E|1<<_LCD_RW|1<<_LCD_RS|0xF0);
	PORTD|=command<<4; // all control signals low RS=0 R/!W=0
	pos_pulse_E();// write low nibble
}

void lcd_write_data(uint8_t data) {
	wait();
	DDRD|=1<<_LCD_E|1<<_LCD_RW|1<<_LCD_RS|0xF0;
	PORTD&=~(1<<_LCD_E|1<<_LCD_RW|1<<_LCD_RS|0xF0);
	PORTD|=(data&0xF0)|1<<_LCD_RS; // RS=1 other control signals 0 R/!W=0
	pos_pulse_E();
	PORTD&=~(1<<_LCD_E|1<<_LCD_RW|1<<_LCD_RS|0xF0);
	PORTD|=(data<<4)|1<<_LCD_RS;
	pos_pulse_E();
}

void lcd_init(void) {
	DDRD|=1<<_LCD_E|1<<_LCD_RW|1<<_LCD_RS|0xF0;
	_lcd_delay_us(15000);
	PORTD&=~(1<<_LCD_E|1<<_LCD_RW|1<<_LCD_RS|0xF0); 
	PORTD|=0x30; // all control signals low RS=0 R/!W=0
	pos_pulse_E(); 
	_lcd_delay_us(4100);
	pos_pulse_E(); 
	_lcd_delay_us(100);
	pos_pulse_E(); 
	_lcd_delay_us(100);
	PORTD&=~0x10; 
	pos_pulse_E(); 
	_lcd_delay_us(100);
	lcd_write_command(0x28);
	lcd_write_command(0x06);
	lcd_cls();
	lcd_cursor(true, true);
}

void lcd_cls(void) {
	lcd_write_command(0x01);
}

void lcd_home(void) {
	lcd_write_command(0x02);
}

void lcd_cursor(bool cursorOn, bool cursorBlinks) { 
	lcd_write_command(0x0C+(cursorOn?0x02:0)+(cursorBlinks?0x01:0));
}

void lcd_goto(uint8_t row, uint8_t column) {
	if (row<2 && column<20) { 
		lcd_write_command(0x80+row*0x40+column);
	}
    // 4x20 displays are usualy implemented as 2x40
    if (row>1 && column<20) { 
		lcd_write_command(0x80+(row-2)*0x40+(column+20));
	}
}

uint8_t lcd_get_row(void) {
	wait();
	return lcd_read_command()&1<<6 ? 1 : 0;

}

uint8_t lcd_get_column(void) {
	wait();
	return lcd_read_command()&0x3f;
}

void lcd_putc(char data) { 
	lcd_write_data(data);
}

void lcd_puts(char *s) {
    while(*s) {
        lcd_putc(*s++);
    }
}

void lcd_puts_P(const char* PROGMEM s) {
	char c; 
	while ((c = pgm_read_byte(s++))) 
		lcd_putc(c);
}
