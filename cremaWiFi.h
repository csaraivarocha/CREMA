
/*
Name:		cremaWIFI.h
Created:	2/18/2018 10:46:29 AM
Author:	crist
Editor:	http://www.visualmicro.com
*/

#ifndef _cremaWIFI_h
#define _cremaWIFI_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#if defined(ESP8266)
#include <ESP8266WiFi.h>  //ESP8266 Core WiFi Library         
#else
#include <WiFi.h>      //ESP32 Core WiFi Library    
#endif

//needed for library
#include <DNSServer.h> //Local DNS Server used for redirecting all requests to the configuration portal ( https://github.com/zhouhan0126/DNSServer---esp32 )
#if defined(ESP8266)
#include <ESP8266WebServer.h> //Local WebServer used to serve the configuration portal
#else
#include <WebServer.h> //Local WebServer used to serve the configuration portal ( https://github.com/zhouhan0126/WebServer-esp32 )
#endif
#include <WiFiManager.h>   // WiFi Configuration Magic ( https://github.com/zhouhan0126/WIFIMANAGER-ESP32 ) >> https://github.com/tzapu/WiFiManager (ORIGINAL)


#define _CREMA_SSID_AP "CREMA"

#include "cremaPinos.h"
#include "cremaConfig.h"

class cremaWiFiClass
{
private:
	WiFiManager _wifiManager;
public:
	cremaWiFiClass();
	~cremaWiFiClass();
	bool autoConnect(cremaConfigClass * Config);
	void displayConfigMode();
	bool startWebServer();
	bool connected();
	bool webServerConfigSaved = false;
};

#endif