#ifndef NTWDEVICE_h
#define NTWDEVICE_h

#include "cDevice.h"
#include "cNetwork.h"

class cTranslator {
  public:
	virtual char* int2str(char* s, int i) = 0;
	virtual int str2int(char* s) = 0; } ;


class cTxtAdapter : public cCmdInterface , public cObserver {
  private:
	cDevice* device ;
	char deviceName[cNameLen] ;
  public:
	cTranslator * format ;
	
	cTxtAdapter(){
		device = NULL;
		format = NULL ;
//		theDevices.append(this);
	}

	cTxtAdapter(cTranslator* sf, cDevice* d, const char* n) {
		device = d;
		device->addObserver(this);
		strcpy(deviceName, n);
		format = sf ; 
		theDevices.insert(this); }
		
	void doComand(char* cmd) { device->doComand(format->str2int(cmd)) ; }
	
	void onEvent(int i, int c) {
//		Serial.println("cTxtAdapter.onEvent");
		char info[cInfoLen] ;
		format->int2str(info, c) ;
		int l = strlen(theEvtTopic) ;
		strcpy((theEvtTopic + l), deviceName);
		theChannels.readFirst()->sendMsg(theEvtTopic, info);
		theEvtTopic[l] = 0 ; }
	
	bool receiveCmd(char* name, char* info) {
//		Log.debug("%s receiveCmd %s %s" ,device, name, info);
		if ( strcmp(deviceName, name) != 0 ) return false ;
		doComand(info) ;
		return true ;} };

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
		if (strcmp(s, "toggle") == 0) return cmd_toggle ;
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
		if (negativ) v = v* (-1) ;
		return v ;} ;
	int decimals ;
  public :
	cValueTranslator(int d) {decimals = d;} } ;

cHeartbeat* newHearbeat(const char*  n) {
	cHeartbeat* d =new cHeartbeat() ;
	new cTxtAdapter(new cValueTranslator(0), d, n);
	return d ; }

cButton* newButton(int p, bool ao,const char*  n) {
	cButton* d =new cButton(p, ao) ;
	new cTxtAdapter(new cStateTranslator(), d, n);
	return d ; }
	
cPoti* newPoti(int p,const char* n) {
	cPoti* d =new cPoti(p) ;
	new cTxtAdapter(new cValueTranslator(0), d, n);
	return d ;}

cLatch* newLatch(int sp, int rp, const char*  n) {
	cLatch* d =new cLatch(sp, rp) ;
	new cTxtAdapter(new cStateTranslator(), d, n);
	return d ; }

cRelais* newRelais(int p, bool ao, const char*  n) {
	cRelais* d =new cRelais(p, ao) ;
	new cTxtAdapter(new cStateTranslator(), d, n);
	return d ; }

cLed* newLed(int p, bool ao, const char*  n) {
	cLed* d =new cLed(p, ao) ;
	new cTxtAdapter(new cStateTranslator(), d, n);
	return d ; }

cLed*newLed(const char* n) {
	return newLed(LED_BUILTIN, !digitalRead(LED_BUILTIN), n) ;}

cClock* newClock(const char* n) {
	cClock* d =new cClock() ;
	new cTxtAdapter(new cValueTranslator(2), d, n);
	return d ; }
	
#if defined(ESP8266)
cHzMesser* newHzM(int p, const char* n) {
	cHzMesser* d =new cHzMesser(p) ;
	new cTxtAdapter(new cValueTranslator(0), d, n);
	return d ; }

cVcc* newVcc( const char* n) {
	cVcc* d = new cVcc() ;
	new cTxtAdapter(new cValueTranslator(2), d, n);
	return d ; }
/*	
#include "cWifi.h" 
cRssi* newRssi(const char* n) {
	cRssi* d = new cRssi() ;
	new cTxtAdapter(new cValueTranslator(0), d, n);
	return d ; }
*/
cHeap* newHeap(const char* n) {
	cHeap* d = new cHeap() ;
	new cTxtAdapter(new cValueTranslator(0), d, n);
	return d ; }
#endif

#if defined(ARDUINO_GENERIC_RP2040)
cPicoTmp* newPicoTmp(const char* n) {
	cPicoTmp* d = new cPicoTmp() ;
	new cTxtAdapter(new cValueTranslator(1), d, n);
	return d ; }
#endif

cUltraSonicSensor* newUSS(int t, int e, const char* n) {
	cUltraSonicSensor* d =new cUltraSonicSensor(t, e);
	new cTxtAdapter(new cValueTranslator(0), d, n);
	return d ; }

#include "cDHT.h"

cDHT22* newDHT22(int p, const char* n) {
	cDHT22* d =new cDHT22(p) ;
	char txt[20] ;
	strcpy(txt, "tmp");
	strcat(txt, n) ;
	new cTxtAdapter(new cValueTranslator(1),d->getTemperatureSensor(), txt) ;
	strcpy(txt, "hum");
	strcat(txt, n) ;
	new cTxtAdapter(new cValueTranslator(0),d->getHumiditySensor(), txt) ;
	return d ;}

#include "cBME280.h"
cBME280* newBME280(const char* n) {
	cBME280* d =new cBME280() ;
	char txt[20] ;
	strcpy(txt, "tmp");
	strcat(txt, n) ;
	new cTxtAdapter(new cValueTranslator(1),d->getTemperatureSensor(), txt) ;
	strcpy(txt, "hum");
	strcat(txt, n) ;
	new cTxtAdapter(new cValueTranslator(1),d->getHumiditySensor(), txt) ;
	strcpy(txt, "prs");
	strcat(txt, n) ;
	new cTxtAdapter(new cValueTranslator(1),d->getPressureSensor(), txt) ;
	return d ; }
#endif
