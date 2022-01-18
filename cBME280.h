/*  Basiert auf Version 3.0.0 der BME280 sensor library for ESPx (.s https://www.arduino.cc/reference/en/libraries/bme280/)
 *  sollte zu allen Prozessoren kompatibel sein 
 *   nutzt I2C nicht SPI 
 * 
 *   Vin (Voltage In)    ->  3.3V
 *   Gnd (Ground)        ->  Gnd
 *   SDA (Serial Data)   ->  A4 on Uno/Pro-Mini, 20 on Mega2560/Due, 2 Leonardo/Pro-Micro
 *   SCK (Serial Clock)  ->  A5 on Uno/Pro-Mini, 21 on Mega2560/Due, 3 Leonardo/Pro-Micro
 * 
 * 
 * */
#ifndef BME280_h
#define BME280_h
#include <Wire.h>
class cBME280Sensor : public cIntervalSensor {
public:

/*****************************************************************/
/* ENUMERATIONS                                                  */
/*****************************************************************/

   enum OSR
   {
      OSR_Off =  0,
      OSR_X1  =  1,
      OSR_X2  =  2,
      OSR_X4  =  3,
      OSR_X8  =  4,
      OSR_X16 =  5
   };

   enum Mode
   {
      Mode_Sleep  = 0,
      Mode_Forced = 1,
      Mode_Normal = 3
   };

   enum StandbyTime
   {
      StandbyTime_500us   = 0,
      StandbyTime_62500us = 1,
      StandbyTime_125ms   = 2,
      StandbyTime_250ms   = 3,
      StandbyTime_50ms    = 4,
      StandbyTime_1000ms  = 5,
      StandbyTime_10ms    = 6,
      StandbyTime_20ms    = 7
   };

   enum Filter
   {
      Filter_Off = 0,
      Filter_2   = 1,
      Filter_4   = 2,
      Filter_8   = 3,
      Filter_16  = 4
   };

   enum SpiEnable
   {
      SpiEnable_False = 0,
      SpiEnable_True = 1
   };

   enum ChipModel
   {
      ChipModel_UNKNOWN = 0,
      ChipModel_BMP280 = 0x58,
      ChipModel_BME280 = 0x60
   };
   
   enum I2CAddr
   {
      I2CAddr_0x76 = 0x76,
      I2CAddr_0x77 = 0x77
   };


/*****************************************************************/
/* STRUCTURES                                                  */
/*****************************************************************/
	struct Settings {
		
		OSR tempOSR;
		OSR humOSR;
		OSR presOSR;
		Mode mode;
		StandbyTime standbyTime;
		Filter filter;
		I2CAddr bme280Addr;
      SpiEnable spiEnable;
	};

/*****************************************************************/
/* CONSTANTS                                                     */
/*****************************************************************/

   static const uint8_t CTRL_HUM_ADDR   = 0xF2;
   static const uint8_t CTRL_MEAS_ADDR  = 0xF4;
   static const uint8_t CONFIG_ADDR     = 0xF5;
   static const uint8_t PRESS_ADDR      = 0xF7;
   static const uint8_t TEMP_ADDR       = 0xFA;
   static const uint8_t HUM_ADDR        = 0xFD;
   static const uint8_t TEMP_DIG_ADDR   = 0x88;
   static const uint8_t PRESS_DIG_ADDR  = 0x8E;
   static const uint8_t HUM_DIG_ADDR1   = 0xA1;
   static const uint8_t HUM_DIG_ADDR2   = 0xE1;
   static const uint8_t ID_ADDR         = 0xD0;

   static const uint8_t TEMP_DIG_LENGTH         = 6;
   static const uint8_t PRESS_DIG_LENGTH        = 18;
   static const uint8_t HUM_DIG_ADDR1_LENGTH    = 1;
   static const uint8_t HUM_DIG_ADDR2_LENGTH    = 7;
   static const uint8_t DIG_LENGTH              = 32;
   static const uint8_t SENSOR_DATA_LENGTH      = 8;
	
/*****************************************************************/
/* VARIABLES                                                     */
/*****************************************************************/
	
	uint8_t pin ;
	cDevice temp ;
	cDevice humid ;
	cDevice press ;
	
	Settings m_settings;

	uint8_t m_dig[32];
	ChipModel m_chip_model;

	bool m_initialized;

   
	bool WriteRegister (  uint8_t addr,  uint8_t data ) {
		Wire.beginTransmission(m_settings.bme280Addr);
		Wire.write(addr);
		Wire.write(data);
		Wire.endTransmission();
		return true; } // TODO: Check return values from wire calls. 

	bool ReadRegister (  uint8_t addr,  uint8_t data[],  uint8_t length) {
		uint8_t ord(0);
		Wire.beginTransmission(m_settings.bme280Addr);
		Wire.write(addr);
		Wire.endTransmission();
		Wire.requestFrom(static_cast<uint8_t>(m_settings.bme280Addr), length);
		while(Wire.available())  {    data[ord++] = Wire.read();  }
		return ord == length; }
		
	bool ReadChipID() {
		uint8_t id[1];
		ReadRegister(ID_ADDR, &id[0], 1);
		switch(id[0]){
		  case ChipModel_BME280:
			m_chip_model = ChipModel_BME280;
			break;
		  case ChipModel_BMP280:
			m_chip_model = ChipModel_BMP280;
			break;
		  default:
			m_chip_model = ChipModel_UNKNOWN;
			return false;}
		return true; }
		
	bool ReadTrim() {
		uint8_t ord(0);
		bool success = true;
		// Temp. Dig
		success &= ReadRegister(TEMP_DIG_ADDR, &m_dig[ord], TEMP_DIG_LENGTH);
		ord += TEMP_DIG_LENGTH;
		// Pressure Dig
		success &= ReadRegister(PRESS_DIG_ADDR, &m_dig[ord], PRESS_DIG_LENGTH);
		ord += PRESS_DIG_LENGTH;
		// Humidity Dig 1
		success &= ReadRegister(HUM_DIG_ADDR1, &m_dig[ord], HUM_DIG_ADDR1_LENGTH);
		ord += HUM_DIG_ADDR1_LENGTH;
		// Humidity Dig 2
		success &= ReadRegister(HUM_DIG_ADDR2, &m_dig[ord], HUM_DIG_ADDR2_LENGTH);
		ord += HUM_DIG_ADDR2_LENGTH;
#ifdef DEBUG_ON
		Serial.print("Dig: ");
		for(int i = 0; i < 32; ++i)   {
			Serial.print(m_dig[i], HEX);
			Serial.print(" "); }
		Serial.println();
#endif
		return success && ord == DIG_LENGTH; }

	void CalculateRegisters ( uint8_t& ctrlHum, uint8_t& ctrlMeas, uint8_t& config ) {
		// ctrl_hum register. (ctrl_hum[2:0] = Humidity oversampling rate.)
		ctrlHum = (uint8_t)m_settings.humOSR;
		// ctrl_meas register. (ctrl_meas[7:5] = temperature oversampling rate, ctrl_meas[4:2] = pressure oversampling rate, ctrl_meas[1:0] = mode.)
		ctrlMeas = ((uint8_t)m_settings.tempOSR << 5) | ((uint8_t)m_settings.presOSR << 2) | (uint8_t)m_settings.mode;
		// config register. (config[7:5] = standby time, config[4:2] = filter, ctrl_meas[0] = spi enable.)
		config = ((uint8_t)m_settings.standbyTime << 5) | ((uint8_t)m_settings.filter << 2) | (uint8_t)m_settings.spiEnable; }

	void WriteSettings() {
		uint8_t ctrlHum, ctrlMeas, config;
		CalculateRegisters(ctrlHum, ctrlMeas, config);
		WriteRegister(CTRL_HUM_ADDR, ctrlHum);
		WriteRegister(CTRL_MEAS_ADDR, ctrlMeas);
		WriteRegister(CONFIG_ADDR, config); }
		
	void InitializeFilter() {
		// Force an unfiltered measurement to populate the filter buffer.
		// This fixes a bug that causes the first read to always be 28.82 °C 81732.34 hPa.
		Filter filter = m_settings.filter;
		m_settings.filter = Filter_Off;

		WriteSettings();

		float dummy;
		read(dummy, dummy, dummy);

		m_settings.filter = filter; }

	bool ReadData ( int32_t data[SENSOR_DATA_LENGTH] ) {
		bool success;
		uint8_t buffer[SENSOR_DATA_LENGTH];
		// For forced mode we need to write the mode to BME280 register before reading
		if (m_settings.mode == Mode_Forced) { WriteSettings(); }
		// Registers are in order. So we can start at the pressure register and read 8 bytes.
		success = ReadRegister(PRESS_ADDR, buffer, SENSOR_DATA_LENGTH);
		for(int i = 0; i < SENSOR_DATA_LENGTH; ++i) { data[i] = static_cast<int32_t>(buffer[i]); }
#ifdef DEBUG_ON
		Serial.print("Data: ");
		for(int i = 0; i < 8; ++i) {
			Serial.print(data[i], HEX);
			Serial.print(" "); }
		Serial.println();
#endif
		return success; }


	float CalculateTemperature ( int32_t raw, int32_t& t_fine ) {
		// Code based on calibration algorthim provided by Bosch.
		int32_t var1, var2, final;
		uint16_t dig_T1 = (m_dig[1] << 8) | m_dig[0];
		int16_t   dig_T2 = (m_dig[3] << 8) | m_dig[2];
		int16_t   dig_T3 = (m_dig[5] << 8) | m_dig[4];
		var1 = ((((raw >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
		var2 = (((((raw >> 4) - ((int32_t)dig_T1)) * ((raw >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
		t_fine = var1 + var2;
		final = (t_fine * 5 + 128) >> 8;
		return final/100.0 ; }
		
	float CalculateHumidity ( int32_t raw, int32_t t_fine ) {
		// Code based on calibration algorthim provided by Bosch.
		int32_t var1;
		uint8_t   dig_H1 =   m_dig[24];
		int16_t dig_H2 = (m_dig[26] << 8) | m_dig[25];
		uint8_t   dig_H3 =   m_dig[27];
		int16_t dig_H4 = (m_dig[28] << 4) | (0x0F & m_dig[29]);
		int16_t dig_H5 = (m_dig[30] << 4) | ((m_dig[29] >> 4) & 0x0F);
		int8_t   dig_H6 =   m_dig[31];

		var1 = (t_fine - ((int32_t)76800));
		var1 = (((((raw << 14) - (((int32_t)dig_H4) << 20) - (((int32_t)dig_H5) * var1)) +
		((int32_t)16384)) >> 15) * (((((((var1 * ((int32_t)dig_H6)) >> 10) * (((var1 *
		((int32_t)dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) *
		((int32_t)dig_H2) + 8192) >> 14));
		var1 = (var1 - (((((var1 >> 15) * (var1 >> 15)) >> 7) * ((int32_t)dig_H1)) >> 4));
		var1 = (var1 < 0 ? 0 : var1);
		var1 = (var1 > 419430400 ? 419430400 : var1);
		return ((uint32_t)(var1 >> 12))/1024.0; }
		
	float CalculatePressure ( int32_t raw, int32_t t_fine ) {
		// Code based on calibration algorthim provided by Bosch.
		int64_t var1, var2, pressure;
		float final;

		uint16_t dig_P1 = (m_dig[7]   << 8) | m_dig[6];
		int16_t   dig_P2 = (m_dig[9]   << 8) | m_dig[8];
		int16_t   dig_P3 = (m_dig[11] << 8) | m_dig[10];
		int16_t   dig_P4 = (m_dig[13] << 8) | m_dig[12];
		int16_t   dig_P5 = (m_dig[15] << 8) | m_dig[14];
		int16_t   dig_P6 = (m_dig[17] << 8) | m_dig[16];
		int16_t   dig_P7 = (m_dig[19] << 8) | m_dig[18];
		int16_t   dig_P8 = (m_dig[21] << 8) | m_dig[20];
		int16_t   dig_P9 = (m_dig[23] << 8) | m_dig[22];

		var1 = (int64_t)t_fine - 128000;
		var2 = var1 * var1 * (int64_t)dig_P6;
		var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
		var2 = var2 + (((int64_t)dig_P4) << 35);
		var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
		var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;
		if (var1 == 0) { return NAN; }                                                         // Don't divide by zero.
		pressure   = 1048576 - raw;
		pressure = (((pressure << 31) - var2) * 3125)/var1;
		var1 = (((int64_t)dig_P9) * (pressure >> 13) * (pressure >> 13)) >> 25;
		var2 = (((int64_t)dig_P8) * pressure) >> 19;
		pressure = ((pressure + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);

		final = ((uint32_t)pressure)/256.0;
		return final; }

	void read ( float& pressure, float& temp, float& humidity ) {
		int32_t data[8];
		int32_t t_fine;
		if(!ReadData(data)){
			pressure = temp = humidity = NAN;
			return; }
		uint32_t rawPressure = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);
		uint32_t rawTemp = (data[3] << 12) | (data[4] << 4) | (data[5] >> 4);
		uint32_t rawHumidity = (data[6] << 8) | data[7];
		temp = CalculateTemperature(rawTemp, t_fine );
		pressure = CalculatePressure(rawPressure, t_fine );
		humidity = CalculateHumidity(rawHumidity, t_fine); }

  public:
	cBME280Sensor() {
		m_settings.tempOSR = OSR_X1 ;
		m_settings.humOSR  = OSR_X1 ;
		m_settings.presOSR = OSR_X1 ;
		m_settings.mode    = Mode_Forced ;
		m_settings.standbyTime = StandbyTime_1000ms ;
		m_settings.filter  = Filter_Off ;
		m_settings.bme280Addr = I2CAddr_0x76 ;
		Wire.begin();
		setInterval(10);
	}
	bool begin() {
		bool success = ReadChipID();
		if(success) {
			success &= ReadTrim();
			if(m_settings.filter != Filter_Off) { InitializeFilter(); }
			WriteSettings(); }
		m_initialized = success;
		return m_initialized; }
		
	void measure() {
		float temp(NAN), hum(NAN), pres(NAN);		
		read(pres, temp, hum); }
	cDevice * getTemperatureSensor() {return &temp ; }
	cDevice * getHumiditySensor() {return &humid ; }
	cDevice * getPressureSensor() {return &press ; }
} ;

class cBME280Factory : public cFactory {
  public:
	cBME280Factory() {strcpy(name,"BME280");}
	cBME280Sensor* create() { return new cBME280Sensor();}
	cBME280Sensor* create(cb_function fTmp, cb_function fHum, cb_function fPrs) {
		cBME280Sensor* d = create();
		d->getTemperatureSensor()->addObserver(new cCallBackAdapter(fTmp, d->getTemperatureSensor())) ;
		d->getHumiditySensor()->addObserver(new cCallBackAdapter(fHum, d->getHumiditySensor())) ;
		d->getPressureSensor()->addObserver(new cCallBackAdapter(fPrs, d->getPressureSensor())) ;
		return d ; }
	cBME280Sensor* create(char*  n) {
		cBME280Sensor* d = create();
		char txt[20] ;
		strcpy(txt, "tmp");
		strcat(txt, n) ;
		new cMsgAdapter(new cValueTranslator(1),txt,d->getTemperatureSensor()) ;
		strcpy(txt, "hum");
		strcat(txt, n) ;
		new cMsgAdapter(new cValueTranslator(1),txt,d->getHumiditySensor()) ;
		strcpy(txt, "press");
		strcat(txt, n) ;
		new cMsgAdapter(new cValueTranslator(1),txt,d->getPressureSensor()) ;
		return d ; } } ;

cBME280Factory newBME280 ;


#endif
