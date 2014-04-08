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
#include "../timers/timers.h"

#define LM35_PIN PINC0

#define PRI_PIN PINB
#define PRI_PORT PORTB
#define PRI_DDR DDRB
#define PRI_BIT 6

#define LCD_UP_DDR DDRD
#define LCD_UP_PORT PORTD
#define LCD_UP_BIT 3


int tempC, LCD_UP, PRI_sensor_counter;

void init_ADC()
{
	DDRC &=~(1<<LM35_PIN);
	DDRC &=~(1<<PRI_PIN);
	ADMUX |= (1<<REFS0);
	ADCSRA |= (1<<ADPS2)  | (1<<ADEN);
	tempC=0;
	PRI_DDR &= ~(1<<PRI_BIT);
	PRI_PORT|=(1<<PRI_BIT);
	LCD_UP_DDR |= (1<<LCD_UP_BIT);
	LCD_UP_PORT |= (1<<LCD_UP_BIT);
	PRI_sensor_counter=0;
	LCD_UP=1;
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

int read_PRI()
{
	if (!(PRI_PIN & (1<<PRI_BIT))) PRI_sensor_counter++; 
	//else PRI_sensor_counter--;
	//if (PRI_sensor_counter<0) PRI_sensor_counter=0;
// 	gabi_goto(12,0);
// 	char str_up[5];
// 	itoa(PRI_sensor_counter, str_up, 10);
// 	gabi_string(str_up);
// 	_delay_ms(10);
	if (PRI_sensor_counter>500) return 1;
	return 0;
}

void handle_PRI()
{
	if (LCD_UP == 0)
	{
		if (read_PRI() == 1)
		{
			LCD_UP=1;
			overflow_count=0;
			LCD_UP_PORT |= (1<<LCD_UP_BIT);
			PRI_sensor_counter=0;
		}
	}
}

void update_lcd_lm35_print()
{
	int average_c = 5;
	uint16_t cur_read=0;
	for (int i=0; i<average_c; i++) 
	{
		cur_read += ReadADC(0);
	}
	tempC = ((cur_read/average_c*500)/1024);
	char cur_str [4];
 	itoa(tempC,cur_str,10);
 	gabi_goto(12,1);
 	gabi_string(cur_str);
 	gabi_data(0b11011111);
 	gabi_data('C');
}

#endif /* SENSORS_H_ */