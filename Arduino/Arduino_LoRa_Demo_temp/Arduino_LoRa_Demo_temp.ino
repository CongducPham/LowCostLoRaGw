/*
 *  Demo of a simple temperature sensor on analog A0 to test the LoRa gateway
 *
 *  Copyright (C) 2017 Congduc Pham, University of Pau, France
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************
 */
 
// Include the SX1272
#include "SX1272.h"

#define OLED

#ifdef OLED
#include <U8x8lib.h>
// the OLED used
#if defined ARDUINO_Heltec_WIFI_LoRa_32 || defined ARDUINO_WIFI_LoRa_32 || defined HELTEC_LORA
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);
#else
//reset is not used, but don't know what the lib will do with that pin, so better to indicate a pn that you do not use
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ A5, /* data=*/ A4, /* reset=*/ A3);
#endif
char oled_msg[20];
#endif

// IMPORTANT
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// please uncomment only 1 choice
//
#define ETSI_EUROPE_REGULATION
//#define FCC_US_REGULATION
//#define SENEGAL_REGULATION
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef ETSI_EUROPE_REGULATION
#define MAX_DBM 14
// previous way for setting output power
// char powerLevel='M';
#elif defined SENEGAL_REGULATION
#define MAX_DBM 10
// previous way for setting output power
// 'H' is actually 6dBm, so better to use the new way to set output power
// char powerLevel='H';
#elif defined FCC_US_REGULATION
#define MAX_DBM 14
#endif

// IMPORTANT
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// uncomment if your radio is an HopeRF RFM92W, HopeRF RFM95W, Modtronix inAir9B, NiceRF1276
// or you known from the circuit diagram that output use the PABOOST line instead of the RFO line
#define PABOOST
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 

#define FLOAT_TEMP

#define DEFAULT_DEST_ADDR 1
#define LORAMODE  1

///////////////////////////////////////////////////////////////////
// CHANGE HERE THE NODE ADDRESS 
#define node_addr 9
//////////////////////////////////////////////////////////////////

#define TEMP_PIN_READ  A0

#if defined ARDUINO_AVR_PRO || defined ARDUINO_AVR_MINI || defined ARDUINO_SAM_DUE || defined __MK20DX256__ || defined __MKL26Z64__ || defined __MK64FX512__ || defined __MK66FX1M0__ || defined __SAMD21G18A__ 
|| defined ARDUINO_Heltec_WIFI_LoRa_32 || defined ARDUINO_WIFI_LoRa_32 || defined HELTEC_LORA
  // if you have a Pro Mini running at 5V, then change here
  // these boards work in 3.3V
  // Nexus board from Ideetron is a Mini
  // __MK66FX1M0__ is for Teensy36
  // __MK64FX512__  is for Teensy35  
  // __MK20DX256__ is for Teensy31/32
  // __MKL26Z64__ is for TeensyLC
  // __SAMD21G18A__ is for Zero/M0 and FeatherM0 (Cortex-M0)
  #define TEMP_SCALE  3300.0
#else // ARDUINO_AVR_NANO || defined ARDUINO_AVR_UNO || defined ARDUINO_AVR_MEGA2560
  // also for all other boards, so change here if required.
  #define TEMP_SCALE  5000.0
#endif

double temp;
unsigned long nextTransmissionTime=0L;
char float_str[20];
uint8_t message[100];

int loraMode=LORAMODE;

void setup()
{
  int e;

  // for the temperature sensor
  pinMode(TEMP_PIN_READ, INPUT);

  delay(3000);
  // Open serial communications and wait for port to open:
  Serial.begin(38400);
  // Print a start message
  Serial.println(F("Simple LoRa temperature sensor demo"));

#ifdef OLED
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.drawString(0, 1, "Simple TempDemo");
  u8x8.drawString(0, 2, "LoRa mode 1");   
#endif  

#if defined ARDUINO_Heltec_WIFI_LoRa_32 || defined ARDUINO_WIFI_LoRa_32 || defined HELTEC_LORA
  // for the Heltec ESP32 WiFi LoRa module
  sx1272.setCSPin(18);
#endif

  // Power ON the module
  sx1272.ON();
  
  // Set transmission mode and print the result
  e = sx1272.setMode(loraMode);
  Serial.print(F("Setting Mode: state "));
  Serial.println(e, DEC);

  // enable carrier sense
  sx1272._enableCarrierSense=true;
    
  // Select frequency channel
  e = sx1272.setChannel(CH_10_868);
  Serial.print(F("Setting Channel: state "));
  Serial.println(e, DEC);
  
  // Select amplifier line; PABOOST or RFO
#ifdef PABOOST
  sx1272._needPABOOST=true;
  // previous way for setting output power
  // powerLevel='x';
#else
  // previous way for setting output power
  // powerLevel='M';  
#endif

  e = sx1272.setPowerDBM((uint8_t)MAX_DBM);
  
  Serial.print(F("Setting Power: state "));
  Serial.println(e);
  
  // Set the node address and print the result
  e = sx1272.setNodeAddress(node_addr);
  Serial.print(F("Setting node addr: state "));
  Serial.println(e, DEC);
  
  // Print a success message
  Serial.println(F("SX1272 successfully configured"));

  delay(500);
}

char *ftoa(char *a, double f, int precision)
{
 long p[] = {0,10,100,1000,10000,100000,1000000,10000000,100000000};
 
 char *ret = a;
 long heiltal = (long)f;
 itoa(heiltal, a, 10);
 while (*a != '\0') a++;
 *a++ = '.';
 long desimal = abs((long)((f - heiltal) * p[precision]));
 if (desimal < p[precision-1]) {
  *a++ = '0';
 }  
 itoa(desimal, a, 10);
 return ret;
}

void loop(void)
{
  long startSend;
  long endSend;
  int e;

  if (millis() > nextTransmissionTime) {

      int value = analogRead(TEMP_PIN_READ);
      Serial.print(F("Reading "));
      Serial.println(value);
            
      // change here how the temperature should be computed depending on your sensor type
      //  
      //LM35DZ
      //the LM35DZ needs at least 4v as supply voltage
      //can be used on 5v board
      //temp = (value*TEMP_SCALE/1024.0)/10;

      //TMP36
      //the TMP36 can work with supply voltage of 2.7v-5.5v
      //can be used on 3.3v board
      //we use a 0.95 factor when powering with less than 3.3v, e.g. 3.1v in the average for instance
      //this setting is for 2 AA batteries
      temp = ((value*0.95*TEMP_SCALE/1024.0)-500)/10;

      // on the Heltec, use the 3.3v pin and a TMP36 sensor
      //temp = ((value*TEMP_SCALE/1024.0)-500)/10;
      
      //set a defined value for testing
      //temp = 22.5;
      
      Serial.print(F("(Temp is "));
      Serial.println(temp);

      uint8_t r_size;

      // then use app_key_offset to skip the app key
#ifdef FLOAT_TEMP
      ftoa(float_str,temp,2);

      r_size=sprintf((char*)message, "\\!TC/%s", float_str);
#else
      r_size=sprintf((char*)message, "\\!TC/%d", (int)temp);   
#endif

#ifdef OLED
      u8x8.clearLine(3);
      u8x8.clearLine(4);
      u8x8.clearLine(5);
      u8x8.drawString(0, 3, "Sending");    
      u8x8.drawString(0, 4, (char*)message);   
#endif 
      Serial.print(F("Sending "));
      Serial.println((char*)(message));

      Serial.print(F("Real payload size is "));
      Serial.println(r_size);
      
      int pl=r_size;
      
      sx1272.CarrierSense();
      
      startSend=millis();

      // indicate that we have an appkey
      sx1272.setPacketType(PKT_TYPE_DATA);
      
      // Send message to the gateway and print the result
      // with the app key if this feature is enabled
      e = sx1272.sendPacketTimeout(DEFAULT_DEST_ADDR, message, pl);
 
      endSend=millis();
      
      Serial.print(F("LoRa pkt seq "));
      Serial.println(sx1272.packet_sent.packnum);
    
      Serial.print(F("LoRa Sent in "));
      Serial.println(endSend-startSend);
          
      Serial.print(F("LoRa Sent w/CAD in "));

      Serial.println(endSend-sx1272._startDoCad);

      Serial.print(F("Packet sent, state "));
      Serial.println(e);

#ifdef OLED
      u8x8.clearLine(3);
      u8x8.clearLine(5);
      sprintf(oled_msg, "Sending done %d", e);
      u8x8.drawString(0, 3, oled_msg);
      u8x8.drawString(0, 5, "Waiting..."); 
#endif      

      // 1 minutes
      nextTransmissionTime=millis()+60000;
  }
}
