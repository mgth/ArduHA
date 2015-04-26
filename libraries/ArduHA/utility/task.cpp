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
#include <util/atomic.h>

#define MAX_MILLIS ((time_t)LONG_MAX)
#define MAX_MICROS (MAX_MILLIS/1000)

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
time_t Task::dueTime() const {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) return _dueTime;
}

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

#define MICROSECONDS_PER_TIMER0_OVERFLOW (clockCyclesToMicroseconds(64 * 256))
#define MILLIS_INC (MICROSECONDS_PER_TIMER0_OVERFLOW / 1000)
#define FRACT_INC ((MICROSECONDS_PER_TIMER0_OVERFLOW % 1000) >> 3)
#define FRACT_MAX (1000 >> 3)

void updateTimer(time_t ms)
{
	extern volatile unsigned long timer0_millis, timer0_overflow_count;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		timer0_millis += ms;

		timer0_overflow_count += ((ms * FRACT_MAX) / (FRACT_MAX + FRACT_INC)) / MILLIS_INC;
	}
}

void Task::sleep(time_t ms) {
	while (ms >= 8000) { _sleep(WDTO_8S); ms -= 8000; updateTimer(8000); } //9
	if (ms >= 4000)    { _sleep(WDTO_4S); ms -= 4000; updateTimer(4000); } //8
	if (ms >= 2000)    { _sleep(WDTO_2S); ms -= 2000; updateTimer(2000); } //7
	if (ms >= 1000)    { _sleep(WDTO_1S); ms -= 1000; updateTimer(1000); } //6
	if (ms >= 500)     { _sleep(WDTO_500MS); ms -= 500; updateTimer(500); }//5
	if (ms >= 250)     { _sleep(WDTO_250MS); ms -= 250; updateTimer(250); }//4
	if (ms >= 125)     { _sleep(WDTO_120MS); ms -= 120; updateTimer(120); }//3
	if (ms >= 64)      { _sleep(WDTO_60MS); ms -= 60; updateTimer(60); }//2
	if (ms >= 32)      { _sleep(WDTO_30MS); ms -= 30; updateTimer(30); }//1
	if (ms >= 16)      { _sleep(WDTO_15MS); ms -= 15; updateTimer(15); }//0
}



// run task if time to, or sleep if wait==true
bool Task::_run(bool wait)
{
	long d = compare(micros());

	if (wait) while (d > 0)	{
		sleep(d);
		d = compare(micros());
	}

	if (d <= 0)
	{
		//remove task from queue before execution, so that actual execution can requeue it.
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{
			unlink();
			_current = this;
		}
		//actual task execution
		run();

		_current = NULL;
		return true;
	}
	else return false;	
}

bool Task::dequeueMillis()
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

// run next task in the queue
void Task::loop(bool sleep)
{
	wdt_reset();
	
	Task* t;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		t = first();
	}
	
	if (t) t->_run(sleep);

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
	return dueTime() - t;
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