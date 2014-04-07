/*
 * sensors.h
 *
 * Created: 4/6/2014 1:03:53 PM
 *  Author: Gabi.Kazav
 */ 


#ifndef SENSORS_H_
#define SENSORS_H_

#include <avr/io.h>
#include "../lcd/lcd_lib.h"
#include <util/delay.h>

#define LM35_PIN PINC0
#define PRI_PIN PINC1

void init_ADC()
{
	DDRC &=~(1<<LM35_PIN);
	DDRC &=~(1<<PRI_PIN);
	ADMUX |= (1<<REFS0);
	ADCSRA |= (1<<ADPS2)  | (1<<ADEN);
}

uint16_t ReadADC(uint8_t ADCchannel)
{
	//select ADC channel with safety mask
	ADMUX = (ADMUX & 0xF0) | (ADCchannel & 0x0F);
	//single conversion mode
	ADCSRA |= (1<<ADSC);
	// wait until ADC conversion is complete (ADSC will come 0 again)
	while( ADCSRA & (1<<ADSC) );
	return ADC;
}

void update_lcd_lm35_print()
{
	int average_c = 5;
	uint16_t cur_read=0;
	for (int i=0; i<average_c; i++) 
	{
		cur_read += ReadADC(0);
	}
	int tempC = ((cur_read/average_c*500)/1024);
	char cur_str [4];
 	itoa(tempC,cur_str,10);
 	gabi_goto(12,1);
 	gabi_string(cur_str);
 	gabi_data(0b11011111);
 	gabi_data('C');
}

#endif /* SENSORS_H_ */