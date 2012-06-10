
/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

#ifndef __RF24_CONFIG_H__
#define __RF24_CONFIG_H__

#include <stddef.h>

// Stuff that is normally provided by Arduino
#include <stdint.h>
#include <stdio.h>
#include <string.h>
//extern HardwareSPI SPI;
#define _BV(x) (1<<(x))

#define IF_SERIAL_DEBUG(x)

//typedef char const char;
typedef uint16_t prog_uint16_t;
#define PSTR(x) (x)
#define printf_P printf
#define strlen_P strlen
#define PROGMEM
#define pgm_read_word(p) (*(p)) 
#define PRIPSTR "%s"

#endif 
