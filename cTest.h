#ifndef TEST_H
#define TEST_H

#include <Arduino.h>
#include "cCore.h"
#include "cNetwork.h"

class cLoopTest : public cLooper {
  private :
	int periode ;
	int n = 0 ;
	unsigned long timeToExpire ;
	char name[5] ; 
  public :
	cLoopTest(int p, char* n) {
		strcpy(name,n);
		periode = p;
		timeToExpire = millis() + (periode *1000) ;}
	
	void onLoop() { if(millis() > timeToExpire ) {
		timeToExpire = millis() + (periode *1000) ;
		Serial.print(name); Serial.print(": ");
		Serial.println (n++) ; 
	} }
} ;

class cObservedTest : public cObserved, public cTimer {
  public :
	int periode ;
	char name[5] ;
	int evt ;
	cObservedTest(int t, char* n) {
		evt = 0 ;
		periode = t ;
		strcpy(name, n) ; 
		setTimer(periode) ;}
		
	void onTimeout() {
		setTimer(periode) ;
		Serial.print(name);Serial.println(".onTimeout");
		fireEvent(evt++); } };
		
class cObserverTest : public cObserver {
  public :
	char name[5] ;
	cObserverTest(char* n) {
		strcpy(name, n) ; }
	
	void onEvent(int i, int evt) {
		Serial.print(name);Serial.print(".onEvent: "); Serial.println(evt); } };

class cTxtLedTest: public cTimer {
  private:
	cLed* myLed ;
  public :
	cTxtLedTest(int pLed = 2) {
#ifdef ARDUINO_AVR_LARDU_328E
		pLed = 13 ;
#endif
		myLed = newLed(pLed, false, "LED1") ;
		setTimer(2) ;
	}
	void onTimeout() {
		setTimer(5);
		setTimer(2);
		myLed->doComand(cmd_toogle); } } ;


class cLedTest: public cTimer {
  private:
	cLed* myLed ;
  public :
	cLedTest() {
		myLed = new cLed(2, false) ;
		setTimer(2) ;
	}
	void onTimeout() {
		setTimer(5);
		setTimer(2);
		myLed->doComand(cmd_toogle);
	}
	
} ;
#endif
