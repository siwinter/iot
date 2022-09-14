#ifndef MQTT_h
#define MQTT_h

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

class cMqttChannel : public cChannel, public cLooper, public cTimer, public cObserver{
  private :
  
	char macAdr[13] ;
	uint16_t brokerPort ;
	IPAddress brokerIp ;

	void getMAC() {
		uint8_t  mac[6] ;
		WiFi.macAddress(mac) ;	
		for(int i=0; i<6; i++) {
			uint8_t h = mac[i]/16 ;
			if (h>9) macAdr[i*2] = h - 10 + 'A' ;
			else macAdr[i*2] = h + '0' ;
			h = mac[i] % 16 ;
			if (h>9) macAdr[i*2+1] = h - 10 + 'A' ;
			else macAdr[i*2+1] = h + '0' ; }
		macAdr[12] = 0 ; }

	bool getBrokerID() {
		uint8_t bytes[6];
		if (theDataBase.getData("broker", (char*)bytes, 6)) {
			brokerPort = bytes[4]*256 + bytes[5];
			brokerIp=IPAddress(bytes[0],bytes[1],bytes[2],bytes[3]);
//			Serial.print("mqtt : connect to broker ");Serial.print(brokerIp); Serial.print(":"); Serial.println(brokerPort);
			return true ; }
		return false ; }

  public :
	cMqttChannel() {
		client.setCallback(mqttCallback);
		dbEvent = theDataBase.addObserver(this) ;
		wifiEvent =theWifi.addObserver(this);
		getMAC() ;
		if (getBrokerID()) reconnect(); }
		
	int dbEvent ;
	int wifiEvent ;
	void onEvent(int i, int evt) { 
		if (i == dbEvent) {
			if (evt == val_on) {
				if (getBrokerID()) reconnect() ; }
		}
		else if (i == wifiEvent) {		// wifi connected to router
			if (evt == val_on) reconnect() ; } } 
		
	bool reconnect() {
//		Serial.println("mqtt reconnect");
		if (!client.connected()) {
			client.setServer(brokerIp, brokerPort);
			if (client.connect(macAdr)) {
//				Serial.println("mqtt connected");
				fireEvent(val_connected) ;
				return true ;}
			setTimer(5);
			return false ; }
		return true ; }
		
	void sendMsg(char* topic, char* info) { client. publish(topic, info); }
	
	bool sendComand(char* topic, char* info) {return false ; }

	void subscribe(char* topic) {   // sbs/cmd/nodename/#
		client.subscribe(topic+4) ;}
	
	void onTimeout() { reconnect() ; }

	void onLoop() { client.loop() ; }
};

cMqttChannel theMqtt ;

void mqttCallback(char* topic, byte* payload, unsigned int length) {
	strcpy(theMqtt.topic, topic) ;
	strncpy(theMqtt.info, (char*)payload,length) ;
	theMqtt.info[length] = 0 ;
	theMqtt.received() ; }


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

	bool ip2Byte(char* txtIP, uint8_t* byteIP) {
		bool result = false ;
		int txtLen = strlen(txtIP) ;
		if(txtLen < 16) {
			result = true;
			int txtIndex = 0;
			int byteIndex = 0 ;
			int byte = 0 ;
			while((txtIndex <= txtLen)&&(byteIndex<5)&&result) {
				int d=txtIP[txtIndex++];
				if ((d>='0')&&(d<='9')) byte = byte*10 +(d-'0') ;
				else if ((d=='.')||(d==0)) { 
					if (byte<256) {
						byteIP[byteIndex++] = byte;
						byte = 0; }
					else result = false; }
				else result = false ;} 
			if (byteIndex !=4 ) result = false; }
		if (!result) for(int i=0 ; i<5 ; i++) byteIP[i] = 0 ;
		return result ;}
	
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
			theScheduler.newNodeName(txt);}
		send(htmlMqtt);
		return false; }
} ; 

cMqttWebsite theMqttWebsite ;

#endif
