/*
 * millis.h
 *
 * Created: 3/31/2014 3:55:13 PM
 *  Author: Gabi.Kazav
 */ 

#pragma once
#ifndef MILLIS1_H_
#define MILLIS1_H_

#include <avr/interrupt.h>
//#include "../misc/sensors.h"
//#include "../lcd/lcd_lib.h"
//#include <stdlib.h>

void millis_setup(void);
unsigned long millis(void);
uint64_t _millis;
//uint16_t _1000us = 0;

void millis_setup(void)
{
	  /* interrup setup */
	  // prescale timer0 to /8 prescale
	  // overflow timer0 every 2.048 ms (~=2ms)
	  _millis=0;
	  TCNT0=0;
	  TCCR0B |= (1<<CS01);
	  // enable timer overflow interrupt
	  TIMSK0 |= 1<<TOIE0;

	  // Enable global interrupts
	  sei();
}
// void millis_start()
// {
// 	millis_setup();
// 	TCNT0=0;
// }
// 
// void millis_stop()
// {
// 	TCNT0=0;
// 	TCCR0B=0;
// 	TIMSK0 &= ~(1 << TOIE0);
// }

unsigned long millis(void)
{
	uint64_t m;
	cli();
	m = _millis;
	sei();
	return m;
}

// void update_lcd_temp()
// {
// 	gabi_goto(12,1);
// 	uint16_t cur_temp = ReadADC(0);
// 	int tempC = ((cur_temp*500)/1024);
// 	char cur_str [4];
// 	itoa(tempC,cur_str,10);
// 	gabi_string(cur_str);
// 	gabi_goto(14,1);
// 	gabi_data(0b11011111);
// 	gabi_string("C");
// }

ISR(TIMER0_OVF_vect) {
	_millis+=2;
// 	_1000us += 256;
// 	while (_1000us > 1000) {
// 		_millis++;
// 		_1000us -= 1000;
// 	}
	//if ((millis() % 1000) == 0) update_lcd_temp();
}

#endif /* MILLIS_H_ */