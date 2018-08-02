/*
 Name:		cremaSensor.cpp
 Created:	2/13/2018 8:07:56 PM
 Author:	crist
 Editor:	http://www.visualmicro.com
*/

/*
..:  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- -- -- -- -- --
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
20: -- -- -- 23 -- -- -- -- -- -- -- -- -- -- -- --
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
50: -- -- -- -- -- -- -- 57 -- -- -- -- -- -- -- --
60: -- -- -- -- -- -- -- -- 68 -- -- -- -- -- -- --
70: -- -- -- -- -- -- 76 --
GPIO pins -> SDA: 21; SCL: 22

76=BME280
23=Luminosidade
57 e 68=RTC
*/

#include "cremaSensor.h"
#include "cremaClass.h"
#define SENSORS_PRESSURE_SEALEVELHPA      (1013.25F)   /**< Average sea level pressure is 1013.25 hPa */

cremaSensorClass::cremaSensorClass()
{
	Wire.begin(_CREMA_I2C_SDA, _CREMA_I2C_SCL);
	Serial_GPS.begin(9600, SERIAL_8N1, _PIN_GPS_RX, _PIN_GPS_TX);
	delay(500);

	pinMode(_PIN_LEN_READ_SENSOR_VALUES, OUTPUT);
	digitalWrite(_PIN_LEN_READ_SENSOR_VALUES, LOW);

	// parametrizações do sensor ultra violeta
	//pinMode(_CREMA_UV_PIN, INPUT);
	//pinMode(_CREMA_UV_PIN_REF, INPUT);
	//analogReadResolution(16);
	//adcAttachPin(_CREMA_UV_PIN);
	//adcAttachPin(_CREMA_UV_PIN_REF);

	// parametrização do sensor DHT (temperatura e umidade)
	//pinMode(_CREMA_DHT_PIN, INPUT);
	//adcAttachPin(_CREMA_DHT_PIN);

	// inicialização dos sensores
	if (!(working[csLuminosidade] = _luxSensor.begin())) {
		Serial.println("\nLux\n++++++++++++++++++++++\nRestating ESP32\n++++++++++++++++++++++");
		//esp_restart();
	}

	_bme.begin(0x76);

	gpsData.age = 0;
	gpsData.HDOP = 0;
	gpsData.satellites = 0;
	gpsData.altitude = 0;
	gpsData.updated = false;
	gpsData.valid = false;
	gpsData.lat = 0;
	gpsData.lng = 0;
}

void cremaSensorClass::readSensors()
{
	digitalWrite(_PIN_LEN_READ_SENSOR_VALUES, HIGH);

	//readGPS();
	Values[csLuminosidade] = _luxSensor.readLightLevel(true);
	Values[csPressao] = _bme.readPressure();
	if (!gpsData.valid)
	{
		Values[csAltitude] = _bme.readAltitude(SENSORS_PRESSURE_SEALEVELHPA);
	}
	Values[csTemperatura] = _bme.readTemperature();
	Values[csUmidade] = _bme.readHumidity();
	Values[csMemory] = esp_get_free_heap_size();
	Values[csLog] = millis() / 1000;
	//Values[csUltraVioleta] = _getUV();

	if (abs(Values[csTemperatura]) > 50) 
	{
		uploadErrorLog(_ERR_SENSOR_READ, _ERR_UPLOAD_LOG_RESTART, _ERR_UPLOAD_LOG_SAVE_CONFIG);
	}

	digitalWrite(_PIN_LEN_READ_SENSOR_VALUES, LOW);
}

void cremaSensorClass::uploadErrorLog(const int error, const bool restart, const bool saveConfig)
{
	Values[csLog] = error;                      // guarda valor do erro para subir para Ubidots
	crema.IoT.publishHTTP(crema.sensor, csLog, csLog);

	if (saveConfig)
	{
		crema.config.setLastError(Values[csLog]);   // guarda erro no arquivo do ESP32 localmente para ser tratado na reinicialização
	}

	if (restart)
	{
		crema.Restart(restart);
	}
}

void cremaSensorClass::readGPS()
{
	int c;

	// se após 30 leituras ainda não encontrou há tradução correta da String, zera a serial. {em TESTE}
	if (_gpsReadesWithError++ > 20)
	{
		Serial.printf("\n~~~~~~~~~~~~\nFlushing Serial_GPS\n");
		_gpsReadesWithError = 0;
		Serial_GPS.flush();
	}

	//Serial.printf("\n_readGPS: Serial_GPS.available()\n");
	while (Serial_GPS.available() > 0)
	{
		c = Serial_GPS.read();
		//if (char(c) == '$') 
		//{
		//	Serial.println();
		//}
		//Serial.print(char(c));

		if (_gps.encode(c) && _gps.location.isValid())
		{
			// http://forest-gis.com/2018/01/acuracia-gps-o-que-sao-pdop-hdop-gdop-multi-caminho-e-outros.html/
			/* Nível DOP	Qualidade	Descrição
			< 1	Ideal	Nível de confiança mais alto; máxima precisão possível em todos os momentos.
			1 - 2	Excelente	Medições precisas
			2 - 5	Bom	Medições com precisão adequadas
			5 - 10	Moderado	Qualidade moderada.Correção recomendada
			10 - 20	Fraco	Nível de confiança baixo.Considere descartar dados
			>20	Ruim	Precisão muito baixa.Erros podem atingir 300 metros*/

			//displayGPSInfo();
			_saveGPS();
			_gpsReadesWithError = 0;

			break;
		}
	}
}

bool cremaSensorClass::_gpsOk()
{
	return gpsData.updated && gpsData.valid;
}

void cremaSensorClass::_saveGPS()
{
	gpsData.age = _gps.location.age();
	gpsData.HDOP = _gps.hdop.value();
	gpsData.satellites = _gps.satellites.value();
	gpsData.altitude = _gps.altitude.meters();
	gpsData.updated = _gps.location.isUpdated();
	gpsData.valid = _gps.location.isValid();
	gpsData.lat = _gps.location.lat();
	gpsData.lng = _gps.location.lng();

	if (_gpsOk())
	{
		Values[csAltitude] = gpsData.altitude;
	}
}

void cremaSensorClass::displayGPSInfo()
{
	//Mostra informacoes no Serial Monitor
	Serial.print(F("\nLocation: "));
	if (_gps.location.isValid())
	{
		Serial.print(_gps.location.lat(), 6); //latitude
		Serial.print(F(","));
		Serial.print(_gps.location.lng(), 6); //longitude
	}
	else
	{
		Serial.print(F("INVALID"));
	}

	Serial.print(F("  Date/Time: "));
	if (_gps.date.isValid())
	{
		Serial.print(_gps.date.day()); //dia
		Serial.print(F("/"));
		Serial.print(_gps.date.month()); //mes
		Serial.print(F("/"));
		Serial.print(_gps.date.year()); //ano
	}
	else
	{
		Serial.print(F("INVALID"));
	}

	Serial.print(F(" "));

	if (_gps.time.isValid())
	{
		if (_gps.time.hour() < 10) Serial.print(F("0"));
		Serial.print(_gps.time.hour()); //hora
		Serial.print(F(":"));
		if (_gps.time.minute() < 10) Serial.print(F("0"));
		Serial.print(_gps.time.minute()); //minuto
		Serial.print(F(":"));
		if (_gps.time.second() < 10) Serial.print(F("0"));
		Serial.print(_gps.time.second()); //segundo
		Serial.print(F("."));
		if (_gps.time.centisecond() < 10) Serial.print(F("0"));
		Serial.print(_gps.time.centisecond());
	}
	else
	{
		Serial.print(F("INVALID"));
	}

	Serial.print("\n[HDOP: ");
	Serial.print(_gps.hdop.value());
	Serial.print("]");

	Serial.print("\n[Sat: ");
	Serial.print(_gps.satellites.value());
	Serial.print("]");

	Serial.print("\n[Alt: ");
	Serial.print(_gps.altitude.meters());
	Serial.print("]");

	Serial.print("\n[Age: ");
	Serial.print(_gps.location.age());
	Serial.print("]");

	Serial.print("\n[Updated: ");
	Serial.print(_gps.location.isUpdated());
	Serial.print("]");

	Serial.print("\n[Valid: ");
	Serial.print(_gps.location.isValid());
	Serial.print("]");
}

float cremaSensorClass::_getUV()
{
	//int uvLevel = _average_UV_AnalogRead(_CREMA_UV_PIN);
	//int uvLevelRef = _average_UV_AnalogRead(_CREMA_UV_PIN_REF);
	//float outputVoltage = 3.3 / uvLevelRef * uvLevel;

	//cremaSerial.print("outputVoltage=");
	//cremaSerial.print(outputVoltage);

	//return _mapfloat_UV_Calc(outputVoltage, 0.99, 2.9, 0.0, 15.0);
}

//Takes an average of readings on a given pin
//Returns the average
int cremaSensorClass::_average_UV_AnalogRead(gpio_num_t pin)
{
	byte numberOfReadings = 8;
	unsigned int runningValue = 0;
	unsigned int r;

	//cremaSerial.print("\n--------------------------\nPino: ");
	//cremaSerial.println(pin);
	for (int x = 0; x < numberOfReadings; x++) {
		r = analogRead(pin);
		runningValue += r;
		//cremaSerial.print("[Average] Leitura ");
		//cremaSerial.print(x);
		//cremaSerial.print("=");
		//cremaSerial.println(r);
	}
	//cremaSerial.print("Total=");
	//cremaSerial.println(runningValue);
	runningValue /= numberOfReadings;
	//cremaSerial.print("Media=");
	//cremaSerial.println(runningValue);

	return(runningValue);

}

float cremaSensorClass::_mapfloat_UV_Calc(float x, float in_min, float in_max, float out_min, float out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
