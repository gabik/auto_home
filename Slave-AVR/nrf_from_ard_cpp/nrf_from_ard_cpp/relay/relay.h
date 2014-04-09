/*
 * relay.h
 *
 * Created: 4/6/2014 11:13:40 AM
 *  Author: Gabi.Kazav
 */ 


#ifndef RELAY_H_
#define RELAY_H_

#include <avr/io.h>
#include "../lcd/lcd_lib.h"

#define RELAY1_EMR_DDR DDRC
#define RELAY1_EMR_PORT PORTC
#define RELAY1_EMR_PIN 2
#define RELAY2_EMR_DDR DDRC
#define RELAY2_EMR_PORT PORTC
#define RELAY2_EMR_PIN 3
#define RELAY2_SSR_DDR DDRC
#define RELAY2_SSR_PORT PORTC
#define RELAY2_SSR_PIN 4
#define RELAY_DELAY 50

#define PIEZO_DDR DDRB
#define PIEZO_PORT PORTB
#define PIEZO_PIN 7
#define tone_c 5000 //3830
#define tone_d 4500 //3400
#define tone_e 4000 //3038
#define tone_f 3500 //2864
#define tone_g 3000 //2550
#define tone_a 2500 //2272
#define tone_b 2000 //2028
#define tone_num 7
#define tone_tempo 20000
#define tone_pause 100
int melody[tone_num] = { tone_c, tone_d, tone_e, tone_f, tone_g, tone_a, tone_b};

#define _NOP() do { __asm__ __volatile__ ("nop"); } while (0) 
	
void tone_delay_us(long n)
{
	while(n--)
		_delay_us(1);
}
	
void play_one_tone(int n)
{
	long tone_elpased_time = 0;
	while (tone_elpased_time < 10)//(tone_tempo))
	{
		long delay_tone = (long)(n/8);
		PIEZO_PORT|=(1<<PIEZO_PIN);
		tone_delay_us(delay_tone);
		PIEZO_PORT&=~(1<<PIEZO_PIN);
		tone_delay_us(delay_tone);
		tone_elpased_time++;
	}
	_delay_us(tone_pause);
}

void play_tone(int up_down)
{
	if (up_down == 1) for (int i=0 ; i<tone_num; i++) play_one_tone(melody[i]);
	else for (int i=tone_num-1 ; i>=0; i--) play_one_tone(melody[i] - 500);
}

uint8_t get_relay1_emr_state()
{
	return ((RELAY1_EMR_PORT >> RELAY1_EMR_PIN) & 1);
}

void update_lcd_relay_state()
{
	gabi_goto(0,0);
	if (get_relay1_emr_state()) gabi_string((char*)"On "); else gabi_string((char*)"Off");
}

void init_relays()
{
	RELAY1_EMR_DDR |= (1<<RELAY1_EMR_PIN);
	RELAY2_EMR_DDR |= (1<<RELAY2_EMR_PIN);
	RELAY2_SSR_DDR |= (1<<RELAY2_SSR_PIN);
	RELAY1_EMR_PORT&=~(1<<RELAY1_EMR_PIN);
	RELAY2_EMR_PORT&=~(1<<RELAY2_EMR_PIN);
	RELAY2_SSR_PORT&=~(1<<RELAY2_SSR_PIN);
	PIEZO_DDR |= (1<<PIEZO_PIN);
	PIEZO_PORT&=~(1<<PIEZO_PIN);
	update_lcd_relay_state();
}

void relays_power_off()
{
	TIMSK1 &= ~(1 << OCIE1A);
	RELAY2_SSR_PORT |= (1<<RELAY2_SSR_PIN);
	_delay_ms(RELAY_DELAY);
	RELAY2_EMR_PORT &= ~(1<<RELAY2_EMR_PIN);
	_delay_ms(RELAY_DELAY);
	RELAY2_SSR_PORT &= ~(1<<RELAY2_SSR_PIN);
	_delay_ms(RELAY_DELAY);
	RELAY1_EMR_PORT &= ~(1<<RELAY1_EMR_PIN);
	TIMSK1 |= (1 << OCIE1A);
	update_lcd_relay_state();
	play_tone(0);
}

void relays_power_on()
{
	TIMSK1 &= ~(1 << OCIE1A);
	RELAY1_EMR_PORT |= (1<<RELAY1_EMR_PIN);
	_delay_ms(RELAY_DELAY);
	RELAY2_SSR_PORT |= (1<<RELAY2_SSR_PIN);
	_delay_ms(RELAY_DELAY);
	RELAY2_EMR_PORT |= (1<<RELAY2_EMR_PIN);
	_delay_ms(RELAY_DELAY);
	RELAY2_SSR_PORT &= ~(1<<RELAY2_SSR_PIN);
	TIMSK1 |= (1 << OCIE1A);
	update_lcd_relay_state();
	play_tone(1);
}

#endif /* RELAY_H_ */