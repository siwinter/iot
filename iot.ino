/*
#include "cCore.h"
//#include "cDevice.h"
#include "cNetwork.h"
#include "cDatabase.h"
#include "cSetup.h"
#include "cDHT.h"
#include "cMqtt.h"
#include "cEnow.h"
#include "cTest.h"
*/
#include "sonoff.h"

void loop() { systemLoop(); }

void setup() {
	Serial.begin(115200);
	delay(5000);
	Serial.println("start");
//	newLed("LED");
//	newHearbeat("HB");
#if defined(ARDUINO_AVR_LARDU_328E)
	changeTopicName("test4");
#endif
}

