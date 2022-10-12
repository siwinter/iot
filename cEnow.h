#ifndef CENOW_H
#define CENOW_H

#if defined(ESP8266) || defined(ESP32)

#if defined(ESP8266)
#include <ESP8266WiFi.h>
extern "C" {
#include <espnow.h>
}
#else
#include <WiFi.h>
extern "C" {
#include <esp_now.h>
}
#endif
#include "cNetwork.h"

#define cMacLen       6

void printAmac(uint8_t* mac) {
	for(int i=0 ; i<6 ; i++) {
		Serial.print(mac[i], HEX); if(i<5) Serial.print(":"); } }

class cMsg {
  public:
	char buf[cTopicLen+cInfoLen];} ;

class cMsgPool {
  private :
	tList<tLink<cMsg>, cMsg>  pool ;
  public :
	cMsg* newMsg() {
		cMsg* m = pool.getFirst() ;
		if (m != NULL) return m ;
		m = new cMsg() ;
		return m ; } 

	void returnMsg(cMsg* m) { pool.insert(m) ; } } ;

cMsgPool theMsgPool ;

class cNowChannel;
tList<tLink<cNowChannel>, cNowChannel>  nowChannels ;

class cNowChannel : public cChannel {
  private :	
	bool busy;
	int failure;
  public :
	uint8_t mac [cMacLen] ;
	tList<tLink<cMsg>, cMsg>  msgs ;
	
	cNowChannel(const uint8_t* m, uint8_t wifiChannel , bool upstream) : cChannel(upstream) {
//		Serial.println("cNowChannel.constructor");
		busy = false;
		failure = 0;
		for (int i=0 ; i<cMacLen ; i++) mac[i] = m[i] ; 
#if defined(ESP8266)
		esp_now_add_peer(mac, ESP_NOW_ROLE_COMBO, wifiChannel, NULL, 0);
#else
		esp_now_peer_info_t peer;
		memset(&peer, 0, sizeof(peer));
		for (int i=0;i<6; i++)peer.peer_addr[i] = mac[i];
		peer.channel = wifiChannel;
		peer.encrypt = false;
		esp_now_add_peer(&peer); 
#endif
		nowChannels.insert(this) ; }
	
	void onSendResult(bool sendOK) {
//		Serial.println("cNowChannel.onSendResult");
		busy = false ;
		if ((sendOK) || (failure > 3)) {
			if (failure >3 ) Serial.println("--> skip msg");
			cMsg* m = msgs.getFirst();				// to delete first entry
			failure = 0 ;
			sendNow() ;}
		else {
			failure = failure + 1 ; 
			sendNow();} }
		
	void sendMsg(char* topic, char* info) {
//		Serial.println("cNowChannel.sendMsg");
		cMsg* m = theMsgPool.newMsg() ;
		strcpy(m->buf,topic);
		strcat(m->buf, ":");
		strcat(m->buf,info);
		msgs.append(m) ;
		if(!busy) {
			failure = 0 ;
			sendNow() ; } }
			
	void sendNow() ;}; 

class cEspNow : public cConfig {
  private:
	bool ready ;
	uint8_t wifiChannel;
	
	bool macEquals(const uint8_t*m1, const uint8_t*m2) {
		for (int i=0 ; i<cMacLen ; i++) if(m1[i] != m2[i]) return false ;
		return true ; }
	void registerCallbacks() ;
	
	cNowChannel* getChannel(uint8_t* mac , bool create) {
		cNowChannel* c = nowChannels.readNext(NULL) ;
		while (c != NULL) {
			if (macEquals(mac, c->mac)) return c ;
			c = nowChannels.readNext(c) ;}
		if ( create ) {
			c = new cNowChannel(mac, wifiChannel, false) ; // is not upstream channel
//			nowChannels.insert(c) ; 
			return c ; }
		return NULL ; }

  public:
	cEspNow() {
		ready = true ;
		WiFi.mode(WIFI_AP_STA);
		WiFi.disconnect();
		wifiChannel = 0 ;
		if (esp_now_init() != 0) ESP.restart();
#if defined(ESP8266)
		esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
#endif
		registerCallbacks() ; }
	
	bool configure(char* key, char* value, int vLen) {
		if (strcmp(key, "mac") == 0) {
//			Serial.print("cEspNow.configure destMAC "); Serial.print(": "); printAmac((uint8_t*)value);
			cNowChannel* nowChannel = new cNowChannel((uint8_t*)value, wifiChannel, true) ;
			nowChannel->connected() ;
			ready = true ;
//			Serial.println("----> ready=true");
			return true ; }
		return false ; }
		
	void onReceived(const uint8_t *m, const uint8_t *data, uint8_t len) {
//		Serial.print("cEspNow.onReceived: "); Serial.println((char*)data);
		uint8_t mac[cMacLen];
		for(int i=0 ; i<cMacLen ; i++) mac[i] = m[i];
		cNowChannel* nowChannel = getChannel(mac,true) ;

		int j = 0 ;
		char* buf = nowChannel->topic;
		for (int i=0 ; i< len ; i++) {
			char c = (char)data[i] ;
			if (c == ':') {
				buf[j] = 0;
				buf=nowChannel->info ;
				j=0 ;}
			else buf[j++] = c ;}
			buf[j] = 0 ;
//			Serial.print("nowChannel received: "); Serial.print(nowChannel->topic);Serial.print(" : "); Serial.println(nowChannel->info);  
			nowChannel->received() ; }

	bool sendNow(uint8_t* mac, char* msg) {
//		Serial.println("cEspNow.sendNow");
		if ( ready ) {
//		Serial.print("----> send ");Serial.print(msg);Serial.print(" to "); printAmac(mac); Serial.println();
			uint8_t result = esp_now_send(mac, (uint8_t *) msg, strlen(msg) + 1);
			if (result) { /*Serial.print("cEspNow.sendNow result: "); Serial.println(result);*/ return false; }
			return true ; }
		return false ; }

	void onSendResult(const uint8_t *m, bool sendStatus) {
		uint8_t mac[cMacLen];
		for(int i=0 ; i<cMacLen ; i++) mac[i] = m[i];
		cNowChannel* c ; 
		if ( (c = getChannel(mac,false)) != NULL)  c->onSendResult(sendStatus) ; } };

cEspNow theEspNow ;

void cNowChannel :: sendNow() {
	Serial.println("cNowChannel.sendNow");
	if (! busy) {
		cMsg* m = msgs.readNext(NULL) ;
		if ( m != NULL) {
			Serial.print("----> send ");Serial.print(m->buf);Serial.print(" to "); printMac(mac); Serial.println();
			if (theEspNow.sendNow(mac, m->buf)) busy = true ; } } }

void cEspNow :: registerCallbacks() {
#if defined(ESP8266)
	esp_now_register_recv_cb([](uint8_t *mac, uint8_t *data, uint8_t l) {theEspNow.onReceived(mac, data,l); }) ;
	esp_now_register_send_cb([](uint8_t* mac, uint8_t sendStatus) {theEspNow.onSendResult(mac, (sendStatus==0)); }) ;}
#else
	esp_now_register_recv_cb([](const uint8_t *mac, const uint8_t *data, int l) {theEspNow.onReceived(mac, data,l); }) ;
	esp_now_register_send_cb([](const uint8_t* mac, esp_now_send_status_t sendStatus) {theEspNow.onSendResult(mac, (sendStatus==0)); }) ;}
#endif
#endif  // if defined(ESP8266) || defined(ESP32) 
#endif 
