/*
 * Vor dem Kompilieren die Datei kopieren nach iot.ino
 *
 * Der Sketch soll auf einem sonoff-switch laufen (noch nicht auf dieser Wardware getestet!!)
 * Das Relais soll über MQTT-Nachrichten und den Button geschaltet werden.
 * Die LED zeigt den Schaltzustand an und blinkt, wenn die Wifi-Verbindung nicht vorhanden ist.
 * 
 * Beachte die unterschiedlich create-Funktionen für die Devices und ggf. das nachträgliche Hinzufügen der Callback-Funktion! 
 */

/*
#include "cDevice.h"
#include "cWifi.h"
#include "cMqtt.h"

cLed* theLed ;
cRelais* theRelais ;
cButton* theButton ;

void onWifi(int evt) {
	if (evt == val_off) theLed->doComand(cmd_blink) ;
	else theLed->doComand(theRelais->getValue()); }

void onButton(int evt) { theRelais->doComand(cmd_toogle) ;}
void onRelais(int evt) { if (theWifi.getValue() == val_on) theLed->doComand(evt) ;}


void setup() {
	theRelais = newRelais.create(12, true, "REL") ;
	new cCallBackAdapter(&onRelais, theRelais);
	theLed = newLed.create(2,false) ;
	theButton = newButton.create(0,false, &onButton) ;
	new cCallBackAdapter(&onWifi, &theWifi);
	theLed->doComand(cmd_blink) ;}

	
void loop() { systemLoop(); }
*/
