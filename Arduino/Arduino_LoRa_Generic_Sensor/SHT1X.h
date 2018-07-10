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

#ifndef SHT1X_h
#define SHT1X_h

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

class SHT1x
{
  public:
    SHT1x(int dataPin, int clockPin);
    float readHumidity();
    float readTemperatureC();
    float readTemperatureF();
  private:
    int _dataPin;
    int _clockPin;
    int _numBits;
    float readTemperatureRaw();
    int shiftIn(int _dataPin, int _clockPin, int _numBits);
    void sendCommandSHT(int _command, int _dataPin, int _clockPin);
    void waitForResultSHT(int _dataPin);
    int getData16SHT(int _dataPin, int _clockPin);
    void skipCrcSHT(int _dataPin, int _clockPin);
};

#endif