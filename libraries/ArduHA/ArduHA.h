/*
  ArduHA - ArduixPL - xPL library for Arduino(tm)
  Copyright (c) 2012/2014 Mathieu GRENET.  All right reserved.

  This file is part of ArduHA / ArduixPL.

    ArduixPL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ArduixPL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ArduixPL.  If not, see <http://www.gnu.org/licenses/>.

	  Modified 2014-3-14 by Mathieu GRENET 
	  mailto:mathieu@mgth.fr
	  http://www.mgth.fr
*/
#ifndef ARDUHA_H
#define ARDUHA_H
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <avr/pgmspace.h>
#include <WString.h>
#include "float.h"
#include "limits.h"

#define GCC_VERSION (__GNUC__ * 10000 \
	+ __GNUC_MINOR__ * 100 \
	+ __GNUC_PATCHLEVEL__)

// RAMSIZE
#ifdef __SAM3X8E__
#define RAMSIZE (96*1024)
#endif

#if !defined(RAMSIZE)
#if defined(RAMSTART)
#define RAMSIZE RAMEND-RAMSTART
#else
#define RAMSIZE RAMEND-0x0FF
#endif
#endif

// Redef min max abs as template instead of macros 
#undef max
#undef min
#undef abs
template<typename T>
const T& max(const T& a, const T& b) { return (a > b) ? a : b; }
template<typename T>
const T& min(const T& a, const T& b) { return (a < b) ? a : b; }
template<typename T>
const T& abs(const T& a) { return (a < 0) ? -a : a; }

// Templates to get max and min system value by type
template<typename T> T maxValue() { return 0; }
template<> inline double maxValue<double>()  { return DBL_MAX; }
template<> inline float maxValue<float>()  { return FLT_MAX; }
template<> inline int maxValue<int>()  { return INT_MAX; }
template<> inline unsigned int maxValue<unsigned int>()  { return UINT_MAX; }
template<> inline long maxValue<long>()  { return LONG_MAX; }
template<> inline unsigned long maxValue<unsigned long>()  { return ULONG_MAX; }
template<typename T> T minValue() { return 0; }
template<> inline double minValue<double>()  { return DBL_MIN; }
template<> inline float minValue<float>()  { return FLT_MIN; }
template<> inline int minValue<int>()  { return INT_MIN; }
template<> inline long minValue<long>()  { return LONG_MIN; }

// Time functions
typedef  long time_t;
inline time_t never() {
	return maxValue<time_t>();
}


typedef const __FlashStringHelper* StringRom;

// Flashstring dedub, do not work on Arduino due
#ifndef __SAM3X8E__
/**
* Create constant string in program memory. Allow IOStream output
* operator. Allows link time reduction of equal strings or substrings.
* @param[in] s string literal (at compile time).
* @return string literal in program memory.
*/

#undef PSTR
#define PSTR(str) __PSTR1(str,__COUNTER__)
#define __PSTR1(str,num) __PSTR2(str,num)
#define __PSTR2(str,num) \
	(__extension__(\
{ \
	const char* ptr; \
	asm volatile (\
	".pushsection .progmem.data, \"SM\", @progbits, 1" "\n\t" \
	"PSTR_" #num ": .string " #str "\n\t" \
	".popsection" "\n\t" \
	); \
	asm volatile (\
	"ldi %A0, lo8(PSTR_" #num ")" "\n\t" \
	"ldi %B0, hi8(PSTR_" #num ")" "\n\t" \
	: "=d" (ptr) \
	); \
	ptr; \
} \
	))
#endif


#include "_configure_.h"
#include "utility/debug.h"

#endif