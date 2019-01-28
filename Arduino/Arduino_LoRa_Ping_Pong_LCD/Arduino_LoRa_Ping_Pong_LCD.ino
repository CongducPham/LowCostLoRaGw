/*
 *  simple ping-pong test by requesting an ACK from the gateway
 *  version with an LCD display using the U8g2 library
 *  
 *  Copyright (C) 2018 Congduc Pham, University of Pau, France
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
 *
 * last update: Jan 15th, 2019 by C. Pham
 */
#include <SPI.h>  
// Include the SX1272
#include "SX1272.h"

/********************************************************************
 _____              __ _                       _   _             
/  __ \            / _(_)                     | | (_)            
| /  \/ ___  _ __ | |_ _  __ _ _   _ _ __ __ _| |_ _  ___  _ __  
| |    / _ \| '_ \|  _| |/ _` | | | | '__/ _` | __| |/ _ \| '_ \ 
| \__/\ (_) | | | | | | | (_| | |_| | | | (_| | |_| | (_) | | | |
 \____/\___/|_| |_|_| |_|\__, |\__,_|_|  \__,_|\__|_|\___/|_| |_|
                          __/ |                                  
                         |___/                                   
********************************************************************/

//new small OLED screen, mostly based on SSD1306 
#define OLED
//if you want to plug the OLED pin directly to consecutive Arduino board GND, 2, 3, 4 for respectively GND, VCC, SCK, SDA
//#define OLED_GND234
//if you are using the old LCD screen
//#define HITACHI

// uncomment if you are sure you have an ESP8266 board and that it is not detected
//#define ESP8266
// uncomment if you are sure you have an Heltec LoRa board and that it is not detected
//#define HELTEC_LORA

// IMPORTANT
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// please uncomment only 1 choice
//
#define ETSI_EUROPE_REGULATION
//#define FCC_US_REGULATION
//#define SENEGAL_REGULATION
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 

// IMPORTANT
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// uncomment if your radio is an HopeRF RFM92W, HopeRF RFM95W, Modtronix inAir9B, NiceRF1276
// or you known from the circuit diagram that output use the PABOOST line instead of the RFO line
// Note the Heltec WIFI LoRa needs PABOOST
#define PABOOST
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 

// IMPORTANT
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// please uncomment only 1 choice
#define BAND868
//#define BAND900
//#define BAND433
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef ETSI_EUROPE_REGULATION
#define MAX_DBM 14
#elif defined SENEGAL_REGULATION
#define MAX_DBM 10
#elif defined FCC_US_REGULATION
#define MAX_DBM 14
#endif

#ifdef BAND868
#ifdef SENEGAL_REGULATION
const uint32_t DEFAULT_CHANNEL=CH_04_868;
#else
const uint32_t DEFAULT_CHANNEL=CH_10_868;
#endif
#elif defined BAND900
const uint32_t DEFAULT_CHANNEL=CH_05_900;
// For HongKong, Japan, Malaysia, Singapore, Thailand, Vietnam: 920.36MHz     
//const uint32_t DEFAULT_CHANNEL=CH_08_900;
#elif defined BAND433
const uint32_t DEFAULT_CHANNEL=CH_00_433;
#endif

///////////////////////////////////////////////////////////////////
// CHANGE HERE THE LORA MODE, NODE ADDRESS 
#define LORAMODE  1
#define node_addr 8
//////////////////////////////////////////////////////////////////

/*****************************
 _____           _      
/  __ \         | |     
| /  \/ ___   __| | ___ 
| |    / _ \ / _` |/ _ \
| \__/\ (_) | (_| |  __/
 \____/\___/ \__,_|\___|
*****************************/ 

#ifdef OLED
//reset is not used
#include <U8x8lib.h>
//you can also power the OLED screen with a digital pin, here pin 8
#define OLED_PWR_PIN 8
// connection may depend on the board. Use A5/A4 for most Arduino boards. On ESP8266-based board we use GPI05 and GPI04. Heltec ESP32 has embedded OLED.
#if defined ARDUINO_Heltec_WIFI_LoRa_32 || defined ARDUINO_WIFI_LoRa_32 || defined HELTEC_LORA
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);
#elif defined ESP8266 || defined ARDUINO_ESP8266_ESP01
//U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 5, /* data=*/ 4, /* reset=*/ U8X8_PIN_NONE);
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 12, /* data=*/ 14, /* reset=*/ U8X8_PIN_NONE);
#else
#ifdef OLED_GND234
#ifdef OLED_PWR_PIN
#undef OLED_PWR_PIN
#define OLED_PWR_PIN 2
#endif
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 3, /* data=*/ 4, /* reset=*/ U8X8_PIN_NONE);
#else
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ A5, /* data=*/ A4, /* reset=*/ U8X8_PIN_NONE);
#endif
#endif
char oled_msg[20];

#elif defined HITACHI
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x20,16,2);  // set the LCD address to 0x20 for a 16 chars and 2 line display

void printLCD(int theLine, const char* theInfo, boolean clearFirst=true) {
        if (clearFirst)
              lcd.clear();
        lcd.setCursor(0, theLine);
        lcd.print(theInfo);
}  
#endif

// we wrapped Serial.println to support the Arduino Zero or M0
#if defined __SAMD21G18A__ && not defined ARDUINO_SAMD_FEATHER_M0
#define PRINTLN                   SerialUSB.println("")              
#define PRINT_CSTSTR(fmt,param)   SerialUSB.print(F(param))
#define PRINT_STR(fmt,param)      SerialUSB.print(param)
#define PRINT_VALUE(fmt,param)    SerialUSB.print(param)
#define FLUSHOUTPUT               SerialUSB.flush();
#else
#define PRINTLN                   Serial.println("")
#define PRINT_CSTSTR(fmt,param)   Serial.print(F(param))
#define PRINT_STR(fmt,param)      Serial.print(param)
#define PRINT_VALUE(fmt,param)    Serial.print(param)
#define FLUSHOUTPUT               Serial.flush();
#endif

#define DEFAULT_DEST_ADDR 1

uint8_t message[30];

int loraMode=LORAMODE;

/*****************************
 _____      _               
/  ___|    | |              
\ `--.  ___| |_ _   _ _ __  
 `--. \/ _ \ __| | | | '_ \ 
/\__/ /  __/ |_| |_| | |_) |
\____/ \___|\__|\__,_| .__/ 
                     | |    
                     |_|    
******************************/

void setup()
{
  int e;

  delay(2000);
  
  // Open serial communications and wait for port to open:
#if defined __SAMD21G18A__ && not defined ARDUINO_SAMD_FEATHER_M0 
  SerialUSB.begin(38400);
#else
  Serial.begin(38400);  
#endif 

  // Print a start message
  PRINT_CSTSTR("%s","Simple LoRa ping-pong with the gateway\n"); 
  
#ifdef OLED
#ifdef OLED_PWR_PIN
  pinMode(OLED_PWR_PIN, OUTPUT);
  digitalWrite(OLED_PWR_PIN, HIGH);
#endif
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.drawString(0, 1, "Simple PingPong");
  u8x8.drawString(0, 2, "LoRa mode 1");   
#elif defined HITACHI
  lcd.begin();
  lcd.backlight();
  lcd.home();
  lcd.print("Setup");
  printLCD(0,"Simple PingPong", true);
  printLCD(1,"LoRa mode 1", false);  
#endif

#ifdef ARDUINO_AVR_PRO
  PRINT_CSTSTR("%s","Arduino Pro Mini detected\n");  
#endif
#ifdef ARDUINO_AVR_NANO
  PRINT_CSTSTR("%s","Arduino Nano detected\n");   
#endif
#ifdef ARDUINO_AVR_MINI
  PRINT_CSTSTR("%s","Arduino MINI/Nexus detected\n");  
#endif
#ifdef ARDUINO_AVR_MEGA2560
  PRINT_CSTSTR("%s","Arduino Mega2560 detected\n");  
#endif
#ifdef ARDUINO_SAM_DUE
  PRINT_CSTSTR("%s","Arduino Due detected\n");  
#endif
#ifdef __MK66FX1M0__
  PRINT_CSTSTR("%s","Teensy36 MK66FX1M0 detected\n");
#endif
#ifdef __MK64FX512__
  PRINT_CSTSTR("%s","Teensy35 MK64FX512 detected\n");
#endif
#ifdef __MK20DX256__
  PRINT_CSTSTR("%s","Teensy31/32 MK20DX256 detected\n");
#endif
#ifdef __MKL26Z64__
  PRINT_CSTSTR("%s","TeensyLC MKL26Z64 detected\n");
#endif
#if defined ARDUINO_SAMD_ZERO && not defined ARDUINO_SAMD_FEATHER_M0
  PRINT_CSTSTR("%s","Arduino M0/Zero detected\n");
#endif
#ifdef ARDUINO_AVR_FEATHER32U4 
  PRINT_CSTSTR("%s","Adafruit Feather32U4 detected\n"); 
#endif
#ifdef  ARDUINO_SAMD_FEATHER_M0
  PRINT_CSTSTR("%s","Adafruit FeatherM0 detected\n");
#endif
#if defined ARDUINO_Heltec_WIFI_LoRa_32 || defined ARDUINO_WIFI_LoRa_32  || defined HELTEC_LORA
  PRINT_CSTSTR("%s","Heltec WiFi LoRa 32 detected\n");
#endif

// See http://www.nongnu.org/avr-libc/user-manual/using_tools.html
// for the list of define from the AVR compiler

#ifdef __AVR_ATmega328P__
  PRINT_CSTSTR("%s","ATmega328P detected\n");
#endif 
#ifdef __AVR_ATmega32U4__
  PRINT_CSTSTR("%s","ATmega32U4 detected\n");
#endif 
#ifdef __AVR_ATmega2560__
  PRINT_CSTSTR("%s","ATmega2560 detected\n");
#endif 
#ifdef __SAMD21G18A__ 
  PRINT_CSTSTR("%s","SAMD21G18A ARM Cortex-M0+ detected\n");
#endif
#ifdef __SAM3X8E__ 
  PRINT_CSTSTR("%s","SAM3X8E ARM Cortex-M3 detected\n");
#endif
#ifdef ESP32 
  PRINT_CSTSTR("%s","ESP32 detected\n");
#endif

#if defined ARDUINO_Heltec_WIFI_LoRa_32 || defined ARDUINO_WIFI_LoRa_32 || defined HELTEC_LORA
  // for the Heltec ESP32 WiFi LoRa module
  sx1272.setCSPin(18);
#endif
    
  // Power ON the module
  sx1272.ON();
  
  // Set transmission mode and print the result
  e = sx1272.setMode(loraMode);
  PRINT_CSTSTR("%s","Setting Mode: state ");
  PRINT_VALUE("%d", e);
  PRINTLN;

  // enable carrier sense
  sx1272._enableCarrierSense=true;
    
  // Select frequency channel
  e = sx1272.setChannel(DEFAULT_CHANNEL);
  PRINT_CSTSTR("%s","Setting Channel: state ");
  PRINT_VALUE("%d", e);
  PRINTLN;
  
  // Select amplifier line; PABOOST or RFO
#ifdef PABOOST
  sx1272._needPABOOST=true;
  // previous way for setting output power
  // powerLevel='x';
#else
  // previous way for setting output power
  // powerLevel='M';  
#endif

  // previous way for setting output power
  // e = sx1272.setPower(powerLevel); 

  e = sx1272.setPowerDBM((uint8_t)MAX_DBM); 
  PRINT_CSTSTR("%s","Setting Power: state ");
  PRINT_VALUE("%d", e);
  PRINTLN;
  
  // Set the node address and print the result
  e = sx1272.setNodeAddress(node_addr);
  PRINT_CSTSTR("%s","Setting node addr: state ");
  PRINT_VALUE("%d", e);
  PRINTLN;
  
  // Print a success message
  PRINT_CSTSTR("%s","SX1272 successfully configured\n");

  delay(500);
}


/*****************************
 _                       
| |                      
| |     ___   ___  _ __  
| |    / _ \ / _ \| '_ \ 
| |___| (_) | (_) | |_) |
\_____/\___/ \___/| .__/ 
                  | |    
                  |_|    
*****************************/

void loop(void)
{
  uint8_t r_size;
  int e;

  sx1272.setPacketType(PKT_TYPE_DATA);
      
  while (1) {

      sx1272.CarrierSense();
      
      r_size=sprintf((char*)message, "Ping");
      PRINT_CSTSTR("%s","Sending Ping");  
      PRINTLN;

#ifdef OLED
      u8x8.drawString(0, 3, "Sending Ping....");
      u8x8.clearLine(4);
      u8x8.clearLine(5);
#elif defined HITACHI
      printLCD(0, "Sending Ping....", true);
#endif
            
      e = sx1272.sendPacketTimeoutACK(DEFAULT_DEST_ADDR, message, r_size);

      // this is the no-ack version
      // e = sx1272.sendPacketTimeout(DEFAULT_DEST_ADDR, message, r_size);
            
      PRINT_CSTSTR("%s","Packet sent, state ");
      PRINT_VALUE("%d", e);
      PRINTLN;

      if (e==1) {
#ifdef OLED          
          u8x8.drawString(0, 3, "Error sending=1 ");
#elif defined HITACHI
          printLCD(0,"Error sending=1 ", true);        
#endif          
      }      
      
      if (e==3) {
          PRINT_CSTSTR("%s","No Pong from gw!");
#ifdef OLED          
          u8x8.drawString(0, 3, "No Pong from gw!");
#elif defined HITACHI
          printLCD(0,"No Pong from gw!", true);         
#endif          
      }

      if (e==1 || e==3) {
#ifdef OLED          
          u8x8.drawString(0, 4, "SNR at gw=N/A");
#elif defined HITACHI
          printLCD(1,"SNR at gw=N/A", false);          
#endif          
      }
        
      if (e==0) {
          char msg[20];

          PRINT_CSTSTR("%s","Pong received from gateway!");
          PRINTLN;
          sprintf(msg,"SNR at gw=%d   ", sx1272._rcv_snr_in_ack);
          PRINT_STR("%s", msg);   
          PRINTLN;

#ifdef OLED             
          u8x8.drawString(0, 3, "Get Pong from gw");
          u8x8.drawString(0, 4, msg);
#elif defined HITACHI
          printLCD(0, "Get Pong from gw", true);
          printLCD(1, msg, false);
#endif

          sx1272.getSNR();
          sx1272.getRSSIpacket();
          sprintf(msg,"gw->SNR=%d:%d", sx1272._SNR, sx1272._RSSIpacket);
          PRINT_STR("%s", msg);
          PRINTLN;

#ifdef OLED
          u8x8.clearLine(5);             
          u8x8.drawString(0, 5, msg);
#elif defined HITACHI
          printLCD(1, msg, false);
#endif
      }

      PRINTLN;
      
      delay(10000);   
  }          
}
