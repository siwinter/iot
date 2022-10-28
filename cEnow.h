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
//#include "cWifi.h"

#define cMacLen       6

class cMsg {
  public:
	char buf[cTopicLen+cInfoLen];
	void write(char* topic, char* info) {
		strcpy(buf, topic);
		strcat(buf, ":");
		strcat(buf, info); } } ;

class cNowChannel;
tList<tLink<cNowChannel>, cNowChannel>  nowChannels ;

tList<tLink<cMsg>, cMsg>  theMsgPool ;

class cNowChannel : public cChannel {
  private :	
	int busy;
	
	void msgDone() {
		tLink<cMsg> * mL = msgs.getFirstLink();
		theMsgPool.insertLink(mL);
		busy = 0 ; }

  public :
	uint8_t mac [cMacLen] ;
	 tList<tLink<cMsg>, cMsg> msgs ;
	
	cNowChannel(const uint8_t* m, uint8_t wifiChannel , bool upstream) : cChannel(upstream) {
//		Serial.println("cNowChannel.constructor");
		busy = 0;
		for (int i=0 ; i<cMacLen ; i++) mac[i] = m[i] ; 
#if defined(ESP8266)
		if ( esp_now_add_peer(mac, ESP_NOW_ROLE_COMBO, wifiChannel, NULL, 0) != 0) Serial.println("Pair failed");
#else
		esp_now_peer_info_t peer;
		memset(&peer, 0, sizeof(peer));
		for (int i=0;i<6; i++)peer.peer_addr[i] = mac[i];
		peer.channel = wifiChannel;
		peer.encrypt = false;
		if ( esp_now_add_peer(&peer) != ESP_OK) Serial.println("Pair failed");
#endif
		nowChannels.insert(this) ; }
	
	void onSendResult(bool sendOK) {
//		Serial.println("cNowChannel.onSendResult");
		if ((sendOK) || (busy > 4)) {
//			Serial.println("--->okay");
			if (busy >4 ) Serial.println("--> skip msg");
			msgDone() ;
			sendNow() ;}
		else sendNow(); }
		
	void sendMsg(char* topic, char* info) {
//		Serial.println("cNowChannel.sendMsg");
		tLink<cMsg> * mL = theMsgPool.getFirstLink() ;
		if (mL == NULL) {
			cMsg* m = new cMsg() ;
			m->write(topic, info) ;
			msgs.append(m); }
		else {
			mL->element->write(topic, info) ;
			msgs.appendLink(mL); }
		if(busy == 0) {
			if ( sendNow() == false) {
				Serial.println("cNowChannel.sendMsg internal error");
			} 
		}
	};

			
	bool sendNow() ;}; 

//####################################### cEspNow ######################################## 
class cEspNow : public cConfig, public cTimer {
  private:
	uint8_t state ;
//	bool ready ;
	uint8_t wifiChannel;
	
	bool macEquals(const uint8_t*m1, const uint8_t*m2) {
		for (int i=0 ; i<cMacLen ; i++) if(m1[i] != m2[i]) return false ;
		return true ; }
	void initNow() ;
	
	cNowChannel* getChannel(uint8_t* mac , bool create) {
		cNowChannel* c = nowChannels.readFirst() ;
		while (c != NULL) {
			if (macEquals(mac, c->mac)) return c ;
			c = nowChannels.readNext() ;}
		if ( create ) {
			c = new cNowChannel(mac, wifiChannel, false) ; // is not upstream channel
//			nowChannels.insert(c) ; 
			return c ; }
		return NULL ; }

  public:
	cEspNow() {
		state = 0 ;
		wifiChannel = 0 ;
		setMillis(1) ; } ;
		
	void onTimeout() { initNow() ; }
	
	bool configure(const char* key, char* value, int vLen) {
//		Serial.print("cEspNow.configure: "); Serial.println(key);
		initNow() ;
		if (strcmp(key, "mac") == 0) {
//			Serial.print("cEspNow.configure destMAC: "); printMac((uint8_t*)value);
			cNowChannel* nowChannel = new cNowChannel((uint8_t*)value, wifiChannel, true) ;
			nowChannel->connected() ;
			state = 2 ;
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
//		Serial.println("theEspNow.sendNow");
		if ( state > 0 ) {
//		Serial.print("----> send ");Serial.print(msg);Serial.print(" to "); printMac(mac); Serial.println();
			int result = esp_now_send(mac, (uint8_t *) msg, strlen(msg) + 1);
			if (result == 0) return true ;
//			else  Serial.println(result,HEX); 
		}
		return false ; }
		
	void onSendResult(const uint8_t *m, bool sendStatus) {
//		Serial.print("theEspNow.onSendResult");
		uint8_t mac[cMacLen];
		for(int i=0 ; i<cMacLen ; i++) mac[i] = m[i];
		cNowChannel* c ; 
		if ( (c = getChannel(mac,false)) != NULL)  c->onSendResult(sendStatus) ; } };

cEspNow theEspNow ;

bool cNowChannel :: sendNow() {
//	Serial.println("cNowChannel.sendNow");
	cMsg* m = msgs.readFirst() ;
	if ( m != NULL) {
//		Serial.print("----> send ");Serial.print(m->buf);Serial.print(" to "); printMac(mac); Serial.println();
		if (theEspNow.sendNow(mac, m->buf)) busy = busy +1 ;
		else return false ; }  // return false --> sendNow failed because of internal error --> skip message
	return true ; }

void cEspNow :: initNow() {
	if (state == 0) {
		WiFi.mode(WIFI_AP_STA);
		WiFi.disconnect();
		if (esp_now_init() != 0) ESP.restart();
#if defined(ESP8266)
		esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
		esp_now_register_recv_cb([](uint8_t *mac, uint8_t *data, uint8_t l) {theEspNow.onReceived(mac, data,l); }) ;
		esp_now_register_send_cb([](uint8_t* mac, uint8_t sendStatus) {theEspNow.onSendResult(mac, (sendStatus==0)); }) ;
#else
		esp_now_register_recv_cb([](const uint8_t *mac, const uint8_t *data, int l) {theEspNow.onReceived(mac, data,l); }) ;
		esp_now_register_send_cb([](const uint8_t* mac, esp_now_send_status_t sendStatus) {theEspNow.onSendResult(mac, (sendStatus==0)); }) ;
#endif
		state = 1 ; } } 

#endif  // if defined(ESP8266) || defined(ESP32) 
#endif 
