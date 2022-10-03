
#include "cCore.h"
#include "cDevice.h"
//#include "cTest.h"
#include "cNetwork.h"
#include "cDatabase.h"
//#include "cDHT.h"
//#include "cMqtt.h"
//#include "cEnow.h"

cLed* l ;

class cSetter : public cTimer {
  public :
	cSetter() { setTimer(10) ; }
	
	void onTimeout() {
		uint8_t mac[6];
		l->toogle();
		setTimer(10) ;
/*
// ESP 1	
	mac[0] = 0x84 ;
	mac[1] = 0x0B ;
	mac[2] = 0x8E ;
	mac[3] = 0xA3 ;
	mac[4] = 0xBD ;
	mac[5] = 0x85 ;
#/	
// ESP 2
	mac[0] = 0x86 ;
	mac[1] = 0x0D;
	mac[2] = 0x8E ;
	mac[3] = 0xA3 ;
	mac[4] = 0xB8 ;
	mac[5] = 0x71 ; 
/*	
// ESP 3
	mac[0] = 0x ;
	mac[1] = 0x ;
	mac[2] = 0x ;
	mac[3] = 0x ;
	mac[4] = 0x ;
	mac[5] = 0x ; 
*/	
//	theDataBase.deleteData("mac"); 
//	theDataBase.setData("mac", (char*) mac, 6); 
} };

void setup() {
	Serial.begin(115200);
	Serial.println("start");
	l = newLed(2, false, "LED");
	new cSetter();

/*
	
	cLoopTest* l1 = new cLoopTest(1, "l1");
	cLoopTest* l2 = new cLoopTest(2, "l2");
	cObservedTest* e1 = new cObservedTest(2, "e1") ;
	cObservedTest* e2 = new cObservedTest(2, "e2") ;
	cObservedTest* e3 = new cObservedTest(2, "e3") ;
	e2->addObserver(new cObserverTest("o2"));
	e3->addObserver(new cObserverTest("o31"));
	e3->addObserver(new cObserverTest("o32"));
	e3->addObserver(new cObserverTest("o33"));
	new cLedTest() ;
	cLoopTest* l2 = new cLoopTest(2, "l2");
//	newLed(2, false, "LED1") ;
	new cTxtLedTest() ;
//	theScheduler.upStreamChannel = new cSerialChannel() ;
//	theScheduler.insertChannel(&theMqtt) ;
//	newDHT22(4,"Wohnz") ;
//	newBME280("Wohnz") ;
	new cTxtLedTest() ;

	newLed(2, false, "LED");
	newDHT22(4,"_1") ;
#/
	//newHeap("Heap") ;
	//newRssi("Rssi") ;
//	new cTxtLedTest() ;
//	char ssid[] = "Hundehaus" ;
//	char pwd[] = "Affenhaus" ;
//	theWifi.configure("ssid", ssid, strlen(ssid)) ;
//	theWifi.configure("pwd", pwd, strlen(pwd)) ;
	l = newLed(2, false, "LED"); */
}


void loop() { 
	systemLoop(); }

/*
//###########################################################################
//slave/receiver  ESP-8266

extern "C" {
#include <espnow.h>
}
#include <ESP8266WiFi.h>

#define CHANNEL 0

unsigned long timeout = millis();
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len);

// Init ESP Now with fallback
void InitESPNow() {
  if (esp_now_init() == 0) {
    Serial.println("ESPNow Init Success");
  }
  else {
    Serial.println("ESPNow Init Failed");
    ESP.restart();
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("ESPNow Basic Receiver Example");

  //Set device in AP mode to begin with
  WiFi.mode(WIFI_AP_STA);
  // This is the mac address of the Slave in AP Mode
  Serial.print("Receiver MAC: "); Serial.println(WiFi.softAPmacAddress());

  // Init ESPNow with a fallback logic
  InitESPNow();
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info.
  esp_now_register_recv_cb(OnDataRecv);
}

// callback when data is recv from Master
void OnDataRecv(uint8_t *mac_addr, uint8_t *data, uint8_t len) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print("Last Packet Recv from: "); Serial.println(macStr);
  Serial.print("Last Packet Recv Data: "); Serial.println((char*)data);
  Serial.println("");
}

void loop() {
  if (millis() - timeout > 3000) {
    Serial.println("Waiting for data ...");
    timeout = millis();
  }
}
/#
//###########################################################################
//master/sender ESP-8266

//#include <Arduino.h>
#include <ESP8266WiFi.h>
extern "C" {
  #include <espnow.h>
}
#define CHANNEL 0
uint8_t remoteMac[] = {0x86, 0x0D, 0x8E, 0xA3, 0xB8, 0x71}; // Replace with the AP MAC address of the slave/receiver
uint8_t data = 1;

void printMacAddress(uint8_t* macaddr) {
  Serial.print("{");
  for (int i = 0; i < 6; i++) {
    Serial.print("0x");
    Serial.print(macaddr[i], HEX);
    if (i < 5) Serial.print(',');
  }
  Serial.println("};");
}

// Init ESP Now with fallback
void InitESPNow() {
  if (esp_now_init() == 0) {
    Serial.println("ESPNow Init Success");
  }
  else {
    Serial.println("ESPNow Init Failed");
    ESP.restart();
  }
//  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
}

void sendData() {
  data++;
  int result = esp_now_send(remoteMac, &data, sizeof(data));
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  //Set device in STA mode to begin with
  WiFi.mode(WIFI_STA);
  Serial.println("ESPNow Basic Master Example");
  // This is the mac address of the Master in Station Mode
  Serial.print("STA MAC: "); Serial.println(WiFi.macAddress());
  // Init ESPNow with a fallback logic
  InitESPNow();

  esp_now_register_send_cb([](uint8_t* macaddr, uint8_t status) {
	  if (status == 0) Serial.println ("send ok");
	  else  Serial.println ("send failure");
  });
  int addStatus = esp_now_add_peer((u8*)remoteMac, ESP_NOW_ROLE_COMBO, CHANNEL, NULL, 0);
}

void loop() {
  sendData();
  delay(10000);
}
#/

#include "cDatabase.h"
#include"cNow.h"

void setup() {
	Serial.begin(115200);
	delay(2000); }
	
void loop() { 
	systemLoop(); }
*/
