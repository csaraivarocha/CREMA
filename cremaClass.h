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

#include <rom/rtc.h>


#define _CREMA_SSID_AP "CREMA"

#include "cremaTime.h"
#include "cremaVisor.h"
#include "cremaSensor.h"
#include "cremaConfig.h"
#include "cremaPinos.h"
#include "cremaErr.h"

class cremaClass
{
protected:  // wifi
	WiFiManager _wifiManager;
	void _initWiFi();
	bool _wifi_autoConnect();
	void _wifi_startWebServer();
protected:  // cloud
	void _uploadToCloud(const cremaSensorsId first, const cremaSensorsId last, const cremaErrorDescription desc = "", const cremaSystemErrorDescription sysErrorMsg = "");
	void _uploadErrorLog(const cremaErrorId error, const bool restart, const bool saveConfig, const cremaSystemErrorDescription sysErrorMsg = "");
protected:
	bool _whatShow = true;
	bool _whatUpload = true;
	void _readGPS();
	void _testGPSSignal();
public:  // cloud
	void uploadSystemHaltError(const cremaErrorId typeSystemHaltError, const cremaSystemErrorDescription sysErrorMsg);
public:  // wifi static functions and variables
	static void __wifi_configModeCallback(WiFiManager * myWiFiManager);  // callback function
	static void __wifi_saveConfigCallback();  // callback function
	static void __displayConfigMode();
	static bool __webServerConfigSaved;
public:
	cremaClass();
	~cremaClass();
	char *_timeSep = " ";
	void init();
	void treatLastError();
	void ShowSensorValues();
	void ShowDateTime();
	void ReadSensors();
	void doGPS();
	void UploadSensorValues();
	void Restart();
	cremaSensorClass *sensor;
	cremaTimeClass *time;
	cremaVisorClass *visor;
	cremaConfigClass *config;
};

static cremaClass *crema;

#endif