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
    void error(const char* txt ...) {
        if (level >= LOG_LEVEL_ERROR) {
            va_start(vl,txt);
            vsprintf(logTxt, txt, vl);
            va_end(vl);
            Serial.println(logTxt);} }
    void warning(const char* txt ...) {
        if (level >= LOG_LEVEL_WARNING) {
            va_start(vl,txt);
            vsprintf(logTxt, txt, vl);
            va_end(vl);
            Serial.println(logTxt);} }
    void info(const char* txt ...) {
        if (level >= LOG_LEVEL_INFO) {
            va_start(vl,txt);
            vsprintf(logTxt, txt, vl);
            va_end(vl);
            Serial.println(logTxt);} }
    void debug(const char* txt ...) {
        if (level >= LOG_LEVEL_DEBUG) {
            va_start(vl,txt);
            vsprintf(logTxt, txt, vl);
            va_end(vl);
            Serial.println(logTxt);} }
    void trace(const char* txt ...) {
        va_start(vl,txt);
        vsprintf(logTxt, txt, vl);
        va_end(vl);
        Serial.println(logTxt); } } ;

cLog Log ;
#endif