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
#define SENSORS_PRESSURE_SEALEVELHPA      (1013.25F)   /**< Average sea level pressure is 1013.25 hPa */

// mqtt receiver function
void ubidots_callback(char* topic, byte* payload, unsigned int length) 
{
	//utilizar mensagem de debug.print("\nMensagem recebida [");
	//utilizar mensagem de debug.print(topic);
	//utilizar mensagem de debug.print("] ");
	//for (int i = 0; i<length; i++) {
	//	utilizar mensagem de debug.print((char)payload[i]);
	//}
}

cremaSensorClass::cremaSensorClass()
{
	Wire.begin(_CREMA_I2C_SDA, _CREMA_I2C_SCL);
	Serial_GPS.begin(9600, SERIAL_8N1, _PIN_GPS_RX, _PIN_GPS_TX);
	delay(500);

	// sensor read led indicator
	pinMode(_PIN_LED_READ_SENSOR_VALUES, OUTPUT);
	digitalWrite(_PIN_LED_READ_SENSOR_VALUES, LOW);

	// IoT upload led indicator
	pinMode(_PIN_LED_UPLOAD_SENSOR_VALUES, OUTPUT);
	digitalWrite(_PIN_LED_UPLOAD_SENSOR_VALUES, LOW);

	// parametrizações do sensor ultra violeta
	//pinMode(_CREMA_UV_PIN, INPUT);
	//pinMode(_CREMA_UV_PIN_REF, INPUT);
	//analogReadResolution(16);
	//adcAttachPin(_CREMA_UV_PIN);
	//adcAttachPin(_CREMA_UV_PIN_REF);

	// parametrização do sensor DHT (temperatura e umidade)
	//pinMode(_CREMA_DHT_PIN, INPUT);
	//adcAttachPin(_CREMA_DHT_PIN);

	gpsData.age = 0;
	gpsData.HDOP = 10000;    // este valor indica péssima precisão do GPS. Com isso, próximoas leituras serão salvas por inidicar precisão melhor
	gpsData.satellites = -5000;
	gpsData.altitude = -5000;
	gpsData.updatedLocation = false;
	gpsData.validLocation = false;
	gpsData.validAltitude = false;
	gpsData.lat = 0;
	gpsData.lng = 0;
}

bool cremaSensorClass::init()
{
	// inicialização dos sensores
	if (!_luxSensor.begin())
	{
		return false;
	}

	if (!_bme.begin(0x76))
	{
		return false;
	}

	return readSensors();
}

bool cremaSensorClass::readSensors()
{
	bool rtn = true;
	try
	{
		digitalWrite(_PIN_LED_READ_SENSOR_VALUES, HIGH);

		Values[csLuminosidade] = _luxSensor.readLightLevel(true);
		delay(10);  // a delay for the I2C bus
		Values[csPressao] = _bme.readPressure();
		delay(10);  // a delay for the I2C bus
		if (!gpsData.validAltitude)
		{
			Values[csAltitude] = _bme.readAltitude(SENSORS_PRESSURE_SEALEVELHPA);
		}
		Values[csTemperatura] = _bme.readTemperature();
		delay(10);  // a delay for the I2C bus
		Values[csUmidade] = _bme.readHumidity();
		delay(10);  // a delay for the I2C bus
		Values[csMemory] = esp_get_free_heap_size();
		Values[csLog] = millis() / 1000;
		//Values[csUltraVioleta] = _getUV();

		rtn = !(abs(Values[csTemperatura]) > 50);
	}
	catch (const std::exception& e)
	{
		g_SetLastSystemError("ReadSensor", e.what());
		rtn = false;
	}

	digitalWrite(_PIN_LED_READ_SENSOR_VALUES, LOW);

	return rtn;
}

void cremaSensorClass::publishHTTP(const cremaSensorsId first = csLuminosidade, const cremaSensorsId last = csUltraVioleta, const cremaErrorDescription desc, const cremaSystemErrorDescription sysErrorMsg)
{
	HTTPClient _http;
	try
	{
		const char _mqttBroker[] = "http://things.ubidots.com";

		cremaSensorsId eCurrent;
		char _payload[2024];            // sensor data values content
		char _topic[250];			   // URL to post to
		char _str_sensor_value[35];    // Space to store values to send

		digitalWrite(_PIN_LED_UPLOAD_SENSOR_VALUES, HIGH);

		sprintf(_topic, "%s%s%s?token=%s", _mqttBroker, "/api/v1.6/devices/", DEVICE_LABEL, TOKEN);
		sprintf(_payload, "%s", "{"); // Cleans the payload

		for (int i = first; i <= last; i++)
		{
			char _context[2024];
			eCurrent = (cremaSensorsId)i;

			/* convert float value do str. 4 is mininum width, 2 is precision; float value is copied onto str_sensor*/
			dtostrf(Values[eCurrent], 10, 4, _str_sensor_value);

			// Adds the variable label and its value
			// inclui informação de GPS. como deve der a string
			// {"temperatura":{"value":10,"context":{"lat":-19.648461,"lng":-43.901583}}}

			// {"temperatura":{"value":23.4500
			sprintf(_payload, "%s\"%s\":{\"value\":%s", _payload, Labels[eCurrent], _str_sensor_value);
			//// TODO: código de _context pode ser uma vez somente, no lugar de ser calculado para cada variável.
			sprintf(_context, "%s", "");

			// se GPS está sincronizado, adiciona informação de localização
			if (gpsData.validLocation)
			{
				char _str_lat[35];             // Space to store latitude value
				char _str_lng[35];             // Space to store longitude value
				char _str_hdop[35];             // Space to store HDOP value

				dtostrf(gpsData.lat, 10, 4, _str_lat);
				dtostrf(gpsData.lng, 10, 4, _str_lng);
				dtostrf(gpsData.HDOP, 5, 0, _str_hdop);

				// "lat":-19.648461,"lng":-43.901583
				sprintf(_context, "%s\"lat\":%s,\"lng\":%s,\"hdop\":%s", _context, _str_lat, _str_lng, _str_hdop);
			}

			// se há uma descrição específica para o upload, a inclui
			if (desc != "")
			{
				if (strlen(_context) > 1)
				{
					// "lat":-19.648461,"lng":-43.901583,
					sprintf(_context, "%s,", _context);
				}
				// "lat":-19.648461,"lng":-43.901583,"desc":"Not controlled restart"
				sprintf(_context, "%s\"desc\":\"%s\"", _context, desc);
			}

			// se há mensagem de erro do sistema, inclui
			if (sysErrorMsg != "")
			{
				if (strlen(_context) > 1)
				{
					// "lat":-19.648461,"lng":-43.901583,"desc":"Not controlled restart",
					sprintf(_context, "%s,", _context);
				}
				// "lat":-19.648461,"lng":-43.901583,"desc":"Not controlled restart","SysError":"General protection fault"
				sprintf(_context, "%s\"SysError\":\"%s\"", _context, sysErrorMsg);
			}

			if (strlen(_context) > 1)
			{
				// {"temperatura":{"value":23.4500,"context":{"lat":-19.648461,"lng":-43.901583,"desc":"Not controlled restart"}
				sprintf(_payload, "%s,\"context\":{%s}", _payload, _context);
			}

			// fecha chaves da informação da variável
			sprintf(_payload, "%s}", _payload);

			// se não é a última variável, ou seja, se ainda há variáveis a incluir, adiciona o separador de variáveis ','
			if (i < last) {
				sprintf(_payload, "%s,", _payload);
			}

		}

		// fecha chaves de todo o bloco com informações das variáveis
		sprintf(_payload, "%s}", _payload);

		//// TODO: tratar _http.begin() se retornar falso
		_http.begin(_topic);
		_http.addHeader("Content-Type", "application/json");             //Specify content-type header

		int httpResponseCode = _http.POST(_payload);   //Send the actual POST request

#if defined(_DEBUG)
		//// TODO: tratar se houve erro no upload a Ubidots
		if (httpResponseCode > 0)   
		{
			String _httpResponse = _http.getString();                       //Get the response to the request
			DynamicJsonBuffer  jsonBuffer(512);
			JsonObject& root = jsonBuffer.parseObject(_httpResponse.c_str());
			if (root.success())
			{
				for (int i = first; i <= last; i++)
				{
					eCurrent = (cremaSensorsId)i;
					int rtnHttpPost = root[Labels[eCurrent]][0]["status_code"];
				}
			}
		}
		_displayGPSInfo();
#endif  // _DEBUG

	}
	catch (const std::exception& e)
	{
		g_SetLastSystemError("publishHTTP", e.what());
	}
	_http.end();  //Free resources
	digitalWrite(_PIN_LED_UPLOAD_SENSOR_VALUES, LOW);
}

void cremaSensorClass::readGPS()
{
	try
	{
		int c;
		int reads = 0;

		// se após 50 leituras ainda não encontrou há tradução correta da String, zera a serial. {em TESTE}
		if (_gpsReadsWithError++ > 50)
		{
			_gpsReadsWithError = 0;
			Serial_GPS.flush();
		}

		while (Serial_GPS.available() > 0)
		{
			c = Serial_GPS.read();
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

				_saveGPS();
				_gpsReadsWithError = 0;

				break;
			}
			if (reads++ > (500)) //to give time to WatchDog interrupts this task process
			{
				reads = 0;
				delayMicroseconds(100);  // testing if watchdog don't crash by interrupt time
				Serial.printf("\n\n>> GPS read more than 500 chars.\n\n");
			}
		}
	}
	catch (const std::exception& e)
	{
		g_SetLastSystemError("readGPS", e.what());
	}
	delay(10);  // a delay for the I2C bus

}

void cremaSensorClass::_saveGPS()
{
	try
	{
		//// TODO: guardar em cfg a melhor localização sincronizada. Para utilizá-la no restart se verificar se está no mesmo local.
		if (1)  //((_gps.hdop.value() <= gpsData.HDOP) && (_gps.hdop.value() > 0))
		{
			gpsData.HDOP = _gps.hdop.value();
			gpsData.age = _gps.location.age();
			gpsData.satellites = _gps.satellites.value();
			gpsData.validAltitude = _gps.altitude.isValid();
			gpsData.altitude = _gps.altitude.meters();
			gpsData.updatedLocation = _gps.location.isUpdated();
			gpsData.validLocation = _gps.location.isValid();
			gpsData.lat = _gps.location.lat();
			gpsData.lng = _gps.location.lng();

			Values[csAltitude] = gpsData.altitude;
		}
	}
	catch (const std::exception& e)
	{
		g_SetLastSystemError("_saveGPS", e.what());
	}
}

#if defined(_DEBUG)
void cremaSensorClass::_displayGPSInfo()
{
	byte l; // Mostra Localização

	byte d;// Mostra Data do GPS
	byte d1;

	byte h;// Mostra Hora do GPS
	byte h1;
		
	byte p;// Mostra parâmetros do sinal
}
#endif  // _CREMA_DEBUG

float cremaSensorClass::_getUV()
{
	//int uvLevel = _average_UV_AnalogRead(_CREMA_UV_PIN);
	//int uvLevelRef = _average_UV_AnalogRead(_CREMA_UV_PIN_REF);
	//float outputVoltage = 3.3 / uvLevelRef * uvLevel;

	//utilizar mensagem de debug.print("outputVoltage=");
	//utilizar mensagem de debug.print(outputVoltage);

	//return _mapfloat_UV_Calc(outputVoltage, 0.99, 2.9, 0.0, 15.0);
}

//Takes an average of readings on a given pin
//Returns the average
int cremaSensorClass::_average_UV_AnalogRead(gpio_num_t pin)
{
	byte numberOfReadings = 8;
	unsigned int runningValue = 0;
	unsigned int r;

	//utilizar mensagem de debug.print("\n--------------------------\nPino: ");
	//utilizar mensagem de debug.println(pin);
	for (int x = 0; x < numberOfReadings; x++) {
		r = analogRead(pin);
		runningValue += r;
		//utilizar mensagem de debug.print("[Average] Leitura ");
		//utilizar mensagem de debug.print(x);
		//utilizar mensagem de debug.print("=");
		//utilizar mensagem de debug.println(r);
	}
	//utilizar mensagem de debug.print("Total=");
	//utilizar mensagem de debug.println(runningValue);
	runningValue /= numberOfReadings;
	//utilizar mensagem de debug.print("Media=");
	//utilizar mensagem de debug.println(runningValue);

	return(runningValue);

}

float cremaSensorClass::_mapfloat_UV_Calc(float x, float in_min, float in_max, float out_min, float out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
