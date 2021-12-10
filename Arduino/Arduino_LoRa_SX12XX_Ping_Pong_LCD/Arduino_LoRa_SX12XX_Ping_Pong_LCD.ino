/*
 *  simple ping-pong test by requesting an ACK from the gateway
 *  version with an LCD display using the U8g2 library
 *  
 *  Copyright (C) 2020 Congduc Pham, University of Pau, France
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
 * last update: Dec 10th, 2021 by C. Pham
 * 
 * NEW: LoRa communicain library moved from Libelium's lib to StuartProject's lib
 * https://github.com/StuartsProjects/SX12XX-LoRa
 * to support SX126X, SX127X and SX128X chips (SX128X is LoRa in 2.4GHz band)
 */
 
#include <SPI.h>
#define USE_SPI_TRANSACTION          //this is the standard behaviour of library, use SPI Transaction switching
// please uncomment only 1 choice 
//#define SX126X
#define SX127X
//#define SX128X

#ifdef SX126X
#include <SX126XLT.h>
SX126XLT LT;                                          
#endif

#ifdef SX127X
#include <SX127XLT.h>
SX127XLT LT;                                          
#endif

#ifdef SX128X
#include <SX128XLT.h>
SX128XLT LT;                                         
#endif

#include "Arduino_LoRa_SX12XX_Ping_Pong_LCD.h"                 //include the setiings file, frequencies, LoRa settings etc   

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
///////////////////////////////////////////////////////////////////
// CHANGE HERE THE NODE ADDRESS 
uint8_t node_addr=8;
//////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// CHANGE HERE THE TIME IN SECONDS BETWEEN 2 READING & TRANSMISSION
unsigned int idlePeriodInSec = 8;
///////////////////////////////////////////////////////////////////

//keep track of the number of successful transmissions
uint32_t TXPacketCount=0;
uint8_t TXPacketL;

#define OLED
//#define OLED_GND234
//#define OLED_9GND876
//#define OLED_7GND654
//#define OLED_GND13_12_11

// if you know you have an ESP8266 based board, uncomment next line
//#define ESP8266

// if you know you have an Heltec WiFi LoRa based board, uncomment next 
//#define ARDUINO_Heltec_WIFI_LoRa_32

#ifdef OLED
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
#elif defined OLED_9GND876
  #ifdef OLED_PWR_PIN
    #undef OLED_PWR_PIN
    #define OLED_PWR_PIN 8
  #endif  
  U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 7, /* data=*/ 6, /* reset=*/ U8X8_PIN_NONE);
#elif defined OLED_7GND654
  #ifdef OLED_PWR_PIN
    #undef OLED_PWR_PIN
    #define OLED_PWR_PIN 6
  #endif  
  U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 5, /* data=*/ 4, /* reset=*/ U8X8_PIN_NONE);  
#elif defined OLED_GND13_12_11
  #ifdef OLED_PWR_PIN
    #undef OLED_PWR_PIN
    #define OLED_PWR_PIN 13
  #endif  
  U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 12, /* data=*/ 11, /* reset=*/ U8X8_PIN_NONE); 
#else
  U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ A5, /* data=*/ A4, /* reset=*/ U8X8_PIN_NONE);
#endif
#endif
char oled_msg[20];
#endif

/*****************************
 _____           _      
/  __ \         | |     
| /  \/ ___   __| | ___ 
| |    / _ \ / _` |/ _ \
| \__/\ (_) | (_| |  __/
 \____/\___/ \__,_|\___|
*****************************/ 

// we wrapped Serial.println to support the Arduino Zero or M0
#if defined __SAMD21G18A__ && not defined ARDUINO_SAMD_FEATHER_M0
#define PRINTLN                   SerialUSB.println("")              
#define PRINT_CSTSTR(param)       SerialUSB.print(F(param))
#define PRINTLN_CSTSTR(param)     SerialUSB.println(F(param))
#define PRINT_STR(fmt,param)      SerialUSB.print(param)
#define PRINTLN_STR(fmt,param)    SerialUSB.println(param)
#define PRINT_VALUE(fmt,param)    SerialUSB.print(param)
#define PRINTLN_VALUE(fmt,param)  SerialUSB.println(param)
#define PRINT_HEX(fmt,param)      SerialUSB.print(param,HEX)
#define PRINTLN_HEX(fmt,param)    SerialUSB.println(param,HEX)
#define FLUSHOUTPUT               SerialUSB.flush()
#else
#define PRINTLN                   Serial.println("")
#define PRINT_CSTSTR(param)       Serial.print(F(param))
#define PRINTLN_CSTSTR(param)     Serial.println(F(param))
#define PRINT_STR(fmt,param)      Serial.print(param)
#define PRINTLN_STR(fmt,param)    Serial.println(param)
#define PRINT_VALUE(fmt,param)    Serial.print(param)
#define PRINTLN_VALUE(fmt,param)  Serial.println(param)
#define PRINT_HEX(fmt,param)      Serial.print(param,HEX)
#define PRINTLN_HEX(fmt,param)    Serial.println(param,HEX)
#define FLUSHOUTPUT               Serial.flush()
#endif

#define DEFAULT_DEST_ADDR 1

uint8_t message[30];

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
void loraConfig() {
  //The function call list below shows the complete setup for the LoRa device using the information defined in the
  //Settings.h file.
  //The 'Setup Lora device' list below can be replaced with a single function call;
  //LT.setupLoRa(Frequency, Offset, SpreadingFactor, Bandwidth, CodeRate, Optimisation);

  //***************************************************************************************************
  //Setup Lora device
  //***************************************************************************************************
  
  LT.setMode(MODE_STDBY_RC);
#ifdef SX126X
  LT.setRegulatorMode(USE_DCDC);
  LT.setPaConfig(0x04, PAAUTO, LORA_DEVICE);
  LT.setDIO3AsTCXOCtrl(TCXO_CTRL_3_3V);
  LT.calibrateDevice(ALLDevices);                //is required after setting TCXO
  LT.calibrateImage(Frequency);
  LT.setDIO2AsRfSwitchCtrl();
#endif
#ifdef SX128X
  LT.setRegulatorMode(USE_LDO);
#endif
  //set for LoRa transmissions                              
  LT.setPacketType(PACKET_TYPE_LORA);
  //set the operating frequency                 
  LT.setRfFrequency(Frequency, Offset);
//run calibration after setting frequency
#ifdef SX127X
  LT.calibrateImage(0);
#endif
  //set LoRa modem parameters
#if defined SX126X || defined SX127X
  LT.setModulationParams(SpreadingFactor, Bandwidth, CodeRate, Optimisation);
#endif
#ifdef SX128X
  LT.setModulationParams(SpreadingFactor, Bandwidth, CodeRate);
#endif                                     
  //where in the SX buffer packets start, TX and RX
  LT.setBufferBaseAddress(0x00, 0x00);
  //set packet parameters
#if defined SX126X || defined SX127X                     
  LT.setPacketParams(8, LORA_PACKET_VARIABLE_LENGTH, 255, LORA_CRC_ON, LORA_IQ_NORMAL);
#endif
#ifdef SX128X
  LT.setPacketParams(12, LORA_PACKET_VARIABLE_LENGTH, 255, LORA_CRC_ON, LORA_IQ_NORMAL, 0, 0);
#endif
  //syncword, LORA_MAC_PRIVATE_SYNCWORD = 0x12, or LORA_MAC_PUBLIC_SYNCWORD = 0x34
  //TODO check for sync word when SX128X 
#if defined SX126X || defined SX127X            
  LT.setSyncWord(LORA_MAC_PRIVATE_SYNCWORD);
  //set for highest sensitivity at expense of slightly higher LNA current
  LT.setHighSensitivity();  //set for maximum gain
#endif
#ifdef SX126X
  //set for IRQ on RX done and timeout on DIO1
  LT.setDioIrqParams(IRQ_RADIO_ALL, (IRQ_RX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);
#endif
#ifdef SX127X
  //set for IRQ on RX done
  LT.setDioIrqParams(IRQ_RADIO_ALL, IRQ_RX_DONE, 0, 0);
#ifdef PABOOST
  LT.setPA_BOOST(true);
  PRINTLN_CSTSTR("Use PA_BOOST amplifier line");
#endif
#endif
#ifdef SX128X
  LT.setDioIrqParams(IRQ_RADIO_ALL, (IRQ_RX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);
#endif    
  
  //***************************************************************************************************  
}

void setup()
{
  delay(2000);
  
  // Open serial communications and wait for port to open:
#if defined __SAMD21G18A__ && not defined ARDUINO_SAMD_FEATHER_M0 
  SerialUSB.begin(38400);
#else
  Serial.begin(38400);  
#endif 

  // Print a start message
  PRINT_CSTSTR("Simple LoRa ping-pong with the gateway using 4-byte header\n"); 

#ifdef OLED
#ifdef OLED_PWR_PIN
  pinMode(OLED_PWR_PIN, OUTPUT);
  digitalWrite(OLED_PWR_PIN, HIGH);
#ifdef OLED_9GND876
  //use pin 9 as ground
  pinMode(9, OUTPUT);
  digitalWrite(9, LOW);
#elif defined OLED_7GND654
  //use pin 7 as ground
  pinMode(7, OUTPUT);
  digitalWrite(7, LOW);
#endif
#endif
#endif

#ifdef ARDUINO_AVR_PRO
  PRINT_CSTSTR("Arduino Pro Mini detected\n");  
#endif
#ifdef ARDUINO_AVR_NANO
  PRINT_CSTSTR("Arduino Nano detected\n");   
#endif
#ifdef ARDUINO_AVR_MINI
  PRINT_CSTSTR("Arduino MINI/Nexus detected\n");  
#endif
#ifdef ARDUINO_AVR_MEGA2560
  PRINT_CSTSTR("Arduino Mega2560 detected\n");  
#endif
#ifdef ARDUINO_SAM_DUE
  PRINT_CSTSTR("Arduino Due detected\n");  
#endif
#ifdef __MK66FX1M0__
  PRINT_CSTSTR("Teensy36 MK66FX1M0 detected\n");
#endif
#ifdef __MK64FX512__
  PRINT_CSTSTR("Teensy35 MK64FX512 detected\n");
#endif
#ifdef __MK20DX256__
  PRINT_CSTSTR("Teensy31/32 MK20DX256 detected\n");
#endif
#ifdef __MKL26Z64__
  PRINT_CSTSTR("TeensyLC MKL26Z64 detected\n");
#endif
#if defined ARDUINO_SAMD_ZERO && not defined ARDUINO_SAMD_FEATHER_M0
  PRINT_CSTSTR("Arduino M0/Zero detected\n");
#endif
#ifdef ARDUINO_AVR_FEATHER32U4 
  PRINT_CSTSTR("Adafruit Feather32U4 detected\n"); 
#endif
#ifdef  ARDUINO_SAMD_FEATHER_M0
  PRINT_CSTSTR("Adafruit FeatherM0 detected\n");
#endif
#if defined ARDUINO_Heltec_WIFI_LoRa_32 || defined ARDUINO_WIFI_LoRa_32  || defined HELTEC_LORA
  PRINT_CSTSTR("Heltec WiFi LoRa 32 detected\n");
#endif

// See http://www.nongnu.org/avr-libc/user-manual/using_tools.html
// for the list of define from the AVR compiler

#ifdef __AVR_ATmega328P__
  PRINT_CSTSTR("ATmega328P detected\n");
#endif 
#ifdef __AVR_ATmega32U4__
  PRINT_CSTSTR("ATmega32U4 detected\n");
#endif 
#ifdef __AVR_ATmega2560__
  PRINT_CSTSTR("ATmega2560 detected\n");
#endif 
#ifdef __SAMD21G18A__ 
  PRINT_CSTSTR("SAMD21G18A ARM Cortex-M0+ detected\n");
#endif
#ifdef __SAM3X8E__ 
  PRINT_CSTSTR("SAM3X8E ARM Cortex-M3 detected\n");
#endif
#ifdef ESP32 
  PRINT_CSTSTR("ESP32 detected\n");
#endif
    
  SPI.begin();

  //setup hardware pins used by device, then check if device is found
#ifdef SX126X
  if (LT.begin(NSS, NRESET, RFBUSY, DIO1, DIO2, DIO3, RX_EN, TX_EN, SW, LORA_DEVICE))
#endif

#ifdef SX127X
  if (LT.begin(NSS, NRESET, DIO0, DIO1, DIO2, LORA_DEVICE))
#endif

#ifdef SX128X
  if (LT.begin(NSS, NRESET, RFBUSY, DIO1, DIO2, DIO3, RX_EN, TX_EN, LORA_DEVICE))
#endif
  {
    PRINTLN_CSTSTR("Lora Device found");
    delay(1000);
  }
  else
  {
    PRINTLN_CSTSTR("No device responding");
    while (1)
    {
    }
  }

  loraConfig();

  PRINTLN;
  LT.printModemSettings();                                     //reads and prints the configured LoRa settings, useful check
  PRINTLN;
  LT.printOperatingSettings();                                 //reads and prints the configured operting settings, useful check
  PRINTLN;
  PRINTLN;
#if defined SX126X || defined SX127X  
  //print contents of device registers, normally 0x00 to 0x4F
  LT.printRegisters(0x00, 0x4F);
#endif                       
#ifdef SX128X
  //print contents of device registers, normally 0x900 to 0x9FF 
  LT.printRegisters(0x900, 0x9FF);
#endif  
  PRINTLN;
  PRINTLN;

  PRINT_CSTSTR("Setting Power: ");
  PRINTLN_VALUE("%d", MAX_DBM); 

  LT.setDevAddr(node_addr);
  PRINT_CSTSTR("node addr: ");
  PRINT_VALUE("%d", node_addr);
  PRINTLN;  
  
#ifdef SX126X
  PRINT_CSTSTR("SX126X - ");
#endif
#ifdef SX127X
  PRINT_CSTSTR("SX127X - ");
#endif
#ifdef SX128X
  PRINT_CSTSTR("SX128X - ");
#endif

  PRINTLN_CSTSTR("ping-pong ready");

#ifdef OLED
  u8x8.begin();
  //u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.setFont(u8x8_font_pxplustandynewtv_r);
  u8x8.drawString(0, 0, "Simple PingPong");
  sprintf(oled_msg,"SF%dBW%d", LT.getLoRaSF(), LT.returnBandwidth()/1000);
  u8x8.drawString(0, 1, oled_msg); 
#endif  
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
  long startSend;
  long endSend;  
  uint8_t r_size;

  uint8_t p_type=PKT_TYPE_DATA | PKT_FLAG_ACK_REQ;
      
  PRINTLN_CSTSTR("Will request an ACK");    
      
  while (1) {
    //LT.CarrierSense();
      
    r_size=sprintf((char*)message, "Ping");
    PRINT_CSTSTR("Sending Ping");  
    PRINTLN;

#ifdef OLED
    u8x8.drawString(0, 3, "Sending Ping....");
    u8x8.clearLine(4);
    u8x8.clearLine(5);
#endif
            
    //will return packet length sent if OK, otherwise 0 if transmit error      
    if (TXPacketL=LT.transmitAddressed(message, r_size, p_type, DEFAULT_DEST_ADDR, LT.readDevAddr(), 10000, MAX_DBM, WAIT_TX))
    {
      endSend = millis();                                          
      TXPacketCount++;
      uint16_t localCRC = LT.CRCCCITT(message, r_size, 0xFFFF);
      PRINT_CSTSTR("CRC,");
      PRINT_HEX("%d", localCRC);

      if (LT.readAckStatus()) {
        PRINTLN;
        PRINT_CSTSTR("Pong received from gateway!");
        PRINTLN;
        sprintf((char*)message,"SNR at gw=%d   ", LT.readPacketSNRinACK());
        PRINT_STR("%s", (char*)message);   
        PRINTLN;

#ifdef OLED             
        u8x8.drawString(0, 3, "Get Pong from gw");
        u8x8.drawString(0, 4, (char*)message);
#endif
        sprintf((char*)message,"gw->SNR=%d:%d", LT.readPacketSNR(), LT.readPacketRSSI());
        PRINT_STR("%s", (char*)message);
        PRINTLN;

#ifdef OLED
        u8x8.clearLine(5);             
        u8x8.drawString(0, 5, (char*)message);
#endif
      }
      else {
        PRINTLN;
        PRINT_CSTSTR("No Pong from gw!");
#ifdef OLED          
        u8x8.drawString(0, 3, "No Pong from gw!");      
#endif         
      }
    }
    else
    {
      //if here there was an error transmitting packet
      uint16_t IRQStatus;
      IRQStatus = LT.readIrqStatus();
      PRINTLN;                      
      PRINT_CSTSTR("SendError,");
      PRINT_CSTSTR(",IRQreg,");
      PRINT_HEX("%d", IRQStatus);
      LT.printIrqStatus();
#ifdef OLED          
      u8x8.drawString(0, 3, "Error sending");  
#endif          
    }

    endSend=millis();

    PRINTLN;  
    delay(idlePeriodInSec*1000);   
  }          
}
