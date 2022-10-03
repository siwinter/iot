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

void printMac(const uint8_t* mac) {
	for(int i=0 ; i<cMacLen ; i++) {
		Serial.print(mac[i], HEX); 
		if(i<(cMacLen-1))Serial.print(":"); }
	Serial.println(); }

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

class cNowChannel : public cChannel {
  private :	
	bool busy;
	int failure;
  public :
	uint8_t mac [cMacLen] ;
	tList<tLink<cMsg>, cMsg>  msgs ;
	
	cNowChannel(const uint8_t* m, bool upstream) : cChannel(upstream) {
		Serial.println("cNowChannel.constructor");
		busy = false;
		failure = 0;
		for (int i=0 ; i<cMacLen ; i++) mac[i] = m[i] ; }
	
	void onSendResult(bool sendOK) {
		Serial.println("cNowChannel.onSendResult");
		busy = false ;
		if ((sendOK) || (failure > 3)) {
			if (failure >3 ) Serial.println("--> skip msg");
			cMsg* m = msgs.getFirst();				// to delete first entry
			failure = 0 ;
			sendNow() ;}
		else {
			failure = failure + 1 ; 
			sendNow();} }
	
	void subscribe(char* topic) {
		Serial.println("cNowChannel.subscribe");
		char m[cTopicLen] = "sbs/" ;
		strcat(m, topic) ;
		sendMsg(m, ""); }
		
	void sendMsg(char* topic, char* info) {
		Serial.println("cNowChannel.sendMsg");
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
	tList<tLink<cNowChannel>, cNowChannel>  nowChannels ;
	bool ready ;
	uint8_t wifiChannel;
	
	bool macEquals(const uint8_t*m1, const uint8_t*m2) {
		for (int i=0 ; i<cMacLen ; i++) if(m1[i] != m2[i]) return false ;
		return true ; }
	void registerCallbacks() ;
	
  public:
	cEspNow() {
		ready = false ;
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
			Serial.print("cEspNow.configure destMAC "); Serial.print(": "); printMac((uint8_t*)value);
			
			cNowChannel* nowChannel = new cNowChannel((uint8_t*)value, true) ;
			nowChannels.insert(nowChannel) ;
#if defined(ESP8266)
			int addStatus = esp_now_add_peer((uint8_t*)value, ESP_NOW_ROLE_COMBO, wifiChannel, NULL, 0);
			if (addStatus == 0) Serial.println("add_peer success");
			else Serial.println("add_peer failure");
#else
			esp_now_peer_info_t peer;
			memset(&peer, 0, sizeof(peer));
			for (int i=0;i<6; i++)peer.peer_addr[i] = mac[i];
			peer.channel = wifiChannel; // pick a channel
			peer.encrypt = false; // no encryption
			esp_now_add_peer(&peer);
#endif
			theChannels.readNext(NULL)->connected() ;
			ready = true ;
			return true ; }
		return false ; }
		
	void onReceived(const uint8_t *mac, const uint8_t *data, uint8_t len) {
		Serial.print("cEspNow.onReceived: "); Serial.println((char*)data);
		uint8_t lMac[cMacLen];
		for(int i=0 ; i<cMacLen ; i++) lMac[i] = mac[i];
		cNowChannel* nowChannel= nowChannels.readNext(NULL) ;
		while (nowChannel != NULL) {
			if (macEquals(lMac, nowChannel->mac)) break ;}
		if ( nowChannel == NULL ) { 
			nowChannel = new cNowChannel(lMac, false) ; // is not upstream channel
			nowChannels.insert(nowChannel) ;
#if defined(ESP8266)
		esp_now_add_peer(lMac, ESP_NOW_ROLE_COMBO, wifiChannel, NULL, 0);
#else
		esp_now_peer_info_t peer;
		memset(&peer, 0, sizeof(peer));
		for (int i=0;i<6; i++)peer.peer_addr[i] = mac[i];
		peer.channel = wifiChannel;
		peer.encrypt = false;
		esp_now_add_peer(&peer); 
#endif
		}
		int j = 0 ;
		char* buf = nowChannel->topic;
		for (int i=0 ; i< len ; i++) {
			char c = (char)data[i] ;
			if (c == ':') {
				buf[j] = 0;
				buf=nowChannel->info ;
				j=0 ;}
			buf[j++] = c ;}
			buf[j] = 0 ;
			nowChannel->received() ; }

	bool sendNow(uint8_t* mac, char* msg) {
		Serial.println("cEspNow.sendNow");
		if ( ready ) {
			uint8_t result = esp_now_send(mac, (uint8_t *) msg, strlen(msg) + 1);
			if (result) { Serial.print("cEspNow.sendNow result: "); Serial.println(result); return false; }
			return true ; }
		return false ; }

	void onSendResult(const uint8_t *mac, bool sendStatus) { 
		Serial.print("cEspNow.onSendResult sendStatus: "); 
		if(sendStatus) Serial.println("ok"); 
		else Serial.println("failure");
		Serial.print("destMAC "); Serial.print(": ");
		printMac(mac);
		cNowChannel* c = nowChannels.readNext(NULL) ;
		while (c != NULL) {
			if (macEquals(mac, c->mac)) {
				c->onSendResult(sendStatus) ;
				break; }
			c = nowChannels.readNext(c) ;} } }; 

cEspNow theEspNow ;

void cNowChannel :: sendNow() {
		if (! busy) {
			cMsg* m = msgs.readNext(NULL) ;
			if ( m != NULL) {
				theEspNow.sendNow(mac, m->buf) ;
				busy = true ; } } }

void cEspNow :: registerCallbacks() {
#if defined(ESP8266)
	esp_now_register_recv_cb([](uint8_t *mac, uint8_t *data, uint8_t l) {theEspNow.onReceived(mac, data,l); }) ;
	esp_now_register_send_cb([](uint8_t* mac, uint8_t sendStatus) {theEspNow.onSendResult(mac, (sendStatus==0)); }) ;
#else
	esp_now_register_recv_cb([](const uint8_t *mac, const uint8_t *data, int l) {theEspNow.onReceived(mac, data,l); }) ;
	esp_now_register_send_cb([](const uint8_t* mac, esp_now_send_status_t sendStatus) {theEspNow.onSendResult(mac, (sendStatus==0)); }) ;
#endif
}

#endif  // if defined(ESP8266) || defined(ESP32) 
#endif 
