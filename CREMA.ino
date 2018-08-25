#include "cremaClass.h"

#define CREMA_TECNICAL_DEBUG_NO

bool cremaClass::__webServerConfigSaved = false;


void setup()
{
#ifdef CREMA_TECNICAL_DEBUG_YES
	//cremaTime.setTime(0, 11, 18, 0, 23, 4, 1970);
	crema_I2C_config();
#endif // CREMA_TECNICAL_DEBUG_YES

	try
	{
		crema = new cremaClass();
		crema->init();
		crema->treatLastError();
	}
	catch (const cremaSystemErrorDescription msg)
	{
		crema->uploadSystemHaltError(ceControlledSystemHalt, msg);
	}
	catch (const std::exception& e)
	{
		cremaSystemErrorDescription msg;
		sprintf(msg, "%s", e.what());
		crema->uploadSystemHaltError(ceUncontrolledSystemHalt, msg);
	}
}

void loop()
{
	try
	{
		crema->ShowDateTime();
		crema->doGPS();
		crema->ReadSensors();
		crema->ShowSensorValues();
		crema->UploadSensorValues();
		delayMicroseconds(100);  // testing if watchdog don't crash by interrupted time
	}
	catch (const cremaSystemErrorDescription msg)
	{
		crema->uploadSystemHaltError(ceControlledSystemHalt, msg);
	}
	catch (const std::exception& e)
	{
		cremaSystemErrorDescription msg;
		sprintf(msg, "%s", e.what());
		crema->uploadSystemHaltError(ceUncontrolledSystemHalt, msg);
	}
	catch (...)
	{
		crema->uploadSystemHaltError(ceUncontrolledSystemHalt, "<UNCRONTOLLED EXCEPTION");
	}
}

