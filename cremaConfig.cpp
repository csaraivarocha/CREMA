// 
// 
// 

#include "cremaConfig.h"

cremaConfigClass::cremaConfigClass()
{
	_defaultValue[ccLastError] = F("");
	_defaultValue[ccLastError].concat(_ERR_NOERROR);

	Values[ccLastError] = F("");
	Values[ccLastError].concat(_ERR_NOERROR);
}

void cremaConfigClass::init()
{
	if (!SPIFFS.begin()) 
	{
		return;
	}

	_okConfig = readConfig();
	setForceConfig(getForceConfig() || (digitalRead(CREMA_WiFi_Manager_PIN) == HIGH));
}


bool cremaConfigClass::readConfig() 
{
	bool rtn = false;

	if (!SPIFFS.exists(_CREMA_CFG_FILE))
	{
		//// TODO: criar aquivo padrão  //utilizar mensagem de debug.println(F("> arquivo nao existe."));
	}
	else
	{
		File configFile = SPIFFS.open(_CREMA_CFG_FILE);
		if (!configFile)
		{
			//// TODO: utilizar padrão  //utilizar mensagem de debug.println(F("> falha ao abrir para leitura."));
		}
		else
		{
			// Allocate the memory pool on the stack.
			// Don't forget to change the capacity to match your JSON document.
			// Use arduinojson.org/assistant to compute the capacity.
			StaticJsonBuffer<512> jsonBuffer;

			// Parse the root object
			JsonObject &root = jsonBuffer.parseObject(configFile);

			if (!root.success())
			{
				//// TODO: utilizar configuração padrão  // utilizar mensagem de debug.println(F("> conteudo invalido do arquivo."));
			}
			else
			{
				size_t i;

				// Copy values from the JsonObject to the Config
				for (i = 0; i < ccCount; i++)
				{
					cremaConfigId key = (cremaConfigId)i;
					//// TODO utilizar agrupamento no JSON
					//strcpy(Values[key], _decode(key, root[_group[key]][_nameKeys[key]] | _CREMA_CFG_INVALID_VALUE));
					try
					{
						Values[key] = F("");
						Values[key].concat(_decode(key, root[nameKeys[key].c_str()]));
						//Values[key] = _decode(key, root[nameKeys[key]] | _CREMA_CFG_INVALID_VALUE);
					}
					catch (const std::exception&)
					{
						Values[key].concat(_defaultValue[key]);
					}
				}

				rtn = true;
				for (i = 0; i < ccCount; i++)
				{
					if (Values[cremaConfigId(i)].equals(_CREMA_CFG_INVALID_VALUE))
					{
						rtn = false;
						break;
					}
				}
			}

			configFile.close();
		}
	}
	return rtn;
}

bool cremaConfigClass::saveConfig()
{
	DynamicJsonBuffer jsonBuffer;
	JsonObject& json = jsonBuffer.createObject();

	for (size_t i = 0; i < ccCount; i++)
	{
		cremaConfigId key = (cremaConfigId)i;
		//json[_group[key]][_nameKeys[key]] = _encode(key);
		//json[nameKeys[key]] = Values[key];
		String e = _encode(key);
		json[nameKeys[key]] = e;
	}

	File configFile = SPIFFS.open(_CREMA_CFG_FILE, "w");
	if (!configFile) {
		//// TODO: utilizar config default   // utilizar mensagem de debug.println(F("> falha ao abrir arquivo para escrita."));
	}
	//json.printTo(Serial);
	json.printTo(configFile);
	configFile.close();
}

String cremaConfigClass::_encode(const cremaConfigId key)
{
	String crip = F("");
	srand(millis());    // configura a semente inicial do random

	if (_criptografa[key] && (Values[key].operator!= _CREMA_CFG_INVALID_VALUE))
	{
		// coloca três números aleatórios no início. para confundir.
		for (size_t i = 0; i < _CREMA_CFG_QTDE_ALEATORIO; i++)
		{
			byte x = rand() % 90 + 1;  // obtém número aleatório menor que 90
			if (x < 33)                // menor que 33 é um caracter não recomendado para este propósito
			{
				x += 33;
			}
			crip.concat(char(x));
		}

		// copia para crip os valores de value, porém, o último valor vem primeiro (inverte a string)
		for (size_t i = Values[key].length(); i > 0; i--)
		{
			crip.concat(char(Values[key].charAt(i-1) + 1));
		}
	}
	else
	{
		crip.concat(Values[key].c_str());
	}

	return crip;
}

String cremaConfigClass::_decode(const cremaConfigId key, const String value)
{
	String crip = F("");

	if (_criptografa[key] && (value.operator!= _CREMA_CFG_INVALID_VALUE))
	{
		// copia para crip os valores de value, porém, o último valor vem primeiro (inverte a string)
		for (size_t i = value.length(); i > _CREMA_CFG_QTDE_ALEATORIO; i--)
		{
			crip.concat(char(value.charAt(i-1) - 1));
		}
	}
	else
	{
		crip.concat(value.c_str());
	}
	
	return crip;
}

bool cremaConfigClass::getConfigOk()
{
	return _okConfig;
}

void cremaConfigClass::setForceConfig(const bool set)
{
	_forceConfig = set;
}

bool cremaConfigClass::getForceConfig()
{
	return _forceConfig;
}

void cremaConfigClass::setLastError(const cremaErrorId error)
{
	Values[ccLastError] = F("");
	Values[ccLastError].concat(cremaErrors[error].code);
	saveConfig();
}
