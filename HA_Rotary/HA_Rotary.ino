#include "ArduHA.h"
#include "task.h"
#include "sensor.h"
#include <avr/pgmspace.h>
#include "ha_encoder.h"
#include <avr/wdt.h>

#include "ApplicationMonitor.h"

Watchdog::CApplicationMonitor ApplicationMonitor;

HA_Encoder<2, 3, 6> rotary;
HA_Encoder<4, 5, 7> rotary2;
//
template<typename T>
class DebugFilter : public Filter < T >
{
	StringRom _header;
public:
	DebugFilter(StringRom s) :_header(s) {}
	void runFilter(T value) override
	{
		//Serial.println(value);
		DBGLN(_header,value);
	}
};



void setup()
{

	Serial.begin(115200);
	Serial.println("========================================================");

	ApplicationMonitor.Dump(Serial);
	
	//delay(5000);

	rotary.out.link(new DebugFilter<int>(F("T1:")));
	rotary2.out.link(new DebugFilter<int>(F("T2:")));
////	rotary.clic.link(new DebugFilter<bool>(F("C:")));
//	cli();
//	//reset watchdog
//	wdt_reset();
//	//set up WDT interrupt
//	WDTCSR = (1 << WDCE) | (1 << WDE);
//	//Start watchdog timer with 4s prescaller
//	WDTCSR = (1 << WDIE) | (1 << WDE) | (1 << WDP3);
//	//Enable global interrupts
//	sei();
//	//debugTask.trigReccurentFixed(10000, 1000000);
//	//debugTask.trigTask();
//

	ApplicationMonitor.EnableWatchdog(Watchdog::CApplicationMonitor::Timeout_4s);
}

/*
time_t _last = micros();

void loop()
{
	//	ApplicationMonitor.IAmAlive();
	//	ApplicationMonitor.SetData(g_nIterations++);

	//DBG_MEM(F("loop"));
	time_t t = micros();
	if (t - _last > 1000000)
	{

		_last = t;
		Serial.print(".");
	}

	Task::loop();
}*/