#ifndef CCHANNEL_H
#define CCHANNEL_H
/*
#include "cCore.h"

// : inf/systemname/devicename
// : cmd/th1/rel1

class cOldSerialChannel : public cOldChannel, cLooper {
  private :
    bool receiving = false ;
    int msgIndex ;
    int maxLen ;
    char buf[cInfoLen] ;
    cMsg * msg ;
  public:
	bool active ;
	cOldSerialChannel() { 
		Serial.begin(115200); 
		initTopic();} 	
	
    void onLoop() {
		char c;
		if (Serial.available()) {
			c = Serial.read();
			switch (c) {
			  case '>':
				msg = receivedQueue->newMsg() ;
				if (msg != NULL) {
					receiving = true ;
					msgIndex = 0 ;
					maxLen = cNamLen ; }
				break ;
			  case ':':
				if (receiving) {
					buf[msgIndex++] = 0 ;
					strcpy(msg->name, buf);
					msgIndex = 0 ;
					maxLen = cInfoLen ; }
				break ;
			  case 13:
			  case 10:
				if (receiving) {
					buf[msgIndex++] = 0 ;
					strcpy(msg->info, buf);
//					Serial.print("cSerialChannel : received  ");Serial.print(msg->name);Serial.print(":");Serial.println(msg->info);
					receivedQueue->insert(msg);
					receiving = false; }
				break ;
			  default :
				if (receiving) buf[msgIndex++] = c ;} } }

      bool sendMsg(cMsg* msg) {
		Serial.print(">");
		Serial.print(setTopic(msg->name)) ;
		Serial.print(":");
		Serial.println(msg->info);
		return true ;} } ;
*/
#endif
