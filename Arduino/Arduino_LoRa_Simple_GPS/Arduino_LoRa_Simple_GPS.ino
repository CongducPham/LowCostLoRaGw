/*
 *  GPS sensor to test the LoRa gateway
 *
 *  Copyright (C) 2017-2019 Congduc Pham & Mamour Diop, University of Pau, France
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
 * last update: May 21th, 2019 by C. Pham
 */
 
#include <SPI.h> 
// Include the SX1272
#include "SX1272.h"

#define SWSERIAL

#ifdef SWSERIAL
#include <SoftwareSerial.h> 
//you can also install and use NeoSWSerial
//#include <NeoSWSerial.h>
// connect GPS_TX <> 6 and GPS_RX <> 5
SoftwareSerial gps_serial(6,5); //rx,tx for Arduino side
//NeoSWSerial gps_serial(6,5);
#endif

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
#define PABOOST
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 

///////////////////////////////////////////////////////////////////
// COMMENT OR UNCOMMENT TO CHANGE FEATURES. 
// ONLY IF YOU KNOW WHAT YOU ARE DOING!!! OTHERWISE LEAVE AS IT IS
#define WITH_EEPROM
#define WITH_APPKEY
#define LOW_POWER
#define LOW_POWER_HIBERNATE
#define SHOW_LOW_POWER_CYCLE
//uncomment to use a customized frequency. TTN plan includes 868.1/868.3/868.5/867.1/867.3/867.5/867.7/867.9 for LoRa
//#define MY_FREQUENCY 868.1
//when sending to a LoRaWAN gateway (e.g. running util_pkt_logger) but with no native LoRaWAN format, just to set the correct sync word
//#define USE_LORAWAN_SW
//#define WITH_ACK
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// ADD HERE OTHER PLATFORMS THAT DO NOT SUPPORT EEPROM NOR LOW POWER
#if defined ARDUINO_SAM_DUE || defined __SAMD21G18A__
#undef WITH_EEPROM
#endif

#if defined ARDUINO_SAM_DUE
#undef LOW_POWER
#endif
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// CHANGE HERE THE LORA MODE, NODE ADDRESS 
#define LORAMODE  1
#define node_addr 15
//////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// CHANGE HERE THE TIME IN MINUTES BETWEEN 2 READING & TRANSMISSION
unsigned int idlePeriodInMin = 20;
#define IDLE_PERIOD_SECONDS (idlePeriodInMin*60)
//you can also specify a given period in seconds
//#define IDLE_PERIOD_SECONDS 30

///////////////////////////////////////////////////////////////////

#define GPS_FIX_ATTEMPT_TIME_IN_MS 35000

///////////////////////////////////////////////////////////////////
// CHANGE HERE THE GPS SERIAL PORT

#ifndef SWSERIAL
//Arduino Pro Mini, Uno, Nano, Arduino Mini/Nexus, Arduino M0/Zero
#if defined ARDUINO_AVR_PRO || defined ARDUINO_AVR_UNO || defined ARDUINO_AVR_NANO || defined ARDUINO_AVR_MINI || defined __SAMD21G18A__
#define gps_serial Serial
// Arduino Mega, Due, Teensy3.1/3.2,
#elif defined ARDUINO_AVR_MEGA2560 || defined ARDUINO_SAM_DUE || defined __MK20DX256__
#define gps_serial Serial3
#endif
#endif

///////////////////////////////////////////////
// CHANGE HERE THE POWER PIN FOR THE GPS MODULE
// use digital 8 to power the GPS
// if commenting GPS_PIN_POWER then the GPS module is put in inactive mode with Power Save Mode but it still consumes a lot of energy
// for deployment, use a MOSFET transistor and uncomment GPS_PIN_POWER
//
//          MOSFET            ARDUINO      GPS
//          Gate   ----------  pin 8
//          Source ----------  GND
//          Drain  ----------------------- GND  
//                             VCC  -----  VCC
//
#define GPS_PIN_POWER 
#define GPS_PIN_POWER_PIN 8
///////////////////////////////////////////////////////////////////

#ifdef WITH_APPKEY
///////////////////////////////////////////////////////////////////
// CHANGE HERE THE APPKEY, BUT IF GW CHECKS FOR APPKEY, MUST BE
// IN THE APPKEY LIST MAINTAINED BY GW.
uint8_t my_appKey[4]={5, 6, 7, 8};
///////////////////////////////////////////////////////////////////
#endif

/*****************************
 _____           _      
/  __ \         | |     
| /  \/ ___   __| | ___ 
| |    / _ \ / _` |/ _ \
| \__/\ (_) | (_| |  __/
 \____/\___/ \__,_|\___|
*****************************/ 

//////////////////////////////////////////////////////////////////

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
#else // for all other boards based on ATMega168, ATMega328P, ATMega32U4, ATMega2560, ATMega256RFR2, ATSAMD21G18A
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
unsigned int nCycle = IDLE_PERIOD_SECONDS/LOW_POWER_PERIOD;
#endif

unsigned long nextTransmissionTime=0L;
uint8_t message[100];

int loraMode=LORAMODE;

double gps_latitude = 0.0;
double gps_longitude = 0.0;
long currentTime; 
long fixTime = -1.0;
uint8_t starting = 1;
char GPSBuffer[250];
byte GPSIndex=0;
char lat_direction, lgt_direction;

uint16_t beaconCounter=0;

#ifdef WITH_EEPROM
struct sx1272config {

  uint8_t flag1;
  uint8_t flag2;
  uint8_t seq;
  // can add other fields such as LoRa mode,...
};

sx1272config my_sx1272config;
#endif

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

/*
volatile bool detect_new_line = false;

static void handleRxChar( uint8_t c )
{
  if (c == '\n') {
    GPSBuffer[GPSIndex++] = '\0';
    detect_new_line==true;
  }
}
*/
    
void parseNMEA(){
  char inByte;
  bool detect_new_line=false;
  
  while (!detect_new_line)
  {
    if (gps_serial.available()) {
      inByte = gps_serial.read();
    
      //Serial.print(inByte); // Output exactly what we read from the GPS to debug
    
      if ((inByte =='$') || (GPSIndex >= 250)){
        GPSIndex = 0;
      }
      
      if (inByte != '\r' && inByte != '\n'){
        GPSBuffer[GPSIndex++] = inByte;
      }
   
      if (inByte == '\n'){
        GPSBuffer[GPSIndex++] = '\0';
        detect_new_line=true;
      }
    }
    //else
    //  Serial.println("-");
  } 
  Serial.print("---->");
  Serial.println(GPSBuffer);
  //Serial.flush();
}

bool isGGA(){
  return (GPSBuffer[1] == 'G' && (GPSBuffer[2] == 'P' || GPSBuffer[2] == 'N') && GPSBuffer[3] == 'G' && GPSBuffer[4] == 'G' && GPSBuffer[5] == 'A');
}

// GPGGA and GNGGA for the Ublox 8 series. Ex:
// no fix -> $GPGGA,,,,,,0,00,99.99,,,,,,*63
// fix    -> $GPGGA,171958.00,4903.61140,N,00203.95016,E,1,07,1.26,55.1,M,46.0,M,,*68

void decodeGGA(){
  int i, j;
  char latitude[30], longitude[30];
  byte latIndex=0, lgtIndex=0;
  
  for (i=7, j=0; (i<GPSIndex) && (j<9); i++) { // We start at 7 so we ignore the '$GNGGA,'
    
    if (GPSBuffer[i] == ','){
      j++;    // Segment index
    }
    else{
      if (j == 1){   //Latitude
        latitude[latIndex++]= GPSBuffer[i];
      }

      else if (j == 2){   //Latitude Direction: N=North, S=South
        lat_direction = GPSBuffer[i];
      }
      
      else if (j == 3){    //Longitude
        longitude[lgtIndex++]= GPSBuffer[i];
      }

      else if (j == 4){   //Longitude Direction: E=East, W=West
        lgt_direction = GPSBuffer[i];
      }
    }
  }
  //Serial.print("Lat Direction: "); Serial.println(lat_direction);
  //Serial.print("Lgt Direction: "); Serial.println(lgt_direction);
  gps_latitude = degree_location(atof(latitude), lat_direction);
  gps_longitude = degree_location(atof(longitude), lgt_direction);
}

bool isValidLocation(){
  return (gps_latitude != 0.0 && gps_longitude != 0.0 && (lat_direction == 'N' || lat_direction=='S') && (lgt_direction=='E'|| lgt_direction=='W'));
}

void display_location(){
  Serial.println("==============");
  Serial.print("Location: ");
  Serial.print(gps_latitude, 5);
  Serial.print(", ");
  Serial.println(gps_longitude, 5);
  Serial.println("=============="); 
}

//////////////////////////////////////////////////////////////////////////////////
//Convert NMEA latitude & longitude to decimal (Degrees)
//  Notice that for latitude of 0302.78469,
//              03 ==> degrees. counted as is
//              02.78469 ==> minutes. divide by 60 before adding to degrees above
//              Hence, the decimal equivalent is: 03 + (02.78469/60) = 3.046412
//  For longitude of 10601.6986,
//              ==> 106+1.6986/60 = 106.02831 degrees
//  Location is latitude or longitude
//  In Southern hemisphere latitude should be negative
//  In Western Hemisphere, longitude degrees should be negative
//////////////////////////////////////////////////////////////////////////////////

double degree_location(double loc, char loc_direction){
  double degLoc = 0.0;
  double degWhole= loc/100; //gives the whole degree part of latitude
  double degDec = (degWhole - ((int)degWhole)) * 100 / 60; //gives fractional part of location
  degLoc = (int)degWhole + degDec; //gives complete correct decimal form of location degrees
  
  if (loc_direction =='S' || loc_direction =='W') {  
    degLoc = (-1)*degLoc;
  }
  return degLoc;  
}
    
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

  delay(3000);
  // Open serial communications and wait for port to open:
#if defined __SAMD21G18A__ && not defined ARDUINO_SAMD_FEATHER_M0 
  SerialUSB.begin(38400);
#else
  Serial.begin(38400);  
#endif  

#ifdef GPS_PIN_POWER
  //to power the GPS
  pinMode(GPS_PIN_POWER_PIN,OUTPUT);
#endif
  
  // Print a start message
  PRINT_CSTSTR("%s","Simple LoRa GPS sensor\n");

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
  PRINT_CSTSTR("%s","SAMD21G18A ARM Cortex-M0+ detected\n");
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

#ifdef MY_FREQUENCY
  e = sx1272.setChannel(MY_FREQUENCY*1000000.0*RH_LORA_FCONVERT);
  PRINT_CSTSTR("%s","Setting customized frequency: ");
  PRINT_VALUE("%f", MY_FREQUENCY);
  PRINTLN;
#else
  e = sx1272.setChannel(DEFAULT_CHANNEL);  
#endif  
  PRINT_CSTSTR("%s","Setting Channel: state ");
  PRINT_VALUE("%d", e);
  PRINTLN;

  // enable carrier sense
  sx1272._enableCarrierSense=true;
#ifdef LOW_POWER
  // TODO: with low power, when setting the radio module in sleep mode
  // there seem to be some issue with RSSI reading
  sx1272._RSSIonSend=false;
#endif   

#ifdef USE_LORAWAN_SW
  e = sx1272.setSyncWord(0x34);
  PRINT_CSTSTR("%s","Set sync word to 0x34: state ");
  PRINT_VALUE("%d", e);
  PRINTLN;
#endif
  
  // Select amplifier line; PABOOST or RFO
#ifdef PABOOST
  sx1272._needPABOOST=true; 
#endif 

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

#ifdef GPS_PIN_POWER
  digitalWrite(GPS_PIN_POWER_PIN,HIGH);
  PRINT_CSTSTR("%s","Setting HIGH digital GPS power pin (1): ");       
  PRINT_VALUE("%d", GPS_PIN_POWER_PIN);
  PRINTLN;      
  delay(200);
#endif  

  //gps_serial.attachInterrupt( handleRxChar );
  gps_serial.begin(9600);
   
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
  uint8_t app_key_offset=0;
  int e;

#ifndef LOW_POWER
  // 600000+random(15,60)*1000
  if (millis() > nextTransmissionTime) {
#else
      //time for next wake up
      nextTransmissionTime=millis()+(unsigned long)IDLE_PERIOD_SECONDS*1000;
#endif        

      if  (!starting) {
#ifdef GPS_PIN_POWER
          digitalWrite(GPS_PIN_POWER_PIN,HIGH);
          PRINT_CSTSTR("%s","Setting HIGH digital GPS power pin (2): ");       
          PRINT_VALUE("%d", GPS_PIN_POWER_PIN);
          PRINTLN;      
          delay(200);
#endif          
      }
          
      currentTime = millis();
      uint8_t get_fix_success = 0, timeout = 0, nb_validGPGGA=3;

      /*
      while (!get_fix_success) {
        parseNMEA();
        if (isGGA()){
          decodeGGA();
          if (isValidLocation()){
            display_location();
            get_fix_success = true;
          }
        }
      }
      */
      
      while (!timeout && !get_fix_success)  {
        
        if (millis() - currentTime > GPS_FIX_ATTEMPT_TIME_IN_MS) {
          //one more time if it is a new start
          if (starting) {
            currentTime = millis();
            starting=0;
          }
          else  
            timeout = 1;
        }
        else{
          parseNMEA();
          if (isGGA()) {
            decodeGGA();
            if (isValidLocation()) {
              fixTime = millis() - currentTime;  
              nb_validGPGGA--;          
              if (nb_validGPGGA==0)
                get_fix_success = 1;
            }
          }
        }
      } // End While

#ifdef GPS_PIN_POWER
      digitalWrite(GPS_PIN_POWER_PIN,LOW);
#endif  
      starting = 0;   
    
#ifdef WITH_APPKEY
      app_key_offset = sizeof(my_appKey);
      // set the app key in the payload
      memcpy(message,my_appKey,app_key_offset);
#endif     
      uint8_t r_size;
      
      if (timeout) {
          PRINT_CSTSTR("%s", "No fix, timeout expired!");
          PRINTLN;
          r_size=sprintf((char*)message+app_key_offset, "\\!BC/%d/LAT/0/LGT/0/FXT/-1", beaconCounter);
      }
      else {       
        PRINT_CSTSTR("%s","Fix is ");
        PRINT_VALUE("%ld", fixTime);
        PRINTLN;
        
        String latString = String(gps_latitude, 5);
        //char latString[10];
        //ftoa(latString,latitude,5);
        
        PRINT_CSTSTR("%s","Latitude is ");
        PRINT_STR("%s", latString);
        PRINTLN;

        String lgtString = String(gps_longitude, 5);
        //char lgtString[10];        
        //ftoa(lgtString,longitude,5);
          
        PRINT_CSTSTR("%s","Longitude is ");
        PRINT_STR("%s", lgtString);
        PRINTLN;
              
        // then use app_key_offset to skip the app key
        r_size=sprintf((char*)message+app_key_offset, "\\!BC/%d/LAT/%s/LGT/%s/FXT/%ld", beaconCounter, latString.c_str(), lgtString.c_str(), fixTime);
        //r_size=sprintf((char*)message+app_key_offset, "\\!BC/%d/LAT/%s/LGT/%s/FXT/%ld", beaconCounter, latString, lgtString, fixTime);
      }

      beaconCounter++;
            
      PRINT_CSTSTR("%s","Sending ");
      PRINT_STR("%s",(char*)(message+app_key_offset));
      PRINTLN;
      
      PRINT_CSTSTR("%s","Real payload size is ");
      PRINT_VALUE("%d", r_size);
      PRINTLN;
      
      int pl=r_size+app_key_offset;
      
      sx1272.CarrierSense();
      
      startSend=millis();

#ifdef WITH_APPKEY
      // indicate that we have an appkey
      sx1272.setPacketType(PKT_TYPE_DATA | PKT_FLAG_DATA_WAPPKEY);
#else
      // just a simple data packet
      sx1272.setPacketType(PKT_TYPE_DATA);
#endif
      
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

      PRINT_CSTSTR("%s","Remaining ToA is ");
      PRINT_VALUE("%d", sx1272.getRemainingToA());
      PRINTLN;
    
#ifdef LOW_POWER
      PRINT_CSTSTR("%s","Switch to power saving mode\n");

      e = sx1272.setSleepMode();

      if (!e)
        PRINT_CSTSTR("%s","Successfully switch LoRa module in sleep mode\n");
      else  
        PRINT_CSTSTR("%s","Could not switch LoRa module in sleep mode\n");
        
      FLUSHOUTPUT
      delay(10);

      //how much do we still have to wait, in millisec?
      unsigned long now_millis=millis();
      unsigned long waiting_t = nextTransmissionTime-now_millis;      

#ifdef __SAMD21G18A__
      // For Arduino M0 or Zero we use the built-in RTC
      rtc.setTime(17, 0, 0);
      rtc.setDate(1, 1, 2000);
      rtc.setAlarmTime(17, (uint8_t)IDLE_PERIOD_SECONDS/60, (uint8_t)(IDLE_PERIOD_SECONDS-((uint8_t)IDLE_PERIOD_SECONDS/60)*60);
      // for testing with 20s
      //rtc.setAlarmTime(17, 0, 20);
      rtc.enableAlarm(rtc.MATCH_HHMMSS);
      //rtc.attachInterrupt(alarmMatch);
      rtc.standbyMode();

      LowPower.standby();
      
      PRINT_CSTSTR("%s","SAMD21G18A wakes up from standby\n");      
      FLUSHOUTPUT
#else

#if defined __MK20DX256__ || defined __MKL26Z64__ || defined __MK64FX512__ || defined __MK66FX1M0__
      // warning, setTimer accepts value from 1ms to 65535ms max
      // milliseconds      
      // by default, LOW_POWER_PERIOD is 60s for those microcontrollers      
      timer.setTimer(LOW_POWER_PERIOD*1000);
#endif   

      //nCycle = IDLE_PERIOD_SECONDS/LOW_POWER_PERIOD;
      
      while (waiting_t>0) {  
        
#if defined ARDUINO_AVR_MEGA2560 || defined ARDUINO_AVR_PRO || defined ARDUINO_AVR_NANO || defined ARDUINO_AVR_UNO || defined ARDUINO_AVR_MINI || defined __AVR_ATmega32U4__    
          // ATmega2560, ATmega328P, ATmega168, ATmega32U4
          // each wake-up introduces an overhead of about 158ms
          if (waiting_t > 8158) {
            LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF); 
            waiting_t = waiting_t - 8158;
#ifdef SHOW_LOW_POWER_CYCLE                  
                  PRINT_CSTSTR("%s","8");
#endif              
          }
          else if (waiting_t > 4158) {
            LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF); 
            waiting_t = waiting_t - 4158;
#ifdef SHOW_LOW_POWER_CYCLE                  
                  PRINT_CSTSTR("%s","4");
#endif 
          }
          else if (waiting_t > 2158) {
            LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF); 
            waiting_t = waiting_t - 2158;
#ifdef SHOW_LOW_POWER_CYCLE                  
                  PRINT_CSTSTR("%s","2");
#endif 
          }
          else if (waiting_t > 1158) {
            LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF); 
            waiting_t = waiting_t - 1158;
#ifdef SHOW_LOW_POWER_CYCLE                  
                  PRINT_CSTSTR("%s","1");
#endif 
          }      
          else {
            delay(waiting_t); 
#ifdef SHOW_LOW_POWER_CYCLE                   
                  PRINT_CSTSTR("%s","D[");
                  PRINT_VALUE("%d", waiting_t);
                  PRINT_CSTSTR("%s","]");
#endif
            waiting_t = 0;
          }

#ifdef SHOW_LOW_POWER_CYCLE
          FLUSHOUTPUT
          delay(1);
#endif
          
#elif defined __MK20DX256__ || defined __MKL26Z64__ || defined __MK64FX512__ || defined __MK66FX1M0__
          // Teensy31/32 & TeensyLC
          if (waiting_t < LOW_POWER_PERIOD*1000) {
            timer.setTimer(waiting_t);
            waiting_t = 0;
          }
          else
            waiting_t = waiting_t - LOW_POWER_PERIOD*1000;
                        
#ifdef LOW_POWER_HIBERNATE
          Snooze.hibernate(sleep_config);
#else            
          Snooze.deepSleep(sleep_config);
#endif

#else
          // use the delay function
          delay(waiting_t);
          waiting_t = 0;
#endif                                            
       }
#endif      
      
#else
      PRINT_VALUE("%ld", nextTransmissionTime);
      PRINTLN;
      PRINT_CSTSTR("%s","Will send next value at\n");
      // can use a random part also to avoid collision
      nextTransmissionTime=millis()+(unsigned long)IDLE_PERIOD_SECONDS*1000; //+(unsigned long)random(15,60)*1000;
      PRINT_VALUE("%ld", nextTransmissionTime);
      PRINTLN;
  }
#endif
}
