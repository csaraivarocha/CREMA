
/*
Name:		cremaWIFI.h
Created:	2/18/2018 10:46:29 AM
Author:	crist
Editor:	http://www.visualmicro.com
*/

#ifndef _cremaIoT_h
#define _cremaIoT_h

#include <WiFi.h>
//#include <PubSubClient.h>
#include <HTTPClient.h>

#include "cremaSensor.h"
#include "ArduinoJson.h"

#define DEVICE_LABEL "esp32_bh"                    // Assig the device label
#define TOKEN "A1E-nuWgdhFqYZUQAIqItVXN67ssBhtJYV" // Put your Ubidots' TOKEN


class cremaIoTClass
{
private:
	char *_mqttBroker = "http://things.ubidots.com";
	HTTPClient _http;
public:
	cremaIoTClass();
	~cremaIoTClass();
	void publishHTTP(cremaSensorClass sensores, const cremaSensorsId first, const cremaSensorsId last);
};


#endif