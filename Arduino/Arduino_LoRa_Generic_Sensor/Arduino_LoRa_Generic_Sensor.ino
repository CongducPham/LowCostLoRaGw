/*
 *  Demonstration of generic sensors with the LoRa gateway
 *
 *  Copyright (C) 2016 Congduc Pham, University of Pau, France
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
 * Modified by Nicolas Bertuol (May 2016), University of Pau, France.
 * first version of generic sensor
 * nicolas.bertuol@etud.univ-pau.fr
 * 
 * last update: Nov. 26th by C. Pham
 */
#include <SPI.h> 
// Include the SX1272
#include "SX1272.h"
// Include sensors
#include "Sensor.h"
#include "DHT22_Humidity.h"
#include "DHT22_Temperature.h"
#include "LeafWetness.h"
#include "LM35.h"
//#include <OneWire.h>
#include "DS18B20.h"
#include "rawAnalog.h"
#include "HCSR04.h"
#include "HRLV.h"

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
#if not defined _VARIANT_ARDUINO_DUE_X_ && not defined __SAMD21G18A__
#define WITH_EEPROM
#endif
#define WITH_APPKEY
#define FLOAT_TEMP
#define LOW_POWER
#define LOW_POWER_HIBERNATE
//#define WITH_ACK
//this will enable a receive window after every transmission
//#define WITH_RCVW
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// CHANGE HERE THE LORA MODE, NODE ADDRESS 
#define LORAMODE  1
uint8_t node_addr=13;
//////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// CHANGE HERE THE TIME IN MINUTES BETWEEN 2 READING & TRANSMISSION
unsigned int idlePeriodInMin = 10;
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// COMMENT THIS LINE IF YOU WANT TO DYNAMICALLY SET THE NODE'S ADDR 
// OR SOME OTHER PARAMETERS BY REMOTE RADIO COMMANDS (WITH_RCVW)
// LEAVE THIS LINE UNCOMMENTED IF YOU WANT TO USE THE DEFAULT VALUE
// AND CONFIGURE YOUR DEVICE BY CHANGING MANUALLY THESE VALUES IN 
// THE SKETCH.
//
// ONCE YOU HAVE FLASHED A BOARD WITHOUT FORCE_DEFAULT_VALUE, YOU 
// WILL BE ABLE TO DYNAMICALLY CONFIGURE IT AND SAVE THIS CONFIGU-
// RATION INTO EEPROM. ON RESET, THE BOARD WILL USE THE SAVED CON-
// FIGURATION.

// IF YOU WANT TO REINITIALIZE A BOARD, YOU HAVE TO FIRST FLASH IT 
// WITH FORCE_DEFAULT_VALUE, WAIT FOR ABOUT 10s SO THAT IT CAN BOOT
// AND FLASH IT AGAIN WITHOUT FORCE_DEFAULT_VALUE. THE BOARD WILL 
// THEN USE THE DEFAULT CONFIGURATION UNTIL NEXT CONFIGURATION.

#define FORCE_DEFAULT_VALUE
///////////////////////////////////////////////////////////////////

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
#define FLUSHOUTPUT               SerialUSB.flush();
#else
#define PRINTLN                   Serial.println("")              
#define PRINT_CSTSTR(fmt,param)   Serial.print(F(param))
#define PRINT_STR(fmt,param)      Serial.print(param)
#define PRINT_VALUE(fmt,param)    Serial.print(param)
#define FLUSHOUTPUT               Serial.flush();
#endif

#ifdef WITH_EEPROM
#include <EEPROM.h>
#endif

#define DEFAULT_DEST_ADDR 1

#ifdef WITH_ACK
#define NB_RETRIES 2
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

unsigned long nextTransmissionTime=0L;
uint8_t message[100];
int loraMode=LORAMODE;

#ifdef WITH_EEPROM
struct sx1272config {

  uint8_t flag1;
  uint8_t flag2;
  uint8_t seq;
  uint8_t addr;  
  unsigned int idle_period;  
  uint8_t overwrite;
  // can add other fields such as LoRa mode,...
};

sx1272config my_sx1272config;
#endif

#ifdef WITH_RCVW

// will wait for 5s before opening the rcv window
#define DELAY_BEFORE_RCVW 5000

long getCmdValue(int &i, char* strBuff=NULL) {
  
    char seqStr[7]="******";
    
    int j=0;
    // character '#' will indicate end of cmd value
    while ((char)message[i]!='#' && (i < strlen((char*)message)) && j<strlen(seqStr)) {
            seqStr[j]=(char)message[i];
            i++;
            j++;
    }
    
    // put the null character at the end
    seqStr[j]='\0';
    
    if (strBuff) {
            strcpy(strBuff, seqStr);        
    }
    else
            return (atol(seqStr));
}   
#endif

// SENSORS DEFINITION 
//////////////////////////////////////////////////////////////////
// CHANGE HERE THE NUMBER OF SENSORS, SOME CAN BE NOT CONNECTED
const int number_of_sensors = 8;
//////////////////////////////////////////////////////////////////

// array containing sensors pointers
Sensor* sensor_ptrs[number_of_sensors];

#if not defined _VARIANT_ARDUINO_DUE_X_ && defined FLOAT_TEMP

char *ftoa(char *a, double f, int precision)
{
 long p[] = {0,10,100,1000,10000,100000,1000000,10000000,100000000};
 
 char *ret = a;
 long heiltal = (long)f;
 itoa(heiltal, a, 10);
 while (*a != '\0') a++;
 *a++ = '.';
 long desimal = abs((long)((f - heiltal) * p[precision]));
 itoa(desimal, a, 10);
 return ret;
}
#endif

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
  sensor_ptrs[0] = new LM35("tc", IS_ANALOG, IS_CONNECTED, low_power_status, (uint8_t) A0, (uint8_t) 8);
  sensor_ptrs[1] = new DHT22_Temperature("TC", IS_NOT_ANALOG, IS_CONNECTED, low_power_status, (uint8_t) 3, (uint8_t) 9);
  sensor_ptrs[2] = new DHT22_Humidity("HU", IS_NOT_ANALOG, IS_CONNECTED, low_power_status, (uint8_t) 3, (uint8_t) 9);
  sensor_ptrs[3] = new LeafWetness("lw", IS_ANALOG, IS_NOT_CONNECTED, low_power_status, (uint8_t) A2, (uint8_t) 7);
  sensor_ptrs[4] = new DS18B20("DS", IS_NOT_ANALOG, IS_CONNECTED, low_power_status, (uint8_t) 4, (uint8_t) 7);
  sensor_ptrs[5] = new rawAnalog("SH", IS_ANALOG, IS_CONNECTED, low_power_status, (uint8_t) A1, (uint8_t) 6);
  sensor_ptrs[6] = new HCSR04("DIS", IS_NOT_ANALOG, IS_CONNECTED, low_power_status, (uint8_t) 39, (uint8_t) 41, (uint8_t) 40);
  sensor_ptrs[7] = new HRLV("DIS_", IS_ANALOG, IS_NOT_CONNECTED, low_power_status, (uint8_t) A3, (uint8_t) 5);
  
  // for non connected sensors, indicate whether you want some fake data, for test purposes for instance
  sensor_ptrs[3]->set_fake_data(true);
  //sensor_ptrs[4]->set_fake_data(true); 

//////////////////////////////////////////////////////////////////  

  delay(3000);
  // Open serial communications and wait for port to open:
#if defined __SAMD21G18A__ && not defined ARDUINO_SAMD_FEATHER_M0 
  SerialUSB.begin(38400);
#else
  Serial.begin(38400);  
#endif 
  // Print a start message
  PRINT_CSTSTR("%s","Generic LoRa sensor\n");

#ifdef ARDUINO_AVR_PRO
  PRINT_CSTSTR("%s","Arduino Pro Mini detected\n");
#endif

#ifdef ARDUINO_AVR_NANO
  PRINT_CSTSTR("%s","Arduino Nano detected\n");
#endif

#ifdef ARDUINO_AVR_MINI
  PRINT_CSTSTR("%s","Arduino MINI/Nexus detected\n");
#endif

#ifdef __MK20DX256__
  PRINT_CSTSTR("%s","Teensy31/32 detected\n");
#endif

#ifdef __MKL26Z64__
  PRINT_CSTSTR("%s","TeensyLC detected\n");
#endif

#ifdef __SAMD21G18A__ 
  PRINT_CSTSTR("%s","Arduino M0/Zero detected\n");
#endif

  // Power ON the module
  sx1272.ON();

#ifdef WITH_EEPROM
  // get config from EEPROM
  EEPROM.get(0, my_sx1272config);

  // found a valid config?
  if (my_sx1272config.flag1==0x12 && my_sx1272config.flag2==0x35) {
    PRINT_CSTSTR("%s","Get back previous sx1272 config\n");

    // set sequence number for SX1272 library
    sx1272._packetNumber=my_sx1272config.seq;
    PRINT_CSTSTR("%s","Using packet sequence number of ");
    PRINT_VALUE("%d", sx1272._packetNumber);
    PRINTLN;

#ifdef FORCE_DEFAULT_VALUE
    PRINT_CSTSTR("%s","Forced to use default parameters\n");
    my_sx1272config.flag1=0x12;
    my_sx1272config.flag2=0x35;
    my_sx1272config.seq=sx1272._packetNumber;
    my_sx1272config.addr=node_addr;
    my_sx1272config.idle_period=idlePeriodInMin;    
    my_sx1272config.overwrite=0;
    EEPROM.put(0, my_sx1272config);
#else
    // get back the node_addr
    if (my_sx1272config.addr!=0 && my_sx1272config.overwrite==1) {
      
        PRINT_CSTSTR("%s","Used stored address\n");
        node_addr=my_sx1272config.addr;        
    }
    else
        PRINT_CSTSTR("%s","Stored node addr is null\n"); 

    // get back the idle period
    if (my_sx1272config.idle_period!=0 && my_sx1272config.overwrite==1) {
      
        PRINT_CSTSTR("%s","Used stored idle period\n");
        idlePeriodInMin=my_sx1272config.idle_period;        
    }
    else
        PRINT_CSTSTR("%s","Stored idle period is null\n");                 
#endif  

#ifdef WITH_AES
    DevAddr[3] = (unsigned char)node_addr;
#endif            
    PRINT_CSTSTR("%s","Using node addr of ");
    PRINT_VALUE("%d", node_addr);
    PRINTLN;   

    PRINT_CSTSTR("%s","Using idle period of ");
    PRINT_VALUE("%d", idlePeriodInMin);
    PRINTLN;     
  }
  else {
    // otherwise, write config and start over
    my_sx1272config.flag1=0x12;
    my_sx1272config.flag2=0x35;
    my_sx1272config.seq=sx1272._packetNumber;
    my_sx1272config.addr=node_addr;
    my_sx1272config.idle_period=idlePeriodInMin;
    my_sx1272config.overwrite=0;
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
    
      char final_str[80] = "\\!";
      char aux[6] = "";

      // main loop for sensors, actually, you don't have to edit anything here
      // just add a predefined sensor if needed or provide a new sensor class instance for a handle a new physical sensor
      for (int i=0; i<number_of_sensors; i++) {

          if (sensor_ptrs[i]->get_is_connected() || sensor_ptrs[i]->has_fake_data()) {
            
              ftoa(aux, sensor_ptrs[i]->get_value(), 2);
          
              if (i==0) {
                  //sprintf(final_str, "%s%s/%s", final_str, nomenclatures_array[i], aux);
                  sprintf(final_str, "%s%s/%s", final_str, sensor_ptrs[i]->get_nomenclature(), aux);
                  
              } 
              else {
                  sprintf(final_str, "%s/%s/%s", final_str, sensor_ptrs[i]->get_nomenclature(), aux);
              }
          }
          //else
          //  strcpy(aux,"");
      }
      
      r_size=sprintf((char*)message+app_key_offset, final_str);

      PRINT_CSTSTR("%s","Sending ");
      PRINT_STR("%s",(char*)(message+app_key_offset));
      PRINTLN;
      
      PRINT_CSTSTR("%s","Real payload size is ");
      PRINT_VALUE("%d", r_size);
      PRINTLN;
      
      int pl=r_size+app_key_offset;
      
      sx1272.CarrierSense();
      
      startSend=millis();

      uint8_t p_type=PKT_TYPE_DATA;
      
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
    
#ifdef WITH_EEPROM
      // save packet number for next packet in case of reboot
      my_sx1272config.seq=sx1272._packetNumber;
      EEPROM.put(0, my_sx1272config);
#endif
      
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

#ifdef WITH_RCVW
      PRINT_CSTSTR("%s","Wait for ");
      PRINT_VALUE("%d", DELAY_BEFORE_RCVW-1000);
      PRINTLN;
      //wait a bit
      delay(DELAY_BEFORE_RCVW-1000);

      PRINT_CSTSTR("%s","Wait for incoming packet\n");
      // wait for incoming packets
      e = sx1272.receivePacketTimeout(10000);
    
      if (!e) {
         int i=0;
         int cmdValue;
         uint8_t tmp_length;

         sx1272.getSNR();
         sx1272.getRSSIpacket();
         
         tmp_length=sx1272._payloadlength;

         sprintf((char*)message, "^p%d,%d,%d,%d,%d,%d,%d\n",
                   sx1272.packet_received.dst,
                   sx1272.packet_received.type,                   
                   sx1272.packet_received.src,
                   sx1272.packet_received.packnum, 
                   tmp_length,
                   sx1272._SNR,
                   sx1272._RSSIpacket);
                                   
         PRINT_STR("%s",(char*)message);         
         
         for ( ; i<tmp_length; i++) {
           PRINT_STR("%c",(char)sx1272.packet_received.data[i]);
           
           message[i]=(char)sx1272.packet_received.data[i];
         }
         
         message[i]=(char)'\0';    
         PRINTLN;
         FLUSHOUTPUT;   

        i=0;

        // commands have following format /@A6#
        //
        if (message[i]=='/' && message[i+1]=='@') {
    
            PRINT_CSTSTR("%s","Parsing command\n");      
            i=i+2;   

            switch ((char)message[i]) {

                  // set the node's address, /@A10# to set the address to 10 for instance
                  case 'A': 

                      i++;
                      cmdValue=getCmdValue(i);
                      
                      // cannot set addr greater than 255
                      if (cmdValue > 255)
                              cmdValue = 255;
                      // cannot set addr lower than 2 since 0 is broadcast and 1 is for gateway
                      if (cmdValue < 2)
                              cmdValue = node_addr;
                      // set node addr        
                      node_addr=cmdValue; 
#ifdef WITH_AES
                      DevAddr[3] = (unsigned char)node_addr;
#endif
                      
                      PRINT_CSTSTR("%s","Set LoRa node addr to ");
                      PRINT_VALUE("%d", node_addr);  
                      PRINTLN;
                      // Set the node address and print the result
                      e = sx1272.setNodeAddress(node_addr);
                      PRINT_CSTSTR("%s","Setting LoRa node addr: state ");
                      PRINT_VALUE("%d",e);     
                      PRINTLN;           

#ifdef WITH_EEPROM
                      // save new node_addr in case of reboot
                      my_sx1272config.addr=node_addr;
                      my_sx1272config.overwrite=1;
                      EEPROM.put(0, my_sx1272config);
#endif

                      break;        

                  // set the time between 2 transmissions, /@I10# to set to 10 minutes for instance
                  case 'I': 

                      i++;
                      cmdValue=getCmdValue(i);

                      // cannot set addr lower than 1 minute
                      if (cmdValue < 1)
                              cmdValue = idlePeriodInMin;
                      // idlePeriodInMin      
                      idlePeriodInMin=cmdValue; 
                      
                      PRINT_CSTSTR("%s","Set duty-cycle to ");
                      PRINT_VALUE("%d", idlePeriodInMin);  
                      PRINTLN;         

#ifdef WITH_EEPROM
                      // save new node_addr in case of reboot
                      my_sx1272config.idle_period=idlePeriodInMin;
                      my_sx1272config.overwrite=1;
                      EEPROM.put(0, my_sx1272config);
#endif

                      break;  
                            
                  /////////////////////////////////////////////////////////////////////////////////////////////////////////////
                  // add here new commands
                  //  

                  //
                  /////////////////////////////////////////////////////////////////////////////////////////////////////////////

                  default:
      
                    PRINT_CSTSTR("%s","Unrecognized cmd\n");       
                    break;
            }
        }          
      }
      else
        PRINT_CSTSTR("%s","No packet\n");
#endif

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
      //LowPower.standby();
      rtc.setTime(17, 0, 0);
      rtc.setDate(1, 1, 2000);
      rtc.setAlarmTime(17, idlePeriodInMin, 0);
      // for testing with 20s
      //rtc.setAlarmTime(17, 0, 20);
      rtc.enableAlarm(rtc.MATCH_HHMMSS);
      //rtc.attachInterrupt(alarmMatch);
      rtc.standbyMode();

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

#if defined ARDUINO_AVR_PRO || defined ARDUINO_AVR_NANO || ARDUINO_AVR_UNO || ARDUINO_AVR_MINI  
          // ATmega328P, ATmega168
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
          FLUSHOUTPUT; 
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
