/*
 * timers.h
 *
 * Created: 4/6/2014 11:11:01 AM
 *  Author: Gabi.Kazav
 */ 


#ifndef TIMERS_H_
#define TIMERS_H_

#include "../lcd/lcd_lib.h"
#include <stdlib.h>

int overflow_count, timer1_fire;
int p_sec, p_min, p_hour, p_of;

void setup_timers()
{
	overflow_count=0;

	//Timer setup
	cli();
	TCCR2A = 0;
	TCCR2B = 0;
	TCNT2  = 0;
	OCR2A  = 78;

	TCCR1A = 0;
	TCCR1B = 0;
	TCNT1  = 0;
	OCR1A = 976;
	//TCCR1B |= (1 << CS10) | (1 << CS12) | (1 << WGM12);     // prescaler 128, set CTC mode. with OCR 10000, TIMER1_COMPA_vect will fire every 1 second
	sei();
	
	p_sec=0;
	p_min=0;
	p_hour=0;
	p_of=0;	
	timer1_fire=0;
}

void stop_timer2()
{
	TCNT2=0;
	TCCR2B=0;
	TCCR2A=0;
	TIMSK2 &= ~(1 << OCIE2A);
}

void start_timer2()
{
	TCNT2=0;
	TCCR2A = (1 << WGM21);
	TCCR2B = (1 << CS20) | (1 << CS22);     // prescaler 128, set CTC mode. with OCR 157, TIMER0_COMPA_vect will fire every 0.01 sec (100 OF = 1 Sec)
	p_of=0;
	TIMSK2 |= (1 << OCIE2A);
}

void stop_timer1()
{
	TCNT1=0;
	TCCR1B=0;
	TIMSK1 &= ~(1 << OCIE1A);
}

void start_timer1()
{
	TCNT1=0;
	TCCR1B |= (1 << CS10) | (1 << WGM12) | (1 << CS12);     // prescaler 1024, set CTC mode. with OCR 976, TIMER1_COMPA_vect will fire every 1 Seconds. 
	overflow_count=0;
	TIMSK1 |= (1 << OCIE1A);
}

ISR(TIMER1_COMPA_vect)          // timer compare interrupt service routine
{
	overflow_count++;
	if (overflow_count > 30) 
	{
		timer1_fire = 1;
	}
}

void update_lcd_clock_print()
{
	gabi_goto(0,1);
	char tmp_hour[3];
	char tmp_minutes[4];
	itoa(p_hour,tmp_hour,10);
	itoa(p_min,tmp_minutes,10);
	gabi_string(tmp_hour); 
	gabi_data(':');
	gabi_string(tmp_minutes);
}

ISR(TIMER2_COMPA_vect)          // timer compare interrupt service routine
{
	p_of++;
	if (p_of > 100)
	{
		p_sec++;
		p_of=0;
		if (p_sec > 60) {
			p_min++;
			p_sec=0;
			if (p_min > 60) {
				p_hour++;
				p_min=0;
			}
			update_lcd_clock_print();
		}
	}
}




#endif /* TIMERS_H_ */