/*
Module		:	Slave
Type		:	Boiler Module.
Firmware	:	2.01
Date		:	07/07/14

ToDO:
V 1.79	60 minutes a hour instead 61
V 1.8	auto power down on p_hour >= 2
V 1.81	fix minutes to be 2 digits on uptime clock
V 1.82	On will not restart clock if already on
V 1.9	get temperture in C
V 1.91	handle movement sensor to light the screen (as well as relay on/off will light the LCD)
V 1.92	SSR-safe. check every 30 seconds that SSR is off - dont forget to handle poweron/off SSR so it will never be together (can use cli() sei())
V 1.93	Piezo on power on/off
V 1.94	turn off lcd blinking and think about pixel(15,0), maybe use /-\| every second or something
V 2.0	test all and remove print_read_write and printDetails on RPi
V 2.01	Special char for ON
V 2.011 Hebrew chars
V 2.012 Hebrew - Print on start the Welcome
X 2.02	save last DDRAM pos and back to is after adding CGRAM char (instead of 0x80)
  2.021 Change DEBUG to second line alone (leave first line)
  2.022 Change On Off to Hebrew, and move SPECIAL CHAR to 0,0
  2.03	turn off lcd blinking , test all, and turn DEBUG off.
*/
#define META_MODULE "Slave"
#define META_TYPE "Boiler"
#define META_FIRMWARE "2.01"

#define F_CPU 1000000

int overflow_count;

#include "nrf/nRF24L01.h"
#include "nrf/RF24.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "lcd/lcd_lib.h"
#include "spi/spi.h"
#include "relay/relay.h"
#include "timers/timers.h"
#include "misc/sensors.h"


void print_reg(char* name, uint8_t reg);
void print_status();
void print_byte(uint8_t byte);
void print_RX_ADDR(uint8_t reg);
void print_read_write(int read_write, unsigned long pkg);

#define MY_ID 12

#define SEQ_BIT 28
#define RW_BIT 23
#define ID_BIT 24
#define CMD_BIT 0

#define CMD_MASK 0xF
#define ID_MASK 0xF
#define RW_MASK 1
#define SEQ_MASK 0xF

#define MINS_BIT 0
#define HOUR_BIT 6
#define MINS_MASK 0x3F
#define HOUR_MASK 0x7

#define STAT_BIT 0
#define STAT_MASK 1

#define TEMP_MASK 0x7F // 7 bit
#define TEMP_BIT 0

// commands:
//	send temp	seq- id-- r/w                  ... ....			.=Temperture (0-127)
//  send stat   seq- Id-- r/w                         .			.=On/Off (1/0)
//  send time   seq- Id-- r/w               | ||.. ....			|=hours, .=minutes
//  to me       seq- Id-- r/w                      -cmd
//              3322 2222 2222 1111 1111 11 
//              1098 7654 3210 9876 5432 1098 7654 3210
// payload=4 0x 0000 0000 0000 0000 0000 0000 0000 0000

#define W 0x1 // W = 0001
#define ON 0x1 // ON = 0001
#define OFF 0x0 // OFF = 0000

#define R			0x0		// R = 0000
#define PWR_STT		0x0		// Read Power State = ....0000
#define P_TIME		0b0110	// Read Power On Timer  ....0110
#define R_STATUS	0xF		// Alive status = ....1111
#define R_TEMP		0b1000	// Get temperture = ....1000

#define DELAY_LCD_PRINT 1500
#define BUTTON_DDR DDRC
#define BUTTON_PORT PORTC
#define BUTTON_PIN 5
#define BUTTON_IN PINC
#define DEBOUNCE_TIME 300

// Hardware configuration

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10
RF24 radio(0,0);
int seq, pressed_counter, release_counter, button_last;
int DEBUG_BIT;

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xf0f0f0f0e1, 0xf0f0f0f0d2 };

// int toggle;
// void toggle_dot()
// {
// 	gabi_goto(15,0);
// 	if (toggle) gabi_data('.');
// 	else gabi_data('|');
// 	toggle^=1;
// }

void reset_seq(){
	seq=1;
}

void print_full_char(int n)
{
	for (int i=0; i<n; i++) gabi_data(0xFF);
}

void print_startup()
{
	gabi_clear();
	_delay_ms(20);
// 	for (int i=0 ; i<27 ; i++)
// 	{
// 		int x=i;
// 		int y=0;
// 		if (i>=16) {
// 			x=i-16;
// 			y=1;
// 		}
// 		add_special_char(5,hebrew[i]);
// 		gabi_goto(x,y);
// 		gabi_data(5);
// 		//_delay_ms(500);
// 	}
// 	gabi_goto(12,1);
// 	gabi_data(0);
// 	gabi_data(1);
// 	gabi_data(2);
// 	gabi_data(3);
// 	_delay_ms(DELAY_LCD_PRINT*2);
//	gabi_home();
	//gabi_goto(0,0);
	//gabi_clear();
	
	print_full_char(6);
	int heb_shalom[5] = {SHIN,LAMED,VAV,FINAL_MEM};
	int heb_gabi[4] = {GIMEL,BET,YOD};
	for (int i=0 ; i<3 ; i++)
	{
		add_special_char(i+5,hebrew[heb_gabi[i]]);
	}	
	gabi_heb(6,0,4,heb_shalom);
	//gabi_string((char*)"Welcome");
	print_full_char(6);
	
	//_delay_ms(DELAY_LCD_PRINT*2);
	gabi_home();
	gabi_goto(0,1);
	print_full_char(7);
	//gabi_heb(7,1,3,heb_gabi);
	int x=7;
	for (int i=2 ; i>=0 ; i--)
	{
		gabi_goto(x++,1);
		gabi_data(i+5);
	}	
	//gabi_string((char*)"Gabi");
	print_full_char(6);
	_delay_ms(DELAY_LCD_PRINT);
	
	gabi_home();
	working_char=0;
	for (int i=0 ; i<4 ; i++ ) 
	{
		update_working_char();
		_delay_ms(500);
	}
	
	gabi_clear();	
	gabi_string((char*)META_TYPE);
	gabi_data(' ');
	gabi_string((char*)META_MODULE);
	gabi_string((char*)" Mod");
	gabi_goto(0,1);
	gabi_string((char*)"Firmware: ");
	gabi_string((char*)META_FIRMWARE);
	_delay_ms(DELAY_LCD_PRINT);
	gabi_clear();
	char str_id[4];
	itoa(MY_ID,str_id,10);
	gabi_string((char*)"My ID: ");
	gabi_string(str_id);
	_delay_ms(DELAY_LCD_PRINT);
	gabi_clear();
	gabi_string((char*)"RX0:");
	print_RX_ADDR(RX_ADDR_P0);
	gabi_goto(0,1);
	gabi_string((char*)"RX1:");
	print_RX_ADDR(RX_ADDR_P1);
	_delay_ms(DELAY_LCD_PRINT);
	gabi_clear();
	gabi_string((char*)"TX :");
	print_RX_ADDR(TX_ADDR);
	gabi_goto(0,1);
	print_reg((char*)"EN_RX:",EN_RXADDR);
	_delay_ms(DELAY_LCD_PRINT);
	gabi_clear();
	print_reg((char*)"0:", RX_PW_P0);
	print_reg((char*)" 1:", RX_PW_P1);
	print_reg((char*)" 2:", RX_PW_P2);
	gabi_goto(0,1);
	print_reg((char*)"3:", RX_PW_P3);
	print_reg((char*)" 4:", RX_PW_P4);
	print_reg((char*)" 5:", RX_PW_P5);
	_delay_ms(DELAY_LCD_PRINT);
	gabi_clear();
	print_reg((char*)"Config:",CONFIG);
	gabi_goto(0,1);
	print_status();
	_delay_ms(DELAY_LCD_PRINT);
	gabi_clear();	
}

void setup(void)
{
	lcd_init();
	gabi_home();
// 	for (int i=0 ; i<4 ; i++) add_special_char(i,special_char[i]);	
// 	gabi_home();
	init_relays();
	// Setup and configure rf radio
	radio.begin();
	
	// increase the delay between retries & # of retries
	radio.setRetries(15,15);
	// reduce the payload size.  seems to improve reliability
	radio.setPayloadSize(4);
	radio.setChannel(107);
	radio.setPALevel(RF24_PA_MAX);

	// Open pipes to other nodes for communication
	radio.openWritingPipe(pipes[1]);
	radio.openReadingPipe(1,pipes[0]);

	// Start listening
	radio.startListening();
	
	reset_seq();

	setup_timers();
	init_ADC();
	BUTTON_DDR&=~(1<<BUTTON_PIN);
	BUTTON_PORT|=(1<<BUTTON_PIN);
	pressed_counter = 0;
	release_counter = 0;
	button_last=0;

DEBUG_BIT=1;	
	print_startup();
	start_timer1();
//	toggle=0;

}

int check_button_pressed()
{
	if (BUTTON_IN & (1<<BUTTON_PIN))
	{
		pressed_counter=0;
		release_counter++;
	} else {
//		toggle_dot();
		release_counter=0;
		pressed_counter++;
	}
	if (pressed_counter >= DEBOUNCE_TIME) return 1;
	return 0;
}


int write_data(unsigned long pkg) {
	_delay_ms(1);
	int  retry;
	long ret_val;
	ret_val = 0;
	retry = 0;
	radio.stopListening();
	while ((!ret_val) && (retry < 5))
	{
		ret_val = radio.write( &pkg, sizeof(unsigned long) );
		retry++;
		_delay_ms(20);
	}
	radio.startListening();

	if (DEBUG_BIT == 1) print_read_write(W,pkg);

	return ret_val;
}

void loop(void)
{
	// Button handle
	if (check_button_pressed())
	{
		if (button_last == 0)
		{
			if (get_relay1_emr_state())
			{
				relays_power_off();
				stop_timer2();
			} else {
				start_timer2();
				relays_power_on();
				update_lcd_clock_print();
			}
			button_last=1;
		}
	} else {
		button_last=0;
	}
	
	handle_PRI();
	
	// Handle LM35
	if (timer1_fire)
	{
		update_lcd_lm35_print();
		timer1_fire=0;
//		toggle_dot();
	}
		
	// if there is data ready
	if ( radio.available() )
	{
		unsigned long got_pkg;
		// Dump the payloads until we've gotten everything
		bool done = false;
		while (!done)
		{
			// Fetch the payload, and see if this was the last one.
			done = radio.read( &got_pkg, sizeof(unsigned long) );

			// Delay just a little bit to let the other unit
			// make the transition to receiver
			_delay_ms(20);
		}
		uint8_t cur_seq = ((got_pkg >> SEQ_BIT)&SEQ_MASK);
		uint8_t cur_data = ((got_pkg >> CMD_BIT) & CMD_MASK);
		uint8_t cur_rw = ((got_pkg >> RW_BIT) & RW_MASK);
		uint8_t cur_id = ((got_pkg >> ID_BIT) & ID_MASK);
		
		//write_data(got_pkg);
		if (DEBUG_BIT == 1) if (got_pkg != 0xFFFFFFFF) print_read_write(R,got_pkg);

		if (cur_seq == seq) {
			switch (seq) {
				case 1:
					if (cur_id == MY_ID)
					{
						// Write to me
						if (cur_rw & W)
						{
							switch (cur_data) {
								case OFF:
									write_data(got_pkg);
									stop_timer2();
									reset_seq();
									relays_power_off();
									break;
								case ON:
									write_data(got_pkg);
									reset_seq();
									start_timer2();
									relays_power_on();
									update_lcd_clock_print();
									break;
								default:
									break;		
							}
						}
						// Read from me
						else
						{
							uint8_t read_val;
							uint32_t tmp_payload;
							switch (cur_data) {
								case PWR_STT:
									read_val = get_relay1_emr_state();
									tmp_payload = (((uint32_t)(seq & SEQ_MASK) << SEQ_BIT) | ((uint32_t)(MY_ID & ID_MASK) << ID_BIT) | ((uint32_t)(R & RW_MASK) << RW_BIT) | ((uint32_t)(read_val & STAT_MASK) << STAT_BIT));
									write_data(tmp_payload);
									reset_seq();
									break;
								case P_TIME:
									//read_val=p_hour;
									tmp_payload = (((uint32_t)(seq & SEQ_MASK) << SEQ_BIT) | ((uint32_t)(MY_ID & ID_MASK) << ID_BIT) | ((uint32_t)(R & RW_MASK) << RW_BIT) | ((uint32_t)(p_hour & HOUR_MASK) << HOUR_BIT) | ((uint32_t)(p_min & MINS_MASK) << MINS_BIT));
									write_data(tmp_payload);
									reset_seq();
									break;
								case R_STATUS: // status - alive
									read_val = 1;
									tmp_payload = (((uint32_t)(seq & SEQ_MASK) << SEQ_BIT) | ((uint32_t)(MY_ID & ID_MASK) << ID_BIT) | ((uint32_t)(R & RW_MASK) << RW_BIT) | ((uint32_t)(read_val & STAT_MASK) << STAT_BIT));
									write_data(tmp_payload);
									reset_seq();
									break;
								case R_TEMP:  // get temperture
									tmp_payload = (((uint32_t)(seq & SEQ_MASK) << SEQ_BIT) | ((uint32_t)(MY_ID & ID_MASK) << ID_BIT) | ((uint32_t)(R & RW_MASK) << RW_BIT) | ((uint32_t)(tempC & TEMP_MASK) << TEMP_BIT));
									write_data(tmp_payload);
									reset_seq();
									break;
								default:
									break;
							}							
						}
					}
					break;
 			}
 		} else {
 			reset_seq();
		}
	}
}

int main() { setup(); while(1) loop(); return 0; }
	
// LCD functions

void print_read_write(int read_write, unsigned long pkg)
{
	char seq_str[10];
	if (read_write == R)
	{
  		gabi_goto(4,0);
	}
	else if (read_write == W)
	{
		gabi_goto(4,1);
	}
	ultoa(pkg,seq_str,16);
	gabi_string(seq_str);
}

void print_reg(char* name, uint8_t reg)
{
	gabi_string(name);
	uint8_t tmp_reg=radio.gabi_read_rf24(reg);
	char tmp_str[3];
	utoa(tmp_reg,tmp_str,16);
	gabi_string(tmp_str);
}

void print_status()
{
	uint8_t tmp_stt = radio.gabi_get_status();
	gabi_string((char*)"Status:");
	for (int i=7; i>=0 ; i--)
	{
		if (tmp_stt & (1<<i))
		gabi_data('1');
		else
		gabi_data('0');
	}
}

void print_byte(uint8_t byte)
{
	for (int i=7; i>=0 ; i--)
	{
		if (byte & (1<<i))
		gabi_data('1');
		else
		gabi_data('0');
	}
}

void print_RX_ADDR(uint8_t reg)
{
	uint8_t buffer[5];
	radio.gabi_read_rf24(reg,buffer,sizeof buffer);
	gabi_string((char*)"0x");
	for (int i=4 ; i>=0; i--)
	{
		uint8_t tmp_uint = buffer[i];
		char tmp_uint_str[3];
		utoa(tmp_uint,tmp_uint_str,16);
		gabi_string(tmp_uint_str);
	}
}
