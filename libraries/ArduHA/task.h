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

	  Modified 2014-3-23 by Mathieu GRENET 
	  mailto:mathieu@mgth.fr
	  http://www.mgth.fr
*/
#
/*
make use of code from Narcoleptic :
https://code.google.com/p/narcoleptic/
*/

#ifndef TASKS_H
#define TASKS_H

#include <ArduHA.h>
#include "linkedlist.h"

class TaskMillis;


/// <summary>Base class for task scheduling</summary>
class Task : public LinkedList<Task>
{
	/// <summary>currently running task</summary>
	 static Task* _current;

	 /// <summary>system is currently sleeping waiting for watchdog to wake it up</summary>
	 static bool volatile _sleeping;

	/// <summary>wakeup from sleep and set watchdog timer</summary>
	 static void _wakeup(uint8_t wdt_period = 9);

	/// <summary>sleep using power reduction</summary>
	static void _sleep(uint8_t wdt_period);
	/// <summary>do nothing for some time, potencialy use power save</summary>
	/// <param name="delay">delay in &micro;s</param>
	static void sleep(time_t delay);

	static TaskMillis _millis;

protected:
	/// <summary>scheduled execution time</summary>
	time_t volatile _dueTime;

	/// <summary>internal task execution engine</summary>
	bool _run(bool sleep=false);



	/// <returns>scheduled position against t (<0 if before, >0 if after)</returns>
	/// <param name="t">time in &micro;s</param>
	long compare(time_t t) const;

	void _trigTaskAt(time_t dueTime, Task*& queue);

public:

	/// <summary>run next task in queue if it's time to</summary>
	/// <param name="sleep">if true reduce power consuption until next task</param>
	/// <returns>-1 : no more task in queue<br>0 : nothing done this time<br>1 : task executed</returns>
	static void loop(bool sleep = false);

	/// <summary>to be overriden for task execution</summary>
	virtual void run() = 0;
	/// <summary>to be overriden for execution at interrupt time</summary>
	/// <reamarq>default action is to trig task in normal task queue, should be kept as short as possible</remark>
	/// <return>true if task should stay attatched to interrupt</remark>
	//virtual bool runInterrupt(byte intNo, time_t time);

	virtual void watchdog();

	/// <summary>schedule next execution at determined due time</summary>
	/// <param name="duetime">time in &micro;s</param>
	void trigTaskAtMicros(time_t duetime);

	/// <summary>schedule next execution at determined delay from now</summary>
	/// <param name="delay">delay in &micro;s</param>
	void trigTaskMicros(time_t delay = 0);

	/// <summary>schedule next execution at determined due time</summary>
	/// <param name="duetime">time in ms</param>
	void trigTaskAt(time_t duetime);

	/// <summary>schedule next execution at determined due time</summary>
	/// <param name="duetime">time in ms</param>
	void trigTaskAtMillis(time_t duetime);

	/// <summary>schedule next execution at determined delay from now</summary>
	/// <param name="delay">delay in ms</param>
	void trigTask(time_t delay = 0);

	/// <summary>schedule reccurent execution at determined delay from now</summary>
	/// <param name="delay">delay in &micro;s</param>
	/// <param name="interval">interval in ms or &micro;s</param>
	Task* trigReccurent(time_t delay, time_t interval);

	/// <summary>schedule reccurent execution at determined delay from now</summary>
	/// <param name="delay">delay in &micro;s</param>
	/// <param name="interval">interval in &micro;s</param>
	Task* trigReccurentFixed(time_t delay, time_t interval);

	/// <summary>schedule reccurent execution  from last execution at determined delay from now</summary>
	/// <param name="delay">delay in ms</param>
	/// <param name="interval">interval in ms</param>
	Task* trigReccurentFromStart(time_t delay, time_t interval);

	/// <summary>currently running task</summary>
	static Task* current() { return _current; };

	/// <summary>get scheduled execution time</summary>
	/// <returns>scheduled execution time</returns>
	/// <remarq>see microTiming() for time unit (ms or &micro;s)</remarq>
	time_t dueTime() const;

	/// <summary>get slepping state</summary>
	/// <remarq>might only be used by watchdog interrupt vector</remarq>
	static bool sleeping() { return _sleeping; }

	/// <summary>internal task execution engine</summary>
	bool dequeueMillis();

	/// <remarq>by default new task is not scheduled</remarq>
	Task(){}

	/// <summary>New task to be scheduled </summary>
	/// <param name="delay">schedule first execution in ms from now</param>
	Task(time_t delay) { trigTask(delay); }

	/// <summary>for Task to be sortable</summary>
	/// <param name="task">the task to compare this to</param>
	/// <returns><0 if this is scheduled before task, >0 if this is scheduled after</returns>
	int compare(const Task& task) const;
};

class TaskMillis : public Task
{
	virtual void run() override;
public:
	Task* Queue = NULL;
};


/// <summary>Reccuring task at interval from last task ending</summary>
class RecurrentTask : public Task
{
protected:
	time_t _interval;
	Task& _task;

	void run()
	{
		_task.run();
		trigTask(_interval);
	}

public:
	RecurrentTask(Task& task, time_t interval) :
		_interval(interval),
		_task(task)
	{}

	/// <summary>get interval for recuring tasks</summary>
	/// <returns>interval in ms or &micro;s </returns>
	time_t interval() const { return _interval; }
};

/// <summary>Reccuring task at fixed interval from initial execution</summary>
class RecurrentTaskFixed : public RecurrentTask
{
	RecurrentTaskFixed(Task& task, time_t interval) :
		RecurrentTask(task, interval)
	{}

	void run()
	{
		_task.run();
		trigTaskAt(_dueTime + _interval);
	}
};

/// <summary>Reccuring task at interval from last execution</summary>
class RecurrentTaskFromStart : public RecurrentTask
{
	RecurrentTaskFromStart(Task& task, time_t interval) :
		RecurrentTask(task, interval)
	{}

	void run()
	{
		trigTask(_interval);
		_task.run();
	}
};


#endif