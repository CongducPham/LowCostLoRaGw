/*
  Epoch time example for Arduino Zero and MKR1000

  Demonstrates how to set time using epoch for the Arduino Zero and MKR1000

  This example code is in the public domain

  created by Sandeep Mistry <s.mistry@arduino.cc>
  31 Dec 2015
  modified
  18 Feb 2016
*/

#include <RTCZero.h>

/* Create an rtc object */
RTCZero rtc;

void setup() {
  Serial.begin(9600);

  rtc.begin(); // initialize RTC

  rtc.setEpoch(1451606400); // Jan 1, 2016
}

void loop() {
  Serial.print("Unix time = ");
  Serial.println(rtc.getEpoch());

  Serial.print("Seconds since Jan 1 2000 = ");
  Serial.println(rtc.getY2kEpoch());

  // Print date...
  Serial.print(rtc.getDay());
  Serial.print("/");
  Serial.print(rtc.getMonth());
  Serial.print("/");
  Serial.print(rtc.getYear());
  Serial.print("\t");

  // ...and time
  print2digits(rtc.getHours());
  Serial.print(":");
  print2digits(rtc.getMinutes());
  Serial.print(":");
  print2digits(rtc.getSeconds());

  Serial.println();

  delay(1000);
}

void print2digits(int number) {
  if (number < 10) {
    Serial.print("0");
  }
  Serial.print(number);
}

