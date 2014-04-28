/*
 * lcd_lib.h
 *
 * Created: 3/31/2014 11:15:30 PM
 *  Author: Gabi.Kazav
 */ 


/*
0x30 8-bit 1 line 5x7 dots
0x38 8-bit 2 line 5x7 dots
0x20 4-bit 1 line 5x7 dots
0x28 4-bit 2 line 5x7 dots
0x06 entry mode
0x08 display off cursor off
0x0E display on cursor on
0x0C display on cursor off
0x0F display on cursor blink
0x18 shift left
0x1C shift right
0x10 move left 1 char
0x14 move right 1 char
0x01 clear
0x80+X set cursor position

Connection: D[7-4]-PD[7-4], E-PD2, RW-PD1, RS-PD0

GDRAM:
0-3: special on character
4: curret hebrew char
*/

#ifndef LCD_LIB_H_
#define LCD_LIB_H_
#pragma once
#define F_CPU 1000000

#include "lcd_lib.h"
#include <avr/io.h>
#include <util/delay.h>

void lcd_init(void);
void lcd_check_BF(void);
void pulse_enable(void);
void gabi_inst(uint8_t , uint8_t );
void gabi_cmd(uint8_t );
void gabi_data(uint8_t );
void gabi_home(void);
void gabi_goto(uint8_t , uint8_t );
void gabi_string(char*);


// LCD interface (should agree with the diagram above)
#define LCD_DDR DDRD
#define LCD_PORT PORTD
#define lcd_D7_port     PORTD                   // lcd D7 connection
#define lcd_D7_bit      PORTD7
#define lcd_D7_ddr      DDRD
#define lcd_D7_pin      PIND                    // busy flag

#define lcd_E_port      PORTD                   // lcd Enable pin
#define lcd_E_bit       PORTD2
#define lcd_E_ddr       DDRD

#define lcd_RS_port     PORTD                   // lcd Register Select pin
#define lcd_RS_bit      PORTD0
#define lcd_RS_ddr      DDRD

#define lcd_RW_port     PORTD                   // lcd Read/Write pin
#define lcd_RW_bit      PORTD1
#define lcd_RW_ddr      DDRD

// LCD module information
#define lcd_LineOne     0x00                    // start of line 1
#define lcd_LineTwo     0x40                    // start of line 2

// LCD instructions
#define lcd_Clear           0b00000001          // replace all characters with ASCII 'space'
#define lcd_Home            0b00000010          // return cursor to first position on first line
#define lcd_EntryMode       0b00000110          // shift cursor from left to right on read/write
#define lcd_DisplayOff      0b00001000          // turn display off
#define lcd_DisplayOn       0b00001111          // display on, cursor on, blink character (change last 00 to 11 to show cursor and blink)
#define lcd_FunctionReset   0b00110000          // reset the LCD
#define lcd_FunctionSet4bit 0b00101000          // 4-bit data, 2-line display, 5 x 7 font
#define lcd_SetCursor       0b10000000          // set cursor position
#define LCD_SETDDRAMADDR 0x80

#define RS_LOW lcd_RS_port &= ~(1<<lcd_RS_bit)
#define RS_HIGH lcd_RS_port |= (1<<lcd_RS_bit)
#define E_LOW lcd_E_port &= ~(1<<lcd_E_bit)
#define E_HIGH lcd_E_port |= (1<<lcd_E_bit)
#define RW_LOW lcd_RW_port &= ~(1<<lcd_RW_bit)
#define RW_HIGH lcd_RW_port |= (1<<lcd_RW_bit)

char special_char[4][8] = {
	{ 18 , 9  , 18 , 8 , 17 , 17 , 17 , 31 }, 
	{ 9  , 18 , 9  , 2 , 17 , 17 , 31 , 31 }, 
	{ 18 , 9  , 18 , 8 , 17 , 31 , 31 , 31 }, 
	{ 9  , 18 , 9  , 0 , 31 , 31 , 31 , 31 }
};

char hebrew[31][8] = {
	{19, 25, 13, 22, 19, 17, 25, 0 }, // Alef = 0
	{30, 30, 2,  2,  2,  31, 31, 0 }, // Bet = 1
	{31, 31, 1,  31, 31, 17, 17, 0 }, // Gimel = 2
	{31, 31, 2,  2,  2,  2,  2,  0 }, // Dalet = 3
	{31, 31, 1,  1,  17, 17, 17, 0 }, // He = 4
	{3,  3,  1,  1,  1,  1,  1,  0 }, // Vav = 5
	{31, 31, 4,  6,  2,  6,  4,  0 }, // Zain = 6
	{31, 31, 17, 17, 17, 17, 17, 0 }, // Het = 7
	{19, 23, 21, 17, 17, 31, 31, 0 }, // Tet = 8
	{3,  3,  1,  0,  0,  0,  0,  0 }, // Yod = 9
	{31, 31, 1,  1,  1,  31, 31, 0 }, // Kaf = 10
	{16, 31, 31, 1,  1,  2,  28, 0 }, // Lamed = 11
	{16, 28, 30, 19, 17, 17, 23, 0 }, // Mem = 12
	{7,  7,  1,  1,  1,  31, 31, 0 }, // Nun = 13
	{31, 31, 17, 17, 17, 17, 14, 0 }, // Sameh = 14
	{27, 27, 9,  13, 5,  31, 31, 0 }, // Ain = 15
	{31, 31, 9,  13, 1,  31, 31, 0 }, // Pe = 16
	{27, 13, 6,  3,  1,  31, 31, 0 }, // Zadik = 17
	{31, 31, 1,  1,  17, 19, 22, 16}, // Kuf = 18
	{31, 31, 1,  1,  1,  1,  1,  0 }, // Resh = 19
	{21, 21, 21, 29, 17, 31, 31, 0 }, // Shin = 20
	{15, 15, 9,  9,  9,  25, 25, 0 }, // Taf = 21
	{15, 15, 1,  1,  1,  1,  1,  1 }, // Final Kaf = 22
	{31, 31, 17, 17, 17, 31, 31, 0 }, // Final Mem = 23
	{3,  3,  1,  1,  1,  1,  1,  1 }, // Final Nun = 24
	{31, 31, 17, 25, 25, 1,  1,  1 }, // Final Pe = 25
	{27, 9,  13, 6,  3,  1,  1,  1 }, // Final Zadik = 26	
	{ 18 , 9  , 18 , 8 , 17 , 17 , 17 , 31 }, // Special 0
	{ 9  , 18 , 9  , 2 , 17 , 17 , 31 , 31 }, // Special 1
	{ 18 , 9  , 18 , 8 , 17 , 31 , 31 , 31 }, // Special 2
	{ 9  , 18 , 9  , 0 , 31 , 31 , 31 , 31 }  // Special 3					
};

#define ALEF 0
#define BET 1
#define GIMEL 2
#define DALET 3
#define HEY 4
#define VAV 5
#define ZAIN 6
#define HET 7
#define TET 8
#define YOD 9
#define KAF 10
#define LAMED 11
#define MEM 12
#define NUN 13
#define SAMEH 14
#define AIN 15
#define PEH 16
#define ZADIK 17
#define KUF 18
#define RESH 19
#define SHIN 20
#define TAF 21
#define FINAL_KAF 22
#define FINAL_MEM 23
#define FINAL_NUN 24
#define FINAL_PEH 25
#define FINAL_ZADIK 26
#define SPECIAL_START 27

void lcd_init(void)
{
	_delay_ms(500); // wait
	LCD_DDR = 0xFF;
	LCD_PORT = 0xFF;
	RS_LOW;
	E_LOW;
	RW_LOW;
	gabi_inst(0,lcd_FunctionReset); //1 reset
	_delay_us(4500);	
	gabi_inst(0,lcd_FunctionReset); //2 reset
	_delay_us(150);
	gabi_inst(0,lcd_FunctionReset); //3 reset
	
	// 4bit mode
	lcd_check_BF();
	gabi_inst(0,lcd_FunctionSet4bit); //only high nimbble needed here
	lcd_check_BF();
	gabi_cmd(lcd_FunctionSet4bit); //now both nimbbles
	
	// display off
	lcd_check_BF();
	gabi_cmd(lcd_DisplayOff);
	
	// clear
	lcd_check_BF();
	gabi_cmd(lcd_Clear);

	// Entry mode	
	lcd_check_BF();
	gabi_cmd(lcd_EntryMode);

	// Display On
	lcd_check_BF();
	gabi_cmd(lcd_DisplayOn);
}

/*...........................................................................
  Name:     lcd_check_BF_4
  Purpose:  check busy flag, wait until LCD is ready
  Entry:    no parameters
  Exit:     no parameters
  Notes:    main program will hang if LCD module is defective or missing
            data is read while 'E' is high
            both nibbles must be read even though desired information is only in the high nibble
*/
void lcd_check_BF(void)
{
	// busy flag 'mirror'
    uint8_t busy_flag_copy;                         

	// set D7 data direction to input
    lcd_D7_ddr &= ~(1<<lcd_D7_bit);  
	// select the Instruction Register (RS low)               
    RS_LOW;    
	// read from LCD module (RW high)            
    RW_HIGH;                 

    do
    {
        busy_flag_copy = 0;          
		// Enable pin high               
        E_HIGH;    
		// implement 'Delay data time' (160 nS) and 'Enable pulse width' (230 nS)           
        _delay_us(1);                               

		// get actual busy flag status
        busy_flag_copy |= (lcd_D7_pin & (1<<lcd_D7_bit));  

		// Enable pin low
        E_LOW;             
		// implement 'Address hold time' (10 nS), 'Data hold time' (10 nS), and 'Enable cycle time' (500 nS ) 
        _delay_us(1);                               
        
		// read and discard alternate nibbles (D3 information)
		// Enable pin high
        E_HIGH;      
		// implement 'Delay data time' (160 nS) and 'Enable pulse width' (230 nS)         
        _delay_us(1);  
		// Enable pin low                             
        E_LOW;             
		// implement 'Address hold time (10 nS), 'Data hold time' (10 nS), and 'Enable cycle time' (500 nS ) 
        _delay_us(1);                               
        
    } while (busy_flag_copy); // check again if busy flag was high

	// arrive here if busy flag is clear -  clean up and return 
	// write to LCD module (RW low)
    RW_LOW;              
	// reset D7 data direction to output  
    lcd_D7_ddr |= (1<<lcd_D7_bit);                  
}

void pulse_enable()
{
  E_LOW;
  _delay_us(1);
  E_HIGH;
  _delay_us(1);
  E_LOW;
  _delay_us(100);
}


void gabi_inst(uint8_t RS_pin, uint8_t data) // rs_pin 1 for data , 0 for instructions
{
	if (RS_pin > 0) RS_HIGH; else RS_LOW;
    RW_LOW;
	LCD_DDR |= 0xF0;
	uint8_t d5,d6,d7,d8;
	d5=0|(0b00010000&data);
	d6=0|(0b00100000&data);
	d7=0|(0b01000000&data);
	d8=0|(0b10000000&data);
	LCD_PORT&=0x0F;
	LCD_PORT|=d5;
	LCD_PORT|=d6;
	LCD_PORT|=d7;
	LCD_PORT|=d8;
	pulse_enable();
}

void gabi_cmd(uint8_t data)
{
	gabi_inst(0,data);
	gabi_inst(0,data<<4);
}

void gabi_data(uint8_t data)
{
	gabi_inst(1,data);
	gabi_inst(1,data<<4);
}

void gabi_home()
{
	lcd_check_BF();
	gabi_cmd(0x02);
	_delay_us(2000);
}

void gabi_clear()
{
	lcd_check_BF();
	gabi_cmd(lcd_Clear);
}

void gabi_goto(uint8_t col, uint8_t row)
{
	lcd_check_BF();
	int row_offsets[] = {0x00, 0x40};
	gabi_cmd(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

void gabi_string(char *data)
{
	lcd_check_BF();
	while (*data > 0)
	{
		gabi_data(*data);
		data++;
	}
}

void add_special_char(int pos,  char *data)
{
	gabi_cmd(0x40 + (pos*8));
	for (int i=0; i<8; i++)
		gabi_data(data[i]);
	//gabi_cmd(0x80);
}

void gabi_heb(int x, int y, int len, int* heb_phase)
{
	if (len <= 7)
	{
		for (int i=len-1 ; i>=0 ; i--)
		{
			add_special_char(i+1,hebrew[heb_phase[i]]);
			gabi_goto(x++,y);
			gabi_data(i+1);	
		}
	}
}


 #endif /* LCD_LIB_H_ */
