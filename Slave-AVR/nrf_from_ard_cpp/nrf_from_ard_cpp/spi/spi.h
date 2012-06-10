#ifndef _SPI85_H_INCLUDED
#define _SPI85_H_INCLUDED

#include <stdio.h>
#include <avr/io.h>
//#include <avr/pgmspace.h>

#define __LPM_classic__(addr)   \
(__extension__({                \
	uint16_t __addr16 = (uint16_t)(addr); \
	uint8_t __result;           \
	__asm__                     \
	(                           \
	"lpm" "\n\t"            \
	"mov %0, r0" "\n\t"     \
	: "=r" (__result)       \
	: "z" (__addr16)        \
	: "r0"                  \
	);                          \
	__result;                   \
}))
#define 	pgm_read_byte(address_short)   __LPM_classic__((uint16_t)(address_short))


#define LSBFIRST 0
#define MSBFIRST 1

#define SPI_CLOCK_DIV4 0x00
#define SPI_CLOCK_DIV16 0x01
#define SPI_CLOCK_DIV64 0x02
#define SPI_CLOCK_DIV128 0x03
#define SPI_CLOCK_DIV2 0x04
#define SPI_CLOCK_DIV8 0x05
#define SPI_CLOCK_DIV32 0x06
//#define SPI_CLOCK_DIV64 0x07

#define SPI_MODE0 0x00
#define SPI_MODE1 0x04
#define SPI_MODE2 0x08
#define SPI_MODE3 0x0C

#define SPI_MODE_MASK 0x0C  // CPOL = bit 3, CPHA = bit 2 on SPCR
#define SPI_CLOCK_MASK 0x03  // SPR1 = bit 1, SPR0 = bit 0 on SPCR
#define SPI_2XCLOCK_MASK 0x01  // SPI2X = bit 0 on SPSR

class SPIClass {
public:
	inline static uint8_t transfer(uint8_t _data);

	// SPI Configuration methods
	inline static void attachInterrupt();
	inline static void detachInterrupt(); // Default  

	static void begin(); // Default
	static void end();

	static void setBitOrder(uint8_t);
	static void setDataMode(uint8_t);
	static void setClockDivider(uint8_t);
};

extern SPIClass SPI;

uint8_t SPIClass::transfer(uint8_t _data) {
	  SPDR = _data;
	  while (!(SPSR & _BV(SPIF)))
	  ;
	  return SPDR;
}

void SPIClass::attachInterrupt() {
	SPCR |= (1<<SPIE);
}

void SPIClass::detachInterrupt() {
	SPCR &= ~(1<<SPIE);
}
#endif