/*
 PubSubClient.h - A simple client for MQTT.
  Nick O'Leary
  http://knolleary.net
/

#ifndef MQTT_h
#define MQTT_h

#include <Arduino.h>
#include "IPAddress.h"
#include "Client.h"
#include "Stream.h"

#include "cWifi.h"
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
	
const char* htmlMqtt = 
	"<h3>MQTT-Settings</h3>"
		"<p><form action=\"/setup\">"
			"<p>Broker IP: <input type=\"text\" name=\"broker_ip\"pattern=\"{,9}\"></p>"
			"<p>Broker Port: <input type=\"text\" name=\"broker_port\"pattern=\"{,9}\"></p>"
			"<p><input type=\"submit\"></p></form></p>"
		"<p><form action=\"/setup\">"
			"<p>Main MQTT Topic: <input type=\"text\" name=\"topic\"pattern=\"{,9}\" >"
			"<input type=\"submit\"></p></form></p>" ;

#define MQTT_VERSION_3_1      3
#define MQTT_VERSION_3_1_1    4

// MQTT_VERSION : Pick the version
//#define MQTT_VERSION MQTT_VERSION_3_1
#ifndef MQTT_VERSION
#define MQTT_VERSION MQTT_VERSION_3_1_1
#endif

// MQTT_MAX_PACKET_SIZE : Maximum packet size. Override with setBufferSize().
#ifndef MQTT_MAX_PACKET_SIZE
#define MQTT_MAX_PACKET_SIZE 256
#endif

// MQTT_KEEPALIVE : keepAlive interval in Seconds. Override with setKeepAlive()
#ifndef MQTT_KEEPALIVE
#define MQTT_KEEPALIVE 15
#endif

// MQTT_SOCKET_TIMEOUT: socket timeout interval in Seconds. Override with setSocketTimeout()
#ifndef MQTT_SOCKET_TIMEOUT
#define MQTT_SOCKET_TIMEOUT 15
#endif

// MQTT_MAX_TRANSFER_SIZE : limit how much data is passed to the network client
//  in each write call. Needed for the Arduino Wifi Shield. Leave undefined to
//  pass the entire MQTT packet in each write call.
//#define MQTT_MAX_TRANSFER_SIZE 80

// Possible values for client.state()
#define MQTT_CONNECTION_TIMEOUT     -4
#define MQTT_CONNECTION_LOST        -3
#define MQTT_CONNECT_FAILED         -2
#define MQTT_DISCONNECTED           -1
#define MQTT_CONNECTED               0
#define MQTT_CONNECT_BAD_PROTOCOL    1
#define MQTT_CONNECT_BAD_CLIENT_ID   2
#define MQTT_CONNECT_UNAVAILABLE     3
#define MQTT_CONNECT_BAD_CREDENTIALS 4
#define MQTT_CONNECT_UNAUTHORIZED    5

#define MQTTCONNECT     1 << 4  // Client request to connect to Server
#define MQTTCONNACK     2 << 4  // Connect Acknowledgment
#define MQTTPUBLISH     3 << 4  // Publish message
#define MQTTPUBACK      4 << 4  // Publish Acknowledgment
#define MQTTPUBREC      5 << 4  // Publish Received (assured delivery part 1)
#define MQTTPUBREL      6 << 4  // Publish Release (assured delivery part 2)
#define MQTTPUBCOMP     7 << 4  // Publish Complete (assured delivery part 3)
#define MQTTSUBSCRIBE   8 << 4  // Client Subscribe request
#define MQTTSUBACK      9 << 4  // Subscribe Acknowledgment
#define MQTTUNSUBSCRIBE 10 << 4 // Client Unsubscribe request
#define MQTTUNSUBACK    11 << 4 // Unsubscribe Acknowledgment
#define MQTTPINGREQ     12 << 4 // PING Request
#define MQTTPINGRESP    13 << 4 // PING Response
#define MQTTDISCONNECT  14 << 4 // Client is Disconnecting
#define MQTTReserved    15 << 4 // Reserved

#define MQTTQOS0        (0 << 1)
#define MQTTQOS1        (1 << 1)
#define MQTTQOS2        (2 << 1)

// Maximum size of fixed header and variable length size header
#define MQTT_MAX_HEADER_SIZE 5

#define CHECK_STRING_LENGTH(l,s) if (l+2+strnlen(s, this->bufferSize) > this->bufferSize) { client->stop();return false;}
class cMqttChannel : public Print, public cObserved, public cObserver, public cChannel, public cTimer, public cLooper, public cWebElement {
  private:
	Client*  client;
	uint8_t* buffer;
	uint16_t bufferSize;
	uint16_t keepAlive;
	uint16_t socketTimeout;
	uint16_t nextMsgId;
	unsigned long lastOutActivity;
	unsigned long lastInActivity;
	bool pingOutstanding;
	
//	WiFiClient wifiClient;

	uint32_t readPacket(uint8_t* lengthLength) {
		uint16_t len = 0;
		if(!readByte(buffer, &len)) return 0;
		bool isPublish = (buffer[0]&0xF0) == MQTTPUBLISH;
		uint32_t multiplier = 1;
		uint32_t length = 0;
		uint8_t digit = 0;
		uint16_t skip = 0;
		uint32_t start = 0;
		do {
			if (len == 5) {
				// Invalid remaining length encoding - kill the connection
				setState (MQTT_DISCONNECTED);
				client->stop();
				return 0; }
			if(!readByte(&digit)) return 0;
			buffer[len++] = digit;
			length += (digit & 127) * multiplier;
			multiplier <<=7; //multiplier *= 128
		} while ((digit & 128) != 0);
		*lengthLength = len-1;

		if (isPublish) {
			// Read in topic length to calculate bytes to skip over for Stream writing
			if(!readByte(buffer, &len)) return 0;
			if(!readByte(buffer, &len)) return 0;
			skip = (buffer[*lengthLength+1]<<8)+buffer[*lengthLength+2];
			start = 2;
			if (buffer[0]&MQTTQOS1) {
				// skip message id
				skip += 2; } }
				
		uint32_t idx = len;

		for (uint32_t i = start;i<length;i++) {
			if(!readByte(&digit)) return 0;
			if (stream) {
				if (isPublish && idx-*lengthLength-2>skip) {
					stream->write(digit); } }

			if (len < bufferSize) {
				buffer[len] = digit;
				len++; }
			idx++; }

			if (!stream && idx > bufferSize) len = 0; // This will cause the packet to be ignored.

		return len; }

	// reads a byte into result
	bool readByte(uint8_t * result) {
		uint32_t previousMillis = millis();
		while(!client->available()) {
			yield();
			uint32_t currentMillis = millis();
			if(currentMillis - previousMillis >= ((int32_t) this->socketTimeout * 1000)) return false; }
		*result = client->read();
		return true; }

	// reads a byte into result[*index] and increments index
	bool readByte(uint8_t * result, uint16_t * index){
		uint16_t current_index = *index;
		uint8_t * write_address = &(result[current_index]);
		if(readByte(write_address)){
			*index = current_index + 1;
			return true; }
		return false; }

	bool write(uint8_t header, uint8_t* buf, uint16_t length) {
		uint16_t rc;
		uint8_t hlen = buildHeader(header, buf, length);

#ifdef MQTT_MAX_TRANSFER_SIZE
		uint8_t* writeBuf = buf+(MQTT_MAX_HEADER_SIZE-hlen);
		uint16_t bytesRemaining = length+hlen;  //Match the length type
		uint8_t bytesToWrite;
		boolean result = true;
		while((bytesRemaining > 0) && result) {
			bytesToWrite = (bytesRemaining > MQTT_MAX_TRANSFER_SIZE)?MQTT_MAX_TRANSFER_SIZE:bytesRemaining;
			rc = client->write(writeBuf,bytesToWrite);
			result = (rc == bytesToWrite);
			bytesRemaining -= rc;
			writeBuf += rc;
		}
		return result;
#else
		rc = client->write(buf+(MQTT_MAX_HEADER_SIZE-hlen),length+hlen);
		lastOutActivity = millis();
		return (rc == hlen+length); 
#endif
	}

	uint16_t writeString(const char* string, uint8_t* buf, uint16_t pos) {
		const char* idp = string;
		uint16_t i = 0;
		pos += 2;
		while (*idp) {
			buf[pos++] = *idp++;
			i++;
		}
		buf[pos-i-2] = (i >> 8);
		buf[pos-i-1] = (i & 0xFF);
		return pos; }

	// Build up the header ready to send
	// Returns the size of the header
	// Note: the header is built at the end of the first MQTT_MAX_HEADER_SIZE bytes, so will start
	//       (MQTT_MAX_HEADER_SIZE - <returned size>) bytes into the buffer
	size_t buildHeader(uint8_t header, uint8_t* buf, uint16_t length) {
		uint8_t lenBuf[4];
		uint8_t llen = 0;
		uint8_t digit;
		uint8_t pos = 0;
		uint16_t len = length;
		do {
			digit = len  & 127; //digit = len %128
			len >>= 7; //len = len / 128
			if (len > 0) { digit |= 0x80; }
			lenBuf[pos++] = digit;
			llen++;
		} while(len>0);

		buf[4-llen] = header;
		for (int i=0;i<llen;i++) { buf[MQTT_MAX_HEADER_SIZE-llen+i] = lenBuf[i]; }
		return llen+1; } // Full header size is variable length bit plus the 1-byte fixed header
		
	size_t write(uint8_t data) {
		lastOutActivity = millis();
		return client->write(data); }

	IPAddress brokerIP;
	uint16_t brokerPort;
	Stream* stream;
	int state;
	void setState(int s) {
		if ((s!= MQTT_CONNECTED) && (state == MQTT_CONNECTED)) fireEvent(val_off) ;
		if ((state!= MQTT_CONNECTED) && (s == MQTT_CONNECTED)) fireEvent(val_on) ;
		state = s ; }
		
  public:

	void  setServer(IPAddress ip, uint16_t port) {
		brokerIP = ip;
		brokerPort = port;}

	void setServer(uint8_t * ip, uint16_t port) {
		IPAddress addr(ip[0],ip[1],ip[2],ip[3]);
		setServer(addr,port); }

	bool setBufferSize(uint16_t size) {
		if (size == 0) {
			// Cannot set it back to 0
			return false; }
		if (bufferSize == 0) { buffer = (uint8_t*)malloc(size); } 
		else {
			uint8_t* newBuffer = (uint8_t*)realloc(buffer, size);
			if (newBuffer != NULL) buffer = newBuffer; 
			else { return false; }}
		bufferSize = size;
		return (buffer != NULL); }

	bool connect(const char* id) {
		return connect(id,NULL,NULL,0,0,0,0,1); }

	bool connect(const char *id, const char *user, const char *pass, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage, boolean cleanSession) {
		if (!connected()) {
			int result = 0;
			if(client->connected()) { result = 1; }
			else { result = client->connect(brokerIP, brokerPort); }
			if (result == 1) {
				nextMsgId = 1;
				// Leave room in the buffer for header and variable length field
				uint16_t length = MQTT_MAX_HEADER_SIZE;
				unsigned int j;

#if MQTT_VERSION == MQTT_VERSION_3_1
				uint8_t d[9] = {0x00,0x06,'M','Q','I','s','d','p', MQTT_VERSION};
#define MQTT_HEADER_VERSION_LENGTH 9
#elif MQTT_VERSION == MQTT_VERSION_3_1_1
				uint8_t d[7] = {0x00,0x04,'M','Q','T','T',MQTT_VERSION};
#define MQTT_HEADER_VERSION_LENGTH 7
#endif
				for (j = 0;j<MQTT_HEADER_VERSION_LENGTH;j++) { buffer[length++] = d[j]; }

				uint8_t v;
				if (willTopic) { v = 0x04|(willQos<<3)|(willRetain<<5); }
				else { v = 0x00; }
				if (cleanSession) { v = v|0x02; }
				if(user != NULL) {
					v = v|0x80;
					if(pass != NULL) { v = v|(0x80>>1); } }
				buffer[length++] = v;
				buffer[length++] = ((keepAlive) >> 8);
				buffer[length++] = ((keepAlive) & 0xFF);

				CHECK_STRING_LENGTH(length,id)
				length = writeString(id,buffer,length);
				if (willTopic) {
					CHECK_STRING_LENGTH(length,willTopic)
					length = writeString(willTopic,buffer,length);
					CHECK_STRING_LENGTH(length,willMessage)
					length = writeString(willMessage,buffer,length); }

				if(user != NULL) {
					CHECK_STRING_LENGTH(length,user)
					length = writeString(user,buffer,length);
					if(pass != NULL) {
						CHECK_STRING_LENGTH(length,pass)
						length = writeString(pass,buffer,length); } }

				write(MQTTCONNECT,buffer,length-MQTT_MAX_HEADER_SIZE);

				lastInActivity = lastOutActivity = millis();

				while (!client->available()) {
					unsigned long t = millis();
					if (t-lastInActivity >= ((int32_t) socketTimeout*1000UL)) {
						setState(MQTT_CONNECTION_TIMEOUT);
						client->stop();
						return false; } }
				uint8_t llen;
				uint32_t len = readPacket(&llen);

				if (len == 4) {
					if (buffer[3] == 0) {
						lastInActivity = millis();
						pingOutstanding = false;
						setState(MQTT_CONNECTED);
						return true; }
					else {
						setState(buffer[3]); } }
				client->stop(); }
			else { setState(MQTT_CONNECT_FAILED); }
			return false;}
		return true;}

	bool publish(const char* topic, const char* payload, boolean retained){
		unsigned int plength = strnlen(payload,bufferSize) ;
		if (connected()) {
			if (bufferSize < MQTT_MAX_HEADER_SIZE + 2+strnlen(topic, bufferSize) + plength) return false;  // Too long
			// Leave room in the buffer for header and variable length field
			uint16_t length = MQTT_MAX_HEADER_SIZE;
			length = writeString(topic,buffer,length);

			// Add payload
			uint16_t i;
			for (i=0;i<plength;i++) { buffer[length++] = payload[i]; }

			// Write the header
			uint8_t header = MQTTPUBLISH;
			if (retained) header |= 1;
			return write(header, buffer,length-MQTT_MAX_HEADER_SIZE); }
		return false; }

	bool subscribe(const char* topic) {
		return subscribe(topic, 0);	}

	bool subscribe(const char* topic, uint8_t qos) {
		size_t topicLength = strnlen(topic, bufferSize);
		if (topic == 0) return false;
		if (qos > 1) return false;
		if (bufferSize < 9 + topicLength) return false;  // Too long
    
		if (connected()) {
			// Leave room in the buffer for header and variable length field
			uint16_t length = MQTT_MAX_HEADER_SIZE;
			nextMsgId++;
			if (nextMsgId == 0) nextMsgId = 1;
			
			buffer[length++] = (nextMsgId >> 8);
			buffer[length++] = (nextMsgId & 0xFF);
			length = writeString((char*)topic, buffer,length);
			buffer[length++] = qos;
			return write(MQTTSUBSCRIBE|MQTTQOS1,buffer,length-MQTT_MAX_HEADER_SIZE); }
		return false; }

	void onLoop() {
		if (connected()) {
			unsigned long t = millis();
			if ((t - lastInActivity > keepAlive*1000UL) || (t - lastOutActivity > keepAlive*1000UL)) {
				if (pingOutstanding) {
					setState(MQTT_CONNECTION_TIMEOUT);
					client->stop(); }
				else {
					buffer[0] = MQTTPINGREQ;
					buffer[1] = 0;
					client->write(buffer,2);
					lastOutActivity = t;
					lastInActivity = t;
					pingOutstanding = true; } }
					
			if ( client->available()) {
				uint8_t llen;
				uint16_t len = readPacket(&llen);
				uint16_t msgId = 0;
				uint8_t *payload;
				if (len > 0) {
					lastInActivity = t;
					uint8_t type = buffer[0]&0xF0;
					if (type == MQTTPUBLISH) {
						uint16_t tl = (buffer[llen+1]<<8)+buffer[llen+2]; /* topic length in bytes */
						memmove(buffer+llen+2,buffer+llen+3,tl); /* move topic inside buffer 1 byte to front */
						buffer[llen+2+tl] = 0; /* end the topic as a 'C' string with \x00 */
						char *topic = (char*) buffer+llen+2;
						// msgId only present for QOS>0
						if ((buffer[0]&0x06) == MQTTQOS1) {
							msgId = (buffer[llen+3+tl]<<8)+buffer[llen+3+tl+1];
							payload = buffer+llen+3+tl+2;
							received(topic, (char*) payload,len-llen-3-tl-2);
							buffer[0] = MQTTPUBACK;
							buffer[1] = 2;
							buffer[2] = (msgId >> 8);
							buffer[3] = (msgId & 0xFF);
							client->write(buffer,4);
							lastOutActivity = t;}
						else {
							payload = buffer+llen+3+tl;
							received(topic, (char*) payload,len-llen-3-tl);	} }
					else if (type == MQTTPINGREQ) {
						buffer[0] = MQTTPINGRESP;
						buffer[1] = 0;
						client->write(buffer,2); } 
					else if (type == MQTTPINGRESP) {
						pingOutstanding = false; } }
				else if (!connected()) { } } } }

	bool connected() {
		bool rc;
		if ( client == NULL ) { rc = false; }
		else {
			rc = (int)client->connected();
			if (!rc) {
				if (state == MQTT_CONNECTED) {
					setState(MQTT_CONNECTION_LOST);
					client->flush();
					client->stop(); } }
			else { return state == MQTT_CONNECTED; } }
		return rc; }

//##################################### new methods #####################################
  private:
#define  cMaxTopicLen 20		// siwi: 
	char topic[cMaxTopicLen] ;			// siwi: sollte der systemname werden!
    int topicLen ;
	char macAdr[13] ;
	
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

  public:
	cMqttChannel() : cWebElement("/setup") {
		keepAlive = MQTT_KEEPALIVE;
		socketTimeout = MQTT_SOCKET_TIMEOUT;
		stream = NULL;
		client = NULL ;
		bufferSize = 0;
		setBufferSize(MQTT_MAX_PACKET_SIZE);
		getMAC() ;
		theWifi.addObserver(this); }
		
	void start() {
		if (!theDataBase.getData("topic", topic, 8)) strcpy(topic, "test");
		initTopic(topic) ;
//		topicLen = strlen(topic) ;
		setState(MQTT_DISCONNECTED);
		if (client != NULL) client->stop();
		client = new WiFiClient();
		uint8_t bytes[6];
		if (theDataBase.getData("broker", (char*)bytes, 6)) {
			brokerPort = bytes[4]*256 + bytes[5];
			brokerIP=IPAddress(bytes[0],bytes[1],bytes[2],bytes[3]);
			Serial.print("connect to mqtt broker ");Serial.print(brokerIP); Serial.print(":"); Serial.println(brokerPort);
			reconnect(); }
		else Serial.println("mqtt : start : no broker"); }

	void reconnect() {
		if ( !connect(macAdr) ) { setTimer(5);} 
		else {
			Serial.println("mqtt : connected");
			char subsTopic[cNamLen] ;
			strcpy(subsTopic,"cmd/") ;
			strcat(subsTopic,systemName);
			strcat(subsTopic,"/#");
			if (!subscribe(subsTopic)) Serial.println("mqtt : subscribe error");}			
			cMsg* m= new cMsg();
			strcpy(m->name,"IP");
			strcpy(m->info,theWifi.getLocalIP());
			sendMsg(m);
 }

	void onTimeout() {reconnect();}
	void onEvent(int i, int e) { start(); }

//###################################### cChannel ######################################
	void received(char* t, char* c, unsigned int l) {
		cMsg * msg = receivedQueue->newMsg() ;
		strcpy(msg->name, t) ;
		strcpy(msg->info, c) ;
		receivedQueue->insert(msg) ; }

//	bool sendMsg(cMsg* msg) { 
//		Serial.println("cMqttChannel :: onMsg ");
//		if ( connected() ) {
//		topic[topicLen] = 0 ;
//		strcat(topic, msg->name);
//			Serial.println("mqtt : sendMsg");
//			publish(topic, msg->info, false) ;
//		return true ; } }
		
	bool sendMsg(cMsg* msg) { 
		if ( connected() ) {
			publish(setTopic(msg->name), msg->info, false) ;
			return true ; }
		return true; }
		
//#################################### cWebelement #####################################
	void newBroker(uint8_t* ip, uint16_t port) {
//		Serial.print("MQTT : newBroker : ");
//		for (int i=0; i<4; i++) {Serial.print(ip[i]); Serial.print("."); }
//		Serial.print(" port: "); Serial.println(port);
		uint8_t bytes[6];
		for (int i=0; i<4; i++) bytes[i] = ip[i] ;
		bytes[4] = port / 256;
		bytes[5] = port % 256;
		theDataBase.setData("broker",(char*)bytes, 6); 
		start();}
		
	void newTopic(char* t) {
//		Serial.print("MQTT : newTopic : "); Serial.println(t); 
		theDataBase.setData("topic", t, strlen(t)); 
		start();}

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
			if (getArgument("topic", txt, 8)) newTopic(txt);
			send(htmlMqtt);
		return false; }
 };
 
cMqttChannel* theMqtt = NULL ;

cMqttChannel* getTheMqtt() { 
	if (theMqtt == NULL) theMqtt = new cMqttChannel();
//	theMqtt->start() ;
	return theMqtt ; }

#endif


/*
//###################################### Webserver ######################################
#if defined(ESP8266)
#include <ESP8266WebServer.h>
ESP8266WebServer theWebServer(80);
#else
#include <WebServer.h>
#include <Update.h>
WebServer theWebServer(80);
#endif
#include "cCore.h"
//###################################### OTA-Update ######################################

const char* htmlUpdate = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";

void handleUpdate(){
	theWebServer.sendHeader("Connection", "close");
	theWebServer.send(200, "text/html", htmlUpdate); }

void handleUpdate1() {
	theWebServer.sendHeader("Connection", "close");
	theWebServer.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
	ESP.restart(); }

void handleUpdate2() {
	HTTPUpload& upload = theWebServer.upload();
	if (upload.status == UPLOAD_FILE_START) {
		Serial.setDebugOutput(true);
		Serial.printf("Update: %s\n", upload.filename.c_str());
		
#if defined(ESP8266)
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
#else
        uint32_t maxSketchSpace = (1048576 - 0x1000) & 0xFFFFF000;
#endif
        if (!Update.begin(maxSketchSpace)) { //start with max available size
          Update.printError(Serial);} }
	else if (upload.status == UPLOAD_FILE_WRITE) {
		if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
			Update.printError(Serial); } }
	else if (upload.status == UPLOAD_FILE_END) {
		if (Update.end(true)) { //true to set the size to the current progress
			Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);} 
		else { Update.printError(Serial);}
		Serial.setDebugOutput(false); }
		else {
			Serial.printf("Update Failed Unexpectedly (likely broken connection): status=%d\n", upload.status); } }

//##################################### cWebElement #####################################

class cWebElement ;
cWebElement* aWebElement = NULL;


class cWebElement {
  protected: 
	char uri[20] ;
	void send(const char* txt) {theWebServer.sendContent(txt);}
	bool getArgument(char* arg, char* param, int pLen) {
		if (!theWebServer.hasArg(arg)) return false;
		theWebServer.arg(arg).toCharArray(param, pLen) ;
		return true;}
  public:
	cWebElement* nextElement;
	cWebElement(char* u){
		strncpy(uri,u,20);
		nextElement = aWebElement;
		aWebElement = this;}
	virtual bool handleElement() {return true; } 
	virtual bool handleElement(char*u){
		if (strlen(u) != strlen(uri)) return false;
		for(int i=0; i<strlen(uri); i++) if (u[i] != uri[i]) return false ;
		return handleElement(); } } ;
//######################################## cWifi ########################################

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include "cDevice.h"
#include "cDatabase.h"

const char* htmlWifi = 
	"<h3>WLAN</h3>"
		"<p><form action=\"/setup\">"
			"<p>SSID: <input type=\"text\" name=\"ssid\"pattern=\"{,9}\"></p>"
			"<p>PWD: <input type=\"text\" name=\"pwd\"pattern=\"{,9}\" ></p>"
			"<p><input type=\"submit\"></p></form></p>";

class cWifi : public cTimer, public cObserved, public cObserver , public cWebElement {
  private :
	char stationPwd[20] ;
	char stationSsid[20] ;
	char apSsid[20] ;
  public :
	cWifi() : cWebElement("/setup") { theDataBase.addObserver(this);}
	
	void onEvent(int i, int evt) { start();}			// EEPROM available
	void start(){
//		Serial.println("cWifi : start");
		if (theDataBase.getData("ssid", stationSsid, 20)) {
			theDataBase.getData("pwd", stationPwd, 20);
			if(theDataBase.getData("apSSid", apSsid, 20)) {
				WiFi.mode(WIFI_AP_STA);
				WiFi.softAP(apSsid); }
			else  WiFi.mode(WIFI_STA);
		WiFi.disconnect();
		reconnect();
		setTimer(10);}
		else {
			WiFi.mode(WIFI_AP);
			WiFi.softAP("newIoT"); } }

	void reconnect() {
//		WiFi.disconnect();
		Serial.print("reconnect ");Serial.print(stationSsid);Serial.print(" ");Serial.println(stationPwd);
		WiFi.begin(stationSsid, stationPwd); }
		
	void setAuth(char* ssid, char* pw) {
		strcpy(stationSsid, ssid);
		theDataBase.setData("ssid",stationSsid,strlen(stationSsid));
		strcpy(stationPwd, pw);
		theDataBase.setData("pwd",stationPwd,strlen(stationPwd));
		start(); }

	void onDisconnected() {
		Serial.println("cWifi : Disconnected");
		fireEvent(0);
		reconnect();}

	void onTimeout() {
		if (WiFi.status() != WL_CONNECTED) {
			WiFi.disconnect() ;
			WiFi.mode(WIFI_AP);
			WiFi.softAP("newIoT");
			 } }
			
	void onGotIP(){ 
		Serial.print("cWifi : onGotIP "); Serial.println(WiFi.localIP());
		fireEvent(1);}
	 
	void onConnected(){ }

	bool handleElement() {
		char s[20];
		if (getArgument("ssid", s, 20)) {
			char p[20];
			getArgument("pwd", p, 20);
			setAuth(s,p);}
		send(htmlWifi);
		return false; } } ;
			
cWifi theWifi ;
#if defined(ESP8266)
WiFiEventHandler callbackConnected    = WiFi.onStationModeConnected([](const WiFiEventStationModeConnected& e){theWifi.onConnected();});
WiFiEventHandler callbackDisconnected = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected& e){theWifi.onDisconnected();});
WiFiEventHandler callbackGotIP        = WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP& e){ theWifi.onGotIP();});
#else
WiFiEventId_t callbackConnected     = WiFi.onEvent([](WiFiEvent_t e, WiFiEventInfo_t i){theWifi.onConnected();}, WiFiEvent_t::SYSTEM_EVENT_STA_CONNECTED);
WiFiEventId_t callbackDisconnected  = WiFi.onEvent([](WiFiEvent_t e, WiFiEventInfo_t i){theWifi.onDisconnected();}, WiFiEvent_t::SYSTEM_EVENT_STA_DISCONNECTED);
WiFiEventId_t callbackGotIP         = WiFi.onEvent([](WiFiEvent_t e, WiFiEventInfo_t i){theWifi.onGotIP();}, WiFiEvent_t::SYSTEM_EVENT_STA_GOT_IP);
#endif

//######################################## cWeb #########################################
const char* htmlHead = "<!DOCTYPE html><html>"
                    "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
                    "<link rel=\"icon\" href=\"data:,\">"
                    "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}"
                    ".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px; text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}"
                    ".button2 {background-color: #77878A;}</style></head><body>" ;
#if defined(ESP8266)
const char* htmlRoot = "<h1>ESP8266 Web Server</h1>"
#else
const char* htmlRoot = "<h1>ESP32 Web Server</h1>"
#endif
					"<p><a href=\"/setup\"><button class=\"button\">setup</button></a></p>"
					"<p><a href=\"/update\"><button class=\"button\">update</button></a></p>"
					"<p><a href=\"/restart\"><button class=\"button\">restart</button></a></p>" ;

class cWeb : public cLooper, public cWebElement, public cObserver {
  public :
	cWeb() : cWebElement("/") {	theWifi.addObserver(this);}
		
	void onEvent(int i, int e) {
		theWebServer.on("/update",  HTTP_GET,handleUpdate);
		theWebServer.on("/update", HTTP_POST, handleUpdate1, handleUpdate2) ;
		theWebServer.onNotFound([](){handleWebPages();});
		theWebServer.begin(); }

	static void handleWebPages(){
		char u[20];
		theWebServer.uri().toCharArray(u, 20) ;
		theWebServer.sendContent(htmlHead);
		cWebElement* e=aWebElement;
		while(e!=NULL) {
			if (e->handleElement(u)) break; 
			e=e->nextElement;}
		theWebServer.sendContent("</head><body>");}
		
	bool handleElement() {
		send(htmlRoot);
		return true; }

	void onLoop(){theWebServer.handleClient();} };

cWeb theWeb ;
*/
