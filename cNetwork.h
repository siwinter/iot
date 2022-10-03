#ifndef NETWORK_h
#define NETWORK_h

#include "cCore.h"

#define cTopicLen 20
#define cNameLen 20
#define cInfoLen 20
#define cNodeNameLen 8

class cTxtAdapter ;
tList<tLink<cTxtAdapter>, cTxtAdapter>  theDevices ;

class cChannel ;
tList<tLink<cChannel>, cChannel>  theChannels ;

char theEvtTopic[cTopicLen] = "evt/";
char* theNodeName = theEvtTopic + 4;

//##################################### cTxtAdapter ###################################### 
class cTranslator {
  public:
	virtual char* int2str(char* s, int i) = 0;
	virtual int str2int(char* s) = 0; } ;

class cTxtAdapter : public cObserver {
  private:
	cDevice* device ;
	char deviceName[cNameLen] ;
  public:
	cTranslator * format ;

	cTxtAdapter(cTranslator* sf, cDevice* d, char* n) {
		device = d;
		device->addObserver(this);
		strcpy(deviceName, n);
		format = sf ; 
		theDevices.insert(this); }
		
	void doComand(char* cmd) { device->doComand(format->str2int(cmd)) ; }
	
	void onEvent(int i, int c) ;
	
	bool receiveCmd(char* name, char* info) {
		if ( strcmp(deviceName, name) != 0 ) return false ;
		doComand(info) ;
		return true ;} };

//####################################### cChannel ########################################
class cNode {
  public:
	cNode(char* n) {
		int i ;
		for(i=0; i<cNodeNameLen; i++) {
			if (n[i] == '/') return;
			name[i] = n[i]; }
		name[i] = 0 ; }

	char name[cNodeNameLen]; } ;

class cChannel : public cObserved {
  private :
	tList<tLink<cNode>, cNode>  myNodes ;	// Liste der Nodes für die  
	int localCmd(char* topic) {
		char* t = topic + 4 ;
		int nameLength = strlen(theNodeName) ;
		int i ;
		for (i=0; i<nameLength; i++) { if(t[i] != theNodeName[i]) return 0; }
		return i + 4; }
	void storeNodeName(char* t) {			// t: nodeName/#
		cNode* n = new cNode(t) ;				// s.w. Doppeleinträge werden hier nicht verhindert !!!
		myNodes.insert(n); }
			
	bool checkNodeName(char* t) { return true ; }
  public :
	virtual void subscribe(char* topic)  = 0 ;  // topic = cmd/nodename/#
	virtual void sendMsg(char* topic, char* info) = 0;
	virtual bool sendComand(char* topic, char* info) {
		if (checkNodeName(topic)) {
			sendMsg(topic, info) ;
			return true ; }
		return false ;}

	cChannel(bool upstream) {
		if (upstream) theChannels.insert(this) ; 
		else theChannels.append(this) ;} // wird vorn in die Liste eingetragen und wird dadurch zum upstream-Channel	
	
	char topic[cTopicLen] ;
	char info[cInfoLen] ;
	void subscribeMe() {
		if ( theChannels.readNext(NULL) == this ) 
			if ( strlen(theNodeName) > 0 ) {
				char sbsTxt[cTopicLen] ;
				strcpy(sbsTxt, "cmd/");
				strcat(sbsTxt, theNodeName); 
				strcat(sbsTxt, "#"); 
				subscribe(sbsTxt); } }

	void connected() {
		char sbs[cTopicLen] ;
		strcpy(sbs, "cmd/");
		strcat(sbs, theNodeName); 
		strcat(sbs, "#"); 
		subscribe(sbs);
		fireEvent(val_connected) ; }

	void received() {
		Serial.println("received");
		if ((topic[0]=='e')&&(topic[1]=='v')&&(topic[2]=='t')&&(topic[3]=='/')) { // topic; evt/nodeName/deviceName
			theChannels.getNext(NULL)->sendMsg(topic, info) ;
			return ; } 
		if ((topic[0]=='c')&&(topic[1]=='m')&&(topic[2]=='d')&&(topic[3]=='/')) { // topic: cmd/nodeName/deviceName
			int devicePointer = 0;
			if (devicePointer = localCmd(topic)) {
				char* deviceName = topic + devicePointer;
				cTxtAdapter * d = theDevices.getNext(NULL) ;
				while (d != NULL) {
					if (d->receiveCmd(deviceName, info)) return ;
					d = theDevices.getNext(d) ; } }
			else {
				cChannel* channel = theChannels.getNext(theChannels.getNext(NULL)) ; //start with second Channel (= downstream-Channel)
				while (channel != NULL) {
					if (channel->sendComand(topic, info)) return ;
					channel = theChannels.getNext(channel) ; } }
			return;}
		if ((topic[0]=='s')&&(topic[1]=='b')&&(topic[2]=='s')&&(topic[3]=='/')) { // topic: sbs/nodeName/#
			theChannels.getNext(NULL)->subscribe(topic + 4) ;
			storeNodeName(topic + 4) ;
			return ; } } };
//-----------------------------------------------------------------------------------------
void cTxtAdapter :: onEvent(int i, int c) {
	char info[cInfoLen] ;
	format->int2str(info, c) ;
	int l = strlen(theEvtTopic) ;
	strcpy((theEvtTopic + l), deviceName);
	theChannels.readNext(NULL)->sendMsg(theEvtTopic, info);
	theEvtTopic[l] = 0 ; }

//##################################### cNodeName ########################################
class cNodeName : public cConfig {
  public :
	bool configure(char* key, char* value, int vLen) {
		if (strcmp(key, "node") == 0) {
			strcpy(theNodeName,value);
			strcat(theNodeName,"/");
			theChannels.readNext(NULL)->subscribeMe();
			return true ; }
		return false ; } } ;

cNodeName theNode ;
void changeTopicName(char* t) { theNode.configure("node", t, strlen(t)); }

//################################### cSerialChannel #####################################
class cSerialChannel : public cChannel, cLooper {
  private :
    bool receiving = false ;
    int msgIndex ;
    int maxLen ;
    char buf[cInfoLen] ;
  public:
	bool active ;
	cSerialChannel() : cChannel(true) {		// insert as upstream (may be changed by next channel)  
		Serial.begin(115200); 
		connected(); }
	
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

	void subscribe(char* topic) {
		Serial.print(">sbs/");
		Serial.println(topic); }
		
	void sendMsg(char* topic, char* info) {
		Serial.print(">");
		Serial.print(topic) ;
		Serial.print(":");
		Serial.println(info); } } ;
		
cSerialChannel theSerial ;
#include "cNtwDevices.h"

#endif
