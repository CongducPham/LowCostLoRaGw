/**
 * SHT1x Library
 *
 * Copyright 2009 Jonathan Oxer <jon@oxer.com.au> / <www.practicalarduino.com>
 * Based on previous work by:
 *    Maurice Ribble: <www.glacialwanderer.com/hobbyrobotics/?p=5>
 *    Wayne ?: <ragingreality.blogspot.com/2008/01/ardunio-and-sht15.html>
 *
 * Updated for Arduino 1.6.5 Library Manger by Joel Bartlett 
 * SparkFun Electronics 
 * September 16, 2015
 *
 * Manages communication with SHT1x series (SHT10, SHT11, SHT15)
 * temperature / humidity sensors from Sensirion (www.sensirion.com).
 */

#include "SHT1x.h"

SHT1x::SHT1x(int dataPin, int clockPin)
{
  _dataPin = dataPin;
  _clockPin = clockPin;
}


/* ================  Public methods ================ */
//Reads the current temperature in degrees Celsius

float SHT1x::readTemperatureC()
{
  int _val;                // Raw value returned from sensor
  float _temperature;      // Temperature derived from raw value

  // Conversion coefficients from SHT15 datasheet
  const float D1 = -40.0;  // for 14 Bit @ 5V
  const float D2 =   0.01; // for 14 Bit DEGC

  // Fetch raw value
  _val = readTemperatureRaw();

  // Convert raw value to degrees Celsius
  _temperature = (_val * D2) + D1;

  return (_temperature);
}

////////////////////////////////////////////////////////////////////////
//Reads the current temperature in degrees Fahrenheit
float SHT1x::readTemperatureF()
{
  int _val;                 // Raw value returned from sensor
  float _temperature;       // Temperature derived from raw value

  // Conversion coefficients from SHT15 datasheet
  const float D1 = -40.0;   // for 14 Bit @ 5V
  const float D2 =   0.018; // for 14 Bit DEGF

  // Fetch raw value
  _val = readTemperatureRaw();

  // Convert raw value to degrees Fahrenheit
  _temperature = (_val * D2) + D1;

  return (_temperature);
}
////////////////////////////////////////////////////////////////////////
//Reads current temperature-corrected relative humidity
float SHT1x::readHumidity()
{
  int _val;                    // Raw humidity value returned from sensor
  float _linearHumidity;       // Humidity with linear correction applied
  float _correctedHumidity;    // Temperature-corrected humidity
  float _temperature;          // Raw temperature value
  
  // Conversion coefficients from SHT15 datasheet
  //const float C1 = -4.0;       //  for V3 sensors
  //const float C2 =  0.0405;    //  for V3 sensors, 12-bit precision
  //const float C3 = -0.0000028; //  for V3 sensors, 12-bit precision
  const float C1  = -2.0468;        // for V4 sensors
  const float C2 =  0.0367;        // for V4 sensors, 12-bit precision
  const float C3 = -1.5955E-6;     // for V4 sensors, 12-bit precision
  const float T1 =  0.01;      // for 14 Bit @ 5V
  const float T2 =  0.00008;   // for 14 Bit @ 5V

  // Command to send to the SHT1x to request humidity
  int _gHumidCmd = 0b00000101;

  // Fetch the value from the sensor
  sendCommandSHT(_gHumidCmd, _dataPin, _clockPin);
  waitForResultSHT(_dataPin);
  _val = getData16SHT(_dataPin, _clockPin);
  skipCrcSHT(_dataPin, _clockPin);

  // Apply linear conversion to raw value
  _linearHumidity = C1 + C2 * _val + C3 * _val * _val;

  // Get current temperature for humidity correction
  _temperature = readTemperatureC();

  // Correct humidity value for current temperature
  _correctedHumidity = (_temperature - 25.0 ) * (T1 + T2 * _val) + _linearHumidity;

  return (_correctedHumidity);
}


/* ================  Private methods ================ */
float SHT1x::readTemperatureRaw()
{
  int _val;

  // Command to send to the SHT1x to request Temperature
  int _gTempCmd  = 0b00000011;

  sendCommandSHT(_gTempCmd, _dataPin, _clockPin);
  waitForResultSHT(_dataPin);
  _val = getData16SHT(_dataPin, _clockPin);
  skipCrcSHT(_dataPin, _clockPin);

  return (_val);
}
////////////////////////////////////////////////////////////////////////
int SHT1x::shiftIn(int _dataPin, int _clockPin, int _numBits)// commands for reading/sending data to a SHTx sensor 
{
  int ret = 0;
  int i;

  for (i=0; i<_numBits; ++i)
  {
     digitalWrite(_clockPin, HIGH);
     delay(10);  // I don't know why I need this, but without it I don't get my 8 lsb of temp
     ret = ret*2 + digitalRead(_dataPin);
     digitalWrite(_clockPin, LOW);
  }

  return(ret);
}
////////////////////////////////////////////////////////////////////////
void SHT1x::sendCommandSHT(int _command, int _dataPin, int _clockPin)// send a command to the SHTx sensor 
{
  int ack;

  // Transmission Start
  pinMode(_dataPin, OUTPUT);
  pinMode(_clockPin, OUTPUT);
  digitalWrite(_dataPin, HIGH);
  digitalWrite(_clockPin, HIGH);
  digitalWrite(_dataPin, LOW);
  digitalWrite(_clockPin, LOW);
  digitalWrite(_clockPin, HIGH);
  digitalWrite(_dataPin, HIGH);
  digitalWrite(_clockPin, LOW);

  // The command (3 msb are address and must be 000, and last 5 bits are command)
  shiftOut(_dataPin, _clockPin, MSBFIRST, _command);

  // Verify we get the correct ack
  digitalWrite(_clockPin, HIGH);
  pinMode(_dataPin, INPUT);
  ack = digitalRead(_dataPin);
  if (ack != LOW) {
    //Serial.println("Ack Error 0");
  }
  digitalWrite(_clockPin, LOW);
  ack = digitalRead(_dataPin);
  if (ack != HIGH) {
    //Serial.println("Ack Error 1");
  }
}
////////////////////////////////////////////////////////////////////////
void SHT1x::waitForResultSHT(int _dataPin)// wait for the SHTx answer 
{
  int i;
  int ack;

  pinMode(_dataPin, INPUT);

  for(i= 0; i < 100; ++i)
  {
    delay(10);
    ack = digitalRead(_dataPin);

    if (ack == LOW) {
      break;
    }
  }

  if (ack == HIGH) {
    //Serial.println("Ack Error 2"); // Can't do serial stuff here, need another way of reporting errors
  }
}
////////////////////////////////////////////////////////////////////////
int SHT1x::getData16SHT(int _dataPin, int _clockPin)// get data from the SHTx sensor 
{
  int val; 
 
  // get the MSB (most significant bits) 
  pinMode(_dataPin, INPUT); 
  pinMode(_clockPin, OUTPUT); 
  val = shiftIn(_dataPin, _clockPin, 8); 
  val *= 256; // this is equivalent to val << 8; 
  
  // send the required ACK 
  pinMode(_dataPin, OUTPUT); 
  digitalWrite(_dataPin, HIGH); 
  digitalWrite(_dataPin, LOW); 
  digitalWrite(_clockPin, HIGH); 
  digitalWrite(_clockPin, LOW); 
  
  // get the LSB (less significant bits) 
  pinMode(_dataPin, INPUT); 
  val |= shiftIn(_dataPin, _clockPin, 8); 
  
  return val; 
}
////////////////////////////////////////////////////////////////////////
void SHT1x::skipCrcSHT(int _dataPin, int _clockPin)
{
  // Skip acknowledge to end trans (no CRC)
  pinMode(_dataPin, OUTPUT);
  pinMode(_clockPin, OUTPUT);

  digitalWrite(_dataPin, HIGH);
  digitalWrite(_clockPin, HIGH);
  digitalWrite(_clockPin, LOW);
}
