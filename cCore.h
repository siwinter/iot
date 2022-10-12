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

//bool global_p=false ;

template < typename T >
class tLink {
  public:
	T* element ;
	tLink* next ;};

template <typename Tl, typename Te>	//Tl = Link, Te = Element
class tList {
  private :
	Tl* anchor;
	Tl* index ;
	Tl* emptyChain ;
	int i;
	
	Tl * newLink() {
		if (emptyChain == NULL ) return new Tl() ;
		else {
			Tl* r = emptyChain ;
			emptyChain = r->next ;
			return r ; } }
			
	void returnLink(Tl* l) {
		l->next = emptyChain ;
		emptyChain = l ; }

  public:
  	tList() {
		anchor = NULL ;
		index = NULL ;
		emptyChain = NULL ;}
		
	void insert(Te* e, Tl* l = NULL) {
		Tl* nl = newLink() ;
		nl->element = e ;
		if (l == NULL) {
			nl->next = anchor ;
			anchor = nl ; }
		else {
			nl->next = l->next ;
			l->next = nl ; }}
			
	void append(Te* e) {
		if (anchor == NULL) insert(e);
		else {
			Tl* l=anchor;
			while(l->next != NULL) l=l->next;
			insert(e, l) ; } }
			
	Te* getFirst() {
		if (anchor != NULL) {
			Tl* l = anchor ;
			Te* e = anchor->element ;
			anchor = anchor->next ;
			returnLink(l);
			return e ; }
		else return NULL; }
	
	Te* getNext(Te* e) {
		if (e == NULL) index = anchor ;
		if (index == NULL) return NULL;
		Te* re = index->element ;
		index = index->next ;
		return re ; }
		
	Te* readNext(Te* e) { return getNext(e); }
			
	Tl* getNextLink(Tl* l) {
		if (l == NULL ) return anchor ;
		return l->next ; }
		
	void deleteElement(Te* e) {
		Tl* l = anchor ;
		Tl* vl = NULL ;
		while (l != NULL) {
			if (l->element == e) {
				if (vl == NULL) anchor = l->next ;
				else vl->next = l->next ;
				returnLink(l) ;
				break ; }
			vl = l ;
			l = l->next; } }
};

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
		theTimers.deleteElement(this); }

	void setTimer(uint32_t t) {
		if (timeToExpire != 0) resetTimer() ;
		timeToExpire = millis() + t*1000;
		insertTimer() ;}

	void setMillis(uint32_t t) {
		if (timeToExpire != 0) resetTimer() ;
		timeToExpire = millis() + t;
		insertTimer() ;}

	void insertTimer() {
		tLink<cTimer>* actualLink = theTimers.getNextLink(NULL);
		tLink<cTimer>* lastLink = NULL;
		while(actualLink != 0) {
			if  (actualLink->element->timeToExpire > timeToExpire) break;
			lastLink = actualLink;
			actualLink = theTimers.getNextLink(lastLink) ; }
		theTimers.insert(this, lastLink) ; } };

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
		cObserver * o = theObservers.getNext(NULL);
		while (o != NULL) {
			o->onEvent(index,c) ;
			o = theObservers.getNext(o);
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
	virtual bool configure(char* key, char* value, int vLen = 0) {return false;} } ;

class cConfigurator {
  protected:	
	void configure(char* key, char* value, int vLen = 0) {
		cConfig* c = theConfigs.getNext(NULL);
		while ( c != NULL) { 
			if ( c->configure(key, value, vLen) ) break ;
			c = theConfigs.getNext(c); } } ;
  public:
	cConfigurator() {theConfigurator = this ;}
	virtual void setConfig(char* key, char* value, int len=0) = 0 ;} ;
	
//##################################### systemLoop ####################################### 
void systemLoop() {
	cLooper* l = theLoopers.getNext(NULL) ;
	while ( l != NULL) {
		l->onLoop();
		l = theLoopers.getNext(l) ; }
	cTimer*t = theTimers.getNext(NULL);
	if (t!= NULL) {
		if (millis() < t->timeToExpire ) return ;
		t->resetTimer();
		t->onTimeout(); } }


#endif
