/*
Name:		cremaVisor.h
Created:	2/11/2018 11:30:18 AM
Author:		crist
Editor:	http://www.visualmicro.com
*/

#ifndef _CREMAVISOR_h
#define _CREMAVISOR_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <PCD8544.h>
#include "cremaPinos.h"

#define _LCD_X     84
#define _LCD_Y     48
#define _CREMA_VISOR_INI_COL 1
#define _CREMA_VISOR_INI_ROW 0
#define _CREMA_VISOR_MAX_ROW 6

extern uint8_t SmallFont[];

class cremaVisorClass
{
private:
	unsigned int _contraste = 57;
	const unsigned int _addRow = 1;
	int _row, _col = 0;
public:
	cremaVisorClass();
	PCD8544 _lcd = PCD8544(_PIN_SCLK, _PIN_SDIN, _PIN_DC, _PIN_RESET, _PIN_SCE);
	void write(const String txt);
	void write(const double dbl, const int digits = 0);
	void write(const int lng, const int digits = 0);
	void write(const unsigned long lng, const int digits = 0);
	void writeln(const String txt = "");
	void clear();
	void clearLine(const int line);
	void showMessage(const String msg);
};

#endif

