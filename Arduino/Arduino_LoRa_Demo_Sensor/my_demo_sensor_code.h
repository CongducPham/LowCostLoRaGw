#ifndef MY_DEMO_SENSOR_CODE
#define MY_DEMO_SENSOR_CODE

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

extern char nomenclature_str[4];
void sensor_Init();
double sensor_getValue();

///////////////////////////////////////////////////////////////////
// CHANGE HERE THE PIN TO READ AND POWER SENSOR
// IF YOU USE A COMPLETELY DIFFERENT SENSOR, YOU MAY NOT NEED THIS LINE 

#define PIN_READ  A0
#define PIN_POWER 9
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// CHANGE HERE THE VOLTAGE SCALE
// 3300.0 for 3.3V and 5000.0 for 5V

#define VOLTAGE_SCALE  3300.0
///////////////////////////////////////////////////////////////////

#endif
