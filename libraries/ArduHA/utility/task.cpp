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

#include "task.h"
#include <avr/wdt.h>
#include <avr/sleep.h>
#include "debug.h"

#define MAX_MILLIS ((time_t)2^31)
#define MAX_MICROS ((time_t)MAX_MILLIS/1000)

volatile bool Task::_sleeping = false;

//Task* volatile Task::_current = NULL;
//Task* volatile Task::_millisQueue = NULL;
Task* Task::_current = NULL;
TaskMillis Task::_millis;

//# Task* Task::_millisQueue = NULL;

//#if defined(__AVR_ATmega2560__)
//#define PROGRAM_COUNTER_SIZE 3 /* bytes*/
//#else
//#define PROGRAM_COUNTER_SIZE 2 /* bytes*/
//#endif
//
//uint32_t  addr = 0;
//
//SIGNAL(WDT_vect) {
//		
//	if (Task::sleeping())
//	{
//		wdt_disable();
//		wdt_reset();
//		WDTCSR &= ~_BV(WDIE);
//	}
//	else
//	{
//		register uint8_t  *upStack = (uint8_t *)SP;
//		upStack++;
//
//
//		memcpy(&addr, upStack, PROGRAM_COUNTER_SIZE);
//
//		DBG(F("\naddr:"));
//
//		Serial.print(addr * 2, HEX); DBG("\n");
//		//++upStack;
//		//Serial.print(*upStack, HEX); DBG("\n");
//		/*
//		if (Task::current())
//			Task::current()->watchdog();
//		else
//			DBG(F("\nWD:No running task\n"));
//
//		DBG(F("\nQ:"))
//		foreach (Task,t) { t->watchdog(); }
//
//		wdt_enable(WDTO_8S);
//		while (true)
//			;
//	}
//}

void Task::_wakeup(uint8_t wdt_period)
{
	//wdt_disable();
	wdt_enable(wdt_period);
	wdt_reset();
	WDTCSR &= ~_BV(WDIE);
}

void Task::_sleep(uint8_t wdt_period) {
	Task::_sleeping = true;

	wdt_enable(wdt_period);
	wdt_reset();
	WDTCSR |= _BV(WDIE);
	//  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	set_sleep_mode(SLEEP_MODE_EXT_STANDBY);
	sleep_mode();
	_wakeup();

	Task::_sleeping = false;
}
/*
void Task::sleep(time_t us) {
	while (us >= 8000000) { _sleep(WDTO_8S); us -= 8000000; } //9
	if (us >= 4000000)    { _sleep(WDTO_4S); us -= 4000000; } //8
	if (us >= 2000000)    { _sleep(WDTO_2S); us -= 2000000; } //7
	if (us >= 1000000)    { _sleep(WDTO_1S); us -= 1000000; } //6
	if (us >= 500000)     { _sleep(WDTO_500MS); us -= 500000; }//5
	if (us >= 250000)     { _sleep(WDTO_250MS); us -= 250000; }//4
	if (us >= 125000)     { _sleep(WDTO_120MS); us -= 120000; }//3
	if (us >= 64000)      { _sleep(WDTO_60MS); us -= 60000; }//2
	if (us >= 32000)      { _sleep(WDTO_30MS); us -= 30000; }//1
	if (us >= 16000)      { _sleep(WDTO_15MS); us -= 15000; }//0
}
*/
// sleep reducing power consumption
void Task::sleep(time_t us) {
	time_t t = 8000000;
	for (uint8_t wdto = 9; t >= 15000; t /= 2, wdto--)
	{
		while (us > t) { _sleep(wdto); us -= t; }
	}
}


// run task if time to, or sleep if wait==true
bool Task::_run(bool wait)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		long d = compare(micros());

		if (wait) while (d > 0)	{
			NONATOMIC_BLOCK(NONATOMIC_RESTORESTATE)
			{
				sleep(d);
			}
			d = compare(micros());
		}

		if (d <= 0)
		{
			if (_current) return false;
			//detach task before execution
			unlink();

			_current = this;

			//actual task execution
			NONATOMIC_BLOCK(NONATOMIC_RESTORESTATE)
			{
				run();
			}

			if (_current == this) _current = NULL;
			return true;
		}
		else return false;
	}
}

bool Task::dequeueMillis()
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		long d = compare(millis() + MAX_MICROS);
		if (d < 0)
		{
			unlink(_millis.Queue);
			trigTaskAt(_dueTime);
			return true;
		}
		else return false;
	}
}

// run next task in the queue
void Task::loop(bool sleep)
{
	wdt_reset();

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		Task* t = first();

		if (t) t->_run(sleep);
	}
	//ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	//{
	
}

void Task::watchdog()
{
	Serial.println("watchdog");
}

void Task::_trigTaskAt(time_t dueTime, Task*& queue)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		_dueTime = dueTime;
		relocate(queue);
	}
}

void Task::trigTaskAtMicros(time_t dueTime)
{
	_trigTaskAt(dueTime, first());
}

void Task::trigTaskMicros(time_t delay)
{
	trigTaskAtMicros(micros() + delay);
}

void Task::trigTaskAt(time_t dueTime)
{
	_trigTaskAt(dueTime, _millis.Queue);
	_millis.trigTask();
}


void Task::trigTask(time_t delay)
{
	// if delay within micros range schedule with micros
	if (delay <= MAX_MICROS)
		trigTaskMicros(delay * 1000);
	else
		trigTaskAt(millis() + delay);
}

Task* Task::trigReccurent(time_t delay, time_t interval)
{
	Task* t = new RecurrentTask(*this, interval);
	if (t) t->trigTask(delay);
	return t;
}

Task* Task::trigReccurentFixed(time_t delay, time_t interval)
{
	Task* t = new RecurrentTask(*this, interval);
	if (t) t->trigTask(delay);
	return t;
}

Task* Task::trigReccurentFromStart(time_t delay, time_t interval)
{
	Task* t = new RecurrentTask(*this, interval);
	if (t) t->trigTask(delay);
	return t;
}

// returns scheduled position against t
long Task::compare(time_t t) const
{
	return _dueTime - t;
}

//for Task to be sortable
int Task::compare(const Task& task) const
{
	long diff = compare(task.dueTime());
	return sgn(diff);
}


void TaskMillis::run()
{
	Task* t = Queue;
	while (t && t->dequeueMillis())
	{
		t = Queue;
	}
	if (t) trigTask(min(MAX_MICROS,t->dueTime()-(time_t)millis()-MAX_MICROS));
}