#ifndef CLOG_H
#define CLOG_H

#include <Arduino.h>
#include <stdarg.h>

//#define DISABLE_LOGGING               // uncomment to prevent logging
#define LOG_LEVEL_SILENT  0
#define LOG_LEVEL_ERROR   1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_INFO    3
#define LOG_LEVEL_DEBUG   4

class cLog {
  private :
    va_list vl ;
    char logTxt[100] ;
    uint8_t level ;
    
  public :
    cLog() { level = LOG_LEVEL_SILENT ; }
    void begin(uint8_t l) {level = l ;}
    void error(char* txt ...) {
        if (level >= LOG_LEVEL_ERROR) {
            va_start(vl,txt);
            int ret = vsprintf(logTxt, txt, vl);
            va_end(vl);
            Serial.println(logTxt);} }
    void warning(char* txt ...) {
        if (level >= LOG_LEVEL_WARNING) {
            va_start(vl,txt);
            int ret = vsprintf(logTxt, txt, vl);
            va_end(vl);
            Serial.println(logTxt);} }
    void info(char* txt ...) {
        if (level >= LOG_LEVEL_INFO) {
            va_start(vl,txt);
            int ret = vsprintf(logTxt, txt, vl);
            va_end(vl);
            Serial.println(logTxt);} }
    void debug(char* txt ...) {
        if (level >= LOG_LEVEL_DEBUG) {
            va_start(vl,txt);
            int ret = vsprintf(logTxt, txt, vl);
            va_end(vl);
            Serial.println(logTxt);} } } ;

cLog Log ;
#endif