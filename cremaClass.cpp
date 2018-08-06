// 
// 
// 

#include "cremaClass.h"

/*
void ___ubidots_callback(char* topic, byte* payload, unsigned int length) {
	crema.serial->print("\nMensagem recebida [");
	crema.serial->print(topic);
	crema.serial->print("] ");
	for (int i = 0; i<length; i++) {
		crema.serial->print((char)payload[i]);
	}
}
*/

void cremaClass::init()
{
	visor->showMessage(F("Inicializando"));
	config->init();        // config.init tem que ser antes. para ler as configurações do arquivo
	wifi->autoConnect(config);
	treatLastError();
	sensor->readSensors();
	visor->clear();

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
		sensor->uploadErrorLog(_ERR_NOERROR, _ERR_UPLOAD_LOG_DONT_RESTART, _ERR_UPLOAD_LOG_SAVE_CONFIG);
	}
	else if (config->Values[ccLastError].toInt() == _ERR_NOERROR)
	{
		Serial.println(F("Uncrontoled restart.\n"));
		sensor->uploadErrorLog(_ERR_NOT_CONTROLED_RESTART, _ERR_UPLOAD_LOG_DONT_RESTART, _ERR_UPLOAD_LOG_DONT_SAVE_CONFIG);
		config->setLastError(_ERR_NOERROR);
	}

	Serial.printf("\nNew error setted: %s\n", config->Values[ccLastError].c_str());
}

void cremaClass::ShowSensorValues()
{
	if (time->IsTimeToAction(caShowSensorValues)) {
	
		visor->clearLine(0);
		visor->write(sensor->Values[csTemperatura], sensor->Decimals[csTemperatura]);
		visor->write("C    ");

		visor->write(sensor->Values[csUmidade], sensor->Decimals[csUmidade]);
		visor->write("%");

		if (_whatShow) {
			visor->clearLine(2);
			visor->write(sensor->Values[csPressao], sensor->Decimals[csPressao]);
			visor->write("mP  ");

			visor->write(sensor->Values[csAltitude], sensor->Decimals[csAltitude]);
			visor->write("m");

			visor->clearLine(3);
		}
		else {
			visor->clearLine(2);
			visor->write(sensor->Values[csLuminosidade], sensor->Decimals[csLuminosidade]);
			visor->writeln(" luz");

			visor->clearLine(3);
			visor->write(sensor->Values[csUltraVioleta], sensor->Decimals[csUltraVioleta]);
			visor->write(" uv");
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
		visor->write(time->strDMY("/", true, true, false));
		visor->write(" - ");
		visor->write(time->strHMS(_timeSep, true, true, false));
	}
}

void cremaClass::ReadSensors()
{
	if (time->IsTimeToAction(caReadSensors)) {
		sensor->readSensors();
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
			sensor->uploadErrorLog(_ERR_SENSOR_GPS_POOR_SIGNAL, _ERR_UPLOAD_LOG_DONT_RESTART, _ERR_UPLOAD_LOG_DONT_SAVE_CONFIG);
		}
	}
}

 void cremaClass::_sayDate()
{
	serial->print("\n");
	serial->print(time->strDateTimeExtenso());
	serial->print("\n");
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
			IoT->publishHTTP(sensor, _IoT_Update_IniSensor, _IoT_Update_FimSensor);
			// TODO Ler sensor em outra task do processador
			// https://www.dobitaobyte.com.br/selecionar-uma-cpu-para-executar-tasks-com-esp32/
			//xTaskHandle * taskUploadSensorValue;
			//xTaskCreatePinnedToCore(&__uploadSensorValues, "UploadSensorValues", 2048, NULL, 1, taskUploadSensorValue, 1);
			//Serial.printf("{created task to upload: %d.", taskUploadSensorValue);
		}
	}
}

void cremaClass::Restart(const bool force)
{
	if (force) {
		//IoT.publishHTTP(sensor, csLog, csLog);  // upload é feito pela função uploadErrorLog()
		esp_restart();
	}
}