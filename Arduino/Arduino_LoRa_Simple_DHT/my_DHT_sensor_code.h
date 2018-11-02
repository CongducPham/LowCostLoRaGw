#ifndef MY_DHT_SENSOR_CODE
#define MY_DHT_SENSOR_CODE

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include "DHT.h"

extern char nomenclature_str[4];
void sensor_Init();
double sensor_getValue();

///////////////////////////////////////////////////////////////////
// CHANGE HERE THE READ PIN AND THE POWER PIN FOR THE TEMP. SENSOR
#define PIN_READ  A0
// use digital 9 to power the temperature sensor if needed
// but on most ESP8266 boards pin 9 can not be used, so use pin 2 instead
#if defined ARDUINO_ESP8266_ESP01 || defined ARDUINO_ESP8266_NODEMCU || defined IOTMCU_LORA_RADIO_NODE
#define PIN_POWER 2
#else
#define PIN_POWER 9
#endif
///////////////////////////////////////////////////////////////////

//#define DHTTYPE DHT11       // DHT 11 
#define DHTTYPE DHT22         // DHT 22  (AM2302 & AM2305)

#endif
