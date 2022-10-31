#ifndef CORE_H
#define CORE_H

#include <Arduino.h>
#include "flags.h"

#define cmd_off    0
#define cmd_on     1
#define cmd_toggle 2
#define cmd_blink  3

#define val_off    		 0
#define val_on     		 1
#define val_toogle 		 2
#define val_blink  		 3
#define val_connected 	 4
#define val_wifiAP 		12

//#################################### List Template #####################################

template < typename T >
class tLink {
  public:
	T* element ;
	tLink* next ;};

template <typename Tl, typename Te>	//Tl = Link, Te = Element
class tList {
  private :
	Tl* anchor;
	Tl* emptyChain ;
	Tl* actualLink ;

  public:
  	tList() {
		anchor = NULL ;
		actualLink = NULL;
		emptyChain = NULL ;}

	Tl** getAnchor() { return &anchor; }

	Tl * newLink() {
		if (emptyChain == NULL ) return new Tl() ;
		Tl* r = emptyChain ;
		emptyChain = r->next ;
		return r ; }

	void returnLink(Tl* l) {
		l->next = emptyChain ;
		emptyChain = l ; }

	void insert(Te* e) {
		Tl* nL = newLink() ;
		nL->element = e ;
		insertLink(nL); }

	void insert(Te* e, Tl* l ) {
		Tl* nL = newLink() ;
		nL->element = e ;
		nL->next = l->next ;
		l->next = nL ; }

	void insertLink(Tl*l) {
		l->next = anchor ;
		anchor = l ; }

	void appendLink(Tl* l) {
		l->next = NULL ;
		Tl** p = &anchor ;
		while((*p) != NULL) p= &((*p)->next) ;
		l->next = (*p) ;
		(*p) = l ; }

	void append(Te* e) {
		Tl* nL = newLink() ;
		nL->element = e ;
		appendLink(nL) ; }

	Te* getFirst() {
		if (anchor == NULL) return NULL;
		Tl* l = anchor ;
		Te* e = anchor->element ;
		anchor = anchor->next ;
		returnLink(l);
		return e ; }

	Te* readFirst() {
		actualLink = anchor ;
		return readNext() ; }

	Te* readNext() {
		if (actualLink == NULL) return NULL ;
		Te* e = actualLink->element;
		actualLink =actualLink->next ;
		return e ;}
		
	Tl* getFirstLink() {
		if (anchor == NULL) return NULL ;
		Tl* l = anchor ;
		anchor = l->next ;
		return l ; } };

//####################################### cLooper ######################################## 

class cLooper ;
tList<tLink<cLooper>, cLooper>  theLoopers ;
class cLooper {
  public:
	cLooper() { theLoopers.insert(this); }
    virtual void onLoop() = 0; } ;

//####################################### cTimer ######################################### 
class cTimer ;
tList<tLink<cTimer>, cTimer>  theTimers ;

class cTimer {
  public:
	uint64_t timeToExpire ;
	virtual void onTimeout() = 0 ; 
	cTimer() {timeToExpire = 0 ;}

	void resetTimer() {
		timeToExpire = 0 ;
		tLink<cTimer>** p = theTimers.getAnchor() ;
		while ( *p != NULL) {
			if ((*p)->element == this) {
				tLink<cTimer>* l=(*p); 
				(*p) = (*p)->next;
				theTimers.returnLink(l);
				return ; }
			p = &((*p)->next); }}

	void setTimer(uint32_t t) {
		if (timeToExpire != 0) resetTimer() ;
		timeToExpire = millis() + t*1000;
		insertTimer() ;}

	void setMillis(uint32_t t) {
		if (timeToExpire != 0) resetTimer() ;
		timeToExpire = millis() + t;
		insertTimer() ;}

	void insertTimer() {
		tLink<cTimer>* newL = theTimers.newLink();
		newL->element = this ;
		tLink<cTimer>** p = theTimers.getAnchor();
		while((*p) != NULL) {
			if((*p)->element->timeToExpire > timeToExpire) {
				newL-> next = (*p);
				(*p) = newL ;
				return; } 
			p = &((*p)->next); }
		newL->next = NULL ;
		(*p) = newL;
		return; } };

//###################################### cObserver ####################################### 
class cObserver {
  public:
	cObserver() {} ;
    virtual void onEvent(int i, int evt)=0; } ;

class cObserved {
  private :
	int index ;
  public :

	tList<tLink<cObserver>, cObserver>  theObservers ;
	static int oNumber ;			// statisch für alle Objekte (Objektzähler)
	cObserved() {
		oNumber = oNumber++ ;
		index = oNumber ; };
	cObserved(cObserver * o) : cObserved() {addObserver(o);}
	int addObserver(cObserver * o){
		theObservers.insert(o);
		return index ; }
		
	int getIndex() {return index ;}

	void fireEvent(int c) {
		cObserver * o = theObservers.readFirst();
		while (o != NULL) {
			o->onEvent(index,c) ;
			o = theObservers.readNext();
		} } };
			
int  cObserved :: oNumber = 0 ;	// Initialisisierung des Objektzählers

//#################################### cConfigurator ##################################### 
class cConfig ;
tList<tLink<cConfig>, cConfig>  theConfigs ;

class cConfigurator ;
cConfigurator* theConfigurator = NULL ;

class cConfig {
  public:
	cConfig() { theConfigs.insert(this); }
	virtual bool configure(const char* key, char* value, int vLen = 0) = 0 ;
	virtual void start() = 0 ;} ;
	
class cConfigurator {
  protected :
	void configure(char* key, char* value, int vLen = 0) {
		cConfig* c = theConfigs.readFirst();
		while ( c != NULL) { 
			if ( c->configure(key, value, vLen) ) break ;
			c = theConfigs.readNext(); } } ;

	void start() {
		cConfig* c = theConfigs.readFirst();
		while ( c != NULL) { 
			c->start() ;
			c = theConfigs.readNext(); } } ;
  public:
	cConfigurator() {theConfigurator = this ;}
	virtual void setConfig(char* key, char* value, int len=0) = 0 ;} ;
	
//##################################### systemLoop ####################################### 

void systemLoop() {
	cLooper* l = theLoopers.readFirst() ;
	while ( l != NULL) {
		l->onLoop();
		l = theLoopers.readNext() ; }
	cTimer*t = theTimers.readFirst();
	if (t!= NULL) {
		if (millis() < t->timeToExpire ) return ;
		t->resetTimer();
		t->onTimeout(); } }


#endif
