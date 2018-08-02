#include "cremaIoT.h"
#include "cremaClass.h"

/* Source from at 
 http://help.ubidots.com/connect-your-devices/connect-the-esp32-devkitc-to-ubidots-in-under-30-minutes
**************************************************/

void ubidots_callback(char* topic, byte* payload, unsigned int length) {
	crema.serial.print("\nMensagem recebida [");
	crema.serial.print(topic);
	crema.serial.print("] ");
	for (int i = 0; i<length; i++) {
		crema.serial.print((char)payload[i]);
	}
}


cremaIoTClass::cremaIoTClass()
{
	pinMode(_PIN_LED_UPLOAD_SENSOR_VALUES, OUTPUT);
	digitalWrite(_PIN_LED_UPLOAD_SENSOR_VALUES, LOW);
}


cremaIoTClass::~cremaIoTClass()
{
}

void cremaIoTClass::publishHTTP(cremaSensorClass sensores, const cremaSensorsId first = csLuminosidade,	const cremaSensorsId last = csUltraVioleta)
{
	if (!crema.wifi.connected()) {
		crema.config.setForceConfig(false);     // se necessário iniciar webServer, informar apenas dados de WiFi
		crema.wifi.autoConnect(crema.config);
	}

	digitalWrite(_PIN_LED_UPLOAD_SENSOR_VALUES, HIGH);

	cremaSensorsId eCurrent;
	char _payload[2024];            // sensor data values content
	char _topic[250];			   // URL to post to
	char _str_sensor_value[35];    // Space to store values to send

	char _str_lat[35];             // Space to store latitude value
	char _str_lng[35];             // Space to store longitude value

	String _httpResponse;

	sprintf(_topic, "%s%s%s?token=%s", _mqttBroker, "/api/v1.6/devices/", DEVICE_LABEL, TOKEN);
	sprintf(_payload, "%s", "{"); // Cleans the payload

	for (int i = first; i <= last; i++)
	{
		eCurrent = (cremaSensorsId)i;

		/* convert float value do str. 4 is mininum width, 2 is precision; float value is copied onto str_sensor*/
		dtostrf(sensores.Values[eCurrent], 10, 4, _str_sensor_value);
		
		// Adds the variable label and its value
		// inclui informação de GPS. como deve der a string
		// {"temperatura":{"value":10,"context":{"lat":-19.648461,"lng":-43.901583}}}

		// {"temperatura":
		sprintf(_payload, "%s\"%s\":", _payload, sensores.Labels[eCurrent]);
		if (sensores.gpsData.valid)
		{
			// ficará assim: {"temperatura":{"value":10,"context":{"lat":-19.648461,"lng":-43.901583}}
			dtostrf(sensores.gpsData.lat, 10, 6, _str_lat);
			dtostrf(sensores.gpsData.lng, 10, 6, _str_lng);
			sprintf(_payload, "%s{\"value\":%s,\"context\":{\"lat\":%s,\"lng\":%s}}", _payload, _str_sensor_value, _str_lat, _str_lng);
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

	//Serial.print("payload=");
	//Serial.println(_payload);

	_http.begin(_topic);
	_http.addHeader("Content-Type", "application/json");             //Specify content-type header
	int httpResponseCode = _http.POST(_payload);   //Send the actual POST request

	if (httpResponseCode > 0) {
		_httpResponse = _http.getString();                       //Get the response to the request
		crema.serial.print("HTTP return code: ");
		Serial.print(httpResponseCode);   //Print return code
		Serial.printf("  [JSON size: %d]\n", _httpResponse.length());
		//Serial.println(_httpResponse);
	}
	else {
		Serial.print("Error on sending POST: ");
		Serial.println(httpResponseCode);
	}
	_http.end();  //Free resources

	{
		DynamicJsonBuffer  jsonBuffer(512);
		JsonObject& root = jsonBuffer.parseObject(_httpResponse.c_str());
		if (root.success())
		{
			for (int i = first; i <= last; i++)
			{
				eCurrent = (cremaSensorsId)i;
				int rtnHttpPost = root[sensores.Labels[eCurrent]][0]["status_code"];
				Serial.printf("%s = %.*f       [%d]", sensores.Labels[eCurrent], sensores.Decimals[eCurrent], sensores.Values[eCurrent], rtnHttpPost);
				if (rtnHttpPost = 201)
				{
					Serial.println(" ok!");
				}
				else
				{
					Serial.println(" ERRO!");
				}
			}
		}
	}
	sensores.displayGPSInfo();

	digitalWrite(_PIN_LED_UPLOAD_SENSOR_VALUES, LOW);
}
