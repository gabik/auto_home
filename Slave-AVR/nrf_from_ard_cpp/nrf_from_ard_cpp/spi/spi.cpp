//#include "pins_arduino.h"
#include "spi.h"

#define SPI_DDR DDRB
#define SPI_PORT PORTB
const static uint8_t SS   = PB2;
const static uint8_t MOSI = PB3;
const static uint8_t MISO = PB4;
const static uint8_t SCK  = PB5;


SPIClass SPI;

void SPIClass::begin() {
	
	SPI_DDR |= (1<<SCK) | (1<<MOSI) | (1<<SS);
	SPI_DDR &=~ (1<<MISO);
	PORTB |= (1<<MISO) | (1<<SS);
	PORTB &= ~((1<<SCK) | (1<<MOSI));
	
	SPCR |= (1<<MSTR) | (1<<SPE);	
}

void SPIClass::end() {
	SPCR &=~(1<<SPE);
}



void SPIClass::setBitOrder(uint8_t bitOrder)
{
	if(bitOrder == LSBFIRST) {
		SPCR |= (1<<DORD);
	} else {
		SPCR &= ~(1<<DORD);
	}
}

void SPIClass::setDataMode(uint8_t mode)
{
	SPCR = (SPCR & ~SPI_MODE_MASK) | mode;
}

void SPIClass::setClockDivider(uint8_t rate)
{
	SPCR = (SPCR & ~SPI_CLOCK_MASK) | (rate & SPI_CLOCK_MASK);
	SPSR = (SPSR & ~SPI_2XCLOCK_MASK) | ((rate >> 2) & SPI_2XCLOCK_MASK);	
}
