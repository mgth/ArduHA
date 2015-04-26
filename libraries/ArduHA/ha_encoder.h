#ifndef HA_ENCODER_H
#define	HA_ENCODER_H

#include "task.h"
#include "interrupt.h"
#include "Arduino.h"

//#define _pinA 2
//#define _pinB 3
const int8_t _pos[] = { 2, 3, 1, 0 };
const int8_t _inv[] = { 3, 2, 0, 1 };

template<int _pinA, int _pinB, int _pinClic = -1, byte _indent = 2>
class HA_Encoder : public Task, private Interrupt<_pinA>, private Interrupt<_pinB>, private Interrupt<_pinClic>
{
	int volatile _raw  = 0;
	bool volatile _clic = false;

	int _last = 0;
	int _min = 0;
	int _max = 100;


public:
	FilterPin<int> out;
	FilterPin<time_t> timing;
	FilterPin<bool> clic;

	int value() { return (_raw +2) >>_indent; }

	/// <summary>set current value</summary>
	bool setValue(int val) {

		if (val > _max) val = _max;
		else if (val < _min) val = _min;

		if (val != value())
		{
			_raw = val<<_indent;
			return true;
		}
		return false;
	}

	void setMax(int max) { _max = max; setValue(value()); }
	void setMin(int min) { _min = min; setValue(value()); }

	//	0	0	0	0	no movement
	//	0	0	0	1	+1
	//	0	0	1	0	-1
	//	0	0	1	1	+2  (assume pin1 edges only)
	//	0	1	0	0	-1
	//	0	1	0	1	no movement
	//	0	1	1	0	-2  (assume pin1 edges only)
	//	0	1	1	1	+1
	//	1	0	0	0	+1
	//	1	0	0	1	-2  (assume pin1 edges only)
	//	1	0	1	0	no movement
	//	1	0	1	1	-1
	//	1	1	0	0	+2  (assume pin1 edges only)
	//	1	1	0	1	-1
	//	1	1	1	0	+1
	//	1	1	1	1	no movement

	int8_t _dir = 0;

	void interrupt(byte in) {
			int raw = _raw & ~(0B11) | _pos[in];

			int8_t delta = raw - _raw;

			if (delta == -3
				|| (delta == -2 && _dir == 1)
				) raw += 4;

			else if (delta == 3
				|| (delta == 2 && _dir == -1)
				) raw -= 4;

			if (_raw != raw)
			{
				_dir = sgn(raw - _raw);
				_raw = raw;
			}

			if (value() != _last) trigTask(10);
	}

	virtual void run() override
	{
		if (_pinClic!=-1 && _clic)
		{
			_clic = false;
			clic.write(true);
		}

		int v = value();

		if (_last != v)
		{
			_last = v;
			out.write(_last);
		}
	}

	virtual void runInterrupt(int pin, bool val, time_t time) override
	{
		interrupt(((pin == _pinA) ? val : digitalReadFast(_pinA)) << 1 | ((pin == _pinB) ? val : digitalReadFast(_pinB)));
	}

	virtual void watchdog() override { DBG("arg..."); }
};

#endif