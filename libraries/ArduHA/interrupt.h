/*
ArduHA - Home Automation API library for Arduino(tm)
Copyright (c) 2012/2014 Mathieu GRENET.  All right reserved.

This file is part of ArduHA.

ArduHA is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

ArduixPL is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with ArduHA.  If not, see <http://www.gnu.org/licenses/>.

Modified 2015-4-21 by Mathieu GRENET
mailto:mathieu@mgth.fr
http://www.mgth.fr

*/#ifndef HA_INTERRUPT_H
#define	HA_INTERRUPT_H

#include <Arduino.h>
#include "linkedlist.h"
#include "utility/EnableInterrupt.h"
#include "utility/digitalWriteFast.h"

class InterruptBase
{
public:
	virtual void runInterrupt(int pin, bool value, time_t time) = 0;
};

/// <summary>Base class for task scheduling</summary>
template<int pinNo>
class Interrupt : public InterruptBase, public AutoList < Interrupt<pinNo> >
{	
	static void _runInterrupt() {
			time_t now = micros();
			foreach(Interrupt<pinNo>, interrupt)
			{
				interrupt->runInterrupt(pinNo, digitalReadFast(pinNo), now);
			}
	}

public:
	Interrupt() {
		enableInterrupt(pinNo, Interrupt<pinNo>::_runInterrupt, CHANGE);
	}
};

template<int intNo>
class InterruptTask : public Interrupt<intNo>
{
	Task& _task;

public:
	InterruptTask(Task& task) :_task(task) {}

	virtual void run(time_t time)
	{
		_task.trigTask(time);
	}
};

#endif