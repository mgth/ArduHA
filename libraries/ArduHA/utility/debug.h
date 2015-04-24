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

#ifndef DEBUG_H
#define DEBUG_H

#include <ArduHA.h>

#ifdef DEBUG_LCD
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
extern LiquidCrystal_I2C lcd;
#endif

#if defined(SLOWDEBUG)
#define DBGLN(s,v) {Serial.print(s);Serial.println(v); delay(SLOWDEBUG);}
#define DBG(s) {Serial.print(s); delay(SLOWDEBUG);}
#elif defined(DEBUG)
#define DBGLN(s,v) {Serial.print(s);Serial.println(v);}
#define DBG(s) {Serial.print(s);}
#else
#define DBGLN(s,v) ;
#define DBG(s) ;
#endif

#if DEBUG>=1
#define DBGLN_1(s,v) DBGLN(s,v);
#define DBG_1(s) DBG(s);
#else
#define DBGLN_1(s,v);
#define DBG_1(s);
#endif

#if DEBUG>=2
#define DBGLN_2(s,v) DBGLN(s,v);
#define DBG_2(s) DBG(s);
#else
#define DBGLN_2(s,v);
#define DBG_2(s);
#endif

#if DEBUG>=3
#define DBGLN_3(s,v) DBGLN(s,v);
#define DBG_3(s) DBG(s);
#else
#define DBGLN_3(s,v);
#define DBG_3(s);
#endif

#ifdef DEBUG_MEM
#define DBG_MEM(s) printMemCost(s); //delay(500);
#else
#define DBG_MEM(s) ;	
#endif

extern int  __bss_end;


struct __freelist
{
	size_t sz;
	struct __freelist *nx;
};

extern struct __freelist *__flp;
extern uint8_t* __brkval;


long get_free_memory();
void printMemLCD();
long printMemCost(const StringRom msg);

#ifdef DEBUG
#define _SETUP() void setup() { Serial.begin(115200);
#define _LOOP() } void loop() {DBG_MEM(F("loop"));
#else
#define _SETUP() int main(void) {
#define _LOOP() while(1)
#endif
#define _END  }

extern bool debug_flag;

#endif
