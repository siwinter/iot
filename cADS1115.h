#ifndef ADS1115_h
#define ADS1115_h


/* ****************************************************************************************
 * 
 * Der ADS115 ist ein 4-Kanal AnalogDigitalWandler.
 * Jeder einzelne Kanal wird als eigenständiges (Sensor-)Device betrachtet.
 * Die Datei beruht auf Version 0.3.4 der Library ADS1x15.
 * (s. https://www.arduino.cc/reference/en/libraries/ads1x15/)
 * 
 * Deren h- und cpp-Datei wurde einfach hintereinander in diese Datei kopiert.
 * An Ende befindet sich dann die eigen Klasse cADS1115 und die zugehörige Factory-Klasse.
 * Konstruktor und onTimeout der Klasse cADS1115 sind in Anlehnung an ein Beispiel entstanden. 
 * 
 * ToDo: den ganzen Overhead ungenutzter Funktionen entfernen und die verbleibenden in die 
 * neue Klasse überführen. 
 * 
 * Das Modul ADS1115 nutzt den i2c-Bus. Seine Adresse ist 0x48 
 * (kann durch Beschaltung des Adresspins geändert werden)
 * 
 * Es werden die Standard i2c-Leitungen genutzt.
 * Für den ESP8266 gilt: SDA ist GPIO 04 (D2) ,SCL ist GPIO 05 (D1).
 * Andere Prozessoren müssen noch getestet werden.
 * 
 **************************************************************************************** */
 

//####################################### ADS1x15.h #######################################


#include "Arduino.h"
#include "Wire.h"

#define ADS1X15_LIB_VERSION               (F("0.3.4"))

// allow compile time default address
// address in { 0x48, 0x49, 0x4A, 0x4B }, no test...
#ifndef ADS1015_ADDRESS
#define ADS1015_ADDRESS             0x48
#endif

#ifndef ADS1115_ADDRESS
#define ADS1115_ADDRESS             0x48
#endif


#define ADS1X15_OK                  0
#define ADS1X15_INVALID_VOLTAGE     -100
#define ADS1X15_INVALID_GAIN        0xFF
#define ADS1X15_INVALID_MODE        0xFE


class ADS1X15
{
public:
  void     reset();

#if defined (ESP8266) || defined(ESP32)
  bool     begin(uint8_t sda, uint8_t scl);
#endif
  bool     begin();
  bool     isConnected();

  //       GAIN
  // 0  =  ±6.144V  default
  // 1  =  ±4.096V
  // 2  =  ±2.048V
  // 4  =  ±1.024V
  // 8  =  ±0.512V
  // 16 =  ±0.256V
  void     setGain(uint8_t gain = 0);    // invalid values are mapped to 0 (default).
  uint8_t  getGain();                    // 0xFF == invalid gain error.

  // both may return ADS1X15_INVALID_VOLTAGE if the gain is invalid.
  float    toVoltage(int16_t value = 1); //  converts raw to voltage
  float    getMaxVoltage();              //  -100 == invalid voltage error

  // 0  =  CONTINUOUS
  // 1  =  SINGLE      default
  void     setMode(uint8_t mode = 1);    //  invalid values are mapped to 1 (default)
  uint8_t  getMode();                    //  0xFE == invalid mode error.

  // 0  =  slowest
  // 7  =  fastest
  // 4  =  default
  void     setDataRate(uint8_t dataRate = 4); // invalid values are mapped on 4 (default)
  uint8_t  getDataRate();                     // actual speed depends on device

  int16_t  readADC(uint8_t pin);
  int16_t  readADC_Differential_0_1();

  // used by continuous mode and async mode.
  int16_t  getLastValue() { return getValue(); };  // will be obsolete in the future 0.4.0
  int16_t  getValue();


  // ASYNC INTERFACE
  // requestADC(pin) -> isBusy() or isReady() -> getValue(); 
  // see examples
  void     requestADC(uint8_t pin);
  void     requestADC_Differential_0_1();
  bool     isBusy();
  bool     isReady();


  // COMPARATOR
  // 0    = TRADITIONAL   > high          => on      < low   => off
  // else = WINDOW        > high or < low => on      between => off
  void     setComparatorMode(uint8_t mode) { _compMode = mode == 0 ? 0 : 1; };
  uint8_t  getComparatorMode()             { return _compMode; };

  // 0    = LOW (default)
  // else = HIGH
  void     setComparatorPolarity(uint8_t pol) { _compPol = pol ? 0 : 1; };
  uint8_t  getComparatorPolarity()            { return _compPol; };

  // 0    = NON LATCH
  // else = LATCH
  void     setComparatorLatch(uint8_t latch) { _compLatch = latch ? 0 : 1; };
  uint8_t  getComparatorLatch()              { return _compLatch; };

  // 0   = trigger alert after 1 conversion
  // 1   = trigger alert after 2 conversions
  // 2   = trigger alert after 4 conversions
  // 3   = Disable comparator =  default, also for all other values.
  void     setComparatorQueConvert(uint8_t mode) { _compQueConvert = (mode < 3) ? mode : 3; };
  uint8_t  getComparatorQueConvert()             { return _compQueConvert; };

  void     setComparatorThresholdLow(int16_t lo);
  int16_t  getComparatorThresholdLow();
  void     setComparatorThresholdHigh(int16_t hi);
  int16_t  getComparatorThresholdHigh();


  int8_t   getError();

  void     setWireClock(uint32_t clockSpeed);
  // proto - getWireClock returns the value set by setWireClock not necessary the actual value
  uint32_t getWireClock();

protected:
  ADS1X15();

  // CONFIGURATION
  // BIT  DESCRIPTION
  // 0    # channels        0 == 1    1 == 4;
  // 1    0
  // 2    # resolution      0 == 12   1 == 16
  // 3    0
  // 4    has gain          0 = NO    1 = YES
  // 5    has comparator    0 = NO    1 = YES
  // 6    0
  // 7    0
  uint8_t  _config;
  uint8_t  _maxPorts;
  uint8_t  _address;
  uint8_t  _conversionDelay;
  uint8_t  _bitShift;
  uint16_t _gain;
  uint16_t _mode;
  uint16_t _datarate;

  // COMPARATOR variables
  // TODO merge these into one COMPARATOR MASK?  (low priority)
  //      would speed up code in _requestADC() and save 3 bytes RAM.
  // TODO boolean flags for first three, or make it mask value that 
  //      can be or-ed.   (low priority)
  uint8_t  _compMode;
  uint8_t  _compPol;
  uint8_t  _compLatch;
  uint8_t  _compQueConvert;

  int16_t  _readADC(uint16_t readmode);
  void     _requestADC(uint16_t readmode);
  bool     _writeRegister(uint8_t address, uint8_t reg, uint16_t value);
  uint16_t _readRegister(uint8_t address, uint8_t reg);
  int8_t   _err = ADS1X15_OK;

  TwoWire*  _wire;
  uint32_t  _clockSpeed = 0;
};

///////////////////////////////////////////////////////////////////////////
//
// Derived classes from ADS1X15
//
class ADS1013 : public ADS1X15
{
public:
  ADS1013(uint8_t Address = ADS1015_ADDRESS, TwoWire *wire = &Wire);
};

class ADS1014 : public ADS1X15
{
public:
  ADS1014(uint8_t Address = ADS1015_ADDRESS, TwoWire *wire = &Wire);
};

class ADS1015 : public ADS1X15
{
public:
  ADS1015(uint8_t Address = ADS1015_ADDRESS, TwoWire *wire = &Wire);
  int16_t  readADC_Differential_0_3();
  int16_t  readADC_Differential_1_3();
  int16_t  readADC_Differential_2_3();
  int16_t  readADC_Differential_0_2();  // not possible in async
  int16_t  readADC_Differential_1_2();  // not possible in async
  void     requestADC_Differential_0_3();
  void     requestADC_Differential_1_3();
  void     requestADC_Differential_2_3();
};

///////////////////////////////////////////////////////////////////////////
//
// Derived classes from ADS1X15
//
class ADS1113 : public ADS1X15
{
public:
  ADS1113(uint8_t address = ADS1115_ADDRESS, TwoWire *wire = &Wire);
};

class ADS1114 : public ADS1X15
{
public:
  ADS1114(uint8_t address = ADS1115_ADDRESS, TwoWire *wire = &Wire);
};

class ADS1115 : public ADS1X15
{
public:
  ADS1115(uint8_t address = ADS1115_ADDRESS, TwoWire *wire = &Wire);
  int16_t  readADC_Differential_0_3();
  int16_t  readADC_Differential_1_3();
  int16_t  readADC_Differential_2_3();
  int16_t  readADC_Differential_0_2();  // not possible in async
  int16_t  readADC_Differential_1_2();  // not possible in async
  void     requestADC_Differential_0_3();
  void     requestADC_Differential_1_3();
  void     requestADC_Differential_2_3();
};



//###################################### ADS1x15.cpp ######################################

#define ADS1015_CONVERSION_DELAY    1
#define ADS1115_CONVERSION_DELAY    8


// Kept #defines a bit in line with Adafruit library.

// REGISTERS
#define ADS1X15_REG_CONVERT         0x00
#define ADS1X15_REG_CONFIG          0x01
#define ADS1X15_REG_LOW_THRESHOLD   0x02
#define ADS1X15_REG_HIGH_THRESHOLD  0x03


// CONFIG REGISTER

// BIT 15       Operational Status         // 1 << 15
#define ADS1X15_OS_BUSY             0x0000
#define ADS1X15_OS_NOT_BUSY         0x8000
#define ADS1X15_OS_START_SINGLE     0x8000

// BIT 12-14    read differential
#define ADS1X15_MUX_DIFF_0_1        0x0000
#define ADS1X15_MUX_DIFF_0_3        0x1000
#define ADS1X15_MUX_DIFF_1_3        0x2000
#define ADS1X15_MUX_DIFF_2_3        0x3000
//              read single
#define ADS1X15_READ_0              0x4000  // pin << 12
#define ADS1X15_READ_1              0x5000  // pin = 0..3
#define ADS1X15_READ_2              0x6000
#define ADS1X15_READ_3              0x7000


// BIT 9-11     gain                        // (0..5) << 9
#define ADS1X15_PGA_6_144V          0x0000  // voltage
#define ADS1X15_PGA_4_096V          0x0200  //
#define ADS1X15_PGA_2_048V          0x0400  // default
#define ADS1X15_PGA_1_024V          0x0600
#define ADS1X15_PGA_0_512V          0x0800
#define ADS1X15_PGA_0_256V          0x0A00

// BIT 8        mode                        // 1 << 8
#define ADS1X15_MODE_CONTINUE       0x0000
#define ADS1X15_MODE_SINGLE         0x0100

// BIT 5-7      data rate sample per second  // (0..7) << 5
/*
differs for different devices, check datasheet or readme.md

| data rate | ADS101x | ADS 111x | Notes   |
|:---------:|--------:|---------:|:-------:|
|     0     |   128   |   8      | slowest |
|     1     |   250   |   16     |         |
|     2     |   490   |   32     |         |
|     3     |   920   |   64     |         |
|     4     |   1600  |   128    | default |
|     5     |   2400  |   250    |         |
|     6     |   3300  |   475    |         |
|     7     |   3300  |   860    | fastest |
*/

// BIT 4 comparator modi                    // 1 << 4
#define ADS1X15_COMP_MODE_TRADITIONAL   0x0000
#define ADS1X15_COMP_MODE_WINDOW        0x0010

// BIT 3 ALERT active value                 // 1 << 3
#define ADS1X15_COMP_POL_ACTIV_LOW      0x0000
#define ADS1X15_COMP_POL_ACTIV_HIGH     0x0008

// BIT 2 ALERT latching                     // 1 << 2
#define ADS1X15_COMP_NON_LATCH          0x0000
#define ADS1X15_COMP_LATCH              0x0004

// BIT 0-1 ALERT mode                       // (0..3)
#define ADS1X15_COMP_QUE_1_CONV         0x0000  // trigger alert after 1 convert
#define ADS1X15_COMP_QUE_2_CONV         0x0001  // trigger alert after 2 converts
#define ADS1X15_COMP_QUE_4_CONV         0x0002  // trigger alert after 4 converts
#define ADS1X15_COMP_QUE_NONE           0x0003  // disable comparator


// _CONFIG masks
//
// | bit  | description          |
// |:----:|:---------------------|
// |  0   | # channels           |
// |  1   | -                    |
// |  2   | resolution           |
// |  3   | -                    |
// |  4   | GAIN supported       |
// |  5   | COMPARATOR supported |
// |  6   | -                    |
// |  7   | -                    |
//
#define ADS_CONF_CHAN_1  0x00
#define ADS_CONF_CHAN_4  0x01
#define ADS_CONF_RES_12  0x00
#define ADS_CONF_RES_16  0x04
#define ADS_CONF_NOGAIN  0x00
#define ADS_CONF_GAIN    0x10
#define ADS_CONF_NOCOMP  0x00
#define ADS_CONF_COMP    0x20


//////////////////////////////////////////////////////
//
// BASE CONSTRUCTOR
//
ADS1X15::ADS1X15()
{
  reset();
}


//////////////////////////////////////////////////////
//
// PUBLIC
//
void ADS1X15::reset()
{
  setGain(0);      // _gain = ADS1X15_PGA_6_144V;
  setMode(1);      // _mode = ADS1X15_MODE_SINGLE;
  setDataRate(4);  // middle speed, depends on device.

  // COMPARATOR variables   # see notes .h
  _compMode       = 0;
  _compPol        = 1;
  _compLatch      = 0;
  _compQueConvert = 3;
}


#if defined (ESP8266) || defined(ESP32)
bool ADS1X15::begin(uint8_t sda, uint8_t scl)
{
  _wire = &Wire;
  _wire->begin(sda, scl);
  if ((_address < 0x48) || (_address > 0x4B)) return false;
  if (! isConnected()) return false;
  return true;
}
#endif


bool ADS1X15::begin()
{
  _wire->begin();
  if ((_address < 0x48) || (_address > 0x4B)) return false;
  if (! isConnected()) return false;
  return true;
}


bool ADS1X15::isBusy()
{
  return isReady() == false;
}


bool ADS1X15::isReady()
{
  uint16_t val = _readRegister(_address, ADS1X15_REG_CONFIG);
  return ((val & ADS1X15_OS_NOT_BUSY) > 0);
}


bool ADS1X15::isConnected()
{
  _wire->beginTransmission(_address);
  return (_wire->endTransmission() == 0);
}


void ADS1X15::setGain(uint8_t gain)
{
  if (!(_config & ADS_CONF_GAIN)) gain = 0;
  switch (gain)
  {
    default:  // catch invalid values and go for the safest gain.
    case 0:  _gain = ADS1X15_PGA_6_144V;  break;
    case 1:  _gain = ADS1X15_PGA_4_096V;  break;
    case 2:  _gain = ADS1X15_PGA_2_048V;  break;
    case 4:  _gain = ADS1X15_PGA_1_024V;  break;
    case 8:  _gain = ADS1X15_PGA_0_512V;  break;
    case 16: _gain = ADS1X15_PGA_0_256V;  break;
  }
}


uint8_t ADS1X15::getGain()
{
  if (!(_config & ADS_CONF_GAIN)) return 0;
  switch (_gain)
  {
    case ADS1X15_PGA_6_144V: return 0;
    case ADS1X15_PGA_4_096V: return 1;
    case ADS1X15_PGA_2_048V: return 2;
    case ADS1X15_PGA_1_024V: return 4;
    case ADS1X15_PGA_0_512V: return 8;
    case ADS1X15_PGA_0_256V: return 16;
  }
  _err = ADS1X15_INVALID_GAIN;
  return _err;
}


float ADS1X15::toVoltage(int16_t value)
{
  if (value == 0) return 0;

  float volts = getMaxVoltage();
  if (volts < 0) return volts;

  volts *= value;
  if (_config & ADS_CONF_RES_16)
  {
    volts /= 32767;  // value = 16 bits - sign bit = 15 bits mantissa
  }
  else
  {
    volts /= 2047;   // value = 12 bits - sign bit = 11 bit mantissa
  }
  return volts;
}


float ADS1X15::getMaxVoltage()
{
  switch (_gain)
  {
    case ADS1X15_PGA_6_144V: return 6.144;
    case ADS1X15_PGA_4_096V: return 4.096;
    case ADS1X15_PGA_2_048V: return 2.048;
    case ADS1X15_PGA_1_024V: return 1.024;
    case ADS1X15_PGA_0_512V: return 0.512;
    case ADS1X15_PGA_0_256V: return 0.256;
  }
  _err = ADS1X15_INVALID_VOLTAGE;
  return _err;
}


void ADS1X15::setMode(uint8_t mode)
{
  switch (mode)
  {
    case 0: _mode = ADS1X15_MODE_CONTINUE; break;
    default:
    case 1: _mode = ADS1X15_MODE_SINGLE;   break;
  }
}


uint8_t ADS1X15::getMode(void)
{
  switch (_mode)
  {
    case ADS1X15_MODE_CONTINUE: return 0;
    case ADS1X15_MODE_SINGLE:   return 1;
  }
  _err = ADS1X15_INVALID_MODE;
  return _err;
}


void ADS1X15::setDataRate(uint8_t dataRate)
{
  _datarate = dataRate;
  if (_datarate > 7) _datarate = 4;  // default
  _datarate <<= 5;      // convert 0..7 to mask needed.
}


uint8_t ADS1X15::getDataRate(void)
{
  return (_datarate >> 5) & 0x07;  // convert mask back to 0..7
}


int16_t ADS1X15::readADC(uint8_t pin)
{
  if (pin >= _maxPorts) return 0;
  uint16_t mode = ((4 + pin) << 12); // pin to mask
  return _readADC(mode);
}


void  ADS1X15::requestADC_Differential_0_1()
{
  _requestADC(ADS1X15_MUX_DIFF_0_1);
}


int16_t ADS1X15::readADC_Differential_0_1()
{
  return _readADC(ADS1X15_MUX_DIFF_0_1);
}


void ADS1X15::requestADC(uint8_t pin)
{
  if (pin >= _maxPorts) return;
  uint16_t mode = ((4 + pin) << 12);   // pin to mask
  _requestADC(mode);
}


int16_t ADS1X15::getValue()
{
  int16_t raw = _readRegister(_address, ADS1X15_REG_CONVERT);
  if (_bitShift) raw >>= _bitShift;  // Shift 12-bit results
  return raw;
}


void ADS1X15::setComparatorThresholdLow(int16_t lo)
{
  _writeRegister(_address, ADS1X15_REG_LOW_THRESHOLD, lo);
};


int16_t ADS1X15::getComparatorThresholdLow()
{
  return _readRegister(_address, ADS1X15_REG_LOW_THRESHOLD);
};


void ADS1X15::setComparatorThresholdHigh(int16_t hi)
{
  _writeRegister(_address, ADS1X15_REG_HIGH_THRESHOLD, hi);
};


int16_t ADS1X15::getComparatorThresholdHigh()
{
  return _readRegister(_address, ADS1X15_REG_HIGH_THRESHOLD);
};


int8_t ADS1X15::getError()
{
  int8_t rv = _err;
  _err = ADS1X15_OK;
  return rv;
}


void ADS1X15::setWireClock(uint32_t clockSpeed)
{
  _clockSpeed = clockSpeed;
  _wire->setClock(_clockSpeed);
}


// TODO: get the real clock speed from the I2C interface if possible.
// ESP ==> ??
uint32_t ADS1X15::getWireClock()
{
#if defined(__AVR__)
  uint32_t speed = F_CPU / ((TWBR * 2) + 16);
  return speed;

#elif defined(ESP32)
  return (uint32_t) _wire->getClock();

// #elif defined(ESP8266)
// core_esp8266_si2c.cpp holds the data see => void Twi::setClock(
// not supported.
// return -1;

#else  // best effort ...
  return _clockSpeed;
#endif
}


//////////////////////////////////////////////////////
//
// PROTECTED
//
int16_t ADS1X15::_readADC(uint16_t readmode)
{
  _requestADC(readmode);
  if (_mode == ADS1X15_MODE_SINGLE)
  {
    while ( isBusy() ) yield();   // wait for conversion; yield for ESP.
  }
  else
  {
    delay(_conversionDelay);      // TODO needed in continuous mode?
  }
  return getValue();
}


void ADS1X15::_requestADC(uint16_t readmode)
{
  // write to register is needed in continuous mode as other flags can be changed
  uint16_t config = ADS1X15_OS_START_SINGLE;  // bit 15     force wake up if needed
  config |= readmode;                         // bit 12-14
  config |= _gain;                            // bit 9-11
  config |= _mode;                            // bit 8
  config |= _datarate;                        // bit 5-7
  if (_compMode)  config |= ADS1X15_COMP_MODE_WINDOW;         // bit 4      comparator modi
  else            config |= ADS1X15_COMP_MODE_TRADITIONAL;
  if (_compPol)   config |= ADS1X15_COMP_POL_ACTIV_HIGH;      // bit 3      ALERT active value
  else            config |= ADS1X15_COMP_POL_ACTIV_LOW;
  if (_compLatch) config |= ADS1X15_COMP_LATCH;
  else            config |= ADS1X15_COMP_NON_LATCH;           // bit 2      ALERT latching
  config |= _compQueConvert;                                  // bit 0..1   ALERT mode
  _writeRegister(_address, ADS1X15_REG_CONFIG, config);
}


bool ADS1X15::_writeRegister(uint8_t address, uint8_t reg, uint16_t value)
{
  _wire->beginTransmission(address);
  _wire->write((uint8_t)reg);
  _wire->write((uint8_t)(value >> 8));
  _wire->write((uint8_t)(value & 0xFF));
  return (_wire->endTransmission() == 0);
}


uint16_t ADS1X15::_readRegister(uint8_t address, uint8_t reg)
{
  _wire->beginTransmission(address);
  _wire->write(reg);
  _wire->endTransmission();

  int rv = _wire->requestFrom(address, (uint8_t) 2);
  if (rv == 2)
  {
    uint16_t value = _wire->read() << 8;
    value += _wire->read();
    return value;
  }
  return 0x0000;
}


///////////////////////////////////////////////////////////////////////////
//
// ADS1013
//
ADS1013::ADS1013(uint8_t address, TwoWire *wire)
{
  _address = address;
  _wire = wire;
  _config = ADS_CONF_NOCOMP | ADS_CONF_NOGAIN | ADS_CONF_RES_12 | ADS_CONF_CHAN_1;
  _conversionDelay = ADS1015_CONVERSION_DELAY;
  _bitShift = 4;
  _maxPorts = 1;
}


///////////////////////////////////////////////////////////////////////////
//
// ADS1014
//
ADS1014::ADS1014(uint8_t address, TwoWire *wire)
{
  _address = address;
  _wire = wire;
  _config = ADS_CONF_COMP | ADS_CONF_GAIN | ADS_CONF_RES_12 | ADS_CONF_CHAN_1;
  _conversionDelay = ADS1015_CONVERSION_DELAY;
  _bitShift = 4;
  _maxPorts = 1;
}


///////////////////////////////////////////////////////////////////////////
//
// ADS1015
//
ADS1015::ADS1015(uint8_t address, TwoWire *wire)
{
  _address = address;
  _wire = wire;
  _config = ADS_CONF_COMP | ADS_CONF_GAIN | ADS_CONF_RES_12 | ADS_CONF_CHAN_4;
  _conversionDelay = ADS1015_CONVERSION_DELAY;
  _bitShift = 4;
  _maxPorts = 4;
}


int16_t ADS1015::readADC_Differential_0_3()
{
  return _readADC(ADS1X15_MUX_DIFF_0_3);
}


int16_t ADS1015::readADC_Differential_1_3()
{
  return _readADC(ADS1X15_MUX_DIFF_1_3);
}


int16_t ADS1015::readADC_Differential_2_3()
{
  return _readADC(ADS1X15_MUX_DIFF_2_3);
}


int16_t ADS1015::readADC_Differential_0_2()
{
  return readADC(2) - readADC(0);
}


int16_t ADS1015::readADC_Differential_1_2()
{
  return readADC(2) - readADC(1);;
}


void ADS1015::requestADC_Differential_0_3()
{
  _requestADC(ADS1X15_MUX_DIFF_0_3);
}


void ADS1015::requestADC_Differential_1_3()
{
  _requestADC(ADS1X15_MUX_DIFF_1_3);
}


void ADS1015::requestADC_Differential_2_3()
{
  _requestADC(ADS1X15_MUX_DIFF_2_3);
}


///////////////////////////////////////////////////////////////////////////
//
// ADS1113
//
ADS1113::ADS1113(uint8_t address, TwoWire *wire)
{
  _address = address;
  _wire = wire;
  _config = ADS_CONF_NOCOMP | ADS_CONF_NOGAIN | ADS_CONF_RES_16 | ADS_CONF_CHAN_1;
  _conversionDelay = ADS1115_CONVERSION_DELAY;
  _bitShift = 0;
  _maxPorts = 1;
}


///////////////////////////////////////////////////////////////////////////
//
// ADS1114
//
ADS1114::ADS1114(uint8_t address, TwoWire *wire)
{
  _address = address;
  _wire = wire;
  _config = ADS_CONF_COMP | ADS_CONF_GAIN | ADS_CONF_RES_16 | ADS_CONF_CHAN_1;
  _conversionDelay = ADS1115_CONVERSION_DELAY;
  _bitShift = 0;
  _maxPorts = 1;
}


///////////////////////////////////////////////////////////////////////////
//
// ADS1115
//
ADS1115::ADS1115(uint8_t address, TwoWire *wire)
{
  _address = address;
  _wire = wire;
  _config = ADS_CONF_COMP | ADS_CONF_GAIN | ADS_CONF_RES_16 | ADS_CONF_CHAN_4;
  _conversionDelay = ADS1115_CONVERSION_DELAY;
  _bitShift = 0;
  _maxPorts = 4;
}


int16_t ADS1115::readADC_Differential_0_3()
{
  return _readADC(ADS1X15_MUX_DIFF_0_3);
}


int16_t ADS1115::readADC_Differential_1_3()
{
  return _readADC(ADS1X15_MUX_DIFF_1_3);
}


int16_t ADS1115::readADC_Differential_2_3()
{
  return _readADC(ADS1X15_MUX_DIFF_2_3);
}


int16_t ADS1115::readADC_Differential_0_2()
{
  return readADC(2) - readADC(0);
}


int16_t ADS1115::readADC_Differential_1_2()
{
  return readADC(2) - readADC(1);;
}


void ADS1115::requestADC_Differential_0_3()
{
  _requestADC(ADS1X15_MUX_DIFF_0_3);
}


void ADS1115::requestADC_Differential_1_3()
{
  _requestADC(ADS1X15_MUX_DIFF_1_3);
}


void ADS1115::requestADC_Differential_2_3()
{
  _requestADC(ADS1X15_MUX_DIFF_2_3);
}

//####################################### cADS1115.h ########################################

#include "cDevice.h"

class cADS1115 : public cIntervalSensor, public cLooper {
  private :
	ADS1115* ads ;
	int nbrOfChannels;
	cDevice sensors[4] ;
	uint16_t values[4] ;
	int channel ;          //0..3
	int interval ;
	int accuracy ;
	bool isIdle ;

  public :
	cADS1115(int s, int i) {
		ads = new ADS1115(ADS1115_ADDRESS) ;
		if(!ads->begin()) Serial.println("ADS1115 not connected");
		if(s<=4) nbrOfChannels = s; else nbrOfChannels = 4 ;
		for (int i=0 ; i<nbrOfChannels ; i++) values[i] = 0 ;
		ads->setGain(0);  // 6.144 volt notwendige Einstellung unklar
		setInterval(i) ;
		isIdle = true;
		channel = 0 ;
		accuracy = 500 ;
		setInterval(i) ;
		if ( interval == 0) {
			isIdle = false ;
			ads->requestADC(channel) ; }
		else setTimer(interval) ; }
		
	void onLoop() {
		if (isIdle) return ;
		if ( ads->isBusy() ) return ;
		uint16_t v = ads->getValue() ;
		
		if ((interval != 0) || ( abs(values[channel] - v) > accuracy )) {
			values[channel] = v ;
			sensors[channel].setValue(ads->toVoltage(v)*100) ; }
		channel = channel + 1 ;
		if (channel >= 4) channel = 0 ;
		if ( interval == 0 ) ads->requestADC(channel) ;
		else isIdle = true ; }
		
	void measure() {
		setTimer(interval);
		ads->requestADC(channel) ;   // 0..3
		isIdle = false ; }
				
	cDevice * getSensor(int i) { if (i<nbrOfChannels) return &sensors[i]; else return NULL; }
	
	void setInterval(int i) {interval = i*60/nbrOfChannels;}
	void setAccuracy(int a) {accuracy = a;} } ;

class cADS1115Factory : public cFactory {
  public:
	cADS1115Factory() {strcpy(name,"ADS1115");}
	cADS1115* create(int s, int i) { return new cADS1115(s, i);}
	cADS1115* create(int s) { return new cADS1115(s, 0);}
	cADS1115* create(int s, int i, cb_function  f1, cb_function  f2=NULL, cb_function  f3=NULL, cb_function  f4=NULL) {
		cADS1115* d = create(s, i);
		if ( f1!=NULL ) new cCallBackAdapter(f1, d->getSensor(0)) ;
		if ( f2!=NULL ) new cCallBackAdapter(f2, d->getSensor(1)) ;
		if ( f3!=NULL ) new cCallBackAdapter(f3, d->getSensor(2)) ;
		if ( f4!=NULL ) new cCallBackAdapter(f4, d->getSensor(3)) ;
		return d ; }	
	cADS1115* create(int s, cb_function  f1, cb_function  f2=NULL, cb_function  f3=NULL, cb_function  f4=NULL) {
		cADS1115* d = create(s, 0);
		if ( f1!=NULL ) new cCallBackAdapter(f1, d->getSensor(0)) ;
		if ( f2!=NULL ) new cCallBackAdapter(f2, d->getSensor(1)) ;
		if ( f3!=NULL ) new cCallBackAdapter(f3, d->getSensor(2)) ;
		if ( f4!=NULL ) new cCallBackAdapter(f4, d->getSensor(3)) ;
		return d ; }
	cADS1115* create(int s, char*  n) {
		cADS1115* d = create(s, 0);
		char txt[20] ;
		strcpy(txt, "sensor1");
		strcat(txt, n) ;
		new cMsgAdapter(new cValueTranslator(2),txt,d->getSensor(0)) ;
		strcpy(txt, "sensor2");
		strcat(txt, n) ;
		new cMsgAdapter(new cValueTranslator(2),txt,d->getSensor(1)) ;
		strcpy(txt, "sensor3");
		strcat(txt, n) ;
		new cMsgAdapter(new cValueTranslator(2),txt,d->getSensor(2)) ;
		strcpy(txt, "sensor4");
		strcat(txt, n) ;
		new cMsgAdapter(new cValueTranslator(2),txt,d->getSensor(3)) ;
		return d ; }
	cADS1115* create(int s, int i, char*  n) {
		cADS1115* d = create(s, i);
		char txt[20] ;
		strcpy(txt, "_1");
		strcat(txt, n) ;
		new cMsgAdapter(new cValueTranslator(2),txt,d->getSensor(0)) ;
		strcpy(txt, "_2");
		strcat(txt, n) ;
		new cMsgAdapter(new cValueTranslator(2),txt,d->getSensor(1)) ;
		strcpy(txt, "_3");
		strcat(txt, n) ;
		new cMsgAdapter(new cValueTranslator(2),txt,d->getSensor(2)) ;
		strcpy(txt, "_4");
		strcat(txt, n) ;
		new cMsgAdapter(new cValueTranslator(2),txt,d->getSensor(3)) ;
		return d ; }
		 } ;

cADS1115Factory newADS1115 ;

#endif
