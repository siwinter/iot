#ifndef CWIFI_H
#define CWIFI_H
#if defined(WIFI_AV)
#include "cCore.h"

//###################################### Webserver ######################################
#if defined(ESP8266)
#include <ESP8266WebServer.h>
ESP8266WebServer theWebServer(80);
#else
#include <WebServer.h>
#include <Update.h>
WebServer theWebServer(80);
#endif
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
tList<tLink<cWebElement>, cWebElement>  theWebElements ;

class cWebElement {
  protected: 
	char uri[20] ;
	void send(const char* txt) {theWebServer.sendContent(txt);}
	bool getArgument(const char* arg, char* param, int pLen) {
		if (!theWebServer.hasArg(arg)) return false;
		theWebServer.arg(arg).toCharArray(param, pLen) ;
		return true;}
  public:
	cWebElement(const char* u){
		strncpy(uri,u,20);
		theWebElements.insert(this) ; }
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

#define state_idle 0
#define state_doNotStart 2

class cWifi : public cTimer, public cDevice, public cConfig {
  private :
	char stationPwd[20] ;
	char stationSsid[20] ;
	char apSsid[20] ;
	uint8_t state ;

  public :
	cWifi() {
		state = state_idle ;
		strcpy(stationSsid,"") ;
		strcpy(stationPwd,"") ; 
		setTimer(10) ; }

	bool configure(const char* key, const char* value, int vLen) {
		Serial.print("cWifi.configure key: "), Serial.print(key); Serial.print(" value: ") ; Serial.println(value) ;
		if (strcmp(key, "ssid") == 0) {
			strcpy(stationSsid, value);
			return true ;}
		if (strcmp(key, "pwd") == 0) {
			strcpy(stationPwd, value);
			return true ;}
		if (strcmp(key, "prot") == 0)
			if (strcmp(value, "Enow") == 0) state = state_doNotStart ;
		return false ; }

	void start(){
		if (state != state_doNotStart) {
			Serial.println("cWifi.start");
			WiFi.mode(WIFI_STA);
			WiFi.disconnect() ;
			reconnect() ; } }

	void reconnect() {
		Serial.print("reconnect ");Serial.print(stationSsid);Serial.print(" ");Serial.println(stationPwd);
		WiFi.begin(stationSsid, stationPwd); }

	void onDisconnected() {
//		Serial.println("cWifi : Disconnected");
		setValue(val_off);
		reconnect();}

	void onTimeout() {
//		Serial.println("cWiFi.onTimeout");
		if (state == state_doNotStart) return ;
		if (WiFi.status() != WL_CONNECTED) {
			WiFi.disconnect() ;
			WiFi.mode(WIFI_AP);
			WiFi.softAP("newIoT");
//			Serial.print("cWiFi new softAP: "); Serial.println("newIoT");
			setValue(val_wifiAP); } }
			
	void onGotIP(){
		Serial.print("cWifi : onGotIP "); Serial.println(WiFi.localIP().toString());
		setValue(val_on);}

	void onConnected(){ 
//		Serial.println("cWiFi.onConnected");
} } ;
			
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

//######################################## cWeb ########################################## 
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
		if (e>0) {
		theWebServer.on("/update",  HTTP_GET,handleUpdate);
		theWebServer.on("/update", HTTP_POST, handleUpdate1, handleUpdate2) ;
		theWebServer.onNotFound([](){handleWebPages();});
		theWebServer.begin(); } }

	static void handleWebPages(){
		char u[20];
		theWebServer.uri().toCharArray(u, 20) ;
		theWebServer.sendContent(htmlHead);
		cWebElement* e = theWebElements.readFirst() ;
		while(e!=NULL) {
			if (e->handleElement(u)) break; 
			e=theWebElements.readNext();}
		theWebServer.sendContent("</head><body>");}
		
	bool handleElement() {
		send(htmlRoot);
		return true; }

	void onLoop(){theWebServer.handleClient();} };

cWeb theWeb ;

//##################################### cWebElement ###################################### 
const char* htmlWifi = 
	"<h3>WLAN</h3>"
		"<p><form action=\"/setup\">"
			"<p>SSID: <input type=\"text\" name=\"ssid\"pattern=\"{,9}\"></p>"
			"<p>PWD: <input type=\"text\" name=\"pwd\"pattern=\"{,9}\" ></p>"
			"<p><input type=\"submit\"></p></form></p>";

class cWiFiWebsite : public cWebElement {
  public :
	cWiFiWebsite() : cWebElement("/setup") {}

	bool handleElement() {
		char s[20];
		if (getArgument("ssid", s, 20)) {
			theConfigurator->setConfig("ssid", s) ;
//			theDataBase.setData("ssid",s);
			char p[20];
			getArgument("pwd", p, 20);
			theConfigurator->setConfig("pwd", p) ; }
//			theDataBase.setData("pwd",p);}
		send(htmlWifi);
		return false; } } ; 

cWiFiWebsite theWiFiWebsite ;

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 
 *	RssiSensor misst die Feldstärke des verbundenen WiFi-Accespoints
 * 
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

class cRssi : public cIntervalSensor {
  public :
    cRssi() {
		setInterval(30); }
    void measure(){
		uint16_t f=WiFi.RSSI();
//		f = (f + f/10) / 10 ;
		setValue(f); } } ;

cRssi* newRssi(cb_function  f) {
	cRssi* d =new cRssi() ;
	cCallBackAdapter* cb = new cCallBackAdapter(f, d);
	return d ; }

#endif
#endif
