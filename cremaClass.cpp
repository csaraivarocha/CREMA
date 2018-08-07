// 
// 
// 

#include "cremaClass.h"
cremaClass::cremaClass()
{
	_initWiFi();
}

void cremaClass::init()
{
	Serial.begin(115200);

	visor.showMessage(F("Inicializando"));
	config->init();        // config.init tem que ser antes. para ler as configurações do arquivo
	_wifi_autoConnect();
	treatLastError();
	if (!sensor->init())
	{
		_uploadErrorLog(_ERR_SENSOR_INIT, _ERR_UPLOAD_LOG_RESTART, _ERR_UPLOAD_LOG_SAVE_CONFIG);
	}

	visor.clear();

	Serial_GPS.flush();
}

// esta função é chamada somente uma vês, no início do sistema, quando cria a classe cremaClass.
void cremaClass::treatLastError()
{
	Serial.printf("Treating last error: %d\n", config->Values[ccLastError].toInt());

	for (size_t i = 0; i < ccCount; i++)
	{
		cremaConfigId key = (cremaConfigId)i;
		Serial.printf("%s=%s\n", config->nameKeys[key].c_str(), config->Values[key].c_str());
	}

	// maior que 4 indica conteúdo inválido, pois a quantidade maior de caracteres é 4 (-999)
	if (config->Values[ccLastError].length() > 4)
	{
		Serial.println(F("Invalid last error! (length > 4)\n"));
		config->setLastError(_ERR_NOERROR);
	}
	else if (config->Values[ccLastError].toInt() == _ERR_SENSOR_READ)
	{
		Serial.println(F("Sensor reading error. Wait 5 seconds to inicialize...\n"));
		delay(5000);
		// necessário fazer upload de novo valor para gerar trigger de evento no Ubidots
		_uploadErrorLog(_ERR_NOERROR, _ERR_UPLOAD_LOG_DONT_RESTART, _ERR_UPLOAD_LOG_SAVE_CONFIG);
	}
	else if (config->Values[ccLastError].toInt() == _ERR_NOERROR)
	{
		Serial.println(F("Uncrontoled restart.\n"));
		_uploadErrorLog(_ERR_NOT_CONTROLED_RESTART, _ERR_UPLOAD_LOG_DONT_RESTART, _ERR_UPLOAD_LOG_DONT_SAVE_CONFIG);
		config->setLastError(_ERR_NOERROR);
	}

	Serial.printf("\nNew error setted: %s\n", config->Values[ccLastError].c_str());
}

void cremaClass::ShowSensorValues()
{
	if (time->IsTimeToAction(caShowSensorValues)) {
	
		visor.clearLine(0);
		visor.write(sensor->Values[csTemperatura], sensor->Decimals[csTemperatura]);
		visor.write("C    ");

		visor.write(sensor->Values[csUmidade], sensor->Decimals[csUmidade]);
		visor.write("%");

		if (_whatShow) {
			visor.clearLine(2);
			visor.write(sensor->Values[csPressao], sensor->Decimals[csPressao]);
			visor.write("mP  ");

			visor.write(sensor->Values[csAltitude], sensor->Decimals[csAltitude]);
			visor.write("m");

			visor.clearLine(3);
		}
		else {
			visor.clearLine(2);
			visor.write(sensor->Values[csLuminosidade], sensor->Decimals[csLuminosidade]);
			visor.writeln(" luz");

			visor.clearLine(3);
			visor.write(sensor->Values[csUltraVioleta], sensor->Decimals[csUltraVioleta]);
			visor.write(" uv");
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

		visor.clearLine(5);
		visor.write(time->strDMY("/", true, true, false));
		visor.write(" - ");
		visor.write(time->strHMS(_timeSep, true, true, false));
	}
}

void cremaClass::ReadSensors()
{
	if (time->IsTimeToAction(caReadSensors))
	{
		if (!sensor->readSensors())   // retorna false se erro na leitura de sensores (06/08/2018: Temp > 50)
		{
			_uploadErrorLog(_ERR_SENSOR_READ, _ERR_UPLOAD_LOG_RESTART, _ERR_UPLOAD_LOG_SAVE_CONFIG);
		}
	}
}

void cremaClass::_uploadToCloud(const cremaSensorsId first = csLuminosidade, const cremaSensorsId last = csUltraVioleta)
{
	//todo: verificação de conexão antes da chamada
	if (!WiFi.isConnected()) {
		config->setForceConfig(false);     // se necessário iniciar webServer, informar apenas dados de WiFi
		_wifi_autoConnect();
	}

	sensor->publishHTTP(first, last);
}

//callback que indica que o ESP entrou no modo AP
void _wifi_configModeCallback(WiFiManager *myWiFiManager) {
	//  Serial.println("Entered config mode");
	Serial.println("Entrou no modo de configuracao");
	Serial.println(WiFi.softAPIP().toString()); //imprime o IP do AP
	Serial.println(myWiFiManager->getConfigPortalSSID()); //imprime o SSID criado da rede
	crema->displayConfigMode();
	crema->webServerConfigSaved = false;
}

//callback que indica que salvou as configurações de rede
void _wifi_saveConfigCallback() {
	Serial.println("Configuracao salva");
	Serial.println(WiFi.softAPIP().toString()); //imprime o IP do AP
	crema->webServerConfigSaved = true;
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
	_wifiManager.setAPCallback(_wifi_configModeCallback);

	//callback para quando se conecta em uma rede, ou seja, quando passa a trabalhar em modo estação
	_wifiManager.setSaveConfigCallback(_wifi_saveConfigCallback);

	//_wifiManager.autoConnect(_CREMA_SSID_AP, ""); //cria uma rede sem senha
	//_wifiManager.autoConnect(); //gera automaticamente o SSID com o chip ID do ESP e sem senha

	//  _wifiManager.setMinimumSignalQuality(10); // % minima para ele mostrar no SCAN

	//_wifiManager.setRemoveDuplicateAPs(false); //remover redes duplicadas (SSID iguais)
	//_wifiManager.resetSettings();
	_wifiManager.setConfigPortalTimeout(600); //timeout para o ESP nao ficar esperando para ser configurado para sempre
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

		// todo: utilizar variável global
		if (webServerConfigSaved)
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
		Serial.print("\nConectando ao ultimo WiFi\n");

		visor.clearLine(2);
		visor.write("Conectando");
		visor.clearLine(3);
		visor.write("WiFi...");

		_wifiManager.autoConnect(_CREMA_SSID_AP, "");

		visor.clearLine(4);
		visor.write(_wifiManager.getSSID());
		delay(1500);
	}
	visor.clear();
}

void cremaClass::_wifi_startWebServer()
{
	if (!_wifiManager.startConfigPortal(_CREMA_SSID_AP)) {
		Restart();
	}
	visor.clear();
}

void cremaClass::_uploadErrorLog(const int error, const bool restart, const bool saveConfig)
{
	sensor->Values[csLog] = error;                      // guarda valor do erro para subir para Ubidots
	_uploadToCloud(csLog, csLog);

	if (saveConfig)
	{
		config->setLastError(sensor->Values[csLog]);   // guarda erro no arquivo do ESP32 localmente para ser tratado na reinicialização
	}

	if (restart)
	{
		Restart();
	}
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
		if (!sensor->gpsData.valid)
		{
			_uploadErrorLog(_ERR_SENSOR_GPS_POOR_SIGNAL, _ERR_UPLOAD_LOG_DONT_RESTART, _ERR_UPLOAD_LOG_DONT_SAVE_CONFIG);
		}
	}
}

 void cremaClass::_sayDate()
{
	Serial.print("\n");
	Serial.print(time->strDateTimeExtenso());
	Serial.print("\n");
}



 //void __uploadSensorValues(void *parms)
 //{
	//IoT.publishHTTP(sensor, _IoT_Update_IniSensor, _IoT_Update_FimSensor);
	//vTaskDelete(NULL);
	//return;
 //}

void cremaClass::UploadSensorValues()
{
	if (time->IsTimeToAction(caUploadSensorsValues)) {
		_sayDate();
		{
			_uploadToCloud(_IoT_Update_IniSensor, _IoT_Update_FimSensor);
			// TODO Ler sensor em outra task do processador
			// https://www.dobitaobyte.com.br/selecionar-uma-cpu-para-executar-tasks-com-esp32/
			//xTaskHandle * taskUploadSensorValue;
			//xTaskCreatePinnedToCore(&__uploadSensorValues, "UploadSensorValues", 2048, NULL, 1, taskUploadSensorValue, 1);
			//Serial.printf("{created task to upload: %d.", taskUploadSensorValue);
		}
	}
}

void cremaClass::Restart()
{
	//IoT.publishHTTP(sensor, csLog, csLog);  // upload é feito pela função uploadErrorLog()
	esp_restart();
}

void cremaClass::displayConfigMode() {
	visor.showMessage("_CONFIGURACAO_");
	visor.clearLine(1);
	visor.clearLine(2); visor.write("Conect. a rede");
	visor.clearLine(3); visor.write("   \""); visor.write(_CREMA_SSID_AP); visor.write("\"");
	visor.clearLine(4); visor.write("pelo computador");
	visor.clearLine(5); visor.write("ou celular");
}
