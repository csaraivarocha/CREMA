/*
 Name:		cremaTime.cpp
 Created:	2/17/2018 11:05:21 AM
 Author:	crist
 Editor:	http://www.visualmicro.com
*/

#include "cremaTime.h"
#include "cremaClass.h"

cremaTimeClass::cremaTimeClass()
{
	rtc.begin();
	readTime();
	for (int i = 0; i < caCount; i++) {
		cremaActions a = cremaActions(i);
		_lastAction[a] = millis() - 10000;  // incializa com -10 segundos. para executar ações na primeira passada
		_actionDotsDisplayed[a] = 0;
		_actionLastDot[a] = 0;
	}
}

String cremaTimeClass::_addStr(byte v)
{
	String rtn = "";
	if (v < 10) {
		rtn = "0";
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

String cremaTimeClass::NomeDiaDaSemana()
{
	switch (DateTimeSaved.dayOfTheWeek()) {
	case 0: return("Domingo");
	case 1: return("Segunda-feira");
	case 2: return("Terça-feira");
	case 3: return("Quarta-feira");
	case 4: return("Quinta-feira");
	case 5: return("Sexta-feira");
	case 6: return("Sábado");
	default: return("?");
	}
}

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

String cremaTimeClass::strDateTimeExtenso(const bool lTime)
{
	String rtn = "";

	rtn += NomeDiaDaSemana();
	rtn += ", ";

	rtn += DateTimeSaved.day();
	rtn += " de ";
	rtn += NomeMes();
	rtn += " de ";
	rtn += DateTimeSaved.year();
	rtn += ". ";

	if (lTime) {
		rtn += DateTimeSaved.hour();
		rtn += "H ";
		rtn += DateTimeSaved.minute();
		rtn += "min ";
		rtn += DateTimeSaved.second();
		rtn += "seg.";
	}
	return rtn;
}

bool cremaTimeClass::IsTimeToAction(cremaActions action, const bool showDot)
{
	unsigned long m = millis() / 1000;
/*
	Serial.print(action);
	//Serial.print("\nDateTimeSaved.seconstime()=");
	Serial.print(" ");
	Serial.print(m);
	//Serial.print("\n_lastAction[action]=");
	Serial.print(" ");
	Serial.print(_lastAction[action]);
	//Serial.print("\n_actionTime[action]=");
	Serial.print(" ");
	Serial.print(_actionTime[action]);
	//Serial.print("\nsubtracao=");
	Serial.print(" (");
	Serial.print(m - _lastAction[action]);
*/
	if (m < _lastAction[action]) {
		// reset controls. because millis() will overflow (go back to zero), after approximately 50 days.
		// this peace of code treats this situation
		_lastAction[action] = 0;
		_actionLastDot[action] = 0;
	}

	if ((m - _lastAction[action]) >= _actionTime[action]) {
		_lastAction[action] = m;
		_actionDotsDisplayed[action] = 0;
		//Serial.print(" [run]\n");
		return true;
	}
	if ((showDot) && (m - _actionLastDot[action] >= 1)) {
		crema.serial->print(".");
		_actionDotsDisplayed[action]++;
		_actionLastDot[action] = m;
	}
	//cremaSerial.print(" [wait]\n");
	return false;
}

String cremaTimeClass::NomeMes()
{
	switch (DateTimeSaved.month()) {
	case 1: return("Janeiro");
	case 2: return("Fevereiro");
	case 3: return("Março");
	case 4: return("Abril");
	case 5: return("Maio");
	case 6: return("Junho");
	case 7: return("Julho");
	case 8: return("Agosto");
	case 9: return("Setembro");
	case 10: return("Outubro");
	case 11: return("Novembro");
	case 12: return("Dezembro");
	default: return("?");
	}
};
