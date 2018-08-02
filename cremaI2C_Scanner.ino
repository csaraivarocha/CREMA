/*
I2C Address Devices: found at ESP32 on 12/01/2018
Arduino Project: I2C_Scanner.ino

..:  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- -- -- -- -- --
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
20: -- -- -- 23 -- -- -- -- -- -- -- -- -- -- -- --
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
50: -- -- -- -- -- -- -- 57 -- -- -- -- -- -- -- --
60: -- -- -- -- -- -- -- -- 68 -- -- -- -- -- -- --
70: -- -- -- -- -- -- 76 --

----------------------------------------------------
Addr - Dispositivo
----------------------------------------------------
0x23 - BH1750 (luminosidade)
0x57 - RTC (relógio)
0x68 -
0x76 - BME280 (temperatura, umidade e pressão)
----------------------------------------------------

*/

#ifdef CREMA_DEBUG


#include <driver/i2c.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include "sdkconfig.h"

#define I2C_interruptPin  10  //definir o pino real quando da implementação

void crema_I2C_config() {
	pinMode(I2C_interruptPin, INPUT);
	if (digitalRead(I2C_interruptPin)) {
		I2C_ScannerFunc();
	}

	/*set the interrupt pin as input pullup*/
	pinMode(I2C_interruptPin, INPUT_PULLUP);
	digitalWrite(I2C_interruptPin, LOW);

	/* attach interrupt to the pin
	function I2C_ScannerFunc will be invoked when interrupt occurs
	interrupt occurs whenever the pin change value */
	attachInterrupt(digitalPinToInterrupt(I2C_interruptPin), I2C_ScannerFunc, CHANGE);
}

void I2C_ScannerFunc() {
	ESP_LOGD(tag, ">> i2cScanner");
	i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = GPIO_NUM_21;
	conf.scl_io_num = GPIO_NUM_22;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = 100000;
	i2c_param_config(I2C_NUM_0, &conf);

	i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);

	int i;
	esp_err_t espRc;
	printf("__:  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
	printf("00:         ");
	for (i = 3; i< 0x78; i++) {
		i2c_cmd_handle_t cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (i << 1) | I2C_MASTER_WRITE, 1 /* expect ack */);
		i2c_master_stop(cmd);

		espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
		if (i % 16 == 0) {
			printf("\n%.2x:", i);
		}
		if (espRc == 0) {
			printf(" %.2x", i);
		}
		else {
			printf(" --");
		}
		//ESP_LOGD(tag, "i=%d, rc=%d (0x%x)", i, espRc, espRc);
		i2c_cmd_link_delete(cmd);
	}
	printf("\n");

	Serial.print("GPIO pins -> SDA: ");
	Serial.print(SDA);

	Serial.print("; SCL: ");
	Serial.println(SCL);

	vTaskDelete(NULL);
}

#endif // CREMA_DEBUG
