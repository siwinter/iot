#ifndef NETWORK_h
#define NETWORK_h

#include "cCore.h"

#define cTopicLen 50
#define cInfoLen 50
#define cNameLen 20
#define cNodeNameLen 8

class cCmdInterface ;
tList<tLink<cCmdInterface>, cCmdInterface>  theDevices ;

class cChannel ;
tList<tLink<cChannel>, cChannel>  theChannels ;

char theEvtTopic[cTopicLen] = "evt/";
char* theNodeName = theEvtTopic + 4;

//#################################### cCmdInterface ##################################### 
class cCmdInterface {
  public :
	cCmdInterface() { theDevices.append(this); }
	virtual bool receiveCmd(char* name, char* info) = 0 ; } ;
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
	tList<tLink<cNode>, cNode>  myNodes ;	// Liste der Nodes für die der Channel zuständig ist

	bool isConnected = false;
	
	int localCmd(char* topic) {				// cmd/nodeName/deviceName
		char* t = topic + 4 ;
		if((t[0]=='.') && (t[1]=='/')) return 6 ;
		int nameLength = strlen(theNodeName) ;
		int i ;
		for (i=0; i<nameLength; i++) { if(t[i] != theNodeName[i]) return 0; }
		return (i + 4); }
		
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
				
	void upstreamActive() {
		if ( theChannels.readNext(NULL) == this ) {
			char sbsTxt[cTopicLen] ;
			strcpy(sbsTxt, "cmd/");
			strcat(sbsTxt, theNodeName); 
			strcat(sbsTxt, "#"); 
			subscribe(sbsTxt); 
			fireEvent(val_connected) ; } }

	void connected() {
		isConnected = true ;
		if (strlen(theNodeName) > 0) upstreamActive() ; }

	void nameChanged() {
		if (isConnected) upstreamActive() ; }

	void received() {
//		Serial.print("received ");
		if ((topic[0]=='e')&&(topic[1]=='v')&&(topic[2]=='t')&&(topic[3]=='/')) { // topic; evt/nodeName/deviceName
//			Serial.println("evt");
			theChannels.getNext(NULL)->sendMsg(topic, info) ;
			return ; } 
		if ((topic[0]=='c')&&(topic[1]=='m')&&(topic[2]=='d')&&(topic[3]=='/')) { // topic: cmd/nodeName/deviceName
//			Serial.println("cmd");
			int devicePointer = 0;
			if (devicePointer = localCmd(topic)) {
				char* deviceName = topic + devicePointer;
				cCmdInterface * d = theDevices.getNext(NULL) ;
				while (d != NULL) {
					if (d->receiveCmd(deviceName, info)) return ;
					d = theDevices.getNext(d) ; } }
			else {
//				Serial.println("----> to first Channel ");
				cChannel* channel = theChannels.getNext(theChannels.getNext(NULL)) ; //start with second Channel (= downstream-Channel)
				while (channel != NULL) {
					if (channel->sendComand(topic, info)) return ;
					channel = theChannels.getNext(channel) ; } }
			return;}
		if ((topic[0]=='s')&&(topic[1]=='b')&&(topic[2]=='s')&&(topic[3]=='/')) { // topic: sbs/nodeName/#
//			Serial.println("sbs");
			theChannels.getNext(NULL)->subscribe(topic + 4) ;
			storeNodeName(topic + 4) ;
			return ; } } };
//##################################### cNodeName ########################################
class cNodeName : public cConfig {
  public :
	bool configure(char* key, char* value, int vLen) {
		if (strcmp(key, "node") == 0) {
			strcpy(theNodeName,value);
			strcat(theNodeName,"/");
			theChannels.readNext(NULL)->nameChanged();
			return true ; }
		return false ; } } ;

cNodeName theNode ;
void changeTopicName(char* t) { theNode.configure("node", t, strlen(t)); }

//################################### cSerialChannel #####################################
class cSerialChannel : public cChannel, public cLooper {
  private :
    uint8_t receiving = 0 ;
    int msgIndex ;
    int maxLen ;
	char* buf ;
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
				receiving = 1 ;
				msgIndex = 0 ;
				buf = topic ;
				maxLen = cTopicLen ;
				break ;
			  case ':':
				if (receiving == 1) {
					receiving = 2 ;
					buf[msgIndex++] = 0 ;
					buf = info ;
					msgIndex = 0 ;
					maxLen = cInfoLen ; }
				break ;
			  case 13:
			  case 10:
				if (receiving > 0) {
					buf[msgIndex++] = 0 ;
//					Serial.print("cSerialChannel : received  ");Serial.print(topic);Serial.print(":");Serial.println(info);
					received();
					receiving = 0; }
				break ;
			  default :
				if (receiving > 0 ) buf[msgIndex++] = c ;} } }

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
