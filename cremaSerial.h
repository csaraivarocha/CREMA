//#pragma once


#ifndef _cremaSerial_h
#define _cremaSerial_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif


#define CREMA_DEBUG


class cremaSerialClass
{
private:
	void _print(const String cSay, const bool newLine);
public:
	cremaSerialClass(const int bound = 115200);
	~cremaSerialClass();
	void print(const String cSay);
	void print(const float value, const byte decimals);
	void print(const char value);
	void println(const String cSay);
	void println(const float value, const byte decimals);
	void wait(const byte seconds);
};



#endif