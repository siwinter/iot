#ifndef FLAGS_h
#define FLAGS_h
/*
 * Das sind die MACROS (Defines), anhand derer beim Kompilieren die Zielhardware erkannt wird.
 * Dem Compiler wird dieses MACRO in der Regel mit dem Flag -D<MACRO> übergeben.
 * Dabei gibt es kein Leerzeichen nach dem -D.
 * Die Flags stehen in der zugehörigen Datei platform.txt
 * Gefunden habe ich auch -DARDUINO_{build.board}
 * Die Auflösung für {build.board} steht dann in boards.txt
 *  
 * Getestete MACROS sind:
 * 
 * ARDUINO_AVR_LARDU_328E
 * ESP32
 * ESP8266
 * 
 */

#if defined(ESP8266)
#define WIFI_AV			// WiFi available
#define EEPROM_AV		// EEPROM available
#endif


#if defined(ESP32)
#define WIFI_AV			// WiFi available
#define EEPROM_AV		// EEPROM available
#endif

#if defined(ARDUINO_GENERIC_RP2040)  // raspberry pico
#endif

#endif
