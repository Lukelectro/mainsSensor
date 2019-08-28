/*
//  lcd.h 
//
//		Routines to write to Hitachi HD44780 compatible LCD driver 
//
//      (c) 2006, Harry Broeders <J.Z.M.Broeders@hhs.nl>
//      Version 1.0
//
//		This software no longer uses the delay routines written by:
//
//      (c) 2006, Jesse op den Brouw <J.E.J.opdenBrouw@hhs.nl>
//      Version 1.0
*/

#ifndef _LCD_H_
#define _LCD_H_

#include <stdint.h>
#include <stdbool.h>
#include <avr/pgmspace.h>

/* All programs that want to use the LCD should
   start by calling lcd_init() */

void lcd_init(void); /* initialize LCD */

/* low level API (Application Programmers Interface*/

uint8_t lcd_read_data(void); 
void lcd_write_data(uint8_t data);
uint8_t lcd_read_command(void);
void lcd_write_command(uint8_t command);

/* somewhat higher level API */

void lcd_cls(void); /* clear screen */
void lcd_home(void); /* place cursor in home position (upper left corner) */
void lcd_cursor(bool cursorOn, bool cursorBlinks); /* configure cursor */
void lcd_goto(uint8_t row, uint8_t column); /* place cursor on row (0..1) and column (0..39) */
uint8_t lcd_get_row(void); /* get current cursor row position (0..1) */
uint8_t lcd_get_column(void); /* get current cursor column position (0..39) */

void lcd_putc(char c); /* write character c */
void lcd_puts(char* s); /* write string s */
void lcd_puts_P(const char* PROGMEM s); /* write string s from program space */
#endif

