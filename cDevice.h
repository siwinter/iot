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
		setMillis(random(interval * 1000)) ; }
	void onTimeout() {
		measure() ;
		setTimer(interval); } } ;


#define cmd_off    0
#define cmd_on     1
#define cmd_toogle 2
#define cmd_blink  3

#define val_off    0
#define val_on     1
#define val_toogle 2
#define val_blink  3

class cTranslator {
  public:
	virtual char* int2str(char* s, int i) = 0;
	virtual int str2int(char* s) = 0; } ;

class cStateTranslator : public cTranslator {
	char* int2str(char* s, int i) {
		switch (i) {
		  case val_off :
			strcpy(s,"off") ;
			break ;
		  case val_on :
			strcpy(s,"on") ;
			break ;
		  case val_toogle :
			strcpy(s,"toogle") ;
			break ;
		  case val_blink :
			strcpy(s,"blink") ;
			break ; }
		return s; };
		
	int str2int(char* s) {
		if (strcmp(s, "off") == 0) return cmd_off ;
		if (strcmp(s, "on") == 0) return cmd_on ;
		if (strcmp(s, "toogle") == 0) return cmd_toogle ;
		if (strcmp(s, "blink") == 0) return cmd_blink ; 
		return 99 ;} ; } ;

class cValueTranslator : public cTranslator {
	char* int2str(char* s, int v) {
		uint16_t i = 0 ;
		if (v < 0) s[i++] = '-' ;
		v = abs(v) ;
		char rS[15] ;	// reverse string
		uint8_t l = 0 ;
		while (v > 0) {
			uint8_t d = v % 10 ;
			rS[l++] = (char)(d +'0');
			v = v /10 ; }
		while (l<(decimals + 1)) rS[l++] = '0' ;
		while (l > 0) {
			if (l == decimals) if (decimals > 0) s[i++] = '.' ;
			s[i++] = rS[--l] ; }
		s[i++] = 0 ;
		return s;  };
		
	int str2int(char* s) {
		int32_t v = 0 ;
		bool negativ = false ;
		uint8_t i = 0 ;
		uint8_t l = strlen(s) ;
		if (s[0] =='-') {
			negativ = true ;
			i = i+1 ;}
		while (i<l) {
			v = v * 10 ;
			v = v + s[i++] - '0' ; } 
		return v ;} ;
	int decimals ;
  public :
	cValueTranslator(int d) {decimals = d;} } ;


class cMsgAdapter : public cMsgHandler, public cObserver {
  protected:
	cDevice * device ;
	cTranslator * format ;
  public:
	cMsgAdapter(cTranslator* sf, const char* n, cDevice *d) : cMsgHandler(){
		device = d;
		device->addObserver(this);
		strcpy(name, n);
		format = sf ;}

	void onEvent(int i, int c) {
//		Serial.print("cMsgAdapter : onEvent  ");Serial.println(i);
		cMsg* msg = newMsg() ;
		if (msg != NULL) {
			strcpy(msg->name, name) ;
			format->int2str(msg->info, c) ;
			writeMsg(msg) ; } }

	void handleMsg(cMsg * msg) { device->doComand(format->str2int(msg->info)) ; } } ;

typedef void (*cb_function)(int, int);

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
	void onEvent(int i, int c) {if(callBack != NULL)(*callBack)(i, c); } } ;

//----------------------------------------------------------------------

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
		pinMode(pin, OUTPUT);
		btnState = digitalRead(pin) ;
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

class cBtnFactory : public cFactory {
  public:
	cBtnFactory() {strcpy(name,"BTN");}
	cButton* create(int p, bool ao, cb_function  f) {
		cButton* d =new cButton(p, ao) ;
		cCallBackAdapter* cb = new cCallBackAdapter(f, d);
		return d ; }
	cButton* create(int p, bool ao, char*  n) {
		cButton* d =new cButton(p, ao) ;
		cMsgAdapter* cb = new cMsgAdapter(new cStateTranslator(), n, d);
		return d ; }
	cButton* create(int p, bool ao) { return new cButton(p, ao);} } ;

cBtnFactory newButton;
	
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

cRelais* newCbRelais(int p, bool ao, cb_function  f) {
	cRelais* d =new cRelais(p, ao) ;
	cCallBackAdapter* cb = new cCallBackAdapter(f, d);
	return d ; }

class cLed : public cRelais, public cTimer {
  private:
    uint16_t interval ;
    
  public:
	cLed(int p, bool ao) : cRelais(p, ao){interval = 1;};
	
	void setBlink() {
		if ((getValue() == val_off) || (getValue() == val_on) ) {
			setValue(val_blink) ;
			onTimeout() ; } }
	
	void onTimeout() {
		if(getValue() == val_blink) {
			digitalWrite(pin, !digitalRead(pin)) ;
			setTimer(interval); } }

    void doComand(int cmd) {
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

class cLedFactory : public cFactory {
  public:
	cLedFactory() {strcpy(name,"LED");}
	cLed* create(int p, bool ao, cb_function  f) {
		cLed* d =new cLed(p, ao) ;
		cCallBackAdapter* cb = new cCallBackAdapter(f, d);
		return d ; }
	cLed* create(int p, bool ao, char*  n) {
		cLed* d =new cLed(p, ao) ;
		cMsgAdapter* cb = new cMsgAdapter(new cStateTranslator(), n, d);
		return d ; }
	cLed* create(int p, bool ao) { return new cLed(p, ao);} } ;

cLedFactory newLed;
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 
 *  Ein Potentiometer bzw. ein Spannungsmesser
 *  Hier muss stehen welche Pins bei welchen MicroProzessoren verwendet werden kann
 *  und welche Werte gesendet werden. Vieleicht wäre ein Wertebereich von 0 bis 100 gut
 *
 * 
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

class cPoti : public cDevice, cLooper {
  protected:
    int pin ;
    int lastVal = 0 ;
  public :
  
    cPoti(int p) {
      pin = p ; };

    void onLoop() {
      int val = analogRead(pin) ;
      if (abs(lastVal - val) > 8) {
		lastVal = val ;
		setValue(lastVal) ; } } } ;

cPoti* newCbPoti(int p, cb_function  f) {
	cPoti* d =new cPoti(p) ;
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


class cClockFactory : public cFactory {
  public:
	cClockFactory() {strcpy(name,"Clock");}
	cClock* create(cb_function  f) {
		cClock* d =new cClock() ;
		cCallBackAdapter* cb = new cCallBackAdapter(f, d);
		return d ; }
	cClock* create(const char*  n) {
		cClock* d =new cClock() ;
		cMsgAdapter* cb = new cMsgAdapter(new cValueTranslator(2), n, d);
		return d ; }
	cClock* create() { return new cClock();} } ;

cClockFactory newClock;

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


class cHzMFactory : public cFactory {
  public:
	cHzMFactory() {strcpy(name,"HzM");}
	cHzMesser* create(int p, cb_function  f) {
		cHzMesser* d =new cHzMesser(p) ;
		cCallBackAdapter* cb = new cCallBackAdapter(f, d);
		return d ; }
	cHzMesser* create(int p, char*  n) {
		cHzMesser* d =new cHzMesser(p) ;
		cMsgAdapter* cb = new cMsgAdapter(new cValueTranslator(0), n, d);
		return d ; }
	cHzMesser* create(int p) { return new cHzMesser(p);} } ;

cHzMFactory newHzM;
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

class cVccFactory : public cFactory {
  public:
	cVccFactory() {strcpy(name,"Vcc");}
	cVcc* create() { return new cVcc();}
	cVcc* create(cb_function  f) {
		cVcc* d = create() ;
		cCallBackAdapter* cb = new cCallBackAdapter(f, d);
		return d ; }
	cVcc* create(char*  n) {
		cVcc* d = create() ;
		cMsgAdapter* cb = new cMsgAdapter(new cValueTranslator(2), n, d);
		return d ; } } ;

cVccFactory newVcc;

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

class cUltraSonicFactory : public cFactory {
  public:
	cUltraSonicFactory() {strcpy(name,"USS");}
	cUltraSonicSensor* create(int t, int e) { return new cUltraSonicSensor(t, e);}
	cUltraSonicSensor* create(int t, int e, cb_function  f) {
		cUltraSonicSensor* d = create(t, e) ;
		cCallBackAdapter* cb = new cCallBackAdapter(f, d);
		return d ; }
	cUltraSonicSensor* create(int t, int e, char*  n) {
		cUltraSonicSensor* d = create(t, e) ;
		cMsgAdapter* ma = new cMsgAdapter(new cValueTranslator(0), n, d);
		return d ; } } ;

cUltraSonicFactory newUSS;

#endif


