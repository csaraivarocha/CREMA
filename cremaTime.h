/*
 Name:		cremaTime.h
 Created:	2/17/2018 11:05:21 AM
 Author:	crist
 Editor:	http://www.visualmicro.com

 help, see code at http://www.esp32learning.com/code/esp32-and-ds3231-rtc-example.php
*/

#ifndef _cremaTime_h
#define _cremaTime_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "RTClib.h"

typedef enum cremaActions {
	caShowSensorValues,
	caShowDateTime,
	caReadSensors,
	caUploadSensorsValues,
	caReadGPS,
	caTestGPSSignal,
	caCount
} cremaActions;	


class cremaTimeClass
{
private:
	String _addStr(byte v);
	RTC_DS3231 rtc;
	long _lastAction[caCount] = { 0,0,0,0,0,0 };             // última vez que foi executada a ação
	const long _actionTime[caCount] = { 4,1,5,15,2,180 };    // tempo em segundos para executar cada ação

public:
	cremaTimeClass();
	void setTime(uint8_t second, uint8_t minute, uint8_t hour, uint8_t dayOfWeek, uint8_t dayOfMonth, uint8_t month, uint8_t year);
	void readTime();

#if (_VMDEBUG == 1)
	String strDateTimeExtenso(const bool lTime = true);
	String NomeDiaDaSemana();
	String NomeMes();
#endif

	String strDMY(const String sep = "/", const bool lDay = true, const bool lMonth = true, const bool lYear = true);
	String strHMS(const String sep = ":", const bool lHour = true, const bool lMinute = true, const bool lSecond = true);
	DateTime DateTimeSaved;
	bool IsTimeToAction(cremaActions action, const bool showDot = false);
};

#endif