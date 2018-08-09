//// Visor
#define _PIN_RESET GPIO_NUM_33   // Pin 1 on LCD
#define _PIN_SCE   GPIO_NUM_15   // Pin 2 on LCD
#define _PIN_DC    GPIO_NUM_12   // Pin 3 on LCD
#define _PIN_SDIN  GPIO_NUM_13   // Pin 4 on LCD (MOSI)
#define _PIN_SCLK  GPIO_NUM_14   // Pin 5 on LCD (SCLK)

//// Sensores 
//I2C - RTC, luminosidade e BME280 (temperatura e pressao)
#define _CREMA_I2C_SDA    GPIO_NUM_25
#define _CREMA_I2C_SCL    GPIO_NUM_26
// Ultravioleta
//#define _CREMA_UV_PIN       GPIO_NUM_27
//#define _CREMA_UV_PIN_REF   GPIO_NUM_26
// umidade
//#define _CREMA_DHT_PIN		/// GPIO_NUM_?? definir outro

//// Configurar ESP232 para escolher WiFi
#define CREMA_WiFi_Manager_PIN GPIO_NUM_36

//// Led indicators
#define _PIN_LED_UPLOAD_SENSOR_VALUES GPIO_NUM_22
#define _PIN_LED_READ_SENSOR_VALUES   GPIO_NUM_23

//// GPS
#define _PIN_GPS_TX  GPIO_NUM_17
#define _PIN_GPS_RX  GPIO_NUM_16
