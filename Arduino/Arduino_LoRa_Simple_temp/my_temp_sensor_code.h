#ifndef MY_TEMP_SENSOR_CODE
#define MY_TEMP_SENSOR_CODE

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

extern char nomenclature_str[4];
void sensor_Init();
double sensor_getValue();

///////////////////////////////////////////////////////////////////
// CHANGE HERE THE READ PIN AND THE POWER PIN FOR THE TEMP. SENSOR
#define PIN_READ  A1
// use digital 9 to power the temperature sensor if needed
// but on most ESP8266 boards pin 9 can not be used, so use pin 2 instead
#if defined ARDUINO_ESP8266_ESP01 || defined ARDUINO_ESP8266_NODEMCU || defined IOTMCU_LORA_RADIO_NODE
#define PIN_POWER 2
#else
#define PIN_POWER 9
#endif
///////////////////////////////////////////////////////////////////

#if defined ARDUINO_AVR_PRO || defined ARDUINO_AVR_MINI || defined ARDUINO_SAM_DUE || defined __MK20DX256__  || defined __MKL26Z64__ || defined __MK64FX512__ || defined __MK66FX1M0__ || defined __SAMD21G18A__
  // if you have a Pro Mini running at 5V, then change here
  // these boards work in 3.3V
  // Nexus board from Ideetron is a Mini
  // __MK66FX1M0__ is for Teensy36
  // __MK64FX512__  is for Teensy35
  // __MK20DX256__ is for Teensy31/32
  // __MKL26Z64__ is for TeensyLC
  // __SAMD21G18A__ is for Zero/M0 and FeatherM0 (Cortex-M0)
  #define VOLTAGE_SCALE  3300.0
#else // ARDUINO_AVR_NANO || defined ARDUINO_AVR_UNO || defined ARDUINO_AVR_MEGA2560 
  // also for all other boards, so change here if required.
  #define VOLTAGE_SCALE  5000.0
#endif

#endif
