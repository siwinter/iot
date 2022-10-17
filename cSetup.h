#ifndef SETUP_h
#define SETUP_h
#if defined(EEPROM_AV)
/*--------------------------------------------------------------------------------------------------
 * 
 * Mit diesem Modul können Änderungen in der Konfiguration des EEPROMs vorgenommen werden
 * Die Kommando werden als MQTT-Kommandos über einen Channel geschickt. Auf der seriellen
 * Schnittstelle haben sie z.B. folgenes Format:
 * 
 * >cmd/<nodeName>/setup:<Kommando>;<Parameter>
 * 
 * >cmd/<nodeName>/setup:print  gibt den Inhalt des Speichers auf der seriellen Schnittstelle aus
 * >cmd/<nodeName>/setup:delete;<key> löscht eine Eintrag
 * >cmd/<nodeName>/setup:<key>;<value> erzeugt oder ändert einen Eintrag
 * 
 * Besondere Einträge:
 * >cmd/<nodeName>/setup:mac;86.0D.8E.A3.B8.71  Die Bytes werden durch "." getrennt
 * >cmd/<nodeName>/setup:broker;192.168.2.123;1883
 * 
 * 
 */


void printIP(uint8_t* ip) {
	for(int i=0 ; i<4 ; i++) {
		Serial.print(ip[i], DEC); if(i<3) Serial.print("."); } }

bool txt2Mac(const char* txt, uint8_t* mac) {
	int i = 0; int j = 0 ;
	mac[j] = 0 ;
	while ( i < strlen(txt) ) {
		char c = txt[i++] ;
		if ( c == '.') { mac[++j] = 0; c = txt[i++]; }
		if (( c >= 'a') && (c <= 'f')) c = c - 'a' + 'A' ;
		if (( c >= 'A') && (c <= 'F')) mac[j] = 16 * mac[j] + (uint8_t)(c -'A' + 10) ;
		else if (( c >= '0') && (c <= '9')) mac[j] = 16 * mac[j] + (uint8_t)(c -'0') ;
			else return false ; }
	return true ; }

bool txt2IP(const char* txt, uint8_t* ip) {
	int i = 0; int j = 0 ;
	ip[j] = 0 ;
	while ( i < strlen(txt) ) {
		char c = txt[i++] ;
		if ( c == '.') {ip[++j] = 0;; c = txt[i++]; }
		if (( c >= '0') && (c <= '9')) ip[j] = 10 * ip[j] + (uint8_t)(c -'0') ;
		else return false ; }
	return true ; }

bool txt2Port(const char* txt, uint8_t* port) {
	int p = 0 ;
	int i = 0 ;
	while(i<strlen(txt)) {
		char c = txt[i++] ;
		if((c >='0') && (c <= '9')) p = p*10 + c -'0';
		else return false; }
	port[0] = p/256 ;
	port[1] = p%256 ;
	return true; }
	
void printDataBase() {
	int i = 0 ;
	char key[20] ;
	char data[20] ;
	int dataLen ;
	while ( theDataBase.getData(i++, key, data, &dataLen)) {
		Serial.print(dataLen);Serial.print("\t");Serial.print(key);Serial.print("\t");
		if (strcmp("mac", key) == 0) { printMac((uint8_t*)data); Serial.println() ; }
		else if (strcmp("broker", key) == 0) {
			printIP((uint8_t*)data);
			Serial.print(" : ");
			Serial.println((256*  (uint8_t)data[4]) + (uint8_t)data[5]);
			}
			else Serial.println(data) ; } }

class cSetup : public cCmdInterface {
  public :
	char* param;
	
	bool receiveCmd(char* name, char* info) {
		if ( strcmp("setup", name) != 0 ) return false ;   
		int i; for (i=0 ; i<strlen(info) ; i++) if ( info[i] == ';') break;
		if (i<strlen(info)) {
			info[i] = 0 ;
			param = info + strlen(info) + 1; }
		else param = info + strlen(info) ;
		Serial.print("Comand: ") ;	Serial.print(info) ;	Serial.print(" ; ") ;	Serial.println(param) ;
		if ( strcmp("delete", info) == 0 ) { theDataBase.deleteData(param) ; return true ;}
		if ( strcmp("print", info) == 0 ) { printDataBase() ; return true ;}
		if ( strcmp("broker", info) == 0 ) {
			uint8_t brk[6];
			char* txtPort;
			int port;
			int i; for (i=0 ; i<strlen(param) ; i++) if ( param[i] == ';') break;
			if (i==strlen(param)) { brk[4] = 1883/256 ; brk[5] = 1883%256; }
			else {
				param[i] = 0 ;
				txtPort = param + strlen(param) +1 ;
				if ( !txt2Port(txtPort, &brk[4]) ) return true;}
			if (txt2IP(param, brk)) theDataBase.setData("broker", (char*)brk , 6) ;
			return true; }
		if ( strcmp("mac", info) == 0 ) {
			uint8_t mac[6];
			if ( txt2Mac(param, mac) ) theDataBase.setData("mac", (char*)mac , 6) ;
			return true ;}
		theDataBase.setData(info, param, strlen(param)) ;
		return true ;} };

cSetup theSetup ;

#endif
#endif
