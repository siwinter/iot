/*
#include "cCore.h"
#include "cDevice.h"
#include "cNetwork.h"
#include "cDatabase.h"
#include "cSetup.h"
#include "cDHT.h"
#include "cMqtt.h"
#include "cEnow.h"
#include "cTest.h"

//#include "sonoff.h"
//#include "cSetup.h"
*/
#include "cCore.h"
#include "cDevice.h"
#include "cDatabase.h"
#include "cNtwDevices.h"
#include "cEnow.h"
//#include "cSetup.h"

cLed* l;

void loop() { systemLoop(); }

void setup() {
	Serial.begin(115200);
	delay(5000);
	Serial.println("start");
	l = newLed("LED");
	l->setBlink();
	newHearbeat("HB");
#if defined(ARDUINO_AVR_LARDU_328E)
	changeTopicName("test4");
#endif
}
