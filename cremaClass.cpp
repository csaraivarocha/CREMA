// 
// 
// 

#include "cremaClass.h"

/*
void ___ubidots_callback(char* topic, byte* payload, unsigned int length) {
	crema.serial.print("\nMensagem recebida [");
	crema.serial.print(topic);
	crema.serial.print("] ");
	for (int i = 0; i<length; i++) {
		crema.serial.print((char)payload[i]);
	}
}
*/

void cremaClass::init()
{
	crema.visor.showMessage(F("Inicializando"));
	crema.config.init();        // config.init tem que ser antes. para ler as configurações do arquivo
	crema.wifi.autoConnect(crema.config);
	crema.treatLastError();
	crema.sensor.readSensors();
	crema.visor.clear();

	Serial_GPS.flush();
}

// esta função é chamada somente uma vês, no início do sistema, quando cria a classe cremaClass.
void cremaClass::treatLastError()
{
	Serial.printf("Treating last error: %d\n", crema.config.Values[ccLastError].toInt());

	for (size_t i = 0; i < ccCount; i++)
	{
		cremaConfigId key = (cremaConfigId)i;
		Serial.printf("%s=%s\n", crema.config.nameKeys[key].c_str(), crema.config.Values[key].c_str());
		Serial.printf("%s=%s\n", config.nameKeys[key].c_str(), config.Values[key].c_str());
	}

	// maior que 4 indica conteúdo inválido, pois a quantidade maior de caracteres é 4 (-999)
	if (crema.config.Values[ccLastError].length() > 4)
	{
		Serial.println(F("Invalid last error! (length > 4)\n"));
		crema.config.setLastError(_ERR_NOERROR);
	}
	else if (crema.config.Values[ccLastError].toInt() == _ERR_SENSOR_READ)
	{
		Serial.println(F("Sensor reading error. Wait 5 seconds to inicialize...\n"));
		delay(5000);
		// necessário fazer upload de novo valor para gerar trigger de evento no Ubidots
		crema.sensor.uploadErrorLog(_ERR_NOERROR, _ERR_UPLOAD_LOG_DONT_RESTART, _ERR_UPLOAD_LOG_SAVE_CONFIG);
	}
	else if (config.Values[ccLastError].toInt() == _ERR_NOERROR)
	{
		Serial.println(F("Uncrontoled restart.\n"));
		crema.sensor.uploadErrorLog(_ERR_NOT_CONTROLED_RESTART, _ERR_UPLOAD_LOG_DONT_RESTART, _ERR_UPLOAD_LOG_DONT_SAVE_CONFIG);
		crema.config.setLastError(_ERR_NOERROR);
	}

	Serial.printf("\nNew error setted: %s\n", crema.config.Values[ccLastError].c_str());
}

void cremaClass::ShowSensorValues()
{
	if (crema.time.IsTimeToAction(caShowSensorValues)) {
	
		crema.visor.clearLine(0);
		crema.visor.write(crema.sensor.Values[csTemperatura], crema.sensor.Decimals[csTemperatura]);
		crema.visor.write("C    ");

		crema.visor.write(crema.sensor.Values[csUmidade], crema.sensor.Decimals[csUmidade]);
		crema.visor.write("%");

		if (_whatShow) {
			crema.visor.clearLine(2);
			crema.visor.write(crema.sensor.Values[csPressao], crema.sensor.Decimals[csPressao]);
			crema.visor.write("mP  ");

			crema.visor.write(crema.sensor.Values[csAltitude], crema.sensor.Decimals[csAltitude]);
			crema.visor.write("m");

			crema.visor.clearLine(3);
		}
		else {
			crema.visor.clearLine(2);
			crema.visor.write(crema.sensor.Values[csLuminosidade], crema.sensor.Decimals[csLuminosidade]);
			crema.visor.writeln(" luz");

			crema.visor.clearLine(3);
			crema.visor.write(crema.sensor.Values[csUltraVioleta], crema.sensor.Decimals[csUltraVioleta]);
			crema.visor.write(" uv");
		}
		_whatShow = !_whatShow;
	}
}

void cremaClass::ShowDateTime()
{
	if (crema.time.IsTimeToAction(caShowDateTime, true)) {
		crema.time.readTime();
		if (_timeSep == ":") {
			_timeSep = ".";
		}
		else {
			_timeSep = ":";
		}

		crema.visor.clearLine(5);
		crema.visor.write(crema.time.strDMY("/", true, true, false));
		crema.visor.write(" - ");
		crema.visor.write(crema.time.strHMS(_timeSep, true, true, false));
	}
}

void cremaClass::ReadSensors()
{
	if (crema.time.IsTimeToAction(caReadSensors)) {
		crema.sensor.readSensors();
	}
}

void cremaClass::doGPS()
{
	_readGPS();
	_testGPSSignal();
}

void cremaClass::_readGPS()
{
	if (crema.time.IsTimeToAction(caReadGPS))
	{
		crema.sensor.readGPS();
	}
}

void cremaClass::_testGPSSignal()
{
	if (crema.time.IsTimeToAction(caTestGPSSignal))
	{
		if (!crema.sensor.gpsData.valid)
		{
			crema.sensor.uploadErrorLog(_ERR_SENSOR_GPS_POOR_SIGNAL, _ERR_UPLOAD_LOG_DONT_RESTART, _ERR_UPLOAD_LOG_DONT_SAVE_CONFIG);
		}
	}
}

 void sayDate() 
{
	crema.serial.print("\n");
	crema.serial.print(crema.time.strDateTimeExtenso());
	crema.serial.print("\n");
}



 //void __uploadSensorValues(void *parms)
 //{
	//crema.IoT.publishHTTP(crema.sensor, _IoT_Update_IniSensor, _IoT_Update_FimSensor);
	//vTaskDelete(NULL);
	//return;
 //}

void cremaClass::UploadSensorValues()
{
	if (crema.time.IsTimeToAction(caUploadSensorsValues)) {
		sayDate();
		{
			crema.IoT.publishHTTP(crema.sensor, _IoT_Update_IniSensor, _IoT_Update_FimSensor);
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
		//crema.IoT.publishHTTP(crema.sensor, csLog, csLog);  // upload é feito pela função uploadErrorLog()
		esp_restart();
	}
}