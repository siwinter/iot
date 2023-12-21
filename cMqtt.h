#ifndef MQTT_h
#define MQTT_h
#include "flags.h"
#if defined(WIFI_AV)

#include <Arduino.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include "PubSubClient.h"
#include "cNetwork.h"
#include "cWifi.h"
#include "cDatabase.h"

void mqttCallback(char* topic, byte* payload, unsigned int length) ;

WiFiClient wifiClient;
PubSubClient client(wifiClient);


#define state_idle 0
#define state_ready 2
#define state_doNotStart 5

class cMqttChannel : public cChannel, public cLooper, public cTimer, public cObserver, public cConfig {
  private :
	uint8_t state ;
	char macAdr[13] ;
	uint16_t brokerPort ;
	IPAddress brokerIp ;

	void printState() {
		Serial.print("state: ");
		if (state==state_idle) Serial.println("state_idle");
		if (state==state_ready) Serial.println("state_ready");
		if (state==state_doNotStart) Serial.println("state_doNotStart"); }

	void getMAC() {
		uint8_t  mac[6] ;
		WiFi.macAddress(mac) ;
		mac2txt(mac, macAdr) ; }
		
	void connected(){
		cChannel :: connected() ;
		char info[cInfoLen] ;
		uint32_t ip = WiFi.localIP() ;
		ip2txt((uint8_t*)(&ip),info) ;
		theEvtTopic[0] = 'n' ;					// to subscribe this node ;
		int l = strlen(theEvtTopic) ;
		strcpy((theEvtTopic + l), "IP");
		sendEvent(theEvtTopic, info);
		theEvtTopic[l] = 0 ;
		theEvtTopic[0] = 'e' ; }

  public :
	cMqttChannel() {
		client.setCallback(mqttCallback);
//		wifiEvent =theWifi.addObserver(this);
		getMAC() ; 
		state = state_idle ; }

	bool configure(const char* key, const char* value, int vLen) {
//		Serial.println("cMqttChannel.configure");
//		Serial.print("--> key: "); Serial.print(key); Serial.print(" value: "); Serial.println(value) ;
		if (strcmp(key, "broker") == 0) {
			brokerPort = value[4]*256 + value[5] ;
			brokerIp = IPAddress(value[0], value[1], value[2], value[3]) ;
			return true ; }
		if (strcmp(key, "prot") == 0) {
			if (strcmp(value, "MQTT") == 0) state = state_ready ;
			else state = state_doNotStart ; }
		return false ; }

	int wifiEvent ;
	void onEvent(int i, int evt) {
		Log.debug("cMqtt.onEvent %d", evt) ;
		if ((i == wifiEvent) && (evt == val_on)) start() ; }  // Wifi connected to AccessPoint 

	void start() {
		Log.debug("cMqtt.start");
		if (state == state_doNotStart) return ;
		if (brokerIp[0] == 0) return ;
		if (!client.connected()) {
			Log.debug("brokerIP: %d.%d.%d.%d:%i",brokerIp[0],brokerIp[1],brokerIp[2],brokerIp[3], brokerPort);
			client.setServer(brokerIp, brokerPort);
			Serial.println("-----> 1") ;
			if (!client.connect(macAdr)) {
				Serial.println("-----> 2") ;
				setTimer(5);
				Serial.println("-----> 3") ; }
			else {
				Log.info("cMqtt connected") ;
				cChannel* c = theChannels.readFirst() ;
				while(c != NULL) {
					c->resetNodeList();	
					c=theChannels.readNext(); }
				theChannels.insert(this) ; } } }

	void sendMsg(char* topic, char* info) {
		Log.debug("cMqtt.sendMsg: %s : %s", topic, info);
		client. publish(topic, info); }

	void sendEvent(char* topic, char* info) {
		Log.debug("cMqtt.sendEvent %s", topic);
		if ( topic[0] == 'n' ) {
			char t[cTopicLen] = "cmd/";
			uint i = 4;
			while ((topic[i] != '/') && (i< (cTopicLen - 5))) {t[i] = topic[i]; i++ ;}
			t[i++]='/'; t[i++]='#'; t[i++]=0; 
			subscribe(t);}
		topic[0] = 'e';
		sendMsg(topic, info) ; }
	
	
	bool sendComand(char* topic, char* info) {return false ; }	// mqtt always upstream never forwards cmd-message

	void subscribe(char* topic) { 
//		Log.debug("cMqtt.subscribe %s", topic);
		client.subscribe(topic) ;}

	void onTimeout() {
		Log.debug("cMqtt.onTimeout");
		start() ; }

	void onLoop() { client.loop() ; } };

cMqttChannel theMqtt ;

void mqttCallback(char* topic, byte* payload, unsigned int length) {
	strcpy(theMqtt.topic, topic) ;
	strncpy(theMqtt.info, (char*)payload,length) ;
	theMqtt.info[length] = 0 ;
	Serial.print("cMqtt.received: "); Serial.print(theMqtt.topic); Serial.print(" "); Serial.println(theMqtt.info);
	theMqtt.received() ; }

//##################################### cWebElement ###################################### 
const char* htmlMqtt = 
	"<h3>MQTT-Settings</h3>"
		"<p><form action=\"/setup\">"
			"<p>Broker IP: <input type=\"text\" name=\"broker_ip\"pattern=\"{,9}\"></p>"
			"<p>Broker Port: <input type=\"text\" name=\"broker_port\"pattern=\"{,9}\"></p>"
			"<p><input type=\"submit\"></p></form></p>"
		"<p><form action=\"/setup\">"
			"<p>Node Name: <input type=\"text\" name=\"node\"pattern=\"{,9}\"></p>"
			"<p><input type=\"submit\"></p></form></p>" ;

class cMqttWebsite : public cWebElement {
  public :
	cMqttWebsite() : cWebElement("/setup") {}

	void newBroker(uint8_t* ip, uint16_t port) {
//		Serial.print("MQTT : newBroker : ");
//		for (int i=0; i<4; i++) {Serial.print(ip[i]); Serial.print("."); }
//		Serial.print(" port: "); Serial.println(port);
		uint8_t bytes[6];
		for (int i=0; i<4; i++) bytes[i] = ip[i] ;
		bytes[4] = port / 256;
		bytes[5] = port % 256;
		theDataBase.setData("broker",(char*)bytes, 6); }

	
	bool handleElement() {
		char txt[16];
		bool argsOkay = true ;
		uint8_t byteIP[4];
		if (!getArgument("broker_ip", txt, 16)) argsOkay = false ; 
		else {
			if (!ip2Byte(txt, byteIP)) argsOkay = false ; }
		if (argsOkay) {
			int port=1883;
			if (getArgument("broker_port", txt, 16)) {
				int len = strlen(txt) ;
				if (len != 0) {
					if (len > 5) argsOkay = false ;
					else {
						port = 0;
						for (int i=0; i<len; i++){
							if((txt[i]>'9')||(txt[i]<'0')) argsOkay = false ;
							else port = port * 10 + txt[i] -'0';} } } } 
			if (argsOkay) newBroker(byteIP, port);}
		if (getArgument("node", txt, 16)) {
//			Serial.print("handleElement node: ");Serial.println(txt);
			theConfigurator->setConfig("node", txt) ; }
		send(htmlMqtt);
		return false; }
} ; 

cMqttWebsite theMqttWebsite ;

#endif
#endif
