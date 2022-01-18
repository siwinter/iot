/*  Basiert auf Version 1.8 der DHT sensor library for ESPx (.s https://www.arduino.cc/reference/en/libraries/dht-sensor-library-for-espx/)
 * kompatibel zu ATMEL, ESP8266, ESP32 */
 
#ifndef DHT22_h
#define DHT22_h

//######################################## DHTesp.h ########################################

// Reference: http://epb.apogee.net/res/refcomf.asp (References invalid)
enum ComfortState {
  Comfort_OK = 0,
  Comfort_TooHot = 1,
  Comfort_TooCold = 2,
  Comfort_TooDry = 4,
  Comfort_TooHumid = 8,
  Comfort_HotAndHumid = 9,
  Comfort_HotAndDry = 5,
  Comfort_ColdAndHumid = 10,
  Comfort_ColdAndDry = 6
};

// References https://en.wikipedia.org/wiki/Dew_point ==> Relationship to human comfort
enum PerceptionState {
  Perception_Dry = 0,
  Perception_VeryComfy = 1,
  Perception_Comfy = 2,
  Perception_Ok = 3,
  Perception_UnComfy = 4,
  Perception_QuiteUnComfy = 5,
  Perception_VeryUnComfy = 6,
  Perception_SevereUncomfy = 7
};

struct TempAndHumidity {
  float temperature;
  float humidity;
};

struct ComfortProfile
{
  //Represent the 4 line equations:
  //dry, humid, hot, cold, using the y = mx + b formula
  float m_tooHot_m, m_tooHot_b;
  float m_tooCold_m, m_tooHCold_b;
  float m_tooDry_m, m_tooDry_b;
  float m_tooHumid_m, m_tooHumid_b;

  inline bool isTooHot(float temp, float humidity) {return (temp > (humidity * m_tooHot_m + m_tooHot_b));}
  inline bool isTooHumid(float temp, float humidity) {return (temp > (humidity * m_tooHumid_m + m_tooHumid_b));}
  inline bool isTooCold(float temp, float humidity) {return (temp < (humidity * m_tooCold_m + m_tooHCold_b));}
  inline bool isTooDry(float temp, float humidity) {return (temp < (humidity * m_tooDry_m + m_tooDry_b));}

  inline float distanceTooHot(float temp, float humidity) {return temp - (humidity * m_tooHot_m + m_tooHot_b);}
  inline float distanceTooHumid(float temp, float humidity) {return temp - (humidity * m_tooHumid_m + m_tooHumid_b);}
  inline float distanceTooCold(float temp, float humidity) {return (humidity * m_tooCold_m + m_tooHCold_b) - temp;}
  inline float distanceTooDry(float temp, float humidity) {return (humidity * m_tooDry_m + m_tooDry_b) - temp;}
};

class DHTesp
{
public:

  typedef enum {
    AUTO_DETECT,
    DHT11,
    DHT22,
    AM2302,  // Packaged DHT22
    RHT03    // Equivalent to DHT22
  }
  DHT_MODEL_t;

  typedef enum {
    ERROR_NONE = 0,
    ERROR_TIMEOUT,
    ERROR_CHECKSUM
  }
  DHT_ERROR_t;

  TempAndHumidity values;

  // setup(dhtPin) is deprecated, auto detection is not working well on ESP32. Use setup(dhtPin, DHTesp::DHT11) instead!
  void setup(uint8_t dhtPin) __attribute__((deprecated));
  void setup(uint8_t pin, DHT_MODEL_t model=AUTO_DETECT);
  void resetTimer();

  float getTemperature();
  float getHumidity();
  TempAndHumidity getTempAndHumidity();

  DHT_ERROR_t getStatus() { return error; };
  const char* getStatusString();

  DHT_MODEL_t getModel() { return model; }

  int getMinimumSamplingPeriod() { return model == DHT11 ? 1000 : 2000; }

  int8_t getNumberOfDecimalsTemperature() { return model == DHT11 ? 0 : 1; };
  int8_t getLowerBoundTemperature() { return model == DHT11 ? 0 : -40; };
  int8_t getUpperBoundTemperature() { return model == DHT11 ? 50 : 125; };

  int8_t getNumberOfDecimalsHumidity() { return 0; };
  int8_t getLowerBoundHumidity() { return model == DHT11 ? 20 : 0; };
  int8_t getUpperBoundHumidity() { return model == DHT11 ? 90 : 100; };

  static float toFahrenheit(float fromCelcius) { return 1.8 * fromCelcius + 32.0; };
  static float toCelsius(float fromFahrenheit) { return (fromFahrenheit - 32.0) / 1.8; };

  float computeHeatIndex(float temperature, float percentHumidity, bool isFahrenheit=false);
  float computeDewPoint(float temperature, float percentHumidity, bool isFahrenheit=false);
  float getComfortRatio(ComfortState& destComfStatus, float temperature, float percentHumidity, bool isFahrenheit=false);
  ComfortProfile getComfortProfile() {return m_comfort;}
  void setComfortProfile(ComfortProfile& c) {m_comfort = c;}
  inline bool isTooHot(float temp, float humidity) {return m_comfort.isTooHot(temp, humidity);}
  inline bool isTooHumid(float temp, float humidity) {return m_comfort.isTooHumid(temp, humidity);}
  inline bool isTooCold(float temp, float humidity) {return m_comfort.isTooCold(temp, humidity);}
  inline bool isTooDry(float temp, float humidity) {return m_comfort.isTooDry(temp, humidity);}
  byte computePerception(float temperature, float percentHumidity, bool isFahrenheit=false);
  float computeAbsoluteHumidity(float temperature, float percentHumidity, bool isFahrenheit=false);
  uint8_t getPin() { return pin; }
protected:
  void readSensor();

  float temperature;
  float humidity;

  uint8_t pin;

private:
  DHT_MODEL_t model;
  DHT_ERROR_t error;
  unsigned long lastReadTime;
  ComfortProfile m_comfort;
};

//####################################### DHTesp.cpp #######################################


void DHTesp::setup(uint8_t pin, DHT_MODEL_t model)
{
	DHTesp::pin = pin;
	DHTesp::model = model;
	DHTesp::resetTimer(); // Make sure we do read the sensor in the next readSensor()

	if (model == AUTO_DETECT)
	{
		DHTesp::model = DHT22;
		readSensor();
		if (error == ERROR_TIMEOUT)
		{
			DHTesp::model = DHT11;
			// Warning: in case we auto detect a DHT11, you should wait at least 1000 msec
			// before your first read request. Otherwise you will get a time out error.
		}
	}

	//Set default comfort profile.

	//In computing these constants the following reference was used
	//http://epb.apogee.net/res/refcomf.asp
	//It was simplified as 4 straight lines and added very little skew on
	//the vertical lines (+0.1 on x for C,D)
	//The for points used are(from top left, clock wise)
	//A(30%, 30*C) B(70%, 26.2*C) C(70.1%, 20.55*C) D(30.1%, 22.22*C)
	//On the X axis we have the rel humidity in % and on the Y axis the temperature in *C

	//Too hot line AB
	m_comfort.m_tooHot_m = -0.095;
	m_comfort.m_tooHot_b = 32.85;
	//Too humid line BC
	m_comfort.m_tooHumid_m = -56.5;
	m_comfort.m_tooHumid_b = 3981.2;
	//Too cold line DC
	m_comfort.m_tooCold_m = -0.04175;
	m_comfort.m_tooHCold_b = 23.476675;
	//Too dry line AD
	m_comfort.m_tooDry_m = -77.8;
	m_comfort.m_tooDry_b = 2364;
}

void DHTesp::resetTimer()
{
	DHTesp::lastReadTime = millis() - 3000;
}

float DHTesp::getHumidity()
{
	readSensor();
	if (error == ERROR_TIMEOUT)
	{ // Try a second time to read
		readSensor();
	}
	return humidity;
}

float DHTesp::getTemperature()
{
	readSensor();
	if (error == ERROR_TIMEOUT)
	{ // Try a second time to read
		readSensor();
	}
	return temperature;
}

TempAndHumidity DHTesp::getTempAndHumidity()
{
	readSensor();
	if (error == ERROR_TIMEOUT)
	{ // Try a second time to read
		readSensor();
	}
	values.temperature = temperature;
	values.humidity = humidity;
	return values;
}

#ifndef OPTIMIZE_SRAM_SIZE

const char *DHTesp::getStatusString()
{
	switch (error)
	{
	case DHTesp::ERROR_TIMEOUT:
		return "TIMEOUT";

	case DHTesp::ERROR_CHECKSUM:
		return "CHECKSUM";

	default:
		return "OK";
	}
}

#else

// At the expense of 26 bytes of extra PROGMEM, we save 11 bytes of
// SRAM by using the following method:

prog_char P_OK[] PROGMEM = "OK";
prog_char P_TIMEOUT[] PROGMEM = "TIMEOUT";
prog_char P_CHECKSUM[] PROGMEM = "CHECKSUM";

const char *DHTesp::getStatusString()
{
	prog_char *c;
	switch (error)
	{
	case DHTesp::ERROR_CHECKSUM:
		c = P_CHECKSUM;
		break;

	case DHTesp::ERROR_TIMEOUT:
		c = P_TIMEOUT;
		break;

	default:
		c = P_OK;
		break;
	}

	static char buffer[9];
	strcpy_P(buffer, c);

	return buffer;
}

#endif

void DHTesp::readSensor()
{
	// Make sure we don't poll the sensor too often
	// - Max sample rate DHT11 is 1 Hz   (duty cicle 1000 ms)
	// - Max sample rate DHT22 is 0.5 Hz (duty cicle 2000 ms)
	unsigned long startTime = millis();
	if ((unsigned long)(startTime - lastReadTime) < (model == DHT11 ? 999L : 1999L))
	{
		return;
	}
	lastReadTime = startTime;

	temperature = NAN;
	humidity = NAN;

	uint16_t rawHumidity = 0;
	uint16_t rawTemperature = 0;
	uint16_t data = 0;

	// Request sample
	digitalWrite(pin, LOW); // Send start signal
	pinMode(pin, OUTPUT);
	if (model == DHT11)
	{
		delay(18);
	}
	else
	{
		// This will fail for a DHT11 - that's how we can detect such a device
		delay(2);
	}

	pinMode(pin, INPUT);
	digitalWrite(pin, HIGH); // Switch bus to receive data

	// We're going to read 83 edges:
	// - First a FALLING, RISING, and FALLING edge for the start bit
	// - Then 40 bits: RISING and then a FALLING edge per bit
	// To keep our code simple, we accept any HIGH or LOW reading if it's max 85 usecs long

#ifdef ESP32
	// ESP32 is a multi core / multi processing chip
	// It is necessary to disable task switches during the readings
	portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
	portENTER_CRITICAL(&mux);
#else
	//   cli();
	noInterrupts();
#endif
	for (int8_t i = -3; i < 2 * 40; i++)
	{
		byte age;
		startTime = micros();

		do
		{
			age = (unsigned long)(micros() - startTime);
			if (age > 90)
			{
				error = ERROR_TIMEOUT;
#ifdef ESP32
				portEXIT_CRITICAL(&mux);
#else
				// sei();
				interrupts();
#endif
				return;
			}
		} while (digitalRead(pin) == (i & 1) ? HIGH : LOW);

		if (i >= 0 && (i & 1))
		{
			// Now we are being fed our 40 bits
			data <<= 1;

			// A zero max 30 usecs, a one at least 68 usecs.
			if (age > 30)
			{
				data |= 1; // we got a one
			}
		}

		switch (i)
		{
		case 31:
			rawHumidity = data;
			break;
		case 63:
			rawTemperature = data;
			data = 0;
			break;
		}
	}

#ifdef ESP32
	portEXIT_CRITICAL(&mux);
#else
	//   sei();
	interrupts();
#endif

	// Verify checksum

	if ((byte)(((byte)rawHumidity) + (rawHumidity >> 8) + ((byte)rawTemperature) + (rawTemperature >> 8)) != data)
	{
		error = ERROR_CHECKSUM;
		return;
	}

	// Store readings

	if (model == DHT11)
	{
		humidity = (rawHumidity >> 8) + ((rawHumidity & 0x00FF) * 0.1);
		temperature = (rawTemperature >> 8) + ((rawTemperature & 0x007F) * 0.1);
		if (rawTemperature & 0x0080)
		{
			temperature = -temperature;
		}
	}
	else
	{
		humidity = rawHumidity * 0.1;

		if (rawTemperature & 0x8000)
		{
			rawTemperature = -(int16_t)(rawTemperature & 0x7FFF);
		}
		temperature = ((int16_t)rawTemperature) * 0.1;
	}

	error = ERROR_NONE;
}

//boolean isFahrenheit: True == Fahrenheit; False == Celcius
float DHTesp::computeHeatIndex(float temperature, float percentHumidity, bool isFahrenheit)
{
	// Using both Rothfusz and Steadman's equations
	// http://www.wpc.ncep.noaa.gov/html/heatindex_equation.shtml
	float hi;

	if (!isFahrenheit)
	{
		temperature = toFahrenheit(temperature);
	}

	hi = 0.5 * (temperature + 61.0 + ((temperature - 68.0) * 1.2) + (percentHumidity * 0.094));

	if (hi > 79)
	{
		hi = -42.379 +
			 2.04901523 * temperature +
			 10.14333127 * percentHumidity +
			 -0.22475541 * temperature * percentHumidity +
			 -0.00683783 * pow(temperature, 2) +
			 -0.05481717 * pow(percentHumidity, 2) +
			 0.00122874 * pow(temperature, 2) * percentHumidity +
			 0.00085282 * temperature * pow(percentHumidity, 2) +
			 -0.00000199 * pow(temperature, 2) * pow(percentHumidity, 2);

		if ((percentHumidity < 13) && (temperature >= 80.0) && (temperature <= 112.0))
			hi -= ((13.0 - percentHumidity) * 0.25) * sqrt((17.0 - abs(temperature - 95.0)) * 0.05882);

		else if ((percentHumidity > 85.0) && (temperature >= 80.0) && (temperature <= 87.0))
			hi += ((percentHumidity - 85.0) * 0.1) * ((87.0 - temperature) * 0.2);
	}

	return isFahrenheit ? hi : toCelsius(hi);
}

//boolean isFahrenheit: True == Fahrenheit; False == Celcius
float DHTesp::computeDewPoint(float temperature, float percentHumidity, bool isFahrenheit)
{
	// reference: http://wahiduddin.net/calc/density_algorithms.htm
	if (isFahrenheit)
	{
		temperature = toCelsius(temperature);
	}
	double A0 = 373.15 / (273.15 + (double)temperature);
	double SUM = -7.90298 * (A0 - 1);
	SUM += 5.02808 * log10(A0);
	SUM += -1.3816e-7 * (pow(10, (11.344 * (1 - 1 / A0))) - 1);
	SUM += 8.1328e-3 * (pow(10, (-3.49149 * (A0 - 1))) - 1);
	SUM += log10(1013.246);
	double VP = pow(10, SUM - 3) * (double)percentHumidity;
	double Td = log(VP / 0.61078); // temp var
	Td = (241.88 * Td) / (17.558 - Td);
	return isFahrenheit ? toFahrenheit(Td) : Td;
}

//boolean isFahrenheit: True == Fahrenheit; False == Celcius
byte DHTesp::computePerception(float temperature, float percentHumidity, bool isFahrenheit)
{
	// Computing human perception from dew point
	// reference: https://en.wikipedia.org/wiki/Dew_point ==> Relationship to human comfort
	// reference: Horstmeyer, Steve (2006-08-15). "Relative Humidity....Relative to What? The Dew Point Temperature...a better approach". Steve Horstmeyer, Meteorologist, WKRC TV, Cincinnati, Ohio, USA. Retrieved 2009-08-20.
	// Using table
	// Return value Dew point    Human perception[6]
	//    7         Over 26 °C   Severely high, even deadly for asthma related illnesses
	//    6         24–26 °C     Extremely uncomfortable, oppressive
	//    5         21–24 °C     Very humid, quite uncomfortable
	//    4         18–21 °C     Somewhat uncomfortable for most people at upper edge
	//    3         16–18 °C     OK for most, but all perceive the humidity at upper edge
	//    2         13–16 °C     Comfortable
	//    1         10–12 °C     Very comfortable
	//    0         Under 10 °C  A bit dry for some

	if (isFahrenheit)
	{
		temperature = toCelsius(temperature);
	}
	float dewPoint = computeDewPoint(temperature, percentHumidity);

	if (dewPoint < 10.0f)
	{
		return Perception_Dry;
	}
	else if (dewPoint < 13.0f)
	{
		return Perception_VeryComfy;
	}
	else if (dewPoint < 16.0f)
	{
		return Perception_Comfy;
	}
	else if (dewPoint < 18.0f)
	{
		return Perception_Ok;
	}
	else if (dewPoint < 21.0f)
	{
		return Perception_UnComfy;
	}
	else if (dewPoint < 24.0f)
	{
		return Perception_QuiteUnComfy;
	}
	else if (dewPoint < 26.0f)
	{
		return Perception_VeryUnComfy;
	}
	// else dew >= 26.0
	return Perception_SevereUncomfy;
}

//boolean isFahrenheit: True == Fahrenheit; False == Celcius
float DHTesp::getComfortRatio(ComfortState &destComfortStatus, float temperature, float percentHumidity, bool isFahrenheit)
{
	float ratio = 100; //100%
	float distance = 0;
	float kTempFactor = 3;	//take into account the slope of the lines
	float kHumidFactor = 0.1; //take into account the slope of the lines
	uint8_t tempComfort = 0;

	if (isFahrenheit)
	{
		temperature = toCelsius(temperature);
	}

	destComfortStatus = Comfort_OK;

	distance = m_comfort.distanceTooHot(temperature, percentHumidity);
	if (distance > 0)
	{
		//update the comfort descriptor
		tempComfort += (uint8_t)Comfort_TooHot;
		//decrease the comfot ratio taking the distance into account
		ratio -= distance * kTempFactor;
	}

	distance = m_comfort.distanceTooHumid(temperature, percentHumidity);
	if (distance > 0)
	{
		//update the comfort descriptor
		tempComfort += (uint8_t)Comfort_TooHumid;
		//decrease the comfot ratio taking the distance into account
		ratio -= distance * kHumidFactor;
	}

	distance = m_comfort.distanceTooCold(temperature, percentHumidity);
	if (distance > 0)
	{
		//update the comfort descriptor
		tempComfort += (uint8_t)Comfort_TooCold;
		//decrease the comfot ratio taking the distance into account
		ratio -= distance * kTempFactor;
	}

	distance = m_comfort.distanceTooDry(temperature, percentHumidity);
	if (distance > 0)
	{
		//update the comfort descriptor
		tempComfort += (uint8_t)Comfort_TooDry;
		//decrease the comfot ratio taking the distance into account
		ratio -= distance * kHumidFactor;
	}

	destComfortStatus = (ComfortState)tempComfort;

	if (ratio < 0)
		ratio = 0;

	return ratio;
}

float DHTesp::computeAbsoluteHumidity(float temperature, float percentHumidity, bool isFahrenheit)
{
	// Calculate the absolute humidity in g/m³
	// https://carnotcycle.wordpress.com/2012/08/04/how-to-convert-relative-humidity-to-absolute-humidity/
	if (isFahrenheit)
	{
		temperature = toCelsius(temperature);
	}

	float absHumidity;
	float absTemperature;
	absTemperature = temperature + 273.15;

	absHumidity = 6.112;
	absHumidity *= exp((17.67 * temperature) / (243.5 + temperature));
	absHumidity *= percentHumidity;
	absHumidity *= 2.1674;
	absHumidity /= absTemperature;
	return absHumidity; }

//######################################### cDHT.h #########################################

class cDHT22 : public cIntervalSensor{
  private:
	DHTesp dht;
	uint8_t pin ;
	cDevice temp ;
	cDevice humid ;
	
  public:
	cDHT22(int p) { 
		pin = p ;
		setInterval(10) ; 
		dht.setup(pin, DHTesp::DHT22); } // Connect DHT sensor to GPIO 17;
	void measure() {
		humid.setValue((int)dht.getHumidity()) ;
		temp.setValue((int)(dht.getTemperature()*10)) ; }
		
	cDevice * getTemperatureSensor() {return &temp ; }
	cDevice * getHumiditySensor() {return &humid ; } };

//######################################### Factory #########################################

class cDHT22Factory : public cFactory {
  public:
	cDHT22Factory() {strcpy(name,"DHT22");}
	cDHT22* create(int p) { return new cDHT22(p);}
	cDHT22* create(int p, cb_function  fTmp, cb_function  fHum) {
		cDHT22* d = create(p);
		new cCallBackAdapter(fTmp, d->getTemperatureSensor()) ;
		new cCallBackAdapter(fHum, d->getHumiditySensor()) ;
		return d ; }
	cDHT22* create(int p, char*  n) {
		cDHT22* d = create(p);
		char txt[20] ;
		strcpy(txt, "tmp");
		strcat(txt, n) ;
		new cMsgAdapter(new cValueTranslator(1),txt,d->getTemperatureSensor()) ;
		strcpy(txt, "hum");
		strcat(txt, n) ;
		new cMsgAdapter(new cValueTranslator(0),txt,d->getHumiditySensor()) ;
		return d ; } } ;

cDHT22Factory newDHT22;

#endif
