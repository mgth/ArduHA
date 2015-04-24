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

#include "debug.h"

#ifdef DEBUG_LCD
LiquidCrystal_I2C lcd(0x20,16,4);
#endif

bool debug_flag=false;

#if 1
uint8_t * heapptr, * stackptr;
void check_mem() {
  stackptr = (uint8_t *)malloc(4);          // use stackptr temporarily
  heapptr = stackptr;                     // save value of heap pointer
  free(stackptr);      // free up the memory again (sets stackptr to 0)
  stackptr =  (uint8_t *)(SP);           // save value of stack pointer
}
long get_free_memory()
{
	check_mem();
	return stackptr-heapptr;
}
#else
long get_free_memory()
{
	return 0;
}

#endif


long printMemCost(const StringRom msg) {

	static long oldMem = RAMSIZE;

	long newMem = get_free_memory();

	long mem = oldMem - newMem;
	if (mem)
	{
#ifdef DEBUG
		Serial.print(msg);
		Serial.print(F(" mem:"));
		Serial.print(oldMem);
		Serial.print('-');
		Serial.print(newMem);
		Serial.print(F(" cost:"));
		Serial.println(mem);
#endif
	}
	oldMem = newMem;
	return mem;
}

void printMemLCD() {
#ifdef DEBUG_LCD
	lcd.saveCursor();
	lcd.setCursor(0,3); lcd.print(S(_blkline));
	lcd.setCursor(0,3); lcd.print(get_free_memory());
	lcd.restoreCursor();
#endif
}

/*
void fix28135_malloc_bug()
 {
   for (__freelist *fp = __flp, *lfp = 0; fp; fp = fp->nx)
   {
     if (((uint8_t*)fp + fp->sz + 2) == __brkval)
     {
       __brkval = (uint8_t*)fp;
       if (lfp)
         lfp->nx = 0;
       else
         __flp = 0;
       break;
     }
     lfp = fp;
   }
 }


void   atexit( void ) {}
*/
