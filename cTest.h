#ifndef TEST_H
#define TEST_H

#include <Arduino.h>
#include "cCore.h"
//#include "cNetwork.h"

class cTest;
tList<tLink<cTest>, cTest>  theTests ;

class cTest : public cObserved{
  public :
	cTest() { 
		theTests.append(this); 
		registerTest();}
	void registerTest() ;
	
	virtual void startTest() = 0; 	 
	void endTest() {fireEvent(val_off); } };

class cTester : public cObserver  , public cTimer{
  public:
	cTest* nextTest ;
	cTester() { setTimer(1); }
		
	void onTimeout(){
		Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> start Tests");
		nextTest = theTests.readFirst() ;
		onEvent(0,0);}
		
	void onEvent(int i, int c) {
		if(nextTest != NULL) nextTest->startTest() ;
		nextTest = theTests.readNext();
		if (nextTest == NULL) nextTest = theTests.readFirst();} } ;

cTester theTester ;

void cTest :: registerTest() { addObserver(&theTester); } ;

//###################################### Timer-Test ########################################

class cTimerClass : public cTimer {
  public :
	char name[5] ;
	uint8_t t1 ;
	uint8_t t2 ;
	uint8_t t3 ;
	uint8_t cnt ;
	cTimerClass(char* n, uint8_t t_1, uint8_t t_2, uint8_t t_3) {
		strcpy(name,n) ;
		t1 = t_1; t2 = t_2; t3 = t_3; cnt=0;}

	void start() {
		cnt = 0;
		Serial.print("start Timer");Serial.println(name);
		setTimer(t1);
	}
	void onTimeout() {
		Serial.print("onTimeout "); Serial.println(name);
		cnt = cnt + 1;
		if(cnt==1) {
			Serial.print("start second Timer");Serial.println(name);
			setTimer(t2);
			resetTimer();
			setTimer(t3); } } };

class cTimerTest : public cTest , public cTimer{
  public :
  cTimerClass* tim1 ;
  cTimerClass* tim2 ;
  cTimerClass* tim3 ;
	cTimerTest() {
		tim1 = new cTimerClass("3",3,5,4) ;
		tim2 = new cTimerClass("1",1,2,4) ;
		tim3 = new cTimerClass("2",2,1,4) ; }
	void startTest() {
		Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> start Timer Test") ;
		tim1->start();
		tim2->start();
		tim3->start();
		setTimer(10);} 
	void onTimeout(){endTest();} } ;

cTimerTest theTimerTest;

//####################################### LED-Test #########################################

void ledCallBack(int v) {
	if (v == val_on) {
		Serial.println("Callback Event: LED on");
		return ;}
	if (v == val_off) {
		Serial.println("Callback Event: LED off");
		return ;}
	if (v == val_blink) {
		Serial.println("Callback Event: LED blinking");
		return ;}
	Serial.print("Callback Event: LED unknown Event; "); Serial.println(v); }

class cLedTest : public cTest, public cTimer {
  private :
	cLed* l ;
	uint8_t cnt = 0;
  public :
	cLedTest() {
		l = newLed(ledCallBack); }

	void startTest() {
		Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> start LED Test") ;
		cnt = 0;
		setTimer(1); }
	void onTimeout() {
		cnt = cnt+1;
		setTimer(5) ;
		if (cnt==1){ Serial.println(">comand: on"); l->doComand(cmd_on); return; }
		if (cnt==2){ Serial.println(">comand: off"); l->doComand(cmd_off); return; }
		if (cnt==3){ Serial.println(">comand: toogle"); l->doComand(cmd_toggle); return; }
		if (cnt==4){ Serial.println(">comand: blink"); l->doComand(cmd_blink); return; }
		if (cnt==5){ Serial.println(">comand: toggle"); l->doComand(cmd_toggle); return; }
		if (cnt==6){ Serial.println(">comand: on"); l->doComand(cmd_on); return; }
		if (cnt==7){ Serial.println(">comand: off"); l->doComand(cmd_off); return; }
		if ( cnt>7 ) endTest() ;}};

cLedTest theLedTest ;



//################################## Hilfsprogramm-Test ####################################

class cHelperTest : public cTest, public cTimer {
  private :
	uint8_t cnt = 0;
  public :
	void startTest() {
		cnt = 0;
		Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> start Helper Test") ;
		setTimer(1); }
	void onTimeout() {
		cnt = cnt+1;
		setTimer(5) ;
		if (cnt==1){ 
			Serial.println("ip2txt");
			return; }
		if ( cnt>6 ) endTest() ;}};
 
//cHelperTest theHelperTest ;

//################################## Dummy-Test ####################################

class cDummyTest : public cTest, public cTimer {
  public :
	void startTest() {
		Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> start Dummy 0") ;
		setTimer(5); }
	void onTimeout() {
		Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> end Dummy 0") ;
		endTest(); } };
//cDummyTest theDummyTest ;

class cDummyTest1 : public cTest, public cTimer {
  public :
	void startTest() {
		Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> start Dummy 1") ;
		setTimer(5); }
	void onTimeout() {
		Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> end Dummy 1") ;
		endTest(); } };
//cDummyTest1 theDummyTest1 ;

class cDummyTest2 : public cTest, public cTimer {
  public :
	void startTest() {
		Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> start Dummy 2") ;
		setTimer(5); }
	void onTimeout() {
		Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> end Dummy 2") ;
		endTest(); } };
//cDummyTest2 theDummyTest2 ;

#endif
