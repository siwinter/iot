#ifndef DEVICE_H
#define DEVICE_H

#include "cCore.h"

class cDevice : public cObserved{
  private :
  
    int value ;
  public :
	cDevice() : cObserved(NULL) { };
	cDevice(cObserver* o) : cObserved(o) { };
	virtual void doComand(int cmd) {} ;
    void setValue(int v) {
		value = v ;
		fireEvent(v) ; }
		
	int getValue(){ return value ;} } ;
	
class cIntervalSensor : public cDevice, public cTimer {
  private :
	uint16_t interval ;
  public :
	virtual void measure() = 0 ;
	void setInterval(uint16_t i) {
		interval = i ;
		int h = random(interval * 1000) ;
		setMillis(h) ; }
	void onTimeout() {
		measure() ;
		setTimer(interval); } } ;


typedef void (*cb_function)(int);

class cCallBackAdapter : public cObserver {
  protected:
	cDevice * device ;
	cb_function callBack ;
  public:
    cCallBackAdapter(cDevice * d) {
//		Serial.println("cCallBackAdapter(cDevice * d)");
		device = d ;
		device->addObserver(this); } ;
		
	cCallBackAdapter(cb_function  f, cDevice * d) {
//		Serial.println("cCallBackAdapter(cb_function  f, cDevice * d)");
		setCallBack(f) ; 
		device = d ;
		device->addObserver(this) ;}
	void setCallBack(cb_function  f) {callBack = f ;}
	void onEvent(int i, int c) {if(callBack != NULL)(*callBack)(c); } } ;

//######################################################################

class cButton : public cDevice, cLooper {
  private:
    int pin ;
    bool activeOn ;
    bool btnState ;
    bool indefinite ;
    uint32_t  bounceTime ;
    void toogle() { if (btnState == activeOn) setValue(val_on); else setValue(val_off) ; }
  public:
    cButton(int p, bool ao) {
		pin = p;
		activeOn = ao ;
		pinMode(pin, INPUT);
//		btnState = digitalRead(pin) ;
		indefinite = false ;
		toogle() ; }
		
    void onLoop() {
      if (btnState == digitalRead(pin)) { indefinite = false ; }
      else {
		  if (indefinite) {
			  if (millis() > bounceTime) { toogle() ;} }
		  else {
			  indefinite = true ;
			  bounceTime = millis() + 100 ; } } } } ;

cButton* newButton(int p, bool ao, cb_function  f) {
	cButton* d =new cButton(p, ao) ;
	cCallBackAdapter* cb = new cCallBackAdapter(f, d);
	return d ; }

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 
 *  Ein Potentiometer bzw. ein Spannungsmesser
 *  Hier muss stehen welche Pins bei welchen MicroProzessoren verwendet werden kann
 *  und welche Werte gesendet werden. Vieleicht wäre ein Wertebereich von 0 bis 100 gut
 *
 * 
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
class cPoti : public cDevice, cLooper {
  private:
    int pin ;
    int lastVal ;
  public:
    cPoti(int p ) {
		pin = p;
		int lastVal = 0 ;
		Serial.println("");  //s.w. ohne diese Zeile lässt das Programm sich nicht für ESP8266 übersetzen!!! 
		}
		
    
    void onLoop() {
      int val = analogRead(pin) ;
      if (abs(lastVal - val) > 8) {
		lastVal = val ;
		setValue(lastVal) ; } }
	} ;

cPoti* newPoti(int p, cb_function  f) {
	cPoti* d =new cPoti(p) ;
	cCallBackAdapter* cb = new cCallBackAdapter(f, d);
	return d ; }

//######################################################################

class cLatch : public cDevice, public cTimer {
  public:
    int setPin ;
    int resetPin ;
    cLatch(int sp, int rp) {
		setPin   = sp;
		resetPin = rp;
		pinMode(setPin, OUTPUT);
		pinMode(resetPin, OUTPUT);
		setOff() ; }
	
    void setOn() {
		digitalWrite(resetPin,LOW) ;
		digitalWrite(setPin,HIGH) ;
		setValue(val_on) ;
		setMillis(500) ; }
    void setOff() {
		digitalWrite(setPin,LOW) ;
		digitalWrite(resetPin,HIGH) ;
		setValue(val_off) ;
		setMillis(500) ; }
    void toogle() {
		if(getValue() == val_off) setOn() ;
		else setOff() ; }
		
	void onTimeout() {
		digitalWrite(setPin,LOW) ;
		digitalWrite(resetPin,LOW) ; }
		
    virtual void doComand(int cmd) {
		switch (cmd) {
			case cmd_on :
			  setOn() ;
			  break ;
			case cmd_off :
			  setOff() ;
			  break ;
			case cmd_toogle :
			  toogle() ;
			  break ; } } } ;

cLatch* newLatch(int sp, int rp, cb_function f) {
	cLatch* d =new cLatch(sp, rp) ;
	cCallBackAdapter* cb = new cCallBackAdapter(f, d);
	return d ; }


class cRelais : public cDevice {
  public:
    int pin ;
    bool activeOn ;
    cRelais(int p, bool ao) {
		pin = p;
		activeOn = ao ;
		pinMode(pin, OUTPUT);
		setOff() ; }
	
    void setOn() {
		digitalWrite(pin,activeOn) ;
		setValue(val_on) ;
	}
    void setOff() {
		digitalWrite(pin,!activeOn) ;
		setValue(val_off) ;
	}
    void toogle() {
		if(getValue() == val_off) setOn() ;
		else setOff() ;
	}
    virtual void doComand(int cmd) {
		switch (cmd) {
			case cmd_on :
			  setOn() ;
			  break ;
			case cmd_off :
			  setOff() ;
			  break ;
			case cmd_toogle :
			  toogle() ;
			  break ;
		}
	}
} ;

cRelais* newRelais(int p, bool ao, cb_function  f) {
	cRelais* d =new cRelais(p, ao) ;
	cCallBackAdapter* cb = new cCallBackAdapter(f, d);
	return d ; }
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 
 *   LED
 *   hier könnte stehen, wie die Konfiguration der StandardLed auf den
 *   verscihedenen Boards aussieht
 *
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

class cLed : public cRelais, public cTimer {
  private:
    uint16_t interval ;
    
  public:
	cLed(int p, bool ao) : cRelais(p, ao ){interval = 1;};
	
	void setBlink() {
		if ((getValue() == val_off) || (getValue() == val_on) ) {
			setValue(val_blink) ;
			onTimeout() ; } }
	
	void onTimeout() {
		if(getValue() == val_blink) {
			digitalWrite(pin, !digitalRead(pin)) ;
			setTimer(interval); } }

    void doComand(int cmd) {
//		Serial.println("onComand");
		switch (cmd) {
			case cmd_on :
			  setOn() ;
			  break ;
			case cmd_off :
			  setOff() ;
			  break ;
			case cmd_toogle :
			  toogle() ;
			  break ;
			case cmd_blink :
			  setBlink() ;
			  break ; } } } ;
			  
cLed* newLed(int p, bool ao, cb_function  f) {
	cLed* d =new cLed(p, ao) ;
	cCallBackAdapter* cb = new cCallBackAdapter(f, d);
	return d ; }

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 
 *   Uhr
 *   erzeugt die Uhrzeit in Form eines Integerwertes hhmm
 *   z.B. 12:10 -> 1210
 *        00:01 ->    1  
 *
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

class cClock : public cDevice, cTimer {
  protected:
    uint8_t min ;
    uint8_t hour;
    uint64_t nextMillis;
  public:
    cClock() {}
    
    void doComand(int cmd) {
		setTime(cmd); }
    
    void setTime(int t) {
		uint8_t m = t % 100;
		if (m > 59) return ;
		uint8_t h = t / 100;
		if (h > 23) return ;
		min=m; hour=h;
		setValue(h*100 + m) ;
		nextMillis = millis() + 60000;
		setMillis(60000); }
		
	void onTimeout() {
		uint64_t m = millis();
		nextMillis = nextMillis + 60000 ;
		int diff = nextMillis - m ;
		setMillis(diff);
		if(++min > 59) {
			min = 0;
			if(++hour > 23) hour = 0; }
		setValue(hour*100 + min) ;} };

cClock* newClock(cb_function  f) {
	cClock* d =new cClock() ;
	cCallBackAdapter* cb = new cCallBackAdapter(f, d);
	return d ; }

#if defined(ESP8266)

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 
 *   Frequenzmesser
 *   misst die Zahl der Interrupte an Pin p innerhalb von einer Sekunde
 *   funktioniert für den ESP8266  
 *
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

int ticks = 0 ;

void ICACHE_RAM_ATTR handleInterrupt() { ticks = ticks + 1 ; }

class cHzMesser : public cDevice, cTimer {
  protected:
    int pin ;
  public:
    cHzMesser(int p) {
		pin = p ;
		attachInterrupt(pin, handleInterrupt, CHANGE);
		setTimer(1) ; }
	void onTimeout() {
		setValue(ticks) ;
		ticks = 0 ;
		setTimer(1) ; } };

cHzMesser* newHzM(int p, cb_function  f) {
	cHzMesser* d =new cHzMesser(p) ;
	cCallBackAdapter* cb = new cCallBackAdapter(f, d);
	return d ; }


/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 
 *	VccSensor misst die anliegende Versorgungsspannung an einem ESP8266
 * 	genutzt wird dabei der Analog/Digitalwandler. Die Eingangsspannung muss < 1V
 *	Bei eingen Modellen ist die Eingangsspanung über einen Spannungsteiler dort
 * 	angelegt. Damit kann man dann diese Eingangsspannung messen.
 * 
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

ADC_MODE(ADC_VCC);

class cVcc : public cIntervalSensor {
  public :
    cVcc() {
		setInterval(30); }
    void measure(){
		uint16_t f=ESP.getVcc();
		f = (f + f/10) / 10 ;
		setValue(f); } } ;

cVcc* newVcc(cb_function  f) {
	cVcc* d =new cVcc() ;
	cCallBackAdapter* cb = new cCallBackAdapter(f, d);
	return d ; }

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 
 *	HeapSensor gibt die Größe des verbleibenden Heaps aus
 * 
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

class cHeap : public cIntervalSensor {
  public :
    cHeap() {
		setInterval(30); }
    void measure(){
		uint16_t f=ESP.getFreeHeap();
		setValue(f); } } ;

cHeap* newHeap(cb_function  f) {
	cHeap* d =new cHeap() ;
	cCallBackAdapter* cb = new cCallBackAdapter(f, d);
	return d ; }

#endif

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 
 *  Ein Ultraschallsensor, der den Abstand zwischen Sensor und z.B. einer Flüssigkeitsoberfläche misst.
 *  Hier muss stehen welche Pins bei welchen MicroProzessoren verwendet werden können.
 *  Außerdem wäre ein Hinweis auf den Messbereich sinnvoll.
 *
 * 
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

class cUltraSonicSensor : public cIntervalSensor {
  private :
    int triggerPin ;
    int echoPin ;

  public :
	cUltraSonicSensor(int trigger, int echo) {
		triggerPin = trigger ;
		echoPin = echo ;
		pinMode (triggerPin, OUTPUT);
		pinMode (echoPin, INPUT);
		setInterval(10) ;}
    void measure() {
		digitalWrite (triggerPin, LOW); 
		delayMicroseconds (2); 
		digitalWrite (triggerPin, HIGH); 
		delayMicroseconds (10); 
		digitalWrite (triggerPin, LOW); 
		int duration = pulseIn (echoPin, HIGH);
		int distance = (duration / 2) / 29.1;
		setValue(distance); } } ;

cUltraSonicSensor* newUSS(int t, int e, cb_function  f) {
	cUltraSonicSensor* d =new cUltraSonicSensor(t, e); ;
	cCallBackAdapter* cb = new cCallBackAdapter(f, d);
	return d ; }

#endif


