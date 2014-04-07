/*
Module		:	Slave
Type		:	Boiler Module.
Firmware	:	1.7
*/
#define META_MODULE "Slave"
#define META_TYPE "Boiler"
#define META_FIRMWARE "1.7"

#define F_CPU 1000000

#include "nrf/nRF24L01.h"
#include "nrf/RF24.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "lcd/lcd_lib.h"
#include "spi/spi.h"
#include "timers/timers.h"
#include "relay/relay.h"
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

// commands:
//  send stat   seq- Id-- r/w                         .			 .=On/Off (1/0)
//  send time   seq- Id-- r/w               | ||.. ....			 |=hours, .=minutes
//  to me       seq- Id-- r/w                      -cmd
//              3322 2222 2222 1111 1111 11 
//              1098 7654 3210 9876 5432 1098 7654 3210
// payload=4 0x 0000 0000 0000 0000 0000 0000 0000 0000

#define W 0x1 // W = 0001
#define ON 0x1 // ON = 0001
#define OFF 0x0 // OFF = 0000

#define R 0x0 // R = 0000
#define PWR_STT 0x0 // Read Power State = ....0000
#define P_TIME 0b0110 // Read Power On Timer 
#define R_STATUS 0xF // Alive status = ....1111

#define DELAY_LCD_PRINT 1200
#define BUTTON_DDR DDRB
#define BUTTON_PORT PORTB
#define BUTTON_PIN PB5
#define BUTTON_INPORT PINB5
#define DEBOUNCE_TIME 100

// Hardware configuration

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10
RF24 radio(0,0);
int seq, pressed_counter, release_counter, button_last;

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xf0f0f0f0e1, 0xf0f0f0f0d2 };

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
	print_full_char(5);
	gabi_goto(5,0);
	gabi_string((char*)"Welcome");
	print_full_char(4);
	gabi_goto(5,1);
	print_full_char(5);
	gabi_string((char*)"Gabi");
	print_full_char(7);
	_delay_ms(DELAY_LCD_PRINT);
	gabi_clear();	
	gabi_string((char*)META_TYPE);
	gabi_data(' ');
	gabi_string((char*)META_MODULE);
	gabi_string((char*)" Mod");
	gabi_goto(0,1);
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
}

int check_button_pressed()
{
	if (BUTTON_INPORT & (1<<BUTTON_PIN))
	{
		pressed_counter=0;
		release_counter++;
	} else {
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

	print_read_write(W,pkg);

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
			} else {
				relays_power_on();
			}
			button_last=1;
		}
	} else {
		button_last=0;
	}
	
	// Handle LM35
	if (timer1_fire)
	{
		update_lcd_lm35_print();
		timer1_fire=0;
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
		print_read_write(R,got_pkg);
// 		 
//   		char data_str[8];
//   		itoa(cur_data,data_str,16);
// 		gabi_string(data_str);
// 		
// 		gabi_goto(4,1);
// 		char rw_str[8];
// 		itoa(cur_rw,rw_str,16);
// 		gabi_string(rw_str);
// 		gabi_string((char*)" ");
// 		
// 		char id_str[8];
// 		itoa(cur_id,id_str,16);
// 		gabi_string(id_str);

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
									start_timer2();
									write_data(got_pkg);
									reset_seq();
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
// 								case P_TIME_MIN:
// 									read_val=p_min;
// 									write_data((seq << SEQ_BIT) | (R) | (read_val));
// 									reset_seq();
// 									break;
								case R_STATUS: // status - alive
									read_val = 1;
									tmp_payload = (((uint32_t)(seq & SEQ_MASK) << SEQ_BIT) | ((uint32_t)(MY_ID & ID_MASK) << ID_BIT) | ((uint32_t)(R & RW_MASK) << RW_BIT) | ((uint32_t)(read_val & STAT_MASK) << STAT_BIT));
									write_data(tmp_payload);
									reset_seq();
									break;
								default:
									break;
							}							
						}
// 						write_data(got_pkg);
// 						seq++;
					}
					break;
//				case 2:
				// HERE the stuff going --- First Write, Else Read.
//				unsigned long cmd;
//				cmd = (got_pkg & CMD_MASK);
// 				if (got_pkg & W) // Write if true, Read false
// 				{
// 					switch (cmd) {
// 						case OFF:
// 							write_data(got_pkg);
// 							stop_timer2();
// 							reset_seq();
// 							relays_power_off();
// 							break;
// 						case ON:
// 							start_timer2();
// 							write_data(got_pkg);
// 							reset_seq();
// 							relays_power_on();
// 							break;
// 						default:
// 							break;
// 					}
// 				}
// 				else
// 				{
// 					uint8_t read_val;
// 					switch (cmd) {
// 						case PWR_STT:
// 							read_val = get_relay1_emr_state();
// 							write_data((seq << SEQ_BIT) | (R) | (read_val));
// 							reset_seq();
// 							break;
// 						case P_TIME_HOUR:
// 							read_val=p_hour;
// 							write_data((seq << SEQ_BIT) | (R) | (read_val));
// 							reset_seq();
// 							break;
// 						case P_TIME_MIN:
// 							read_val=p_min;
// 							write_data((seq << SEQ_BIT) | (R) | (read_val));
// 							reset_seq();
// 							break;
// 						case R_STATUS: // status - alive
// 							read_val = 1;
// 							write_data((seq << SEQ_BIT) | (R) | (read_val));
// 							reset_seq();
// 							break;
// 						default:
// 							break;
// 					}
// 				}
// 				break;
// 				default:
// 				break;
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
