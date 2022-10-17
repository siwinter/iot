/*
 *
 * Der Sketch soll auf einem sonoff-switch laufen (noch nicht auf dieser Hardware getestet!!)
 * Das Relais soll über MQTT-Nachrichten und den Button geschaltet werden.
 * Die LED zeigt den Schaltzustand an und blinkt, wenn die Wifi-Verbindung nicht vorhanden ist.
 * 
 */
 
#include "cCore.h"
#include "cDevice.h"
#include "cMqtt.h"

class cSonoff : public cObserver {
  private : 
	cLed*    sonoffLed ;
	cRelais* sonoffRelais ;
	cButton* sonoffButton ;

	int RelIndex ;
	int BtnIndex ;
	int WifiIndex ;

  public :
	cSonoff() {
		WifiIndex = theWifi.addObserver(this) ;
		sonoffRelais = newRelais(12, true, "REL") ;
		RelIndex = sonoffRelais->addObserver(this) ;
		sonoffLed = new cLed(2,false) ;
		sonoffButton = newButton(0,false, "BTN") ;
		BtnIndex = sonoffButton->addObserver(this) ;
		sonoffLed->doComand(cmd_blink) ;}

	void onEvent(int index, int evt) {
		if (index == WifiIndex) {
			if (evt == val_off) sonoffLed->doComand(cmd_blink) ;
			else sonoffLed->doComand(sonoffRelais->getValue());
			return; }
		if (index == BtnIndex) {
			sonoffRelais->doComand(cmd_toggle) ;
			return ; }
		if (index == RelIndex) {
			if (theWifi.getValue() == val_on) sonoffLed->doComand(evt) ; } } } ;
