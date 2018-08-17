#include "cremaClass.h"

#define CREMA_TECNICAL_DEBUG_NO

bool cremaClass::__webServerConfigSaved = false;


void setup()
{
#ifdef CREMA_TECNICAL_DEBUG_YES
	//cremaTime.setTime(0, 11, 18, 0, 23, 4, 1970);
	crema_I2C_config();
#endif // CREMA_TECNICAL_DEBUG_YES

	crema = new cremaClass();
	crema->init();
	crema->treatLastError();
}

void loop()
{
	crema->ShowDateTime();
	crema->doGPS();
	crema->ReadSensors();
	crema->ShowSensorValues();
	crema->UploadSensorValues();
}