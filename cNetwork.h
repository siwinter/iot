#ifndef NETWORK_h
#define NETWORK_h

#include "cCore.h"
#include "cDatabase.h"
#include "cDevice.h"

#define cTopicLen 20
#define cNameLen 20
#define cInfoLen 20
#define cNodeNameLen 8
#define cNodeNamesLen 5 * cNodeNameLen
	
class cChannel : public cObserved {
  private :
	char nodeNames[cNodeNamesLen] = "" ;
	void storeNodeName(char* topic) { 
		
	}
	bool checkNodeName(char* topic) {
		return false ; }
		 
  public :
	char topic[cTopicLen] ;
	char info[cInfoLen] ;
	cChannel * nextChannel ;
	virtual bool reconnect() { return true ; }
	virtual void sendMsg(char* topic, char* info);
//	virtual void sendEvent(cMsg* msg)  = 0 ;
	virtual bool sendComand(char* topic, char* info) {
		if (checkNodeName(topic)) {
			sendMsg(topic, info) ;
			return true ; }
		return false ;}
	virtual void subscribe(char* topic)  = 0 ;
	void received() ;			
	void connected() { fireEvent(val_connected) ; } };

class cTranslator {
  public:
	virtual char* int2str(char* s, int i) = 0;
	virtual int str2int(char* s) = 0; } ;

class cTxtAdapter : public cObserver {
  private:
	cDevice* device ;
	char deviceName[cNameLen] ;
  public:
	cTxtAdapter* nextDevice ;
	cTranslator * format ;

	cTxtAdapter(cTranslator* sf, cDevice* d, char* n) {
		device = d;
		device->addObserver(this);
		strcpy(deviceName, n);
		format = sf ; }
		
	void doComand(char* cmd) { device->doComand(format->str2int(cmd)) ; }
	
	void onEvent(int i, int c);
	
	bool receiveCmd(char* name, char* info) {
		if ( strcmp(deviceName, name) != 0 ) return false ;
		doComand(info) ;
		return true ;}
};

class cScheduler : public cObserver {
  private :
	cChannel* downStreamChannel;
	cTxtAdapter* aDevice ;
	
	bool readNodeName() {
//		Serial.println("theScheduler.readNodeName");
		char n[cNodeNameLen] ;
		if (theDataBase.getData("node", n, 8)) {
			setNodeName(n);
			return true ;}
		return false ;}
	
	int localCmd(char* topic) {
		char* t = topic + 4 ;
		int nameLength = strlen(nodeName) ;
		int i ;
		for (i=0; i<nameLength; i++) { if(t[i] != nodeName[i]) return 0; }
		return i + 5; }
  public :
	cChannel* upStreamChannel;
	int channelEvent;
	int dbEvent;
	char evtTopic[20] ;
	char* nodeName = evtTopic + 4 ;
	char* devName ;
	cScheduler() {
		aDevice = NULL ;
		upStreamChannel = NULL ;
		downStreamChannel = NULL ;
		dbEvent = theDataBase.addObserver(this) ;
		strcpy(evtTopic,"evt/");
		if(!readNodeName()) setNodeName(""); }
	
	void newNodeName(char*n) {
//		Serial.print("newNodeName: ");Serial.println(n);
		theDataBase.setData("node",n,strlen(n));
	}
	void setNodeName(char*n) {
		strcpy(nodeName,n);
		devName= nodeName+strlen(nodeName) ; }
	char* getNodeName() { return nodeName; }
	
	void insertChannel(cChannel* c) {
		channelEvent = c->addObserver(this);
		upStreamChannel = c ; }
	
	void onEvent(int i, int e) {
//		Serial.print("theNetwork onEvent: ");
		if ( i == channelEvent ) {
//			Serial.print(" channelEvent: ");Serial.println(e);
			if (e == val_connected) {
				readNodeName() ;
				char txt[cTopicLen] ;
				strcpy(txt,"sbs/cmd/");
				strcat(txt,nodeName);
				strcat(txt,"/#");
				upStreamChannel->subscribe(txt) ; } }
		else if ( i == dbEvent ) {
//			Serial.print(" dbEvent: ");Serial.println(e);
			if ( e == val_on) readNodeName() ; } }
		
	void addDevice(cTxtAdapter* d) {
		d->nextDevice = aDevice ;
		aDevice = d ; }
		
	void receiveEvent(char* topic, char* info) {
		//upStreamChannel->sendEvent(m);
		 }
	void subscribe(char* m) {upStreamChannel->subscribe(m); }
	void receiveCmd(char*topic, char* info) {
		int devicePointer = 0;
		if (devicePointer = localCmd(topic)) {
			char* deviceName = topic + devicePointer;
			cTxtAdapter * d = aDevice ;
			while (d != NULL) {
				if (d->receiveCmd(deviceName, info)) return ;
				d = d->nextDevice ; }}
		else {
			cChannel* channel = downStreamChannel;
			while (channel != NULL) {
				if (channel->sendComand(topic, info)) return ;
				channel =channel->nextChannel ; }}}
	void sendEvent(char* name, char* info) {
		strcpy(devName, "/");
		strcat(devName, name);
		if( upStreamChannel != NULL) upStreamChannel->sendMsg(evtTopic, info);
		strcpy(devName,""); } };

cScheduler theScheduler ;

	
void cTxtAdapter :: onEvent(int i, int c) {
	char info[cInfoLen] ;
	format->int2str(info, c) ;
	theScheduler.sendEvent(deviceName, info); }

//########################################################################################

void cChannel :: received() {
		if ((topic[0]=='e')&&(topic[1]=='v')&&(topic[2]=='t')&&(topic[3]=='/')) { 
			// topic; evt/nodeName/deviceName
			theScheduler.receiveEvent(topic, info); return;}
		if ((topic[0]=='c')&&(topic[1]=='m')&&(topic[2]=='d')&&(topic[3]=='/')) { 
			// topic: cmd/nodeName/deviceName
			theScheduler.receiveCmd(topic, info); return;}
		if ((topic[0]=='s')&&(topic[1]=='b')&&(topic[2]=='s')&&(topic[3]=='/')) {
			// sbs/nodeName/#
			storeNodeName(topic) ;
			theScheduler.subscribe(topic); return;} }

class cSerialChannel : public cChannel, cLooper {
  private :
    bool receiving = false ;
    int msgIndex ;
    int maxLen ;
    char buf[cInfoLen] ;
  public:
	bool active ;
	cSerialChannel() { 
		Serial.begin(115200); } 	
	
    void onLoop() {
		char c;
		if (Serial.available()) {
			c = Serial.read();
			switch (c) {
			  case '>':
				receiving = true ;
				msgIndex = 0 ;
				maxLen = cTopicLen ;
				break ;
			  case ':':
				if (receiving) {
					buf[msgIndex++] = 0 ;
					strcpy(topic, buf);
					msgIndex = 0 ;
					maxLen = cInfoLen ; }
				break ;
			  case 13:
			  case 10:
				if (receiving) {
					buf[msgIndex++] = 0 ;
					strcpy(info, buf);
//					Serial.print("cSerialChannel : received  ");Serial.print(msg.name);Serial.print(":");Serial.println(msg.info);
					received();
					receiving = false; }
				break ;
			  default :
				if (receiving) buf[msgIndex++] = c ;} } }

	void subscribe(char* topic) {};
	bool sendComand(char* topic, char* info) {return false;};
	void sendMsg(char* topic, char* info) {
		Serial.print(">");
		Serial.print(topic) ;
		Serial.print(":");
		Serial.println(info); } } ;
		
cSerialChannel theSerial ;



#include "cNtwDevices.h"

#endif

