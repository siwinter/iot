#ifndef IRREC_H
#define IRREC_H

/*
 * Specify which protocol(s) should be used for decoding.
 * If no protocol is defined, all protocols are active.
 */
//#define DECODE_DENON        // Includes Sharp
//#define DECODE_JVC
//#define DECODE_KASEIKYO
//#define DECODE_PANASONIC    // the same as DECODE_KASEIKYO
//#define DECODE_LG
#define DECODE_NEC          // Includes Apple and Onkyo
//#define DECODE_SAMSUNG
//#define DECODE_SONY
//#define DECODE_RC5
//#define DECODE_RC6

//#define DECODE_BOSEWAVE
//#define DECODE_LEGO_PF
//#define DECODE_MAGIQUEST
//#define DECODE_WHYNTER

//#define DECODE_DISTANCE     // universal decoder for pulse width or pulse distance protocols
//#define DECODE_HASH         // special decoder for all protocols

//#define DEBUG // Activate this for lots of lovely debug output from the decoders.
//#define INFO                // To see valuable informations from universal decoder for pulse width or pulse distance protocols

#include <Arduino.h>
#include "cDevice.h"

#include "libs/IRremote/IRremote.hpp"

class cIrProt : public cDevice {
  private:
	decode_type_t prot ;
	uint16_t address ;
  public:
	cIrProt * nextProt ;
	cIrProt(decode_type_t p, uint16_t a) {
		prot = p ;
		address = a ; 
		nextProt = NULL;}
	bool onCode(decode_type_t p, uint16_t a, uint16_t cmd) {

		Serial.println(cmd);
		Serial.println();
		if(!((prot == p) &&(address == a))) return false ;
		setValue(cmd) ;
		Serial.println(cmd);
		return true ; }

} ;
class cIrRec : public cIrProt, public cLooper {
  private :
	int pin ;
  public :
	cIrRec(int p1, decode_type_t p, uint16_t a) : cIrProt(p, a) {
		pin = p1;
		IrReceiver.begin(pin, false); }
		
	void addProtocol(cIrProt* protocol) {
		cIrProt* p = this;
		while (p->nextProt != NULL) p = p->nextProt ;
		p->nextProt = protocol; }
	
	void onLoop() {
		if (IrReceiver.decode()) {
			IrReceiver.resume();
			cIrProt* p = this;
			while (!p->onCode(IrReceiver.decodedIRData.protocol, IrReceiver.decodedIRData.address, IrReceiver.decodedIRData.command)) {
				p = p->nextProt; 
				if (p==NULL) return;}} } } ;

class cIrProtFactory : public cFactory {
  public:
	cIrProtFactory() {strcpy(name,"IPprt");}
	cIrProt* create(decode_type_t p, uint16_t a, cb_function  f) {
		cIrProt* d =new cIrProt(p, a) ;
		cCallBackAdapter* cb = new cCallBackAdapter(f, d);
		return d ; }
	cIrProt* create(decode_type_t p, uint16_t a, char*  n) {
		cIrProt* d =new cIrProt(p, a) ;
		cMsgAdapter* cb = new cMsgAdapter(new cValueTranslator(0), n, d);
		return d ; }
	cIrProt* create(decode_type_t p, uint16_t a) { return new cIrProt(p, a);} } ;

cIrProtFactory newIrProtocol;

class cIrRecFactory : public cFactory {
  public:
	cIrRecFactory() {strcpy(name,"IPprt");}
	cIrRec* create(int p1, decode_type_t p, uint16_t a, cb_function  f) {
		cIrRec* d =new cIrRec(p1, p, a) ;
		cCallBackAdapter* cb = new cCallBackAdapter(f, d);
		return d ; }
	cIrRec* create(int p1, decode_type_t p, uint16_t a, char*  n) {
		cIrRec* d =new cIrRec(p1, p, a) ;
		cMsgAdapter* cb = new cMsgAdapter(new cValueTranslator(0), n, d);
		return d ; }
	cIrRec* create(int p1, decode_type_t p, uint16_t a) { return new cIrRec(p1, p, a);} } ;

cIrRecFactory newIrReceiver;

#endif
