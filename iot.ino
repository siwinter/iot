
#include "cCore.h"
#include "cDevice.h"
//#include "cTest.h"
#include "cNetwork.h"
#include "cDatabase.h"
#include "cSetup.h"
//#include "cDHT.h"
#include "cMqtt.h"
//#include "cEnow.h"

cLed* l ;

class cSetter : public cTimer {
  public :
	cSetter() { setTimer(10) ; }
	
	void onTimeout() {
		uint8_t mac[6];
		l->toogle();
		setTimer(10) ; } };

void setup() {
	Serial.begin(115200);
	Serial.println("start");
//	newDHT22(4,"DHT") ;
	newLed("LED");
//	newClock("time");
//	changeTopicName("test4");
//	new cSetter();

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
	newDHT22(4,"Wohnz") ;
//	newBME280("Wohnz") ;
	new cTxtLedTest() ;

	newLed(2, false, "LED");
	newDHT22(4,"_1") ;
#/
	//newHeap("Heap") ;
	//newRssi("Rssi") ;
//	new cTxtLedTest() ;
//	char ssid[] = "Hundehaus" ;
//	char pwd[] = "Affenhaus" ;
//	theWifi.configure("ssid", ssid, strlen(ssid)) ;
//	theWifi.configure("pwd", pwd, strlen(pwd)) ;
	l = newLed(2, false, "LED"); */
}

void loop() { systemLoop(); }


