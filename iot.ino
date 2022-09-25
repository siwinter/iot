
#include "cCore.h"
#include "cDevice.h"
#include "cTest.h"
#include "cNetwork.h"
#include "cDatabase.h"
#include "cMqtt.h"

void setup() {
	Serial.begin(115200);

/*
	
	cLoopTest* l1 = new cLoopTest(1, "l1");
	cLoopTest* l2 = new cLoopTest(2, "l2");
	cObservedTest* e1 = new cObservedTest(2, "e1") ;
	cObservedTest* e2 = new cObservedTest(2, "e2") ;
	cObservedTest* e3 = new cObservedTest(2, "e3") ;
	e2->addObserver(new cObserverTest("o2"));
	e3->addObserver(new cObserverTest("o31"));
	e3->addObserver(new cObserverTest("o32"));
	e3->addObserver(new cObserverTest("o33"));
	new cLedTest() ;
	cLoopTest* l2 = new cLoopTest(2, "l2");
//	newLed(2, false, "LED1") ;
	new cTxtLedTest() ;
//	theScheduler.upStreamChannel = new cSerialChannel() ;
//	theScheduler.insertChannel(&theMqtt) ;
//	newDHT22(4,"Wohnz") ;
//	newBME280("Wohnz") ;
*/
	newLed(2, false, "LED"); 
}


void loop() { systemLoop(); }

