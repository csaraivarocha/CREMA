#include "cremaSerial.h"



cremaSerialClass::cremaSerialClass(const int bound)
{
	Serial.begin(bound);
	delay(1000); //Aguarda 1 seg cujo intento é aguardar a Serial ficar disponível.
}


cremaSerialClass::~cremaSerialClass()
{
}

void cremaSerialClass::_print(const String cSay, const bool newLine = false) 
{
#ifdef CREMA_DEBUG
	Serial.print(cSay);
	if (newLine) {
		Serial.println();
	}
#endif // CREMA_DEBUG

}

void cremaSerialClass::print(const String cSay) 
{
	_print(cSay);
}

void cremaSerialClass::print(const float value, const byte decimals = 2)
{
	print(String(value, decimals));
}

void cremaSerialClass::print(const char value)
{
	print(String(value));
}

void cremaSerialClass::println(const String cSay)
{
	_print(cSay, true);
}

void cremaSerialClass::println(const float value, const byte decimals)
{
	println(String(value, decimals));
}

void cremaSerialClass::wait(const byte seconds) 
{
	_print("[");
	for (size_t i = 0; i < seconds; i++) {
		delay(1000);
		_print(".");
	}
	_print("]", true);
}