/*
 Name:		cremaLog.cpp
 Created:	2/15/2018 10:18:50 PM
 Author:	crist
 Editor:	http://www.visualmicro.com
*/

#include "cremaLog.h"

cremaLogClass::cremaLogClass(String estacaoId, const byte pinSD_CS)
{
	logData.llsType = 'R';
	_stationId = estacaoId;
	_pin = pinSD_CS;
}

bool cremaLogClass::writeLog()
{
	File arquivoTXT;
	String logLine = _LOG_INI;
	
	SD.begin(_pin);
	arquivoTXT = SD.open(_fileName, FILE_WRITE);

	if (arquivoTXT) {

		logLine += logData.llsType;
		logLine += _LOG_SEP + String(logData.llsTime);
		logLine += _LOG_SEP + String(logData.llsAltitude, 2);
		logLine += _LOG_SEP + String(logData.llsHumidity, 2);
		logLine += _LOG_SEP + String(logData.llsPressure, 2);
		logLine += _LOG_SEP + String(logData.llstTemperature, 2);
		logLine += _LOG_END;

		arquivoTXT.println(logLine);
		arquivoTXT.close();

#ifdef DEBUG
#ifndef CREMA_WIFI
		cremaSerial.println(logLine);
#endif // !CREMA_WIFI
#endif // DEBUG
	}
	else {
#ifdef DEBUG
#ifndef CREMA_WIFI
		cremaSerial.println("Erro abrindo arquivo " + _fileName);
#endif // !CREMA_WIFI
#endif // DEBUG
	}
	
	return arquivoTXT;
}
