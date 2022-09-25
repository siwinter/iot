#ifndef DATABASE_h
#define DATABASE_h

#include "cCore.h"
#include <EEPROM.h>
/*
 * ToDos:
 * Anpassung Config Data : es sollte für jedes Modul nur einen key geben. einzelne Entrys sind durch ; getrennt
 * 							Müssen alle Entrys dann als charArray gespeichert werden??
 *  
 * 
 */

// IoT<length of Key><Key><length of Data><Data> ...0

#define EEPROMsize 512

#include <EEPROM.h>
class cDatabase : public cObserved, public cTimer {
  private:
	int nextAdr = 0 ; 
	int lastAdr ;
	bool checkEEPROM() {
		if ((EEPROM.read(0) != 'I') || (EEPROM.read(1) != 'o') || ( EEPROM.read(2) != 'T')) return false ;
		int adr = 3 ;
		while (EEPROM.read(adr) != 0) {
			adr = nextKey(adr);
			if (adr > EEPROMsize) return false ; }
		lastAdr = adr ;
		return true ; }
		
	void formatEEPROM() {
		EEPROM.write(0,'I'); EEPROM.write(1,'o'); EEPROM.write(2,'T'); EEPROM.write(3,0) ;
		lastAdr = 3 ;
		if (!EEPROM.commit()) Serial.println("EEPROM: error commit") ; }

	bool keyEqual(int adr, char* key) {
		int len = strlen(key);
		if (len != EEPROM.read(adr++)) return false ;
		for (int i=0 ; i<len; i++) if (key[i] != EEPROM.read(adr+i)) { return false ; }
		return true; }

	int nextKey(int keyAdr){ return keyAdr + EEPROM.read(keyAdr) + EEPROM.read(keyAdr + EEPROM.read(keyAdr)+1)+2 ; }

	int findKey(char* key, int a=3) {
		while (EEPROM.read(a) != 0) { 
			if(keyEqual(a,key)) return a;
			a = nextKey(a); }
		return 0 ; }

  public:
	cDatabase() { 
		setMillis(1) ;}
	
	void onTimeout(){
		if ( nextAdr == 0) {
#if defined(ESP8266)
			EEPROM.begin(EEPROMsize) ;
#else
			if(!EEPROM.begin(EEPROMsize)) {
				setMillis(5);
				return; }
#endif
			if ( !checkEEPROM()) formatEEPROM() ;
//			Serial.println("cDatabase fireEvent");
			fireEvent(val_on);
			nextAdr = 3;
			setMillis(1) ;
			return ;}
//		delay (1000);
		Serial.print("nextAdr: "); Serial.print(nextAdr); Serial.print(" lastAdr; "); Serial.println(lastAdr); 
		char key[30] ;
		char value[30] ;
		int len = EEPROM.read(nextAdr++);
		if ( len == 0 ) return ;
		int i; for (i=0; i<len; i++) key[i] = EEPROM.read(nextAdr++) ;
		key[i] = 0;
		len = EEPROM.read(nextAdr++);
		i; for (i=0; i<len; i++) value[i] = EEPROM.read(nextAdr++) ;
		value[i] = 0;
		Serial.print("cDatabase key: "); Serial.println(key);
		cConfig* c = theConfigs.getNext(NULL);
		while ( c != NULL) {
			Serial.println("cDatabase 1"); 
			if ( c->configure(key, value) ) break ;
			c = theConfigs.getNext(c); }
		Serial.println("cDatabase 2");
		setMillis(1) ; }
	
	void deleteData(char* key) {
		int adr = findKey(key, 3) ;
		while (adr != 0) {
			int destAdr = adr ;
			int sourceAdr = nextKey(adr);
//			Serial.print("delConfig : copy from ");Serial.print(sourceAdr);Serial.print(" to ");Serial.println(destAdr);
			while (sourceAdr<lastAdr)EEPROM.write(destAdr++,EEPROM.read(sourceAdr++));
			lastAdr = destAdr;
			EEPROM.write(lastAdr,0);
//			Serial.print("delConfig : deleted Adr ");Serial.println(adr);
			adr=findKey(key,adr); }
		if (!EEPROM.commit()) Serial.println("EEPROM: error commit") ; }
			
	void setData(char* key, char* data, int dataLen=0) {
		deleteData(key);
		if (dataLen==0) dataLen=strlen(data);
		if((lastAdr + strlen(key) + dataLen + 2) > EEPROMsize) Serial.println("error EEPROM size");
		else {
//			Serial.print("writeConfig at "); Serial.print(adr); Serial.print("  "); Serial.println(strlen(key));
			int adr = lastAdr ;
			EEPROM.write(adr++, strlen(key)) ;
			for(int i=0; i<strlen(key); i++) EEPROM.write(adr++,key[i]) ;
			EEPROM.write(adr++,dataLen);
			for(int i=0; i<dataLen; i++) EEPROM.write(adr++,data[i]) ;
			EEPROM.write(adr,0) ;
			lastAdr = adr;
			if (!EEPROM.commit()) Serial.println("EEPROM: error commit") ;} }
			
	int getData (char* key, char* data, int len) {
//		Serial.print("getdata key: "); Serial.println(key);
//		printEEPROM() ;
		int adr; if ((adr=findKey(key)) == 0) return 0;
		adr = adr + EEPROM.read(adr) +1 ; 	// adr dataLen
		int dataLen = EEPROM.read(adr++) ;	// adr+1 = adr data
		if (len < dataLen) return 0 ;
		int i; for (i=0; i<dataLen; i++) data[i] = EEPROM.read(adr +i) ;
		data[i] = 0;
//		Serial.println(data);
		return dataLen ; }
		
	void printEEPROM() {
		int adr = 0 ;
		Serial.print("EEPROM-length: "); Serial.println(lastAdr);
		while (adr < lastAdr) {
			Serial.print(adr) ; Serial.print(" : ");
			for(int i=0; i<10; i++) {Serial.print(EEPROM.read(adr++)); Serial.print(","); } 
			Serial.println();} } };

cDatabase theDataBase;

#endif
