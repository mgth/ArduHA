#ifndef FILTER_H
#define FILTER_H
#include <arduha.h>
#include "linkedlist.h"


/// <summary>smothing filter</summary>
template<typename T, int _div = 1>
class KalmanFilter
{
	T _ratio;
	T _value;

public:
	/// <summary>input function to feed filter</summary>
	typedef  KalmanFilter < T, _div > _class_;
	SLOT(_class_, T, in)
	{
		if (_value == maxValue<T>()) _value = value;

		_value += (value - _value) * _ratio / (T)_div;

		out.send(_value);
	}

	/// <param name="ration">current to feedback ratio</param>
	KalmanFilter(T ratio)
		:_ratio(ratio) {
		_value = maxValue<T>();
	}

	/// <summary>pin out to link filters to</summary>
	Signal<T> out;

};

#define roundedShift(v,n) ((v >> (n - 1)) & (v >> n))



/// <summary>Seconde order calibration filter for shifted integer</summary>
/// <returns>( ax² + (1+b)x + c )</returns>
template<typename T>
class Calibration2ndOrder  {
private:
	T _ax2;
	T _bx;
	T _c;

public:
	/// <param name="a">a</param>
	/// <param name="b">b-1</param>
	/// <param name="c">c</param>
	Calibration2ndOrder(T ax2, T bx, T c) :_ax2(ax2), _bx(bx), _c(c) {}

	/// <summary>pin out to link filters to</summary>
	Signal<T> out;

	/// <summary>input function to feed filter</summary>
	SLOT(Calibration2ndOrder, T, in)
	{
		out.send(((_ax2 * value) + _bx) * value + _c);
	}
};


/// <summary>Lookup table calibration Element</summary>
template<typename Tin, typename Tout>
class LutElement
	: public LinkedObject<LutElement<Tin, Tout> >
{
	Tin _in;
	Tout _out;

public:
	Tin in() { return _in; }
	Tout out() { return _out; }

	int compare(Tin i) { return sgn(_in - i); }
	int compare(const LutElement& e)
	{
		return compare(e.in());
	}

};

/// <summary>Lookup Table Calibration</summary>
template<typename Tin, typename Tout>
class CalibrationLUT {
	LinkedList< LutElement<Tin, Tout> > _lut;

	SLOT(CalibrationLUT, Tin, input)
	{
		LutElement<Tin, Tout>* prv = _lut.first();

		while (prv)
		{
			LutElement<Tin, Tout>* nxt = prv->next();
			if (!nxt) return prv->out();
			if (value < nxt->in())
			{
				Tin deltaIn = nxt->in() - prv->in();
				Tout deltaOut = nxt->out() - prv->out();
				output.send(prv->out() + ((value - prv->in())*deltaOut) / deltaIn);
				return;
			}
			prv = nxt;
		}
	}

	Signal<Tout> output;
};

/// <summary>Outputs value where threshold is meet</summary>
template<typename T>
class ThresholdFilter {
public:
	void setThreshold(T t, long tt = 0) { _threshold = t; _timeThreshold = tt; }
	ThresholdFilter(T t, long tt = 0) :_reset(true) { setThreshold(t, tt); }

	Signal<T> out;

private:
	T _publicValue;
	time_t _publicTime;

	T _threshold;
	long _timeThreshold;

	bool _reset;

	SLOT(ThresholdFilter<T>, T, input)
	{
		T threshold = _threshold;
		if (!_reset && _timeThreshold > 0)
		{
			time_t duration = (millis() - _publicTime) / _timeThreshold;
			threshold /= 1 << duration;
		}

		if (_reset || abs(value - _publicValue) >= threshold)
		{
			_publicTime = millis();
			out.send(_publicValue = value);
		}
		_reset = false;
	}
};

/// <summary>Outputs value to serial port</summary>
template<typename T>
class OutputFilterSerial
{
	int _nb;
public:
	OutputFilterSerial() :_nb(0) {}

	SLOT(OutputFilterSerial<T>, T, input)
	{
		Serial.print(_nb);
		Serial.print(':');
		Serial.println(value, 5);
		_nb++;
	}
};

#endif