#ifndef DATABASE_h
#define DATABASE_h

#if defined(EEPROM_AV)

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

#define state_db_idle 0
#define state_db_config 1
#define state_db_start 2
#define state_db_ready 3

class cDatabase : public cTimer, public cConfigurator {
  private:
	int state = 0;
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

	bool keyEqual(int adr, const char* key) {
		int len = strlen(key);
		if (len != EEPROM.read(adr++)) return false ;
		for (int i=0 ; i<len; i++) if (key[i] != EEPROM.read(adr+i)) { return false ; }
		return true; }

	int nextKey(int keyAdr){ return keyAdr + EEPROM.read(keyAdr) + EEPROM.read(keyAdr + EEPROM.read(keyAdr)+1)+2 ; } 

	int findKey(const char* key, int a=3) {
		while (EEPROM.read(a) != 0) { 
			if(keyEqual(a,key)) return a;
			a = nextKey(a); }
		return 0 ; }

  public:
	cDatabase() { theConfigurator = this ;
		setMillis(1) ;}
	
	void onTimeout(){
		int len ;
		switch (state) {
		  case state_db_idle :
#if defined(ESP8266)
			EEPROM.begin(EEPROMsize) ;
#else
			if(!EEPROM.begin(EEPROMsize)) {
				setMillis(5);
				return; }
#endif
			if ( !checkEEPROM()) formatEEPROM() ;
			nextAdr = 3;
			setMillis(1) ;
			state = state_db_config ;
			return ;
			
		  case state_db_config :
//			Serial.print("nextAdr: "); Serial.print(nextAdr); Serial.print(" lastAdr; "); Serial.println(lastAdr); 
			char key[30] ;
			char value[30] ;
			len = EEPROM.read(nextAdr++);
			if ( len != 0 ) {
				int i; for (i=0; i<len; i++) key[i] = EEPROM.read(nextAdr++) ;
				key[i] = 0;
				len = EEPROM.read(nextAdr++);
				for (i=0; i<len; i++) value[i] = EEPROM.read(nextAdr++) ;
				value[i] = 0;
//				Serial.print("---> key: "); Serial.println(key);
//				Serial.print("---> len: "); Serial.println(len);
//				Serial.print("---> val: "); for(int j= 0; j<len; j++) Serial.print(value[j]) ; Serial.println();
 				configure(key, value, len); }
			else {
				nextAdr = 3 ;
				state = state_db_start;}
			setMillis(1) ; 
			return ;
			
		  case state_db_start : 
			start() ;
			state = state_db_ready ;
			return ; } }
	
	void deleteData(const char* key) {
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

	void setData(const char* key, const uint8_t* data, int dataLen=0) {
		setData(key, (const char*) data, dataLen) ; }
			
	void setData(const char* key, const char* data, int dataLen=0) {
		deleteData(key);
		if (dataLen==0) dataLen=strlen(data);
		if((lastAdr + strlen(key) + dataLen + 2) > EEPROMsize) Serial.println("error EEPROM size");
		else {
//			Serial.print("writeConfig at "); Serial.print(lastAdr); Serial.print("  "); Serial.println(strlen(key));
			int adr = lastAdr ;
			EEPROM.write(adr++, strlen(key)) ;
			for(uint i=0; i<strlen(key); i++) EEPROM.write(adr++,key[i]) ;
			EEPROM.write(adr++,dataLen);
			for(int i=0; i<dataLen; i++) EEPROM.write(adr++,data[i]) ;
			EEPROM.write(adr,0) ;
			lastAdr = adr;
			if (!EEPROM.commit()) Serial.println("EEPROM: error commit") ;} }
			
	void setConfig(const char* key, const char* value, int len) {
		if (len == 0) len = strlen(value) ;
		setData(key, value, len) ;
		configure(key, value, len);}

	int getData(const char* key, uint8_t* data, int len) {
		return getData(key, (char*)data, len) ; }
		
	int getData (const char* key, char* data, int len) {
//		Serial.print("getdata key: "); Serial.println(key);
//		printEEPROM() ;
		if (state != state_db_ready) return -1 ;
		int adr; if ((adr=findKey(key)) == 0) return 0;
		adr = adr + EEPROM.read(adr) +1 ; 	// adr dataLen
		int dataLen = EEPROM.read(adr++) ;	// adr+1 = adr data
		if (len < dataLen) return 0 ;
		int i; for (i=0; i<dataLen; i++) data[i] = EEPROM.read(adr +i) ;
		data[i] = 0;
//		Serial.println(data);
		return dataLen ; }
	
	bool getData(int nr, char* key, char* data, int* dataLen) {
		int adr = 3;
		for (int i=0 ; i<nr ; i ++) {
			adr = nextKey(adr) ;
			if (adr == 0) return false ;}
		int len = EEPROM.read(adr++);
		if ( len == 0 ) return false;
		int i; for (i=0; i<len; i++) key[i] = EEPROM.read(adr++) ;
		key[i] = 0;
		len = EEPROM.read(adr++);
		for (i=0; i<len; i++) data[i] = EEPROM.read(adr++) ;
		data[i] = 0;
		*dataLen = len ; 
		return true; } };

cDatabase theDataBase;
#endif    // no EEPROM available
#endif
