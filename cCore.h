#ifndef CORE_H
#define CORE_H

#include <Arduino.h>

uint8_t version = 2 ;
const char * txtVersion = "0.0.2" ;

#define cmd_off    0
#define cmd_on     1
#define cmd_toogle 2
#define cmd_blink  3

#define val_off    		 0
#define val_on     		 1
#define val_toogle 		 2
#define val_blink  		 3
#define val_connected 	 4
#define val_wifiAP 		12



class cLooper ;
cLooper * aLooper ;

class cLooper {
  public:
	cLooper * nextL ;
	cLooper() {
		nextL = aLooper ;
		aLooper = this ; }
    virtual void onLoop() = 0; } ;

void systemLoop() {
	cLooper * l = aLooper;
		while (l != NULL) { l->onLoop() ; l= l->nextL; } }
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class cTimer ;
cTimer * aTimer ;
void printTimers() ;
class cTimer {
  public:
    cTimer * nextT = NULL;
    unsigned long timeToExpire ;
    void resetTimer() {
//		Serial.println("cTimer:resetTimer");
//		printTimers();
		if ( aTimer == this ) aTimer = aTimer->nextT ;
		else {
			cTimer * t = aTimer ;
			while (t != NULL) {
			if (t->nextT == this) {
				t->nextT = nextT ;
				break; }
			t = t->nextT ; } }
		timeToExpire = 0 ;
		nextT = NULL; }
    void insert() {
		cTimer ** pTimer = &aTimer ;
		while (*pTimer != NULL) {
			if (timeToExpire < (*pTimer)->timeToExpire) {
				nextT = (*pTimer) ;
				*pTimer = this ;
				return ; }
			pTimer = &(*pTimer)->nextT ; }
		*pTimer = this ;
		nextT = NULL ; }
    void setTimer(int t) {
//		Serial.println("cTimer:setTimer");
		if (timeToExpire != 0) resetTimer() ;
		timeToExpire = millis() + (t *1000) ;
		insert() ;}
    void setMillis(int t) {
//		Serial.println("cTimer:setMillis");
		if (timeToExpire != 0) resetTimer() ;
		timeToExpire = millis() + t ;
		insert() ;};	
    virtual void onTimeout() = 0 ; } ;

void printTimers() {
	Serial.print("number of timers; ") ;
	int tims = 1 ;
	if (aTimer == NULL) Serial.println(0) ;
	else {
		cTimer* lT = aTimer;
		while ( lT->nextT != NULL ) {
			lT = lT->nextT;
			tims = tims + 1 ;
		}
	Serial.println(tims); } }

class cObserver {
  public:
	cObserver() {} ;
    virtual void onEvent(int i, int evt)=0; } ;

class cObserverLink {
  public :
	cObserver * item ;
	cObserverLink * nextLink ; 
	cObserverLink(cObserver* i) {item = i ;}};

class cObserved {
  private :
	int index ;
  public :
	cObserverLink * aObserver ;
	static int oNumber ;
	cObserved() : aObserver(NULL) {
		oNumber = oNumber++ ;
		index = oNumber ; };
	cObserved(cObserver * o) : cObserved() {addObserver(o);}
	int addObserver(cObserver * o){
		if (o != NULL) {
			cObserverLink* n = new cObserverLink(o) ;
			n->nextLink = aObserver ;
			aObserver = n ; }
		return index ; }
		
	int getIndex() {return index ;}

	void fireEvent(int c) {
		cObserverLink* o = aObserver;
		int i = 0 ;
		while (o != NULL){o->item->onEvent(index, c); o = o->nextLink; } } };
int  cObserved :: oNumber = 0 ;

class cCore : public cLooper {
  public:

	cCore() { aTimer = NULL ;}
				
	void onLoop() {
		// treat timers
		if (aTimer != NULL) {
			if (millis() >= aTimer->timeToExpire) {
//				Serial.println("cCore:timer expire");
				cTimer * t = aTimer ;
				aTimer = t->nextT ;
				t->timeToExpire = 0 ;
//				printTimers() ;
				t->onTimeout() ; } } } } ;

cCore theCore;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*
class cFactory ;
cFactory * aFactory;

class cFactory {
  public:
	char name[6];
	cFactory* nextFac;
	cFactory() {nextFac =  NULL; regFactory(); }
    void regFactory() {
		nextFac = aFactory ;
		aFactory = this ; } 
	virtual bool make() {return false  ; } } ;

class cCreator : public cMsgHandler {
	void onTest(cMsg* msg) {
		cFactory* f = aFactory ;
		while( f != NULL ) {f->make(); f = f->nextFac; } }
	void handleMsg(cMsg * msg) {
		Serial.println("cCreator : handleMsg");
		cFactory* getFirstFactory(); } } ;

cCreator Creator;
*/
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class cSetup ;
cSetup * aSetup;

class cSetup {
  public:
	char name[6];
	cSetup* nextSetup;
	cSetup() {nextSetup =  NULL; regSetup(); }
    void regSetup() {
		nextSetup = aSetup ;
		aSetup = this ; } 
	virtual bool configure(char topic, char* data) {return false  ; } } ;
#endif
