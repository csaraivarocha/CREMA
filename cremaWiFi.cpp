#include "cremaWiFi.h"
#include "cremaClass.h"


//callback que indica que o ESP entrou no modo AP
void configModeCallback(WiFiManager *myWiFiManager) {
	//  Serial.println("Entered config mode");
	Serial.println("Entrou no modo de configuracao");
	Serial.println(WiFi.softAPIP().toString()); //imprime o IP do AP
	Serial.println(myWiFiManager->getConfigPortalSSID()); //imprime o SSID criado da rede
	crema.displayConfigMode();
	g_webServerConfigSaved = false;
}

void saveConfigCallback() {
	Serial.println("Configuracao salva");
	Serial.println(WiFi.softAPIP().toString()); //imprime o IP do AP
	g_webServerConfigSaved = true;
}


cremaWiFiClass::cremaWiFiClass()
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
	_wifiManager.setAPCallback(configModeCallback);

	//callback para quando se conecta em uma rede, ou seja, quando passa a trabalhar em modo estação
	_wifiManager.setSaveConfigCallback(saveConfigCallback);

	//_wifiManager.autoConnect(_CREMA_SSID_AP, ""); //cria uma rede sem senha
	//_wifiManager.autoConnect(); //gera automaticamente o SSID com o chip ID do ESP e sem senha

	//  _wifiManager.setMinimumSignalQuality(10); // % minima para ele mostrar no SCAN

	//_wifiManager.setRemoveDuplicateAPs(false); //remover redes duplicadas (SSID iguais)
	//_wifiManager.resetSettings();
	_wifiManager.setConfigPortalTimeout(600); //timeout para o ESP nao ficar esperando para ser configurado para sempre
}


cremaWiFiClass::~cremaWiFiClass()
{
	//
}

bool cremaWiFiClass::autoConnect(cremaConfigClass * config)
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
		startWebServer();

		// todo: utilizar variável global
		if (g_webServerConfigSaved)
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

		crema.visor->clearLine(2);
		crema.visor->write("Conectando");
		crema.visor->clearLine(3);
		crema.visor->write("WiFi...");

		_wifiManager.autoConnect(_CREMA_SSID_AP, "");

		crema.visor->clearLine(4);
		crema.visor->write(_wifiManager.getSSID());
		delay(1500);
	}
	crema.visor->clear();
}

bool cremaWiFiClass::startWebServer()
{
	if (!_wifiManager.startConfigPortal(_CREMA_SSID_AP)) {
		crema.Restart();
	}
	crema.visor->clear();

	return true;
}

bool cremaWiFiClass::connected()
{
	return WiFi.isConnected();
}
