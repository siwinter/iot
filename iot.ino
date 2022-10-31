#include "cCore.h"
#include "cDevice.h"
#include "cDatabase.h"
#include "cNtwDevices.h"
#include "cMqtt.h"
#include "cEnow.h"
#include "cSetup.h"

cLed* l;

void loop() { systemLoop(); }

void setup() {
	Serial.begin(115200);
	Serial.println("start");
	l = newLed("LED");
	l->setBlink();
	newHearbeat("HB");
#if defined(ARDUINO_AVR_LARDU_328E)
	changeTopicName("test4");
#endif
}
