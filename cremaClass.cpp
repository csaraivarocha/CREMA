// 
// 
// 

#include "cremaClass.h"

cremaClass::cremaClass()
{
	visor = new cremaVisorClass();
	config = new cremaConfigClass();
	sensor = new cremaSensorClass();
	time = new cremaTimeClass();
}

cremaClass::~cremaClass()
{
	delete visor;
	delete config;
	delete sensor;
	delete time;
}

void cremaClass::init()
{
	visor->showMessage(F("Inicializando"));

	config->init();        // config.init tem que ser antes. para ler as configurações do arquivo
	
	_initWiFi();
	_wifi_autoConnect();
	
	if (!sensor->init())
	{
		_uploadErrorLog(ceSensorInit, _RESTART, _SAVE_CONFIG);
	}

	visor->clear();

	Serial_GPS.flush();
}

char * get_reset_reason(RESET_REASON reason)
{
	char * rtn;

	switch (reason)
	{
	case 1:  rtn = ("POWERON_RESET"); break;          /**<1,  Vbat power on reset*/
	case 3:  rtn = ("SW_RESET"); break;               /**<3,  Software reset digital core*/
	case 4:  rtn = ("OWDT_RESET"); break;             /**<4,  Legacy watch dog reset digital core*/
	case 5:  rtn = ("DEEPSLEEP_RESET"); break;        /**<5,  Deep Sleep reset digital core*/
	case 6:  rtn = ("SDIO_RESET"); break;             /**<6,  Reset by SLC module, reset digital core*/
	case 7:  rtn = ("TG0WDT_SYS_RESET"); break;       /**<7,  Timer Group0 Watch dog reset digital core*/
	case 8:  rtn = ("TG1WDT_SYS_RESET"); break;       /**<8,  Timer Group1 Watch dog reset digital core*/
	case 9:  rtn = ("RTCWDT_SYS_RESET"); break;       /**<9,  RTC Watch dog Reset digital core*/
	case 10: rtn = ("INTRUSION_RESET"); break;        /**<10, Instrusion tested to reset CPU*/
	case 11: rtn = ("TGWDT_CPU_RESET"); break;        /**<11, Time Group reset CPU*/
	case 12: rtn = ("SW_CPU_RESET"); break;           /**<12, Software reset CPU*/
	case 13: rtn = ("RTCWDT_CPU_RESET"); break;       /**<13, RTC Watch dog Reset CPU*/
	case 14: rtn = ("EXT_CPU_RESET"); break;          /**<14, for APP CPU, reseted by PRO CPU*/
	case 15: rtn = ("RTCWDT_BROWN_OUT_RESET"); break; /**<15, Reset when the vdd voltage is not stable*/
	case 16: rtn = ("RTCWDT_RTC_RESET"); break;       /**<16, RTC Watch dog reset digital core and rtc module*/
	default: rtn = ("NO_MEAN");
	}
	return rtn;
}


// esta função é chamada somente uma vês, no início do sistema, quando cria a classe cremaClass.
void cremaClass::treatLastError()
{
	cremaSystemErrorDescription _cpus_reset_reason;
	sprintf(_cpus_reset_reason, "CPU0=%s / CPU1=%s", get_reset_reason(rtc_get_reset_reason(0)), get_reset_reason(rtc_get_reset_reason(1)));

	if (config->Values[ccLastError].length() > 4) // maior que 4 indica conteúdo inválido, pois a quantidade maior de caracteres é 4 (-999)
	{
		config->setLastError(ceNoError);
	}
	else if (config->Values[ccLastError].toInt() == _ERR_SENSOR_READ)
	{
		delay(5000);
		// necessário fazer upload de novo valor para gerar trigger de evento no Ubidots
		_uploadErrorLog(ceNoError, _DONT_RESTART, _SAVE_CONFIG, _cpus_reset_reason);
	}
	else if (config->Values[ccLastError].toInt() == _ERR_NOERROR)  // uncrotoled restarted
	{
		_uploadErrorLog(ceUncrontrolledRestart, _DONT_RESTART, _DONT_SAVE_CONFIG, _cpus_reset_reason);
		_uploadErrorLog(ceNoError, _DONT_RESTART, _SAVE_CONFIG);
	}
	else
	{
		_uploadErrorLog(ceNoError, _DONT_RESTART, _SAVE_CONFIG);
	}
}

void cremaClass::ShowSensorValues()
{
	if (time->IsTimeToAction(caShowSensorValues)) {
	
		visor->clearLine(0);
		visor->write(sensor->Values[csTemperatura], sensor->Decimals[csTemperatura]);
		visor->write(F("C    "));

		visor->write(sensor->Values[csUmidade], sensor->Decimals[csUmidade]);
		visor->write(F("%"));

		if (_whatShow) {
			visor->clearLine(2);
			visor->write(sensor->Values[csPressao], sensor->Decimals[csPressao]);
			visor->write(F("mP  "));

			visor->write(sensor->Values[csAltitude], sensor->Decimals[csAltitude]);
			visor->write(F("m"));

			visor->clearLine(3);
		}
		else {
			visor->clearLine(2);
			visor->write(sensor->Values[csLuminosidade], sensor->Decimals[csLuminosidade]);
			visor->writeln(F(" luz"));

			visor->clearLine(3);
			visor->write(sensor->Values[csUltraVioleta], sensor->Decimals[csUltraVioleta]);
			visor->write(F(" uv"));
		}
		_whatShow = !_whatShow;
	}
}

void cremaClass::ShowDateTime()
{
	if (time->IsTimeToAction(caShowDateTime, true)) {
		time->readTime();
		if (_timeSep == ":") {
			_timeSep = ".";
		}
		else {
			_timeSep = ":";
		}

		visor->clearLine(5);
		visor->write(time->strDMY(F("/"), true, true, false));
		visor->write(F(" - "));
		visor->write(time->strHMS(_timeSep, true, true, false));
	}
}


void cremaClass::ReadSensors()
{
	if (time->IsTimeToAction(caReadSensors))
	{
		//// TODO: melhorar identificação do erro 
		if (!sensor->readSensors())   // false se erro na leitura de sensores (06/08/2018: Temp > 50)
		{
			_uploadErrorLog(ceI2CError, _DONT_RESTART, _DONT_SAVE_CONFIG);
			Wire.reset();
			if (!sensor->readSensors())
			{
				_uploadErrorLog(ceSensorRead, _RESTART, _SAVE_CONFIG);
			}
		}
	}
}

void cremaClass::uploadSystemHaltError(const cremaErrorId typeSystemHaltError, const cremaSystemErrorDescription sysErrorMsg)
{
	cremaSystemErrorDescription _logMsg;
	_uploadErrorLog(typeSystemHaltError, _RESTART, _SAVE_CONFIG, sysErrorMsg);
}

void cremaClass::_uploadErrorLog(const cremaErrorId error, const bool restart, const bool saveConfig, const cremaSystemErrorDescription sysErrorMsg)
{
	sensor->Values[csLog] = cremaErrors[error].code;                      // guarda valor do erro para subir para Ubidots
	_uploadToCloud(csLog, csLog, cremaErrors[error].description, sysErrorMsg);

	if (saveConfig)
	{
		config->setLastError(error);   // guarda erro no arquivo do ESP32 localmente para ser tratado na reinicialização
	}

	if (restart)
	{
		Restart();
	}
}

void cremaClass::_uploadToCloud(const cremaSensorsId first = csLuminosidade, const cremaSensorsId last = csUltraVioleta, const cremaErrorDescription desc, const cremaSystemErrorDescription sysErrorMsg)
{
	//// TODO: verificação de conexão antes da chamada
	if (!WiFi.isConnected()) 
	{
		config->setForceConfig(false);     // se necessário iniciar webServer, informar apenas dados de WiFi
		//// TODO: se saiu e não está conectado, reiniciar e gravar LastErro em Config.
		_wifi_autoConnect();  
	}

	sensor->publishHTTP(first, last, desc, sysErrorMsg);
}

//callback que indica que o ESP entrou no modo AP
void cremaClass::__wifi_configModeCallback(WiFiManager *myWiFiManager) 
{
	cremaClass::__displayConfigMode();
	cremaClass::__webServerConfigSaved = false;
}

//callback que indica que salvou as configurações de rede
void cremaClass::__wifi_saveConfigCallback() 
{
	cremaClass::__webServerConfigSaved = true;
}

void cremaClass::__displayConfigMode()
{
	cremaVisorClass v;
	
	v.showMessage(F("_CONFIGURACAO_"));
	v.clearLine(1);
	v.clearLine(2); v.write(F("Conect. a rede"));
	v.clearLine(3); v.write(F("   \"")); v.write(_CREMA_SSID_AP); v.write(F("\""));
	v.clearLine(4); v.write(F("pelo computador"));
	v.clearLine(5); v.write(F("ou celular"));
}

void cremaClass::_initWiFi()
{
	//wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT(); 
	//esp_wifi_init(&config);
	//esp_wifi_start();
	//delay(500);

	pinMode(CREMA_WiFi_Manager_PIN, INPUT);

	//utilizando esse comando, as configurações são apagadas da memória
	//caso tiver salvo alguma rede para conectar automaticamente, ela é apagada.
	//_wifiManager.resetSettings();

	//por padrão as mensagens de Debug vão aparecer no monitor serial, caso queira desabilitá-la
	//utilize o comando setDebugOutput(false);
	_wifiManager.setDebugOutput(false);

	//caso queira iniciar o Portal para se conectar a uma rede toda vez, sem tentar conectar 
	//a uma rede salva anteriormente, use o startConfigPortal em vez do autoConnect
	//  _wifiManager.startConfigPortal(char const *apName, char const *apPassword = NULL);

	//setar IP fixo para o ESP (deve-se setar antes do autoConnect)
	//  setAPStaticIPConfig(ip, gateway, subnet);
	//  _wifiManager.setAPStaticIPConfig(IPAddress(192,168,16,2), IPAddress(192,168,16,1), IPAddress(255,255,255,0)); //modo AP

	//  setSTAStaticIPConfig(ip, gateway, subnet);
	//  _wifiManager.setSTAStaticIPConfig(IPAddress(192,168,0,99), IPAddress(192,168,0,1), IPAddress(255,255,255,0)); //modo estação

	//callback para quando entra em modo de configuração AP
	_wifiManager.setAPCallback(__wifi_configModeCallback);

	//callback para quando se conecta em uma rede, ou seja, quando passa a trabalhar em modo estação
	_wifiManager.setSaveConfigCallback(__wifi_saveConfigCallback);

	//_wifiManager.autoConnect(_CREMA_SSID_AP, ""); //cria uma rede sem senha
	//_wifiManager.autoConnect(); //gera automaticamente o SSID com o chip ID do ESP e sem senha

	//  _wifiManager.setMinimumSignalQuality(10); // % minima para ele mostrar no SCAN

	//_wifiManager.setRemoveDuplicateAPs(false); //remover redes duplicadas (SSID iguais)
	//_wifiManager.resetSettings();
	_wifiManager.setConfigPortalTimeout(600); //timeout para o ESP nao ficar esperando para ser configurado para sempre
	_wifiManager.setConnectTimeout(15);
}

bool cremaClass::_wifi_autoConnect()
{
	if (!config->getConfigOk() || config->getForceConfig())
	{
		config->setForceConfig(false);

		WiFiManagerParameter * _wifiParam[ccCount];

		// The extra parameters to be configured (can be either global or just in the setup)
		// After connecting, parameter.getValue() will get you the configured value
		// id/name placeholder/prompt default length
		for (size_t i = 0; i < ccCount; i++)
		{
			cremaConfigId key = cremaConfigId(i);
			if (config->Imputable[key])
			{
				_wifiParam[key] = new WiFiManagerParameter(config->nameKeys[key].c_str(), config->descKeys[key].c_str(), config->Values[key].c_str(), _CREMA_CFG_VALUE_SIZE);
				//WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);

				//add all your parameters here
				_wifiManager.addParameter(_wifiParam[key]);
			}
		}
		_wifi_startWebServer();

		if (cremaClass::__webServerConfigSaved)
		{
			for (size_t i = 0; i < ccCount; i++)
			{
				cremaConfigId key = cremaConfigId(i);
				if (config->Imputable[key])
				{
					config->Values[key] = "";
					config->Values[key].concat(_wifiParam[key]->getValue());
				}
			}
			config->saveConfig();
		}

		for (size_t i = 0; i < ccCount; i++)
		{
			cremaConfigId key = cremaConfigId(i);
			if (config->Imputable[key])
			{
				delete _wifiParam[key];
			}
		}
	}
	else
	{
		visor->clearLine(2);
		visor->write(F("Conectando"));
		visor->clearLine(3);
		visor->write(F("WiFi..."));

		_wifiManager.autoConnect(_CREMA_SSID_AP, "");

		visor->clearLine(4);
		visor->write(_wifiManager.getSSID());
		delay(1500);
	}
	visor->clear();
}

void cremaClass::_wifi_startWebServer()
{
	if (!_wifiManager.startConfigPortal(_CREMA_SSID_AP)) {
		_uploadErrorLog(ceWiFiConfigError, _RESTART, _DONT_SAVE_CONFIG);
	}
	visor->clear();
}

void cremaClass::doGPS()
{
	_readGPS();
	_testGPSSignal();
}

void cremaClass::_readGPS()
{
	if (time->IsTimeToAction(caReadGPS))
	{
		sensor->readGPS();
	}
}

void cremaClass::_testGPSSignal()
{
	if (time->IsTimeToAction(caTestGPSSignal))
	{
		if (!sensor->gpsData.validLocation)
		{
			_uploadErrorLog(ceGPS_PoorSignal, _DONT_RESTART, _DONT_SAVE_CONFIG);
		}
		else if (!sensor->gpsData.validLocation)
		{
			_uploadErrorLog(ceGPS_AltitudeNotMatched, _DONT_RESTART, _DONT_SAVE_CONFIG);
		}
	}
}

//void __uploadSensorValues(void *parms)
 //{
	//IoT.publishHTTP(sensor, _IoT_Update_IniSensor, _IoT_Update_FimSensor);
	//vTaskDelete(NULL);
	//return;
 //}

void cremaClass::UploadSensorValues()
{
	if (time->IsTimeToAction(caUploadSensorsValues)) 
	{

#if defined(_DEBUG)
		byte a; // comentário para configurar o breakpoint de exibição da data por extenso.
#endif // _CREMA_DEBUG

		_uploadToCloud(_IoT_Update_IniSensor, _IoT_Update_FimSensor);
		//// TODO Ler sensor em outra task do processador
		// https://www.dobitaobyte.com.br/selecionar-uma-cpu-para-executar-tasks-com-esp32/
		//xTaskHandle * taskUploadSensorValue;
		//xTaskCreatePinnedToCore(&__uploadSensorValues, "UploadSensorValues", 2048, NULL, 1, taskUploadSensorValue, 1);
		// utilizar mensagem de debug =printf("{created task to upload: %d.", taskUploadSensorValue);
	}
}

void cremaClass::Restart()
{
	//IoT.publishHTTP(sensor, csLog, csLog);  // upload é feito pela função uploadErrorLog()
	esp_restart();
}


