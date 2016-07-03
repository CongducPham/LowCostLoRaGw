/*
 *  temperature sensor on analog 8 to test the LoRa gateway
 *
 *  Copyright (C) 2015 Congduc Pham, University of Pau, France
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

// IMPORTANT
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// please uncomment only 1 choice
//
// uncomment if your radio is an HopeRF RFM92W or RFM95W
//#define RADIO_RFM92_95
// uncomment if your radio is a Modtronix inAirB (the one with +20dBm features), if inAir9, leave commented
//#define RADIO_INAIR9B
// uncomment if you only know that it has 20dBm feature
//#define RADIO_20DBM
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 

// IMPORTANT
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// please uncomment only 1 choice
#define BAND868
//#define BAND900
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _VARIANT_ARDUINO_DUE_X_
#define WITH_EEPROM
#endif
#define WITH_APPKEY
#define FLOAT_TEMP
//#define NEW_DATA_FIELD
#define LOW_POWER
#define LOW_POWER_HIBERNATE
//#define WITH_ACK

#ifdef WITH_EEPROM
#include <EEPROM.h>
#endif

#ifdef WITH_ACK
#define NB_RETRIES 2
#endif

#define DEFAULT_DEST_ADDR 1
#define LORAMODE  1
#define node_addr 8
#define field_index 4

#define TEMP_PIN_READ  A0
// use digital 8 to power the temperature sensor if needed
#define TEMP_PIN_POWER 8
  
#if defined ARDUINO_AVR_PRO || defined ARDUINO_AVR_MINI || defined __MK20DX256__ // Nexus board from Ideetron is a Mini
  #define SX1272_POWER 9
  // the Pro Mini works in 3.3V
  #define TEMP_SCALE  3300.0
#else // ARDUINO_AVR_NANO || defined ARDUINO_AVR_UNO || defined ARDUINO_AVR_MEGA2560 
  // the SX1272 will be powered by the 3.3V pin
  #define TEMP_SCALE  5000.0
#endif

///////////////////////////////////////////////////////////////////
// CHANGE HERE THE TIME IN MINUTES BETWEEN 2 READING & TRANSMISSION
unsigned int idlePeriodInMin = 10;
///////////////////////////////////////////////////////////////////

#ifdef LOW_POWER
// this is for the Teensy31/32
#ifdef __MK20DX256__
#define LOW_POWER_PERIOD 60
#include <Snooze.h>
SnoozeBlock sleep_config;
#else
#define LOW_POWER_PERIOD 8
// you need the LowPower library from RocketScream
// https://github.com/rocketscream/Low-Power
#include "LowPower.h"
#endif
unsigned int nCycle = idlePeriodInMin*60/LOW_POWER_PERIOD;
#endif

double temp;
unsigned long lastTransmissionTime=0;
unsigned long delayBeforeTransmit=0;
char float_str[20];
uint8_t message[100];

int loraMode=LORAMODE;

uint8_t my_appKey[4]={5, 6, 7, 8};
uint8_t DevId[4]={0x01, 0x02, 0x03, 0x04};

#ifdef WITH_EEPROM
struct sx1272config {

  uint8_t flag1;
  uint8_t flag2;
  uint8_t seq;
  // can add other fields such as LoRa mode,...
};

sx1272config my_sx1272config;
#endif

void setup()
{
  int e;
  
#ifdef ARDUINO_AVR_PRO // add here other boards if you power the lora module with a digital pin
  // We now use the VCC pin which delivers 3.3v
  // We observed stability issue when using a digital pin as the current is very limited.
  // OBSOLETE: on the Pro Mini, we use digital 9 to power the SX1272
  //pinMode(SX1272_POWER,OUTPUT);
  //digitalWrite(SX1272_POWER,HIGH); 
#endif

  // for the temperature sensor
  pinMode(TEMP_PIN_READ, INPUT);
  // and to power the temperature sensor
  pinMode(TEMP_PIN_POWER,OUTPUT);

#ifndef LOW_POWER
  digitalWrite(TEMP_PIN_POWER,HIGH);
#endif
  
  delay(3000);
#ifdef _VARIANT_ARDUINO_DUE_X_
  Serial.begin(115200);  
#else  
  // Open serial communications and wait for port to open:
  Serial.begin(38400);
#endif  
  // Print a start message
  Serial.println(F("Simple LoRa temperature sensor"));

#ifdef ARDUINO_AVR_PRO
  Serial.println(F("Arduino Pro Mini detected"));
#endif

#ifdef ARDUINO_AVR_NANO
  Serial.println(F("Arduino Nano detected"));
#endif

#ifdef ARDUINO_AVR_MINI
  Serial.println(F("Arduino MINI/Nexus detected"));
#endif

#ifdef __MK20DX256__
  Serial.println(F("Teensy31/32 detected"));
#endif

  // Power ON the module
  sx1272.ON();

#ifdef WITH_EEPROM
  // get config from EEPROM
  EEPROM.get(0, my_sx1272config);

  // found a valid config?
  if (my_sx1272config.flag1==0x12 && my_sx1272config.flag2==0x34) {
    Serial.println(F("Get back previous sx1272 config"));

    // set sequence number for SX1272 library
    sx1272._packetNumber=my_sx1272config.seq;
    Serial.print(F("Using packet sequence number of "));
    Serial.println(sx1272._packetNumber);
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
  Serial.print(F("Setting Mode: state "));
  Serial.println(e, DEC);

  // enable carrier sense
  sx1272._enableCarrierSense=true;
#ifdef LOW_POWER
  // TODO: with low power, when setting the radio module in sleep mode
  // there seem to be some issue with RSSI reading
  sx1272._RSSIonSend=false;
#endif   
    
#ifdef BAND868
  // Select frequency channel
  e = sx1272.setChannel(CH_10_868);
#else // assuming #defined BAND900
  // Select frequency channel
  e = sx1272.setChannel(CH_05_900);
#endif
  // just a dirty patch to test 433MHz with a radio module working in this band, e.g. inAir4 for instance
  //e = sx1272.setChannel(0x6C4000);
  Serial.print(F("Setting Channel: state "));
  Serial.println(e, DEC);
  
  // Select output power (Max, High or Low)
#if defined RADIO_RFM92_95 || defined RADIO_INAIR9B
  e = sx1272.setPower('x');
#else
  e = sx1272.setPower('M');
#endif  

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

void loop(void)
{
  long startSend;
  long endSend;
  uint8_t app_key_offset=0;
  int e;

#ifndef LOW_POWER
  // 600000+random(15,60)*1000
  if (millis()-lastTransmissionTime > delayBeforeTransmit) {
#endif

#ifdef LOW_POWER
      digitalWrite(TEMP_PIN_POWER,HIGH);
      // security?
      delay(200);    
      int value = analogRead(TEMP_PIN_READ);
      digitalWrite(TEMP_PIN_POWER,LOW);
#else
      int value = analogRead(TEMP_PIN_READ);
#endif
      
      // change here how the temperature should be computed depending on your sensor type
      //  
      temp = value*TEMP_SCALE/1024.0;
    
      Serial.print(F("Reading "));
      Serial.println(value);
      
      //temp = temp - 0.5;
      temp = temp / 10.0;

      Serial.print(F("(Temp is "));
      Serial.println(temp);

#ifdef WITH_APPKEY
      app_key_offset = sizeof(my_appKey);
      // set the app key in the payload
      memcpy(message,my_appKey,app_key_offset);
#endif

      uint8_t r_size;

      // then use app_key_offset to skip the app key
#ifdef _VARIANT_ARDUINO_DUE_X_
#ifdef NEW_DATA_FIELD
      r_size=sprintf((char*)message+app_key_offset, "\\!#%d#TC/%.2f", field_index, temp);
#else
      r_size=sprintf((char*)message+app_key_offset, "\\!#%d#%.2f", field_index, temp);
#endif      
#else
    
#ifdef FLOAT_TEMP
      ftoa(float_str,temp,2);

#ifdef NEW_DATA_FIELD
      r_size=sprintf((char*)message+app_key_offset, "\\!#%d#TC/%s", field_index, float_str);
#else
      // this is for testing, uncomment if you just want to test, without a real temp sensor plugged
      //strcpy(float_str, "21.55567");
      r_size=sprintf((char*)message+app_key_offset, "\\!#%d#%s", field_index, float_str);
#endif
      
#else
      
#ifdef NEW_DATA_FIELD      
      r_size=sprintf((char*)message+app_key_offset, "\\!#%d#TC/%d", field_index, (int)temp);   
#else
      r_size=sprintf((char*)message+app_key_offset, "\\!#%d#%d", field_index, (int)temp);
#endif         
#endif
#endif

      Serial.print(F("Sending "));
      Serial.println((char*)(message+app_key_offset));

      Serial.print(F("Real payload size is "));
      Serial.println(r_size);
      
      int pl=r_size+app_key_offset;
      
      sx1272.CarrierSense();
      
      startSend=millis();

      // indicate that we have an appkey
      sx1272.setPacketType(PKT_TYPE_DATA | PKT_FLAG_DATA_WAPPKEY);
      
      // Send message to the gateway and print the result
      // with the app key if this feature is enabled
#ifdef WITH_ACK
      int n_retry=NB_RETRIES;
      
      do {
        e = sx1272.sendPacketTimeoutACK(DEFAULT_DEST_ADDR, message, pl);

        if (e==3)
          Serial.println(F("No ACK"));
        
        n_retry--;
        
        if (n_retry)
          Serial.println(F("Retry"));
        else
          Serial.println(F("Abort")); 
          
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
      
      Serial.print(F("LoRa pkt seq "));
      Serial.println(sx1272.packet_sent.packnum);
    
      Serial.print(F("LoRa Sent in "));
      Serial.println(endSend-startSend);
          
      Serial.print(F("LoRa Sent w/CAD in "));

      Serial.println(endSend-sx1272._startDoCad);

      Serial.print(F("Packet sent, state "));
      Serial.println(e);
        
#ifdef LOW_POWER
      Serial.println(F("Switch to power saving mode"));

      e = sx1272.setSleepMode();

      if (!e)
        Serial.println(F("Successfully switch LoRa module in sleep mode"));
      else  
        Serial.println(F("Could not switch LoRa module in sleep mode"));
        
      Serial.flush();
      delay(50);
      
      nCycle = idlePeriodInMin*60/LOW_POWER_PERIOD + random(2,4);

#ifdef __MK20DX256__ 
      sleep_config.setTimer(LOW_POWER_PERIOD*1000);// milliseconds
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
#elif defined __MK20DX256__  
          // Teensy3.2
#ifdef LOW_POWER_HIBERNATE
          Snooze.hibernate(sleep_config);
#else            
          Snooze.deepSleep(sleep_config);
#endif  
#else
          // use the delay function
          delay(LOW_POWER_PERIOD*1000);
#endif                        
          Serial.print(".");
          Serial.flush(); 
          delay(10);                        
      }
      
      delay(50);
#else
      // use a random part also to avoid collision
      Serial.println(lastTransmissionTime);
      Serial.println("Will send next value in");
      lastTransmissionTime=millis();
      delayBeforeTransmit=idlePeriodInMin*60*1000+random(15,60)*1000;
      Serial.println(delayBeforeTransmit);
  }
#endif

  delay(50);
}
