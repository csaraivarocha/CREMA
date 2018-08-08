// cremaClass.h

#ifndef _CREMACLASS_h
#define _CREMACLASS_h

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


//Local DNS Server used for redirecting all requests to the configuration portal 
// (https://github.com/zhouhan0126/DNSServer---esp32 )
#include <DNSServer.h> 


#if defined(ESP8266)
#include <ESP8266WebServer.h> //Local WebServer used to serve the configuration portal
#else
//Local WebServer used to serve the configuration portal ( https://github.com/zhouhan0126/WebServer-esp32 )
#include <WebServer.h> 
#endif


// WiFi Configuration Magic ( https://github.com/zhouhan0126/WIFIMANAGER-ESP32 ) >> 
// https://github.com/tzapu/WiFiManager (ORIGINAL)
#include <WiFiManager.h>   


#define _CREMA_SSID_AP "CREMA"

#include "cremaTime.h"
#include "cremaVisor.h"
#include "cremaSensor.h"
#include "cremaConfig.h"
#include "cremaPinos.h"
#include "cremaErr.h"

class cremaClass
{
protected:
	WiFiManager _wifiManager;
	void _initWiFi();
	bool _wifi_autoConnect();
	void _wifi_startWebServer();
protected:
	void _uploadErrorLog(const int error, const bool restart, const bool saveConfig);
	void _uploadToCloud(const cremaSensorsId first, const cremaSensorsId last);
protected:
	bool _whatShow = true;
	bool _whatUpload = true;
	char *_timeSep = " ";
	void _readGPS();
	void _testGPSSignal();
	void _sayDate();
public:
	cremaClass();
	~cremaClass();
	void init();
	void treatLastError();
	void ShowSensorValues();
	void ShowDateTime();
	void ReadSensors();
	void doGPS();
	void UploadSensorValues();
	void Restart();
	void displayConfigMode();
	bool webServerConfigSaved = false;
	cremaSensorClass *sensor;
	cremaTimeClass *time;
	cremaVisorClass *visor = new cremaVisorClass();
	cremaConfigClass *config;
};

#endif