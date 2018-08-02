/*
 Name:		cremaLog.h
 Created:	2/15/2018 10:18:50 PM
 Author:	crist
 Editor:	http://www.visualmicro.com
*/

#ifndef _cremaLog_h
#define _cremaLog_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <SD.h>

#define _LOG_INI "["
#define _LOG_END "]"
#define _LOG_SEP ";"

struct logLine_S
{
	String llsType;    // logLineTyp = { "R", "I" };  // Registro de sensores, informação...
	String llsTime;   // YYMMDDHHMMSS
	double llstTemperature;
	double llsHumidity;
	double llsAltitude;
	double llsPressure;
};

// ***********************************************************************************************************
class cremaLogClass
{
private:
	String _fileName = "crema.log";
	String _stationId;
	byte _pin;
public:
	cremaLogClass(String estacaoId, const byte pinSD_CS);
	bool writeLog();
	logLine_S logData;
};

#endif

