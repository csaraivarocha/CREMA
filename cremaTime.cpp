/*
 Name:		cremaTime.cpp
 Created:	2/17/2018 11:05:21 AM
 Author:	crist
 Editor:	http://www.visualmicro.com
*/

#include "cremaTime.h"

cremaTimeClass::cremaTimeClass()
{
	rtc.begin();
	readTime();
	for (int i = 0; i < caCount; i++) {
		cremaActions a = cremaActions(i);
		_lastAction[a] = millis() - 10000;  // incializa com -10 segundos. para executar ações na primeira passada
	}
}

String cremaTimeClass::_addStr(byte v)
{
	String rtn = "";
	if (v < 10) {
		rtn = F("0");
	}
	rtn += String(v);
	return rtn;
}


void cremaTimeClass::setTime(uint8_t second, uint8_t minute, uint8_t hour, uint8_t dayOfWeek, uint8_t dayOfMonth, uint8_t month, uint8_t year)
{
	// sets time and date data to DS3231
	rtc.adjust(DateTime(year, month, dayOfMonth, hour, minute, second));
	readTime();
}

void cremaTimeClass::readTime()
{
	DateTimeSaved = rtc.now();
}

#if defined(_DEBUG)
String cremaTimeClass::NomeDiaDaSemana()
{
	switch (DateTimeSaved.dayOfTheWeek()) {
	case 0: return(F("Domingo"));
	case 1: return(F("Segunda-feira"));
	case 2: return(F("Terça-feira"));
	case 3: return(F("Quarta-feira"));
	case 4: return(F("Quinta-feira"));
	case 5: return(F("Sexta-feira"));
	case 6: return(F("Sábado"));
	default: return(F("?"));
	}
}
#endif

String cremaTimeClass::strDMY(const String sep, const bool lDay, const bool lMonth, const bool lYear)
{
	String lt = "";

	if (lDay) {
		lt += _addStr(DateTimeSaved.day());
	};

	if (lMonth)
	{
		if (lt != "") {
			lt += sep;
		};
		lt += _addStr(DateTimeSaved.month());
	}

	if (lYear)
	{
		if (lt != "") {
			lt += sep;
		};
		lt += String(DateTimeSaved.year());
	}
	return lt;
}

String cremaTimeClass::strHMS(const String sep, const bool lHour, const bool lMinute, const bool lSecond)
{
	String lt = "";

	if (lHour) {
		lt += _addStr(DateTimeSaved.hour());
	};

	if (lMinute)
	{
		if (lt != "") {
			lt += sep;
		};
		lt += _addStr(DateTimeSaved.minute());
	};

	if (lSecond)
	{
		if (lt != "") {
			lt += sep;
		};
		lt += _addStr(DateTimeSaved.second());
	}

	return lt;
}

#if defined(_DEBUG)
String cremaTimeClass::strDateTimeExtenso(const bool lTime)
{
	String rtn = "";

	rtn += NomeDiaDaSemana();
	rtn += ", ";

	rtn += DateTimeSaved.day();
	rtn += F(" de ");
	rtn += NomeMes();
	rtn += F(" de ");
	rtn += DateTimeSaved.year();
	rtn += F(". ");

	if (lTime) {
		rtn += DateTimeSaved.hour();
		rtn += F("H ");
		rtn += DateTimeSaved.minute();
		rtn += F("min ");
		rtn += DateTimeSaved.second();
		rtn += F("seg.");
	}
	return rtn;
}
#endif

bool cremaTimeClass::IsTimeToAction(cremaActions action, const bool showDot)
{
	unsigned long m = millis() / 1000;

	if (m < _lastAction[action]) {
		// reset controls. because millis() will overflow (go back to zero), after approximately 50 days.
		// this peace of code treats this situation
		_lastAction[action] = 0;
	}

	if ((m - _lastAction[action]) >= _actionTime[action]) {
		_lastAction[action] = m;
		return true;
	}

	return false;
}

#if defined(_DEBUG)
String cremaTimeClass::NomeMes()
{
	switch (DateTimeSaved.month()) {
	case 1: return(F("Janeiro"));
	case 2: return(F("Fevereiro"));
	case 3: return(F("Março"));
	case 4: return(F("Abril"));
	case 5: return(F("Maio"));
	case 6: return(F("Junho"));
	case 7: return(F("Julho"));
	case 8: return(F("Agosto"));
	case 9: return(F("Setembro"));
	case 10: return(F("Outubro"));
	case 11: return(F("Novembro"));
	case 12: return(F("Dezembro"));
	default: return(F("?"));
	}
}
#endif

