// cremaClass.h

#ifndef _CREMACLASS_h
#define _CREMACLASS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "cremaSerial.h"
#include "cremaTime.h"
#include "cremaVisor.h"
#include "cremaSensor.h"
#include "cremaWIFI.h"
#include "cremaIoT.h"
#include "cremaConfig.h"

#include "cremaErr.h"

class cremaClass
{
 protected:
protected:
	bool _whatShow = true;
	bool _whatUpload = true;
	char *_timeSep = " ";
	void _readGPS();
	void _testGPSSignal();
	void _sayDate();
public:
	void init();
	void treatLastError();
	void ShowSensorValues();
	void ShowDateTime();
	void ReadSensors();
	void doGPS();
	void UploadSensorValues();
	void Restart(const bool force = false);
	cremaSensorClass *sensor;
	cremaSerialClass *serial;
	cremaTimeClass *time;
	cremaVisorClass *visor;
	cremaWiFiClass *wifi;
	cremaConfigClass *config;
};

static cremaClass crema;

#endif