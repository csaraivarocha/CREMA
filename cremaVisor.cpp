/*
Name:		cremaVisor.cpp
Created:	2/11/2018 11:30:18 AM
Author:	crist
Editor:	http://www.visualmicro.com
*/

#include "cremaVisor.h"

cremaVisorClass::cremaVisorClass()
{
	_col = _CREMA_VISOR_INI_COL;
	_row = _CREMA_VISOR_INI_ROW;

	_lcd.begin(_LCD_X, _LCD_Y);
	clear();
}

void cremaVisorClass::write(const String txt)
{
	//_lcd.setCursor(_col, _row);
	_lcd.print(txt);
	_col += txt.length();
}

void cremaVisorClass::write(const double dbl, const int digits)
{
	write(String(dbl, digits));
}

void cremaVisorClass::write(const int lng, const int digits)
{
	write(String(lng, digits));
}

void cremaVisorClass::write(const unsigned long lng, const int digits)
{
	write(String(lng, digits));
}

void cremaVisorClass::writeln(const String txt)
{
	write(txt);
	_lcd.println();
	_row += _addRow;
	if (_row < _CREMA_VISOR_MAX_ROW) {
		_lcd.setCursor(_CREMA_VISOR_INI_COL, _row);
	}
	_col = 0;
}

void cremaVisorClass::clear()
{
	_lcd.clear();
	_lcd.setCursor(_CREMA_VISOR_INI_COL, _CREMA_VISOR_INI_ROW);
	_row = 0;
	_col = 0;
}

void cremaVisorClass::clearLine(const int line)
{
	_lcd.setCursor(0, line);
	_lcd.clearLine();
	_lcd.setCursor(0, line);
}

void cremaVisorClass::showMessage(const String msg)
{
	_lcd.clear();
	_lcd.setCursor(1, 0);
	_lcd.print(msg);
}

