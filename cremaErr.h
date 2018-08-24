



#ifndef _cremaError_h
#define _cremaError_h


constexpr auto _RESTART = true;
constexpr auto _DONT_RESTART = false;
constexpr auto _SAVE_CONFIG = true;
constexpr auto _DONT_SAVE_CONFIG = false;


constexpr auto _ERR_NOERROR = 0;

constexpr auto _LOG_WIFI_CONFIG_SAVED = (10);

constexpr auto _ERR_NOT_CONTROLED_RESTART = (-50);

constexpr auto _ERR_SENSOR = (-70);
#define _ERR_SENSOR_READ					(_ERR_SENSOR - 1)
#define _ERR_SENSOR_INIT					(_ERR_SENSOR - 2)
#define _ERR_SENSOR_GPS_POOR_SIGNAL			(_ERR_SENSOR - 3)
#define _ERR_SENSOR_GPS_POOR_PRECISION		(_ERR_SENSOR - 4)
#define _ERR_SENSOR_GPS_ALTITUDE_NOT_MATCH  (_ERR_SENSOR - 5)
#define _ERR_I2C_BUS                        (_ERR_SENSOR - 6)

constexpr auto _ERR_SYSTEM_GPF = (-100);
#define _ERR_SYSTEM_CONTROLLED_HALT         (_ERR_SYSTEM_GPF - 1)
#define _ERR_SYSTEM_UNCONTROLLED_HALT       (_ERR_SYSTEM_GPF - 2)


typedef enum cremaErrorId {
	ceNoError,
	ceSensorRead,
	ceSensorInit,
	ceGPS_PoorSignal,
	ceGPS_PoorPrecision,
	ceUncrontrolledRestart,
	ceGPS_AltitudeNotMatched,
	ceControlledSystemHalt,
	ceUncontrolledSystemHalt,
	ceWiFiConfigError,
	ceI2CError,
	ceCount
} cremaErrorId;

typedef char cremaErrorDescription[30];
typedef char cremaSystemErrorDescription[256];

struct cremaErrorDef
{
	int code;
	cremaErrorDescription description;
};

const cremaErrorDef cremaErrors[ceCount] = {
{_ERR_NOERROR, "No error"},
{_ERR_SENSOR_READ, "Read sensor"},
{_ERR_SENSOR_INIT, "Sensor initialization"},
{_ERR_SENSOR_GPS_POOR_SIGNAL, "Poor GPS signal"},
{_ERR_SENSOR_GPS_POOR_PRECISION, "Poor GPS precision"},
{_ERR_NOT_CONTROLED_RESTART, "Uncontrolled restart"},
{_ERR_SENSOR_GPS_ALTITUDE_NOT_MATCH, "Altitude not matched yet"},
{_ERR_SYSTEM_CONTROLLED_HALT, "Controlled system halt"},
{_ERR_SYSTEM_UNCONTROLLED_HALT, "Uncontrolled system halt"},
{_LOG_WIFI_CONFIG_SAVED, "WiFi config error"},
{_ERR_I2C_BUS, "I2C sensor error"} };


#endif