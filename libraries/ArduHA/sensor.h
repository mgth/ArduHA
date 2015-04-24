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
#ifndef SENSOR_H
#define SENSOR_H
#include <arduha.h>
#include "linkedlist.h"
#include "task.h"

#ifdef HA_FLOAT
typedef float temperature_t;
#else
typedef int16_t temperature_t;
#endif



/// Sensor constructor : 
/// - hardware address
/// - sensor options
/// - interval

/// <summary>Filter object offers an input that will be fed by a <c>FilterPin</c></summary>
template <typename T>
class Filter
#ifdef HA_FILTER_MULTI
	:public LinkedList< Filter<T> >
#endif
{
public:
	virtual void input(T value) = 0;
	virtual void error() {};
};

/// <summary>output pin to link <c>Filter</c> to</summary>
template <typename T>
class FilterPin {
	Filter<T>* _filter;
public:
	FilterPin() :_filter(NULL){};
	/// <summary>Add a filter to listen to this pin</summary>
	Filter<T>* link(Filter<T>* filter) {
#ifdef HA_FILTER_MULTI
		filter->Filter<T>::link(_filter);
#else
		_filter = filter;
#endif
		return filter;
	}

	/// <summary>casted to return desired type</summary>
	template <typename Tobj>
	Tobj& link(Tobj* obj) { return *(Tobj*)link((Filter<T>*)obj); }

	/// <summary>write information to all listening filters on this pin</summary>
	bool write(T value)
	{
#ifdef HA_FILTER_MULTI
		foreachfrom(Filter<T>, f, _filter)
		{
			f->input(value);
		}
#else
		if (_filter) _filter->input(value);
#endif
	}

	void error()
	{
#ifdef HA_FILTER_MULTI
			foreachfrom(Filter<T>, f, _filter)
			{
				f->error();
			}
#else
			if (_filter) _filter->error();
#endif
		}
};

/// <summary>smothing filter</summary>
template<typename T, int _div = 1>
class KalmanFilter : public Filter<T>
{
	T _ratio;
	T _value;


	/// <summary>input function to feed filter</summary>
	void input(T in)
	{
		if (_value == maxValue<T>())
			_value = in;
		
		_value += (in - _value) * _ratio / (T)_div;

		out.write(_value);
	}

public:
	/// <param name="ration">current to feedback ratio</param>
	KalmanFilter(T ratio)
		:_ratio(ratio) {
		_value = initValue(ratio);
	}

	/// <summary>pin out to link filters to</summary>
	FilterPin<T> out;

};

#define roundedShift(v,n) ((v >> (n - 1)) & (v >> n))

/// <summary>Seconde order calibration filter for shifted integer</summary>
/// <returns>( ax² + (1+b)x + c ) << shift</returns>
/// This filter allow to calibrate sensors without requiering floats
/// therefore, coeficients has to be inverted
/// Also note that b is removed 1 before to be inverted
/// and c shouls already be shifted.
class Calibration2ndOrder_Shifted : public Filter<int>
{
	int _inv_a;
	int _inv_b;
	int _nc;
	byte _shift;
public:

	/// <summary>pin out to link filters to</summary>
	FilterPin<int> out;

	/// <param name="inv_a">1/a</param>
	/// <param name="inv_b">1/(b-1)</param>
	/// <param name="nc">c << shift</param>
	/// <param name="shift">number of byte shift</param>
	Calibration2ndOrder_Shifted(int inv_a, int inv_b, int nc, byte shift=7) :_inv_a(inv_a), _inv_b(inv_b), _nc(nc), _shift(shift){}

	/// <summary>input function to feed filter</summary>
	void input(int nx);
};

/// <summary>Seconde order calibration filter for shifted integer</summary>
/// <returns>( ax² + (1+b)x + c )</returns>
class Calibration2ndOrder : public Filter<double> {
public:
	/// <param name="a">a</param>
	/// <param name="b">b-1</param>
	/// <param name="nc">c</param>
	Calibration2ndOrder(double ax2, double bx, double c) :_ax2(ax2), _bx(bx), _c(c) {}

	/// <summary>pin out to link filters to</summary>
	FilterPin<double> out;

private:
	double _ax2;
	double _bx;
	double _c;

	/// <summary>input function to feed filter</summary>
	void input(const double& value);
};

/// <summary>Lookup table calibration Element</summary>
template<typename Tin, typename Tout>
class LutElement
	: public LinkedList<LutElement<Tin,Tout> >
{
	Tin _in;
	Tout _out;


	int compare(Tin i) { return sgn(_in - i); }
	int compare(const LutElement& e)
	{
		if (_in < e._in) return -1;
		if (_in > e._in) return 1;
		return 0;
	}

	Tout convert(Tin in)
	{
		Tout out;
		if (!LutElement::next()) return _out;
		if (in >= LutElement::next()->_in) return LutElement::next()->convert(in);
		Tin deltaIn = LutElement::next()->_in - _in;
		Tout deltaOut = LutElement::next()->_out - _out;
		return _out + ((in - _in)*deltaOut) / deltaIn;
	}
};

/// <summary>Lookup Table Calibration</summary>
template<typename Tin, typename Tout>
class CalibrationLUT : public Filter<Tin> {
	LutElement<Tin,Tout>* _lut;

	void input(Tin in) { output(_lut->convert(in)); }

};

/// <summary>Outputs value where threshold is meet</summary>
template<typename T>
class ThresholdFilter : public Filter<T> {
public:
	void setThreshold(T t, long tt = 0) { _threshold = t; _timeThreshold = tt; }
	ThresholdFilter(T t, long tt = 0) :_reset(true) { setThreshold(t, tt); }

	FilterPin<T> out;

private:
	T _publicValue;
	time_t _publicTime;

	T _threshold;
	long _timeThreshold;

	bool _reset;

	void input(T value)
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
			out.write(_publicValue = value);
		}
		_reset = false;
	}
};

/// <summary>Outputs value to serial port</summary>
template<typename T>
class OutputFilterSerial :public Filter<T>
{
	int _nb;
public:
	OutputFilterSerial() :_nb(0) {}
	void input(T value)
	{
		Serial.print(_nb);
		Serial.print(':');
		Serial.println(value,5);
		_nb++;
	}
};

#endif

