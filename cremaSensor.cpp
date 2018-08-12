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
}

bool cremaSensorClass::init()
{
	// inicialização dos sensores
	if (!(working[csLuminosidade] = _luxSensor.begin())) 
	{
		return false;
	}

	_bme.begin(0x76);

	gpsData.age = 0;
	gpsData.HDOP = 0;
	gpsData.satellites = -5000;
	gpsData.altitude = -5000;
	gpsData.updated = false;
	gpsData.valid = false;
	gpsData.lat = 0;
	gpsData.lng = 0;

	return readSensors();
}

bool cremaSensorClass::readSensors()
{
	digitalWrite(_PIN_LED_READ_SENSOR_VALUES, HIGH);

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

	digitalWrite(_PIN_LED_READ_SENSOR_VALUES, LOW);

	return !(abs(Values[csTemperatura]) > 50);
}

void cremaSensorClass::publishHTTP(const cremaSensorsId first = csLuminosidade, const cremaSensorsId last = csUltraVioleta, const cremaErroDescription desc)
{
	HTTPClient _http;
	const char _mqttBroker[] = "http://things.ubidots.com";

	cremaSensorsId eCurrent;
	char _payload[2024];            // sensor data values content
	char _topic[250];			   // URL to post to
	char _str_sensor_value[35];    // Space to store values to send

	char _str_lat[35];             // Space to store latitude value
	char _str_lng[35];             // Space to store longitude value

	digitalWrite(_PIN_LED_UPLOAD_SENSOR_VALUES, HIGH);

	sprintf(_topic, "%s%s%s?token=%s", _mqttBroker, "/api/v1.6/devices/", DEVICE_LABEL, TOKEN);
	sprintf(_payload, "%s", "{"); // Cleans the payload

	for (int i = first; i <= last; i++)
	{
		eCurrent = (cremaSensorsId)i;

		/* convert float value do str. 4 is mininum width, 2 is precision; float value is copied onto str_sensor*/
		dtostrf(Values[eCurrent], 10, 4, _str_sensor_value);

		// Adds the variable label and its value
		// inclui informação de GPS. como deve der a string
		// {"temperatura":{"value":10,"context":{"lat":-19.648461,"lng":-43.901583}}}

		// {"temperatura":
		sprintf(_payload, "%s\"%s\":", _payload, Labels[eCurrent]);
		if ((gpsData.valid) || (desc != ""))
		{
			bool gps_ok = (gpsData.valid && gpsData.lat != -5000);
			// ficará assim: {"temperatura":{"value":10,"context":{"lat":-19.648461,"lng":-43.901583}}
			dtostrf(gpsData.lat, 10, 6, _str_lat);
			dtostrf(gpsData.lng, 10, 6, _str_lng);

			if ((gps_ok) && (desc != ""))
			{
				//sprintf(_payload, "%s{\"value\":%s,\"context\":{\"lat\":%s,\"lng\":%s}}", _payload, _str_sensor_value, _str_lat, _str_lng);
				sprintf(_payload, "%s{\"value\":%s,\"context\":{\"lat\":%s,\"lng\":%s,\"desc\":\"%s\"}}", _payload, _str_sensor_value, _str_lat, _str_lng, desc);
			}
			if ((gps_ok) && (desc == ""))
			{
				sprintf(_payload, "%s{\"value\":%s,\"context\":{\"lat\":%s,\"lng\":%s}}", _payload, _str_sensor_value, _str_lat, _str_lng);
			}
			if ((!gps_ok) && (desc != ""))
			{
				sprintf(_payload, "%s{\"value\":%s,\"context\":{\"desc\":\"%s\"}}", _payload, _str_sensor_value, desc);
			}

		}
		else
		{
			// ficará assim: {"temperatura":23.4
			sprintf(_payload, "%s%s", _payload, _str_sensor_value);
		}

		// se não é a última variável, ou seja, se ainda há variáveis a incluir, 
		// adiciona o separador de variáveis ','
		if (i < last) {
			sprintf(_payload, "%s%s", _payload, ",");
		}

	}
	sprintf(_payload, "%s%s", _payload, "}");

	bool b = _http.begin(_topic);
	_http.addHeader("Content-Type", "application/json");             //Specify content-type header
	int httpResponseCode = _http.POST(_payload);   //Send the actual POST request

//#if (_VMDEBUG == 1)  // TODO: verificar como mensagem de debug não aparece
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
	displayGPSInfo();
//#endif  //DEBUG

	_http.end();  //Free resources
	digitalWrite(_PIN_LED_UPLOAD_SENSOR_VALUES, LOW);
}

void cremaSensorClass::readGPS()
{
	int c;

	// se após 30 leituras ainda não encontrou há tradução correta da String, zera a serial. {em TESTE}
	if (_gpsReadesWithError++ > 20)
	{
		_gpsReadesWithError = 0;
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
	byte l; // Mostra Localização

	byte d;// Mostra Data do GPS
	byte d1;

	byte h;// Mostra Hora do GPS
	byte h1;
		
	byte p;// Mostra parâmetros do sinal
}

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
