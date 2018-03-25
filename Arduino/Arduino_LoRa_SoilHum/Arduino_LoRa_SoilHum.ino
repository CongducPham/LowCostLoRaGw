/*
 *  Soil humidity sensor
 *  extended version with AES and custom Carrier Sense features
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
 * last update: May 15th, 2017 for the WaterSense project by C. Pham
 * 
 * Sensor 1 is connected to digital 9 for power and A0 for output value
 * Sensor 2 is connected to digital 8 for power and A1 for output value
 */
 
#include <SPI.h> 
// Include the SX1272
#include "SX1272.h"
// Include sensors
#include "Sensor.h"
#include "rawAnalog.h"
//#include "DS18B20.h"

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
// please uncomment only 1 choice
#define BAND868
//#define BAND900
//#define BAND433
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

#ifdef BAND868
#ifdef SENEGAL_REGULATION
const uint32_t DEFAULT_CHANNEL=CH_04_868;
#else
const uint32_t DEFAULT_CHANNEL=CH_10_868;
#endif
#elif defined BAND900
const uint32_t DEFAULT_CHANNEL=CH_05_900;
#elif defined BAND433
const uint32_t DEFAULT_CHANNEL=CH_00_433;
#endif

// IMPORTANT
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// uncomment if your radio is an HopeRF RFM92W, HopeRF RFM95W, Modtronix inAir9B, NiceRF1276
// or you known from the circuit diagram that output use the PABOOST line instead of the RFO line
//#define PABOOST
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 

///////////////////////////////////////////////////////////////////
// COMMENT OR UNCOMMENT TO CHANGE FEATURES. 
// ONLY IF YOU KNOW WHAT YOU ARE DOING!!! OTHERWISE LEAVE AS IT IS
#define WITH_EEPROM
#define WITH_APPKEY
#define LOW_POWER
#define LOW_POWER_HIBERNATE
#define WITH_AES
//#define WITH_ACK
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// CHANGE HERE THE LORA MODE, NODE ADDRESS 
#define LORAMODE  1
// you need to change the node address for each sensor in the same organization/farm
// node address starts at 2 and ends at 255
#define node_addr 2
//////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// CHANGE HERE THE THINGSPEAK FIELD BETWEEN 1 AND 8
#define field_index 1
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// CHANGE HERE THE TIME IN MINUTES BETWEEN 2 READING & TRANSMISSION
unsigned int idlePeriodInMin = 60;
///////////////////////////////////////////////////////////////////

// SENSORS DEFINITION 
//////////////////////////////////////////////////////////////////
// CHANGE HERE THE NUMBER OF SENSORS, SOME CAN BE NOT CONNECTED
const int number_of_sensors = 1;
//////////////////////////////////////////////////////////////////

#ifdef WITH_APPKEY
///////////////////////////////////////////////////////////////////
// CHANGE HERE THE APPKEY, BUT IF GW CHECKS FOR APPKEY, MUST BE
// IN THE APPKEY LIST MAINTAINED BY GW.
uint8_t my_appKey[4]={5, 6, 7, 8};
///////////////////////////////////////////////////////////////////
#endif

// we wrapped Serial.println to support the Arduino Zero or M0
#if defined __SAMD21G18A__ && not defined ARDUINO_SAMD_FEATHER_M0
#define PRINTLN                   SerialUSB.println("")              
#define PRINT_CSTSTR(fmt,param)   SerialUSB.print(F(param))
#define PRINT_STR(fmt,param)      SerialUSB.print(param)
#define PRINT_VALUE(fmt,param)    SerialUSB.print(param)
#define PRINT_HEX(fmt,param)      SerialUSB.print(param,HEX)
#define FLUSHOUTPUT               SerialUSB.flush();
#else
#define PRINTLN                   Serial.println("")
#define PRINT_CSTSTR(fmt,param)   Serial.print(F(param))
#define PRINT_STR(fmt,param)      Serial.print(param)
#define PRINT_VALUE(fmt,param)    Serial.print(param)
#define PRINT_HEX(fmt,param)      Serial.print(param,HEX)
#define FLUSHOUTPUT               Serial.flush();
#endif

#ifdef WITH_EEPROM
#include <EEPROM.h>
#endif

#define DEFAULT_DEST_ADDR 1

#ifdef WITH_ACK
#define NB_RETRIES 2
#endif

#if defined ARDUINO_AVR_PRO || defined ARDUINO_AVR_MINI || defined ARDUINO_SAM_DUE || defined __MK20DX256__ || defined __MKL26Z64__ || defined __MK64FX512__ || defined __MK66FX1M0__ || defined __SAMD21G18A__
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

#ifdef LOW_POWER
// this is for the Teensy36, Teensy35, Teensy31/32 & TeensyLC
// need v6 of Snooze library
#if defined __MK20DX256__ || defined __MKL26Z64__ || defined __MK64FX512__ || defined __MK66FX1M0__
#define LOW_POWER_PERIOD 60
#include <Snooze.h>
SnoozeTimer timer;
SnoozeBlock sleep_config(timer);
#else
#define LOW_POWER_PERIOD 8
// you need the LowPower library from RocketScream
// https://github.com/rocketscream/Low-Power
#include "LowPower.h"

#ifdef __SAMD21G18A__
// use the RTC library
#include "RTCZero.h"
/* Create an rtc object */
RTCZero rtc;
#endif
#endif
unsigned int nCycle = idlePeriodInMin*60/LOW_POWER_PERIOD;
#endif

#ifdef WITH_AES

#include "AES-128_V10.h"
#include "Encrypt_V31.h"

unsigned char AppSkey[16] = {
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
  0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
};

unsigned char NwkSkey[16] = {
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
  0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
};

unsigned char DevAddr[4] = {
  0x00, 0x00, 0x00, node_addr
};

uint16_t Frame_Counter_Up = 0x0000;
// we use the same convention than for LoRaWAN as we will use the same AES convention
// See LoRaWAN specifications
unsigned char Direction = 0x00;
#endif

unsigned long nextTransmissionTime=0L;
uint8_t message[50];
int loraMode=LORAMODE;

#ifdef WITH_EEPROM
struct sx1272config {

  uint8_t flag1;
  uint8_t flag2;
  uint8_t seq;
  // can add other fields such as LoRa mode,...
};

sx1272config my_sx1272config;
#endif

// array containing sensors pointers
Sensor* sensor_ptrs[number_of_sensors];

void setup()
{
  int e;

#ifdef LOW_POWER
  bool low_power_status = IS_LOWPOWER;
#ifdef __SAMD21G18A__
  rtc.begin();
#endif  
#else
  bool low_power_status = IS_NOT_LOWPOWER;
#endif

//////////////////////////////////////////////////////////////////
// ADD YOUR SENSORS HERE   
// Sensor(nomenclature, is_analog, is_connected, is_low_power, pin_read, pin_power, pin_trigger=-1)
  sensor_ptrs[0] = new rawAnalog("SM1", IS_ANALOG, IS_CONNECTED, low_power_status, (uint8_t) A0, (uint8_t) 9);
  //sensor_ptrs[1] = new rawAnalog("SM2", IS_ANALOG, IS_CONNECTED, low_power_status, (uint8_t) A1, (uint8_t) 8);
  //sensor_ptrs[2] = new DS18B20("DS", IS_NOT_ANALOG, IS_CONNECTED, low_power_status, (uint8_t) 4, (uint8_t) 7);  
  //sensor_ptrs[2] = new rawAnalog("SM3", IS_ANALOG, IS_CONNECTED, low_power_status, (uint8_t) A2, (uint8_t) 7);

  sensor_ptrs[0]->set_n_sample(10);
  //sensor_ptrs[1]->set_n_sample(10);   
  //sensor_ptrs[2]->set_n_sample(10);
  
  delay(3000);
  // Open serial communications and wait for port to open:
#if defined __SAMD21G18A__ && not defined ARDUINO_SAMD_FEATHER_M0 
  SerialUSB.begin(38400);
#else
  Serial.begin(38400);  
#endif  
  // Print a start message
  PRINT_CSTSTR("%s","LoRa soil moisture sensor, extended version\n");

#ifdef ARDUINO_AVR_PRO
  PRINT_CSTSTR("%s","Arduino Pro Mini detected\n");  
#endif
#ifdef ARDUINO_AVR_NANO
  PRINT_CSTSTR("%s","Arduino Nano detected\n");   
#endif
#ifdef ARDUINO_AVR_MINI
  PRINT_CSTSTR("%s","Arduino Mini/Nexus detected\n");  
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
#ifdef ARDUINO_SAMD_FEATHER_M0
  PRINT_CSTSTR("%s","Adafruit FeatherM0 detected\n");
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
  PRINT_CSTSTR("%s","SAMD21G18A ARM Cortex-M0 detected\n");
#endif
#ifdef __SAM3X8E__ 
  PRINT_CSTSTR("%s","SAM3X8E ARM Cortex-M3 detected\n");
#endif

  // Power ON the module
  sx1272.ON();

#ifdef WITH_EEPROM
  // get config from EEPROM
  EEPROM.get(0, my_sx1272config);

  // found a valid config?
  if (my_sx1272config.flag1==0x12 && my_sx1272config.flag2==0x34) {
    PRINT_CSTSTR("%s","Get back previous sx1272 config\n");

    // set sequence number for SX1272 library
    sx1272._packetNumber=my_sx1272config.seq;
    PRINT_CSTSTR("%s","Using packet sequence number of ");
    PRINT_VALUE("%d", sx1272._packetNumber);
    PRINTLN;
  }
  else {
    // otherwise, write config and start over
    my_sx1272config.flag1=0x12;
    my_sx1272config.flag2=0x34;
    my_sx1272config.seq=sx1272._packetNumber;
  }
#endif
  
  // Set transmission mode and print the result
  e = sx1272.setMode(loraMode);
  PRINT_CSTSTR("%s","Setting Mode: state ");
  PRINT_VALUE("%d", e);
  PRINTLN;
  
  // enable carrier sense
  sx1272._enableCarrierSense=true;  
#ifdef LOW_POWER
  // TODO: with low power, when setting the radio module in sleep mode
  // there seem to be some issue with RSSI reading
  sx1272._RSSIonSend=false;
#endif  

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

void loop(void)
{
  long startSend;
  long endSend;
  uint8_t app_key_offset=0;
  int e;

#ifndef LOW_POWER
  // 600000+random(15,60)*1000
  if (millis() > nextTransmissionTime) {
#endif

#ifdef WITH_APPKEY
      app_key_offset = sizeof(my_appKey);
      // set the app key in the payload
      memcpy(message,my_appKey,app_key_offset);
#endif

      uint8_t r_size;

      char final_str[40] = "\\!";

      // main loop for sensors, actually, you don't have to edit anything here
      // just add a predefined sensor if needed or provide a new sensor class instance for a handle a new physical sensor
      for (int i=0; i<number_of_sensors; i++) {

          if (sensor_ptrs[i]->get_is_connected() || sensor_ptrs[i]->has_fake_data()) {

              int value = (int)sensor_ptrs[i]->get_value();

              PRINT_CSTSTR("%s","SM");
              PRINT_VALUE("%d", i+1); 
              PRINT_CSTSTR("%s", " on 10 sample is ");
              PRINT_VALUE("%d", value);
              PRINTLN;   
      
              if (i==0) {
                  sprintf(final_str, "%s%s/%d", final_str, sensor_ptrs[i]->get_nomenclature(), value);
              } 
              else {
                  sprintf(final_str, "%s/%s/%d", final_str, sensor_ptrs[i]->get_nomenclature(), value);
              }
          }
      }
      
      r_size=sprintf((char*)message+app_key_offset, final_str);

      PRINT_CSTSTR("%s","Sending ");
      PRINT_STR("%s",(char*)(message+app_key_offset));
      PRINTLN;
      
      PRINT_CSTSTR("%s","Real payload size is ");
      PRINT_VALUE("%d", r_size);
      PRINTLN;
      
      int pl=r_size+app_key_offset;
      
#ifdef WITH_AES
      //
      PRINT_STR("%s",(char*)(message+app_key_offset));
      PRINTLN;
      PRINT_CSTSTR("%s","plain payload hex\n");
      for (int i=0; i<pl;i++) {
        if (message[i]<16)
          PRINT_CSTSTR("%s","0");
        PRINT_HEX("%X", message[i]);
        PRINT_CSTSTR("%s"," ");
      }
      PRINTLN; 

      PRINT_CSTSTR("%s","Encrypting\n");     
      PRINT_CSTSTR("%s","encrypted payload\n");
      Encrypt_Payload((unsigned char*)message, pl, Frame_Counter_Up, Direction);
      //Print encrypted message
      for(int i = 0; i < pl; i++)      
      {
        if (message[i]<16)
          PRINT_CSTSTR("%s","0");
        PRINT_HEX("%X", message[i]);         
        PRINT_CSTSTR("%s"," ");
      }
      PRINTLN;   

      // with encryption, we use for the payload a LoRaWAN packet format to reuse available LoRaWAN encryption libraries
      //
      unsigned char LORAWAN_Data[128];
      unsigned char LORAWAN_Package_Length;
      unsigned char MIC[4];
      //Unconfirmed data up
      unsigned char Mac_Header = 0x40;
      // no ADR, not an ACK and no options
      unsigned char Frame_Control = 0x00;
      // with application data so Frame_Port = 1..223
      unsigned char Frame_Port = 0x01; 

      //Build the Radio Package, LoRaWAN format
      //See LoRaWAN specification
      LORAWAN_Data[0] = Mac_Header;
    
      LORAWAN_Data[1] = DevAddr[3];
      LORAWAN_Data[2] = DevAddr[2];
      LORAWAN_Data[3] = DevAddr[1];
      LORAWAN_Data[4] = DevAddr[0];
    
      LORAWAN_Data[5] = Frame_Control;
    
      LORAWAN_Data[6] = (Frame_Counter_Up & 0x00FF);
      LORAWAN_Data[7] = ((Frame_Counter_Up >> 8) & 0x00FF);
    
      LORAWAN_Data[8] = Frame_Port;
    
      //Set Current package length
      LORAWAN_Package_Length = 9;
      
      //Load Data
      for(int i = 0; i < r_size+app_key_offset; i++)
      {
        // see that we don't take the appkey, just the encrypted data that starts that message[app_key_offset]
        LORAWAN_Data[LORAWAN_Package_Length + i] = message[i];
      }
    
      //Add data Lenth to package length
      LORAWAN_Package_Length = LORAWAN_Package_Length + r_size + app_key_offset;
    
      PRINT_CSTSTR("%s","calculate MIC with NwkSKey\n");
      //Calculate MIC
      Calculate_MIC(LORAWAN_Data, MIC, LORAWAN_Package_Length, Frame_Counter_Up, Direction);
    
      //Load MIC in package
      for(int i=0; i < 4; i++)
      {
        LORAWAN_Data[i + LORAWAN_Package_Length] = MIC[i];
      }
    
      //Add MIC length to package length
      LORAWAN_Package_Length = LORAWAN_Package_Length + 4;
    
      PRINT_CSTSTR("%s","transmitted LoRaWAN-like packet:\n");
      PRINT_CSTSTR("%s","MHDR[1] | DevAddr[4] | FCtrl[1] | FCnt[2] | FPort[1] | EncryptedPayload | MIC[4]\n");
      //Print transmitted data
      for(int i = 0; i < LORAWAN_Package_Length; i++)
      {
        if (LORAWAN_Data[i]<16)
          PRINT_CSTSTR("%s","0");
        PRINT_HEX("%X", LORAWAN_Data[i]);         
        PRINT_CSTSTR("%s"," ");
      }
      PRINTLN;      

      // copy back to message
      memcpy(message,LORAWAN_Data,LORAWAN_Package_Length);
      pl = LORAWAN_Package_Length;

#ifdef LORAWAN
      PRINT_CSTSTR("%s","end-device uses native LoRaWAN packet format\n");
      // indicate to SX1272 lib that raw mode at transmission is required to avoid our own packet header
      sx1272._rawFormat=true;
#else
      PRINT_CSTSTR("%s","end-device uses encapsulated LoRaWAN packet format only for encryption\n");
#endif
      // in any case, we increment Frame_Counter_Up
      // even if the transmission will not succeed
      Frame_Counter_Up++;
#endif
    
      sx1272.CarrierSense();

      startSend=millis();

      uint8_t p_type=PKT_TYPE_DATA;
      
#ifdef WITH_AES
      // indicate that payload is encrypted
      p_type = p_type | PKT_FLAG_DATA_ENCRYPTED;
#endif

#ifdef WITH_APPKEY
      // indicate that we have an appkey
      p_type = p_type | PKT_FLAG_DATA_WAPPKEY;
#endif

      sx1272.setPacketType(p_type);
      
      // Send message to the gateway and print the result
      // with the app key if this feature is enabled
#ifdef WITH_ACK
      int n_retry=NB_RETRIES;
      
      do {
        e = sx1272.sendPacketTimeoutACK(DEFAULT_DEST_ADDR, message, pl);

        if (e==3)
          PRINT_CSTSTR("%s","No ACK");
        
        n_retry--;
        
        if (n_retry)
          PRINT_CSTSTR("%s","Retry");
        else
          PRINT_CSTSTR("%s","Abort"); 
          
      } while (e && n_retry);          
#else
      e = sx1272.sendPacketTimeout(DEFAULT_DEST_ADDR, message, pl);
#endif
  
      endSend=millis();

#ifdef LORAWAN
      // switch back to normal behavior
      sx1272._rawFormat=false;
#endif
          
#ifdef WITH_EEPROM
      // save packet number for next packet in case of reboot
      my_sx1272config.seq=sx1272._packetNumber;
      EEPROM.put(0, my_sx1272config);
#endif

      PRINT_CSTSTR("%s","LoRa pkt size ");
      PRINT_VALUE("%d", pl);
      PRINTLN;
      
      PRINT_CSTSTR("%s","LoRa pkt seq ");
      PRINT_VALUE("%d", sx1272.packet_sent.packnum);
      PRINTLN;
    
      PRINT_CSTSTR("%s","LoRa Sent in ");
      PRINT_VALUE("%ld", endSend-startSend);
      PRINTLN;
          
      PRINT_CSTSTR("%s","LoRa Sent w/CAD in ");
      PRINT_VALUE("%ld", endSend-sx1272._startDoCad);
      PRINTLN;
       
      PRINT_CSTSTR("%s","Packet sent, state ");
      PRINT_VALUE("%d", e);
      PRINTLN;

#ifdef LOW_POWER
      PRINT_CSTSTR("%s","Switch to power saving mode\n");

      e = sx1272.setSleepMode();

      if (!e)
        PRINT_CSTSTR("%s","Successfully switch LoRa module in sleep mode\n");
      else  
        PRINT_CSTSTR("%s","Could not switch LoRa module in sleep mode\n");
        
      FLUSHOUTPUT
      delay(50);
      
#ifdef __SAMD21G18A__
      // For Arduino M0 or Zero we use the built-in RTC
      rtc.setTime(17, 0, 0);
      rtc.setDate(1, 1, 2000);
      rtc.setAlarmTime(17, idlePeriodInMin, 0);
      // for testing with 20s
      //rtc.setAlarmTime(17, 0, 20);
      rtc.enableAlarm(rtc.MATCH_HHMMSS);
      //rtc.attachInterrupt(alarmMatch);
      rtc.standbyMode();

      LowPower.standby();
      
      PRINT_CSTSTR("%s","SAMD21G18A wakes up from standby\n");      
      FLUSHOUTPUT
#else
      nCycle = idlePeriodInMin*60/LOW_POWER_PERIOD + random(2,4);

#if defined __MK20DX256__ || defined __MKL26Z64__ || defined __MK64FX512__ || defined __MK66FX1M0__
      // warning, setTimer accepts value from 1ms to 65535ms max
      timer.setTimer(LOW_POWER_PERIOD*1000 + random(1,5)*1000);// milliseconds

      nCycle = idlePeriodInMin*60/LOW_POWER_PERIOD;
#endif
          
      for (int i=0; i<nCycle; i++) {  

#if defined ARDUINO_AVR_PRO || defined ARDUINO_AVR_NANO || defined ARDUINO_AVR_UNO || defined ARDUINO_AVR_MINI || defined __AVR_ATmega32U4__         
          // ATmega328P, ATmega168, ATmega32U4
          LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
          
          //LowPower.idle(SLEEP_8S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, 
          //              SPI_OFF, USART0_OFF, TWI_OFF);
#elif defined ARDUINO_AVR_MEGA2560
          // ATmega2560
          LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
          
          //LowPower.idle(SLEEP_8S, ADC_OFF, TIMER5_OFF, TIMER4_OFF, TIMER3_OFF, 
          //      TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART3_OFF, 
          //      USART2_OFF, USART1_OFF, USART0_OFF, TWI_OFF);
#elif defined __MK20DX256__ || defined __MKL26Z64__ || defined __MK64FX512__ || defined __MK66FX1M0__
          // Teensy31/32 & TeensyLC
#ifdef LOW_POWER_HIBERNATE
          Snooze.hibernate(sleep_config);
#else            
          Snooze.deepSleep(sleep_config);
#endif  
#else
          // use the delay function
          delay(LOW_POWER_PERIOD*1000);
#endif                        
          PRINT_CSTSTR("%s",".");
          FLUSHOUTPUT
          delay(10);                        
      }
      
      delay(50);
#endif  
      
#else
      PRINT_VALUE("%ld", nextTransmissionTime);
      PRINTLN;
      PRINT_CSTSTR("%s","Will send next value at\n");
      // use a random part also to avoid collision
      nextTransmissionTime=millis()+(unsigned long)idlePeriodInMin*60*1000+(unsigned long)random(15,60)*1000;
      PRINT_VALUE("%ld", nextTransmissionTime);
      PRINTLN;
  }
#endif
}
