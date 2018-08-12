



#ifndef _cremaError_h
#define _cremaError_h


#define _ERR_UPLOAD_LOG_RESTART				true
#define _ERR_UPLOAD_LOG_DONT_RESTART		false
#define _ERR_UPLOAD_LOG_SAVE_CONFIG			true
#define _ERR_UPLOAD_LOG_DONT_SAVE_CONFIG	false


#define _ERR_NOERROR					0

#define _ERR_SENSOR				       (-70)
#define _ERR_SENSOR_READ	           (_ERR_SENSOR - 1)
#define _ERR_SENSOR_INIT	           (_ERR_SENSOR - 2)
#define _ERR_SENSOR_GPS_POOR_SIGNAL    (_ERR_SENSOR - 3)
#define _ERR_SENSOR_GPS_POOR_PRECISION (_ERR_SENSOR - 4)

#define _ERR_NOT_CONTROLED_RESTART     (_ERR_SENSOR - 10)

typedef enum cremaErrorId {
	ceNoError,
	ceSensorRead,
	ceSensorInit,
	ceGPS_PoorSignal,
	ceGPS_PoorPrecision,
	ceUncrontrolledRestart,
	ceCount
} cremaErrorId;

typedef char cremaErroDescription[30];

struct cremaErrorDef
{
	int code;
	cremaErroDescription description;
};

const cremaErrorDef cremaErrors[ceCount] = {
{ _ERR_NOERROR, "No error"},
{_ERR_SENSOR_READ, "Read sensor"},
{_ERR_SENSOR_INIT, "Sensor initialization"},
{_ERR_SENSOR_GPS_POOR_SIGNAL, "Poor GPS signal"},
{_ERR_SENSOR_GPS_POOR_PRECISION, "Poor GPS precision"},
{_ERR_NOT_CONTROLED_RESTART, "Uncontrolled restarted"} };

#endif