#include "ArduHA.h"
#include "task.h"
#include <avr/wdt.h>

class DebugTask : public Task
{

	virtual void run() override {
		Serial.print('.');
		trigTaskAtMicros(dueTime() + 1000000);
	}
public:
	DebugTask() { trigTask(); }

} debugTask;


void loop()
{
	DBG_MEM(F("loop"));
//	for (static time_t last; micros() - last > 1000000; last += 1000000) { Serial.print("."); }

	Task::loop();

}

#ifdef DEBUG

#else
int main(void) {
	setup();
	while (1)
	{
		Task::loop();
	}
}

#endif