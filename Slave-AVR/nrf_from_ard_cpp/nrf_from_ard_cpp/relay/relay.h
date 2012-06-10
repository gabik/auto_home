/*
 * relay.h
 *
 * Created: 4/6/2014 11:13:40 AM
 *  Author: Gabi.Kazav
 */ 


#ifndef RELAY_H_
#define RELAY_H_

#include "../lcd/lcd_lib.h"

#define RELAY1_EMR_DDR DDRC
#define RELAY1_EMR_PORT PORTC
#define RELAY1_EMR_PIN PORTC2
#define RELAY2_EMR_DDR DDRC
#define RELAY2_EMR_PORT PORTC
#define RELAY2_EMR_PIN PORTC3
#define RELAY2_SSR_DDR DDRC
#define RELAY2_SSR_PORT PORTC
#define RELAY2_SSR_PIN PORTC4
#define RELAY_DELAY 50

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
	update_lcd_relay_state();
}

void relays_power_off()
{
	RELAY2_SSR_PORT |= (1<<RELAY2_SSR_PIN);
	_delay_ms(RELAY_DELAY);
	RELAY2_EMR_PORT &= ~(1<<RELAY2_EMR_PIN);
	_delay_ms(RELAY_DELAY);
	RELAY2_SSR_PORT &= ~(1<<RELAY2_SSR_PIN);
	_delay_ms(RELAY_DELAY);
	RELAY1_EMR_PORT &= ~(1<<RELAY1_EMR_PIN);
	update_lcd_relay_state();
}

void relays_power_on()
{
	RELAY1_EMR_PORT |= (1<<RELAY1_EMR_PIN);
	_delay_ms(RELAY_DELAY);
	RELAY2_SSR_PORT |= (1<<RELAY2_SSR_PIN);
	_delay_ms(RELAY_DELAY);
	RELAY2_EMR_PORT |= (1<<RELAY2_EMR_PIN);
	_delay_ms(RELAY_DELAY);
	RELAY2_SSR_PORT &= ~(1<<RELAY2_SSR_PIN);
	update_lcd_relay_state();
}

#endif /* RELAY_H_ */