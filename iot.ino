/*
#include "cDevice.h"
#include "cWifi.h"

#include "cChannel.h"
#include "cWifi.h"
#include "cMqtt.h"
#include "cDHT.h"

class cTest : public cTimer {
	cDHT22* theDHT;
	cLed* 	theLED ;
//	cVcc* 	theVcc ;
	uint16_t interval ;
  public :
	cTest() {
		interval = 10 ;
		setTimer(interval) ;
		theLED = newLed.create(2, false, "led1") ;
//		theDHT = newDHT22.create(5,"1") ;
//		theVcc = newVcc.create("vcc") ;
}
	
	void onTimeout() {
		theLED->toogle();
		/*setTimer(interval); } } ;
		
cTest* theTest;

#include "cDatabase.h"

//cConfigurator* theConfig ;
cMqttChannel * myChannel;
void setup() {
	Serial.begin(115200);
	delay(5000);
	Serial.println("setup");
//	theDataBase.clear();
	strcpy(systemName, "Wohnz");
//	theWifi.setAuth("Hundehaus2", "Affenhaus");
	myChannel = new cMqttChannel();
//	myChannel->newBrokerIP("192.168.178.118");
//	myChannel->newBrokerPort("1883");
	Core.setMainChannel(myChannel);
//	Core.setMainChannel(new cSerialChannel());
	Serial.println("setup");
//	theTest = new cTest();
	delay(5000);
/*	theConfig = new cConfigurator();
	theConfig->printEEPROM(); 
	Serial.println("now formatEEPROM");
	theConfig->formatEEPROM();
	theConfig->printEEPROM();
	delay(5000);
	Serial.println("now addConfig");
	theConfig->addConfig("12", "4567") ;
	theConfig->addConfig("12", "4567") ;
	theConfig->printEEPROM();
		Serial.println("now addConfig");
	theConfig->addConfig("234", "4567") ;
	theConfig->addConfig("12", "4567") ;
	theConfig->printEEPROM();
	delay(5000);
	Serial.println("now deleteConfig");
	theConfig->delConfig("12") ;
	theConfig->printEEPROM();
	Serial.println("now deleteConfig");
	theConfig->delConfig("23") ;
	theConfig->printEEPROM();

}
*/

#include "cDevice.h"
#include "cMqtt.h"
#include "cIRrec.h"

cIrRec* IR;

void setup() {
	Serial.begin(115200);
	Serial.println("start");
	theCore.setMainChannel(&theMqtt);
	cIrRec* irRec = newIrReceiver.create(14, NEC, 0, "irLED") ;
	cIrProt* irTV = newIrProtocol.create(NEC, 48896,"irTV") ;
	irRec->addProtocol(irTV) ; }

//	cLatch* l = newLatch.create(12,14,"LRL"); 
//	cLed* led = newLed.create(2, false, "LED") ; }
	
void loop() { systemLoop(); }
