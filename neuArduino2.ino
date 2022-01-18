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
#include "cWifi.h"*/
//#include "cMqtt.h"
//#include "cDHT.h"
#include "cDevice.h"

class cTest : public cObserver {
//	cDHT22*   theDht;
	cLed* 	  theLed ;
//	cDevice*  theTemp ;
	cClock*   theClock ;
  public:
	cTest() {
		theLed = newLed.create(25, false) ;
//		theDht = newDHT22.create(5,"1") ;
//		theTemp = theDht->getTemperatureSensor(); 
//		theTemp->addObserver(this);
		theLed->setBlink();
		theClock= newClock.create();
		theClock->setTime(0);} ;
	void onEvent(int i, int e) {
		Serial.print("Temp"); Serial.println(e); 
		if (e < 200) theLed->setBlink();
		else theLed->setOff(); } };

cTest test;

void setup() {
	Serial.begin(115200);
	Serial.println("start");
//	theCore.setMainChannel(&theMqtt); }
}

#define hutliburli

void loop() { systemLoop(); 
	delay(1000);
#if defined(ARDUINO_RASPBERRY_PI_PICO)
	Serial.println("ARDUINO_RASPBERRY_PI_PICO");
#endif
#if defined(ARDUINO_GENERIC_RP2040)
	Serial.println("ARDUINO_GENERIC_RP2040");
#endif
#if defined(hutliburli)
	Serial.println("hutliburli");
#endif
#if defined(rp2040)
	Serial.println("rp2040");
#endif
#if defined(xx)
	Serial.println("xx");
#endif

	Serial.printf("Core temperature: %2.1fC\n", analogReadTemp());}
