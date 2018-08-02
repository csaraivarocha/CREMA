// cremaConfig.h

#ifndef _CREMACONFIG_h
#define _CREMACONFIG_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "FS.h"
#include "SPIFFS.h"
#include "ArduinoJson.h"

#include "cremaErr.h"
#include "cremaPinos.h"

#define _CREMA_CFG_FILE F("/crema.cfg")

#define _CREMA_CFG_DEFAULT_SERVER F("http://things.ubidots.com")
#define _CREMA_CFG_DEFAULT_API F("/api/v1.6/devices/")

#define _CREMA_CFG_VALUE_SIZE 45
#define _CREMA_CFG_INVALID_VALUE F("<_none_>")
#define _CREMA_CFG_QTDE_ALEATORIO 3

//#define _CREMA_CFG_GROUP_LIC "lic"
//#define _CREMA_CFG_GROUP_CFG "grp"
//#define _CREMA_CFG_GROUP_RUN "run"

typedef enum cremaConfigId {
	ccCremaId,
	ccToken,
	ccServer,
	ccApi,
	ccLastError,
	ccCount,
} cremaConfigId;

//typedef char CremaConfigItem[_CREMA_CFG_VALUE_SIZE + 1];
typedef String CremaConfigItem;

class cremaConfigClass
{
protected:

	//char* _group[ccCount] = { _CREMA_CFG_GROUP_LIC, _CREMA_CFG_GROUP_LIC, _CREMA_CFG_GROUP_CFG, _CREMA_CFG_GROUP_CFG, _CREMA_CFG_GROUP_RUN };
	 bool _criptografa[ccCount] = { true,true,false,false,false };
	 String _defaultValue[ccCount] = { _CREMA_CFG_INVALID_VALUE, _CREMA_CFG_INVALID_VALUE, _CREMA_CFG_DEFAULT_SERVER,_CREMA_CFG_DEFAULT_API, "" };
	 String _encode(const cremaConfigId key);
	 String _decode(const cremaConfigId key, const String value);
	 bool _okConfig = false;
	 bool _forceConfig = false;
public:
	cremaConfigClass();
	void init();
	CremaConfigItem Values[ccCount];
	String nameKeys[ccCount] = { F("cremaid"), F("token"), F("server"), F("api"), F("err") };
	String descKeys[ccCount] = { F("CREMA Id: nome único da sua estação meteorológica"), F("Número da licença recebida por e-mail"), F("[SYS] Nome do servidor"), F("[SYS] Inteface de comunicação"), F("Último erro controlado") };
	bool Imputable[ccCount] = { true,true,true,true,false };

	bool readConfig();
	bool saveConfig();
	bool getConfigOk();            // se a configuração está feita e correta
	void setForceConfig(const bool set);
	bool getForceConfig();        // para forçar nova leitura de configuração
	void setLastError(const int error);
};


#endif

