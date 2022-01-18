#ifndef BH1750_H
#define BH1750_H

#include <Arduino.h>
#include <Wire.h>


/* ****************************************************************************************
 * 
 * Die Datei beruht auf Version 1.1.5 der Library BH1750WE.
 * (s. https://www.arduino.cc/reference/en/libraries/bh1750_we/)
 * 
 * Deren h- und cpp-Datei wurde einfach hintereinander in diese Datei kopiert.
 * An Ende befindet sich dann die eigen Klasse cBH1750 und die zugehörige Factory-Klasse.
 * Konstruktor und onTimeout der Klasse cBH1750 
 * 
 * ToDo: den ganzen Overhead ungenutzter Funktionen entfernen und die verbleibenden in die 
 * Klasse cBH1750 überführen. 
 * 
 * Das Modul BH1750 nutzt den i2c-Bus. Seine Adresse ist 0x23.
 * 
 * Es werden die Standard i2c-Leitungen genutzt.
 * Für den ESP8266 gilt: SDA ist GPIO 04 ,SCL ist GPIO 05.
 * Andere Prozessoren müssen noch getestet werden.
 * 
 **************************************************************************************** */
 

//######################################### BH1750.h #########################################


#define DATA_REG_RESET 0b00000111   
#define POWER_DOWN 0b00000000   
#define POWER_ON 0b00000001   

typedef enum BH1750Mode {
  CHM =     0b00010000,   //CHM: Continuously H-Resolution Mode
  CHM_2 =   0b00010001,   //CHM_2: Continuously H-Resolution Mode2
  CLM =     0b00010011,   //CLM: Continuously L-Resolution Mode
  OTH =     0b00100000,   //OTH: One Time H-Resolution Mode
  OTH_2 =   0b00100001,   //OTH_2: One Time H-Resolution Mode2
  OTL =     0b00100011    //OTL: One Time L-Resolution Mode
} mode;


class BH1750_WE{
    public:
        BH1750_WE(){};
        BH1750_WE(uint8_t addr);
        BH1750_WE(TwoWire *w, uint8_t addr);
        void init();
        void setI2C_Address(uint8_t addr);
        void setMeasuringTimeFactor(float f);
        void setMode(BH1750Mode d_mode);
        float getLux();
        void resetDataReg();
        void powerOn();
        void powerDown();
                    
    private:
        void writeBH1750_WE(uint8_t);
        uint16_t readBH1750_WE();
        TwoWire *_wire;
        float lux;
        int I2C_Address;
        float mtf;      //measuring time factor
        BH1750Mode deviceMode;
};

//######################################## BH1750.cpp ########################################

BH1750_WE::BH1750_WE(uint8_t addr){
    _wire = &Wire;
    setI2C_Address(addr);
}
    
BH1750_WE::BH1750_WE(TwoWire *w, uint8_t addr){
    _wire = w;
    setI2C_Address(addr);
}

void BH1750_WE::init(){
        setMode(CHM);
        setMeasuringTimeFactor(1.0);
}

void BH1750_WE::setMeasuringTimeFactor(float f){
    mtf = f;
    writeBH1750_WE(deviceMode);
    byte mt = round(mtf*69);
    byte highByteMT = ((mt>>5) | 0b01000000);
    byte lowByteMT = (mt & 0b01111111);
    lowByteMT |= 0b01100000;
    writeBH1750_WE(highByteMT);
    writeBH1750_WE(lowByteMT);
    delay(200);
}

void BH1750_WE::setMode(BH1750Mode d_mode){
    deviceMode = d_mode;
    writeBH1750_WE(deviceMode);
}

float BH1750_WE::getLux(){
    uint16_t rawLux;
    rawLux = readBH1750_WE();
    if((deviceMode==CHM_2)||(deviceMode==OTH_2)){
    lux = (rawLux/2.4)/mtf;     
  }
  else{
    lux = (rawLux/1.2)/mtf;
  }
  return lux;
    
}

void BH1750_WE::resetDataReg(){
    writeBH1750_WE(DATA_REG_RESET);
}

void BH1750_WE::powerOn(){
    writeBH1750_WE(POWER_ON);
}

void BH1750_WE::powerDown(){
    writeBH1750_WE(POWER_DOWN);
}

/************************************************ 
    private functions
*************************************************/


void BH1750_WE::setI2C_Address(uint8_t addr){
    I2C_Address = addr;
}

void BH1750_WE::writeBH1750_WE(uint8_t val){
    _wire->beginTransmission(I2C_Address);
    _wire->write(val);
    _wire->endTransmission();
}

uint16_t BH1750_WE::readBH1750_WE(){
    uint8_t MSbyte = 0, LSbyte = 0;
    _wire->requestFrom(I2C_Address, 2);
    if(_wire->available()){
        MSbyte=_wire->read();
        LSbyte=_wire->read(); 
    }
    return ((MSbyte<<8) + LSbyte);
}


//######################################### cBH1750 #########################################
#include "cBH1750.h"
#define BH1750_ADDRESS  0x23

class cBH1750 : public cIntervalSensor{ 
  private:
	BH1750_WE bh1750;
	cDevice lght ;
  public:
	cBH1750(){
		Wire.begin();
		bh1750.setI2C_Address(BH1750_ADDRESS);
		bh1750.init(); // sets default values: mode = CHM, measuring time factor = 1.0
  // 	bh1750.setMode(CLM);  // uncomment if you want to change default values
  // 	bh1750.setMeasuringTimeFactor(0.45); // uncomment for selection of value between 0.45 and 3.68
		serInterval(10) ;
	void measure() {
		lght.setValue(round( bh1750.getLux())); }

	cDevice * getLightSensor() {return &lght ; } };

//######################################### Factory #########################################

class cBH1750Factory : public cFactory {
  public:
	cBH1750Factory() {strcpy(name,"BH1750");}
	cBH1750* create() { return new cBH1750();}
	cBH1750* create(cb_function  f) {
		cBH1750* d = create();
		new cCallBackAdapter(f, d->getLightSensor()) ;
		return d ; }
	cBH1750* create(char*  n) {
		cBH1750* d = create();
		char txt[20] ;
		strcpy(txt, "lght");
		strcat(txt, n) ;
		new cMsgAdapter(new cValueTranslator(0),txt,d->getLightSensor()) ;
		return d ; } } ;

cBH1750Factory newBH1750;


#endif

