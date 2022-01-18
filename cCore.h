#ifndef CORE_H
#define CORE_H

#include <Arduino.h>

uint8_t version = 2 ;
const char * txtVersion = "0.0.2" ;

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

class cTimer {
  public:
    cTimer * nextT ;
    unsigned long timeToExpire ;
    void resetTimer() {
		cTimer * t = aTimer ;
		while (t != NULL) {
			if (t->nextT == this) {
				t->nextT = nextT ;
				break; }
			t = t->nextT ; }
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
		if (timeToExpire != 0) resetTimer() ;
		timeToExpire = millis() + (t *1000) ;
		insert() ;}
    void setMillis(int t) {
		if (timeToExpire != 0) resetTimer() ;
		timeToExpire = millis() + t ;
		insert() ;};	
    virtual void onTimeout() = 0 ; } ;

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

#define cNamLen 20
#define cInfoLen 20
class cMsg {
  public :
	cMsg * nextMsg ;
	char name[cNamLen] ;
	char info[cInfoLen] ;
	cMsg() { nextMsg = NULL; }  } ;

class cMsgQueue {
  private :
	char name[5] ;

	cMsg * first ;
	cMsg * last ;
	static cMsg * freeAnchor ;
  public :
	cMsgQueue() { first = last = NULL; }
	void setName(const char* n) {strcpy(name, n);}
	void insert(cMsg * m) {
//		Serial.print(name);Serial.print(" : insert "); Serial.println((uint32_t)m) ;
		if (first == NULL) first = m;
		else last->nextMsg = m ;
		last = m ;
		last->nextMsg = NULL ;
		printF();}
	void remove() {
		cMsg* m = first ;
//		Serial.print(name);Serial.print(" : remove ") ; Serial.println((uint32_t)m) ;
		if (m != NULL) first = m->nextMsg ;
		m->nextMsg = freeAnchor ;
		freeAnchor = m ; 
		printF();}
	cMsg* read() { return first;}
	cMsg* newMsg() {
		cMsg* m = freeAnchor;
//		Serial.print(name);Serial.print(" : newMsg ") ; Serial.println((uint32_t)m) ;
		if(m != NULL) freeAnchor = m->nextMsg ;
		else m= new cMsg();
		printF() ;
		return m ; }
	void printF() { }/*
		Serial.println("Free:") ;
		cMsg* a =freeAnchor ;
		while (a != NULL) { Serial.println((uint32_t)a); a=a->nextMsg;}
		a=first;
		Serial.print(name) ;Serial.println(":") ;
		while (a != NULL) { Serial.println((uint32_t)a); a=a->nextMsg;}
		Serial.println(); }*/ } ;

cMsg* cMsgQueue :: freeAnchor = NULL ;  // static freeAnchor of common chain for all queues 

cMsgQueue * outQueue ;
cMsgQueue * inQueue ;

class cMsgHandler ;
cMsgHandler* aHandler;

class cMsgHandler {
  protected:
	cMsgQueue * writeQueue ;
	char name[10] ;
  public:
	cMsgHandler* nextHandler ;
	cMsgHandler() {
		nextHandler = aHandler;
		aHandler = this ; 
		writeQueue = outQueue ;}
		
	cMsg* newMsg() {return writeQueue->newMsg();}
	void writeMsg(cMsg* msg) {writeQueue->insert(msg); }
	
	bool onMsg( cMsg * m) {
//		Serial.print("cMsgHandler : onMsg : msg->name : "); Serial.println(m->name);
		int j ;
		for(int i=0; i<strlen(m->name);i++) if (m->name[i] == '/') j=i ;
//		Serial.println(&m->name[j+1]);
		if ( strcmp(&m->name[j+1], name) != 0 ) return false ;
		handleMsg(m) ;
		return true ;
		}
	
	virtual void handleMsg(cMsg* m) {}	
};

char systemName[8] ;			// siwi: sollte der systemname werden!

class cChannel {
  protected:
	cMsgQueue * receivedQueue ;
    int topicLen ;
    char pubTopic[cInfoLen] ;
	void initTopic(char* t) {
		strcpy(pubTopic,"evt/");
		strcat(pubTopic, t); 
		strcat(pubTopic, "/") ;
		topicLen = strlen(pubTopic); }
	char* setTopic(char* t) {
		strcpy(&pubTopic[topicLen],t) ;
		return pubTopic; }
  public:
	void setQueue(cMsgQueue* q) { receivedQueue = q ; }
	virtual bool sendMsg(cMsg * msg) = 0 ; } ;

class cCore : public cLooper {
  public:
	cChannel  * mainChannel ;

	cCore() {
		outQueue = new cMsgQueue() ;
		outQueue->setName("out");
		inQueue = new cMsgQueue() ;
		inQueue->setName("in"); 
		aTimer = NULL ;}

	void setMainChannel(cChannel * c) {
		mainChannel = c ;
		mainChannel->setQueue(inQueue); }
				
	void onLoop() {
		// treat timers
		if (aTimer != NULL) {
			if (millis() >= aTimer->timeToExpire) {
				cTimer * t = aTimer ;
				aTimer = t->nextT ;
				t->timeToExpire = 0 ;
				t->onTimeout() ; } }
		// treat outQueue (send message)
		cMsg* m = outQueue->read() ;
		if (m != NULL) { if (mainChannel->sendMsg(m)){outQueue->remove() ;}}
		// treat inQueue (receive message)
		m = inQueue->read() ;
		if(m != NULL) {
//			Serial.print("onLoop : inQueue read ");Serial.print(m->name);Serial.print(":");Serial.println(m->info);
			cMsgHandler * d = aHandler ;
			while(d != NULL) {
				if(d->onMsg(m)) break ;
				d=d->nextHandler ;}
			inQueue->remove(); } } } ;

cCore theCore;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
