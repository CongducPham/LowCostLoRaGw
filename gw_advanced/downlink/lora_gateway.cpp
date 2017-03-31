/* 
 *  LoRa low-level gateway to receive and send command
 *
 *  Copyright (C) 2015-2017 Congduc Pham
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
 *  Version:                1.72
 *  Design:                 C. Pham
 *  Implementation:         C. Pham
 *
 *  waits for command or data on serial port or from the LoRa module
 *    - command starts with /@ and ends with #
 *  
 *  LoRa parameters
 *    - /@M1#: set LoRa mode 1
 *    - /@C12#: use channel 12 (868MHz)
 *    - /@SF8#: set SF to 8
 *    - /@PL/H/M/x/X#: set power to Low, High or Max; extreme (PA_BOOST at +14dBm), eXtreme (PA_BOOST at +20dBm)
 *    - /@W34#: set sync word to 0x34
 *    - /@ON# or /@OFF#: power on/off the LoRa module
 * 
 *  CAD, DIFS/SIFS mechanism, RSSI checking, extended IFS
 *    - /@CAD# performs an SIFS CAD, i.e. 3 or 6 CAD depending on the LoRa mode
 *    - /@CADON3# uses 3 CAD when sending data (normally SIFS is 3 or 6 CAD, DIFS=3SIFS)
 *    - /@CADOFF# disables CAD (IFS) when sending data
 *    - /@RSSI# toggles checking of RSSI before transmission and after CAD
 *    - /@EIFS# toggles for extended IFS wait
 *
 *    - just connect the Arduino board with LoRa module
 *    - use any serial tool to view data that is received
 *    - with a python script to read serial port, all received data could be forwarded to another application through
 *      standart output
 *    - remote configuration needs to be allowed by unlocking the gateway with command '/@U' with an unlock pin
 *      - '/@U1234#'
 *    - allowed commands for a gateway are
 *      - M, C, P, A, ON, OFF
 *      - ACKON, ACKOFF (if using unmodified SX1272 lib)
 *
 *  if compiled with LORA_LAS
 *    - add LAS support
 *      - sending message will use LAS service
 *      - /@LASS# prints LAS statistics
 *      - /@LASR# resets LAS service
 *      - /@LASON# enables LAS service
 *      - /@LASOFF# disables LAS service
 *      - /@LASI# initiate the INITrestart/INIT procedure that ask end-devices to send the REG msg
 *      
 *   IMPORTANT NOTICE   
 *    - the gateway use data prefix to indicate data received from radio. The prefix is 2 bytes: 0xFF0xFE 
 *    - the post-processing stage looks for this sequence to dissect the data, according to the format adopted by the sender 
 *    - if you use our LoRa_Temp example, then the packet format as expected by the post-processing script is as follows:
 *      - without application key and without encryption, payload starts immediately: [payload]
 *      - without application key and with encryption: original (clear) format is [size(1B)][payload]. 
 *        size is the real size of the clear payload
 *      - if application key is used, without encryption: [AppKey(4B)][payload]
 *      - if application key is used, with encryption: original (clear) format is [AppKey(4B)][size(1B)][payload]. 
 *        size is the real size of the clear payload
 *    - for more details on the underlying packet format used by our modified SX1272 library
 *      - refer to the SX1272.h
 *      - see http://cpham.perso.univ-pau.fr/LORA/RPIgateway.html
 *      
*/

/*  Change logs
 *  Mar, 29th, 2017. v1.72
 *        Add periodic status (every 10 minutes) from the low-level gateway
 *        Add reset of radio module when receive error from radio is detected
 *        Change receive windows from 1000ms, or 2500ms in mode 1, to 10000ms for all modes, reducing overheads of receive loop 
 *  Mar, 2nd, 2017. v1.71
 *        Add preamble length verification and correction to the default value of 8 if it is not the case    
 *  Dec, 14st, 2016. v1.7   
 *        Reduce dynamic memory usage for having a simple relay gateway running on an Arduino Pro Mini
 *          - about 600B of free memory should be available
 *        Add support for a simple relay gateway with an Arduino node, uncomment GW_RELAY
 *          - the simple gateway has minimum output as it is not intended to be connected to a computer
 *          - continously waits for packets then will re-sent the packet (destination is 1) by keeping the original packet header
 *  Oct, 21st, 2016. v1.6S
 *        Split the lora_gateway sketch into 2 parts:   
 *          - lora_gateway: for gateway, similar to previous IS_RCV_GATEWAY
 *          - lora_interactivedevice, similar to previous IS_SEND_GATEWAY
 *        Add support for 4 channels in the 433MHz band. Default is CH_00_433 = 0x6C5333 for 433.3MHz. See SX1272.h.         
 *  Oct, 9th, 2016. v1.6
 *		  Change the downlink strategy
 *			    - the lora_gateway.cpp program will check for a downlink.txt file after each LoRa packet reception
 *			    - this behavior can be disable with --ndl option
 *			    - after a LoRa packet reception, lora_gateway.cpp will wait for interDownlinkCheckTime before checking for downlink.txt file
 *			    - downlink.txt will be normally generated by post_processing_gw.py
 *			    - post_processing_gw.py periodically check for downlink-post.txt and will build a queue of downlink requests
 *			    - when a lora packet from device i is processed by post_processing_gw.py, it will check if there is a pending request for device i
 *			    - if it is the case, then post_processing_gw.py generates the corresponding downlink.txt which will contain in most cases only 1 entry
 *  August, 7th, 2016. v1.5
 *        Add preliminary and simple support for downlink request (only for Linux-based gateway)
 *          - will periodically check for downlink.txt file
 *          - the file contains a series of line in JSON format: { "status" : "send_request", "dst" : 3, "data" : "/@Px#" }
 *          - mandatory key are "status", "dst" and "data". "status" must be "send_request"
 *          - each line must be terminated by \n. Do not leave an empty line at the end, just \n
 *          - you can add other fields for logging/information purposes
 *          - every interDownlinkCheckTime the existence of downlink.txt will be checked
 *          - all requests will be stored in memory and downlink.txt will be renamed, e.g. downlink-backup-2016-08-01T20:25:44.txt
 *          - downlink-queued.txt will be appended with new send_request, marked as "queued"
 *          - when there are pending send request, then every interDownlinkSendTime a transmission will occur
 *          - downlink-send.txt will be appended with new transmissions, marked as "sent" or "sent_fail"
 *          - there is no reliability implemented
 *          - it is expected that new sending request will be indicated in a new downlink.txt file
 *          - this file can be created in various ways: interactive mode, MQTT, ftp, http,...
 *        Change the CarrierSense behavior with an "only once" behavior that that a busy channel will not block the gateway
 *          - CarrierSense now accept an optional parameter that is false by default. true indicates "only once" behavior
 *          - CarrierSense now returns an integer. 1 means that CarrierSense found a busy channel under "only once" behavior      
 *  June, 14th, 2016. v1.4
 *        Fix bug on serial port for the RPI3 and for the Bluetooth interface on RPI3 which uses the serial port
 *  Mar, 25th, 2016. v1.3
 *        Add command to set the spreading factor between 6 and 12:
 *          - /@SF8#: set SF to 8
 *  Fev, 25th, 2016. v1.2
 *        Add 900MHz support when specifying a channel in command line
 *        Use by default channel 10 (865.2MHz) in 868MHz band and channel 5 (913.88) in 900MHz band
 *  Jan, 22th, 2016. v1.1
 *        Add advanced configuration options when running on Linux (Raspberry typically)
 *          - options are: --mode 4 --bw 500 --cr 5 --sf 12 --freq 868.1 --ch 10 --sw 34 --raw 
 *        Add raw output option in the Linux version. The gateway will forward all the payload without any interpretation  
 *          - this feature is implemented in the SX1272 library, see the corresponding CHANGES.log file
 *          - this is useful when the packet interpretation is left to the post-processing stage (e.g. for LoRaWAN)
 *  Dec, 30th, 2015. v1.0
 *        SX1272 library has been modified to allow for sync word setting, a new mode 11 is introduced to test with LoRaWAN
 *        BW=125kHz, CR=4/5 and SF=7. When using mode 11, sync word is set to 0x34. Normally, use the newly defined CH_18_868=868.1MHz
 *        Add possibility to set the sync word
 *          - /@W34# set the sync word to 0x34
 *  Nov, 13th, 2015. v0.9
 *        SX1272 library has been modified to support dynamic ACK request using the retry field of the packet header
 *        Gateway now always use receivePacketTimeout() and sender either use sendPacketTimeout() or sendPacketTimeoutACK()
 *  Nov, 10th, 2015. v0.8a
 *        Add an unlock pin to allow the gateway to accept remote commands
 *        A limited number of attempts is allowed
 *          - /@U1234#: try to unlock with pin 1234. To lock, issue the same command again.
 *  Oct, 8th, 2015. v0.8
 *        Can change packet size for periodic packet transmission
 *          - /@Z200# sets the packet payload size to 200. The real size is 205B with the Libelium header.
 *            Maximum size that can be indicated is then 250.
 *        Add possibility to send periodically at random time interval
 *          - /@TR5000#: send a message at random time interval between [2000, 5000]ms.
 *        Check RSSI value before transmitting a packet. This is done after successful CAD
 *          - CAD must be ON
 *          - /@RSSI# toggles checking of RSSI, must be above -90dBm to transmit, otherwise, repeat 10 times
 *  Sep, 22nd, 2015. v0.7
 *        Add ACK support when sending packets
 *          - /@ACKON# enables ACK
 *          - /@ACKOFF# disables ACK
 *        Can use extended IFS wait to: 
 *          - CAD must be ON
 *          - wait a random number of CAD after a successful IFS
 *          - perform an IFS one more time before packet tranmission 
 *          - /@EIFS# toggles for extended IFS wait
 *  Jul, 1st, 2015. v0.6
 *        Add support of the LoRa Activity Sharing (LAS) mechanism (device side), uncomment #define LORA_LAS
 *          - sending message will use LAS service
 *          - /@LASS# prints LAS statistics
 *          - /@LASR# resets LAS service
 *          - /@LASON# enables LAS service
 *          - /@LASOFF# disables LAS service
 *          - /@REG# sends a REG message if IS_SEND_GATEWAY
 *          - /@INIT# sends an INIT(0,delay) message for restarting if IS_SEND_GATEWAY
 *  June, 29th, 2015. v0.5
 *        Add a CAD_TEST behavior to see continuously channel activity, uncomment #define CAD_TEST
 *        Add LoRa ToA computation when sending data
 *        Add CAD test when sending data
 *          - /@CADON3# uses 3 CAD when sending data (normally SIFS is 3 or 6 CAD, DIFS=3SIFS)
 *          - /@CADOFF# disables CAD when sending data
 *        Add CAD feature for testing
 *          - /@CAD# performs an SIFS CAD, i.e. 3 or 6 CAD depending on the LoRa mode
 *        Add ON and OFF command to power on/off the LoRa module
 *          - /@ON# or /@OFF#
 *        Add the S command to send a string of arbitrary size
 *          - /@S50# sends a 50B user payload packet filled with '#'. The real size is 55B with the Libelium header 
 *        The gateway can accept command from serial or from the LoRa module
 *  May, 11th, 2015. v0.4
 *        Add periodic sending of packet for range test
 *          - /@T5000#: send a message every 5s. Use /@T0# to disable periodic sending
 *  Apr, 17th, 2015. v0.3
 *        Add possibility to configure the LoRa operation mode
 *          - /@M1#: set LoRa mode 1
 *          - /@C12#: use channel 12 (868MHz)
 *          - /@PL/H/M#: set power to Low, High or Max
 *          - /@A9#: set node addr to 9    
 *  Apr, 16th, 2015. v0.2
 *        Integration of receive gateway and send gateway: 
 *          - #define IS_SEND_GATEWAY will produce a sending gateway to send remote commands
 *  Apr, 14th, 2015. v0.1
 *        First version of receive gateway
 */

// Include the SX1272 
#include "SX1272.h"

#ifdef ARDUINO
// IMPORTANT when using an Arduino only. For a Raspberry-based gateway the distribution uses a radio.makefile file
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// please uncomment only 1 choice
//
// uncomment if your radio is an HopeRF RFM92W, HopeRF RFM95W, Modtronix inAir9B, NiceRF1276
// or you known from the circuit diagram that output use the PABOOST line instead of the RFO line
//#define PABOOST
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
#endif

// IMPORTANT
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// please uncomment only 1 choice
#define BAND868
//#define BAND900
//#define BAND433
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// For a Raspberry-based gateway the distribution uses a radio.makefile file that can define MAX_DBM
//
#ifndef MAX_DBM
#define MAX_DBM 14
#endif

#ifndef ARDUINO
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h> 
#include  <signal.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#endif

#ifdef ARDUINO
// and SPI library on Arduino platforms
#include <SPI.h>
  
  #define PRINTLN                   Serial.println("")              
  #define PRINT_CSTSTR(fmt,param)   Serial.print(F(param))
  #define PRINT_STR(fmt,param)      Serial.print(param)
  #define PRINT_VALUE(fmt,param)    Serial.print(param)
  #define PRINT_HEX(fmt,param)      Serial.print(param,HEX)
  #define FLUSHOUTPUT               Serial.flush();
#else
  #define PRINTLN                   printf("\n")
  #define PRINT_CSTSTR(fmt,param)   printf(fmt,param)
  #define PRINT_STR(fmt,param)      PRINT_CSTSTR(fmt,param)
  #define PRINT_VALUE(fmt,param)    PRINT_CSTSTR(fmt,param)
  #define PRINT_HEX(fmt,param)      PRINT_VALUE(fmt,param)
  #define FLUSHOUTPUT               fflush(stdout);
#endif

#ifdef DEBUG
  #define DEBUGLN                 PRINTLN
  #define DEBUG_CSTSTR(fmt,param) PRINT_CSTSTR(fmt,param)  
  #define DEBUG_STR(fmt,param)    PRINT_CSTSTR(fmt,param)  
  #define DEBUG_VALUE(fmt,param)  PRINT_VALUE(fmt,param)  
#else
  #define DEBUGLN
  #define DEBUG_CSTSTR(fmt,param)
  #define DEBUG_STR(fmt,param)    
  #define DEBUG_VALUE(fmt,param)  
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// FOR DOWNLINK FEATURES
//
#if not defined ARDUINO && defined DOWNLINK
#define MAX_DOWNLINK_ENTRY 100

#include "rapidjson/reader.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <iostream>

using namespace rapidjson;
using namespace std;

char* json_entry[MAX_DOWNLINK_ENTRY];

size_t json_entry_size=100;
FILE *fp;
int dl_line_index;
ssize_t dl_line_size;
int dl_total_line;
bool hasDownlinkEntry=false;
bool enableDownlinkCheck=false;
bool optNDL=false;

unsigned long lastDownlinkCheckTime=0;
// we set to 10s after the gw receives a lora packet
// to give some time for the post-processing stage to generate a downlink.txt file if any
unsigned long interDownlinkCheckTime=10000L;
//unsigned long interDownlinkCheckTime=120000L;
unsigned long lastDownlinkSendTime=0;
// 20s between 2 downlink transmissions when there are queued requests
unsigned long interDownlinkSendTime=20000L;
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//#define SHOW_FREEMEMORY
//#define GW_RELAY
//#define RECEIVE_ALL 
//#define CAD_TEST
//#define LORA_LAS
//#define WINPUT

#ifdef BAND868
#define MAX_NB_CHANNEL 15
#define STARTING_CHANNEL 4
#define ENDING_CHANNEL 18
#ifdef SENEGAL_REGULATION
uint8_t loraChannelIndex=0;
#else
uint8_t loraChannelIndex=6;
#endif
uint32_t loraChannelArray[MAX_NB_CHANNEL]={CH_04_868,CH_05_868,CH_06_868,CH_07_868,CH_08_868,CH_09_868,
                                            CH_10_868,CH_11_868,CH_12_868,CH_13_868,CH_14_868,CH_15_868,CH_16_868,CH_17_868,CH_18_868};

#elif defined BAND900 
#define MAX_NB_CHANNEL 13
#define STARTING_CHANNEL 0
#define ENDING_CHANNEL 12
uint8_t loraChannelIndex=5;
uint32_t loraChannelArray[MAX_NB_CHANNEL]={CH_00_900,CH_01_900,CH_02_900,CH_03_900,CH_04_900,CH_05_900,CH_06_900,CH_07_900,CH_08_900,
                                            CH_09_900,CH_10_900,CH_11_900,CH_12_900};
#elif defined BAND433
#define MAX_NB_CHANNEL 4
#define STARTING_CHANNEL 0
#define ENDING_CHANNEL 3
uint8_t loraChannelIndex=0;
uint32_t loraChannelArray[MAX_NB_CHANNEL]={CH_00_433,CH_01_433,CH_02_433,CH_03_433};                                              
#endif

// use the dynamic ACK feature of our modified SX1272 lib
#define GW_AUTO_ACK

///////////////////////////////////////////////////////////////////
// DEFAULT LORA MODE
#define LORAMODE 1
// the special mode to test BW=125MHz, CR=4/5, SF=12
// on the 868.1MHz channel
//#define LORAMODE 11
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// GATEWAY HAS ADDRESS 1
#define LORA_ADDR 1
///////////////////////////////////////////////////////////////////

// to unlock remote configuration feature
#define UNLOCK_PIN 1234
// will use 0xFF0xFE to prefix data received from LoRa, so that post-processing stage can differenciate
// data received from radio
#define WITH_DATA_PREFIX

#ifdef WITH_DATA_PREFIX
#define DATA_PREFIX_0 0xFF
#define DATA_PREFIX_1 0xFE
#endif

#ifdef LORA_LAS
#include "LoRaActivitySharing.h"
// acting as the LR-BS
LASBase loraLAS = LASBase();
#endif

///////////////////////////////////////////////////////////////////
// CONFIGURATION VARIABLES
//
#ifndef ARDUINO
char keyPressBuff[30];
uint8_t keyIndex=0;
int ch;
#endif

// be careful, max command length is 60 characters
#define MAX_CMD_LENGTH 60

char cmd[MAX_CMD_LENGTH]="****************";
int msg_sn=0;

// number of retries to unlock remote configuration feature
uint8_t unlocked_try=3;
boolean unlocked=false;
boolean receivedFromSerial=false;
boolean receivedFromLoRa=false;
boolean withAck=false;

bool radioON=false;
bool RSSIonSend=true;

uint8_t loraMode=LORAMODE;

uint32_t loraChannel=loraChannelArray[loraChannelIndex];
#if defined PABOOST || defined RADIO_RFM92_95 || defined RADIO_INAIR9B || defined RADIO_20DBM
// HopeRF 92W/95W and inAir9B need the PA_BOOST
// so 'x' set the PA_BOOST but then limit the power to +14dBm 
char loraPower='x';
#else
// other radio board such as Libelium LoRa or inAir9 do not need the PA_BOOST
// so 'M' set the output power to 15 to get +14dBm
char loraPower='M';
#endif

uint8_t loraAddr=LORA_ADDR;

int status_counter=0;
unsigned long startDoCad, endDoCad;
bool extendedIFS=true;
uint8_t SIFS_cad_number;
// set to 0 to disable carrier sense based on CAD
uint8_t send_cad_number=3;
uint8_t SIFS_value[11]={0, 183, 94, 44, 47, 23, 24, 12, 12, 7, 4};
uint8_t CAD_value[11]={0, 62, 31, 16, 16, 8, 9, 5, 3, 1, 1};

bool optAESgw=false;
uint16_t optBW=0; 
uint8_t optSF=0;
uint8_t optCR=0;
uint8_t optCH=0;
bool  optRAW=false;
double optFQ=-1.0;
uint8_t optSW=0x12;
///////////////////////////////////////////////////////////////////

#if defined ARDUINO && defined SHOW_FREEMEMORY && not defined __MK20DX256__ && not defined __MKL26Z64__ && not defined  __SAMD21G18A__ && not defined _VARIANT_ARDUINO_DUE_X_
int freeMemory () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
#endif

long getCmdValue(int &i, char* strBuff=NULL) {
        
        char seqStr[7]="******";

        int j=0;
        // character '#' will indicate end of cmd value
        while ((char)cmd[i]!='#' && (i < strlen(cmd)) && j<strlen(seqStr)) {
                seqStr[j]=(char)cmd[i];
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

void startConfig() {

  int e;

  // has customized LoRa settings    
  if (optBW!=0 || optCR!=0 || optSF!=0) {

    e = sx1272.setCR(optCR-4);
    PRINT_CSTSTR("%s","^$LoRa CR ");
    PRINT_VALUE("%d", optCR);    
    PRINT_CSTSTR("%s",": state ");
    PRINT_VALUE("%d", e);
    PRINTLN;

    e = sx1272.setSF(optSF);
    PRINT_CSTSTR("%s","^$LoRa SF ");
    PRINT_VALUE("%d", optSF);    
    PRINT_CSTSTR("%s",": state ");
    PRINT_VALUE("%d", e);
    PRINTLN;
    
    e = sx1272.setBW( (optBW==125)?BW_125:((optBW==250)?BW_250:BW_500) );
    PRINT_CSTSTR("%s","^$LoRa BW ");
    PRINT_VALUE("%d", optBW);    
    PRINT_CSTSTR("%s",": state ");
    PRINT_VALUE("%d", e);
    PRINTLN;

    // indicate that we have a custom setting
    loraMode=0;
  
    if (optSF<10)
      SIFS_cad_number=6;
    else 
      SIFS_cad_number=3;
      
  }
  else {
    
    // Set transmission mode and print the result
    PRINT_CSTSTR("%s","^$LoRa mode ");
    PRINT_VALUE("%d", loraMode);
    PRINTLN;
        
    e = sx1272.setMode(loraMode);
    PRINT_CSTSTR("%s","^$Setting mode: state ");
    PRINT_VALUE("%d", e);
    PRINTLN;
  
  #ifdef LORA_LAS
    loraLAS.setSIFS(loraMode);
  #endif
  
    if (loraMode>7)
      SIFS_cad_number=6;
    else 
      SIFS_cad_number=3;

  }
  
  // Select frequency channel
  if (loraMode==11) {
    // if we start with mode 11, then switch to 868.1MHz for LoRaWAN test
    // Note: if you change to mode 11 later using command /@M11# for instance, you have to use /@C18# to change to the correct channel
    e = sx1272.setChannel(CH_18_868);
    PRINT_CSTSTR("%s","^$Channel CH_18_868: state ");    
  }
  else {
    e = sx1272.setChannel(loraChannel);

    if (optFQ>0.0) {
      PRINT_CSTSTR("%s","^$Frequency ");
      PRINT_VALUE("%f", optFQ);
      PRINT_CSTSTR("%s",": state ");      
    }
    else {
#ifdef BAND868
    if (loraChannelIndex>5) {      
      PRINT_CSTSTR("%s","^$Channel CH_1");
      PRINT_VALUE("%d", loraChannelIndex-6);      
    }
    else {
      PRINT_CSTSTR("%s","^$Channel CH_0");
      PRINT_VALUE("%d", loraChannelIndex+STARTING_CHANNEL);        
    }
    PRINT_CSTSTR("%s","_868: state ");
#elif defined BAND900
    PRINT_CSTSTR("%s","^$Channel CH_");
    PRINT_VALUE("%d", loraChannelIndex);
    PRINT_CSTSTR("%s","_900: state ");
#elif defined BAND433
    //e = sx1272.setChannel(0x6C4000);
    PRINT_CSTSTR("%s","^$Channel CH_");
    PRINT_VALUE("%d", loraChannelIndex);  
    PRINT_CSTSTR("%s","_433: state ");  
#endif
    }
  }  
  PRINT_VALUE("%d", e);
  PRINTLN; 

  // Select amplifier line; PABOOST or RFO
#ifdef PABOOST
  sx1272._needPABOOST=true;
  // previous way for setting output power
  // loraPower='x';
  PRINT_CSTSTR("%s","^$Use PA_BOOST amplifier line");
  PRINTLN;   
#else
  // previous way for setting output power
  // loraPower='M';  
#endif
  
  // Select output power in dBm
  e = sx1272.setPowerDBM((uint8_t)MAX_DBM);
  
  PRINT_CSTSTR("%s","^$Set LoRa power dBm to ");
  PRINT_VALUE("%d",(uint8_t)MAX_DBM);  
  PRINTLN;
                
  PRINT_CSTSTR("%s","^$Power: state ");
  PRINT_VALUE("%d", e);
  PRINTLN;
 
  // get preamble length
  e = sx1272.getPreambleLength();
  PRINT_CSTSTR("%s","^$Get Preamble Length: state ");
  PRINT_VALUE("%d", e);
  PRINTLN; 
  PRINT_CSTSTR("%s","^$Preamble Length: ");
  PRINT_VALUE("%d", sx1272._preamblelength);
  PRINTLN;

  if (sx1272._preamblelength != 8) {
      PRINT_CSTSTR("%s","^$Bad Preamble Length: set back to 8");
      sx1272.setPreambleLength(8);
      e = sx1272.getPreambleLength();
      PRINT_CSTSTR("%s","^$Get Preamble Length: state ");
      PRINT_VALUE("%d", e);
      PRINTLN;
      PRINT_CSTSTR("%s","^$Preamble Length: ");
      PRINT_VALUE("%d", sx1272._preamblelength);
      PRINTLN;
  }  
  
  // Set the node address and print the result
  //e = sx1272.setNodeAddress(loraAddr);
  sx1272._nodeAddress=loraAddr;
  e=0;
  PRINT_CSTSTR("%s","^$LoRa addr ");
  PRINT_VALUE("%d", loraAddr);
  PRINT_CSTSTR("%s",": state ");
  PRINT_VALUE("%d", e);
  PRINTLN;

  if (optAESgw)
      PRINT_CSTSTR("%s","^$Handle AES encrypted data\n");

  if (optRAW) {
      PRINT_CSTSTR("%s","^$Raw format, not assuming any header in reception\n");  
      // when operating n raw format, the SX1272 library do not decode the packet header but will pass all the payload to stdout
      // note that in this case, the gateway may process packet that are not addressed explicitly to it as the dst field is not checked at all
      // this would be similar to a promiscuous sniffer, but most of real LoRa gateway works this way 
      sx1272._rawFormat=true;
  }
  
  // Print a success message
  PRINT_CSTSTR("%s","^$SX1272/76 configured ");
  PRINT_CSTSTR("%s","as LR-BS. Waiting RF input for transparent RF-serial bridge\n");
#if defined ARDUINO && defined GW_RELAY
  PRINT_CSTSTR("%s","^$Act as a simple relay gateway\n");
#endif
}

void setup()
{
  int e;
#ifdef ARDUINO
  delay(3000);
  randomSeed(analogRead(14));

  // Open serial communications and wait for port to open:
#ifdef __SAMD21G18A__  
  SerialUSB.begin(38400);
#else
  Serial.begin(38400);  
#endif

#if defined ARDUINO && defined SHOW_FREEMEMORY && not defined __MK20DX256__ && not defined __MKL26Z64__ && not defined  __SAMD21G18A__ && not defined _VARIANT_ARDUINO_DUE_X_
    // Print a start message
  Serial.print(freeMemory());
  Serial.println(F(" bytes of free memory.")); 
#endif  

#else
  srand (time(NULL));
#endif

  // Power ON the module
  e = sx1272.ON();
  
  PRINT_CSTSTR("%s","^$**********Power ON: state ");
  PRINT_VALUE("%d", e);
  PRINTLN;

  e = sx1272.getSyncWord();

  if (!e) {
    PRINT_CSTSTR("%s","^$Default sync word: 0x"); 
    PRINT_HEX("%X", sx1272._syncWord);
    PRINTLN;
  }    

  if (optSW!=0x12) {
    e = sx1272.setSyncWord(optSW);

    PRINT_CSTSTR("%s","^$Set sync word to 0x");
    PRINT_HEX("%X", optSW);
    PRINTLN;
    PRINT_CSTSTR("%s","^$LoRa sync word: state ");
    PRINT_VALUE("%d",e);  
    PRINTLN;
  }
  
  if (!e) {
    radioON=true;
    startConfig();
  }
  
  FLUSHOUTPUT;
  delay(1000);

#ifdef LORA_LAS
  loraLAS.ON(LAS_ON_WRESET);
#endif

#ifdef CAD_TEST
  PRINT_CSTSTR("%s","Do CAD test\n");
#endif 

#if not defined ARDUINO && defined DOWNLINK

  lastDownlinkCheckTime=millis();

#endif
}


// we could use the CarrierSense function added in the SX1272 library, but it is more convenient to duplicate it here
// so that we could easily modify it for testing
// 
// in v1.5 the "only once" behavior is implemented for the gateway when it transmit downlink packets
// to avoid blocking the gateway on a busy channel. Therefore from v1.5 the implementation differs from the
// carrier sense function added in the SX1272 library
//
int CarrierSense(bool onlyOnce=false) {

  int e;
  bool carrierSenseRetry=false;
  
  if (send_cad_number) {
    do { 
      do {
        
        // check for free channel (SIFS/DIFS)        
        startDoCad=millis();
        e = sx1272.doCAD(send_cad_number);
        endDoCad=millis();
        
        PRINT_CSTSTR("%s","--> CAD duration ");
        PRINT_VALUE("%ld",endDoCad-startDoCad);
        PRINTLN;
        
        if (!e) {
          PRINT_CSTSTR("%s","OK1\n");
          
          if (extendedIFS)  {          
            // wait for random number of CAD
#ifdef ARDUINO                
            uint8_t w = random(1,8);
#else
            uint8_t w = rand() % 8 + 1;
#endif
  
            PRINT_CSTSTR("%s","--> waiting for ");
            PRINT_VALUE("%d",w);
            PRINT_CSTSTR("%s"," CAD = ");
            PRINT_VALUE("%d",CAD_value[loraMode]*w);
            PRINTLN;
            
            delay(CAD_value[loraMode]*w);
            
            // check for free channel (SIFS/DIFS) once again
            startDoCad=millis();
            e = sx1272.doCAD(send_cad_number);
            endDoCad=millis();
 
            PRINT_CSTSTR("%s","--> CAD duration ");
            PRINT_VALUE("%ld",endDoCad-startDoCad);
            PRINTLN;
        
            if (!e)
              PRINT_CSTSTR("%s","OK2");            
            else
              PRINT_CSTSTR("%s","###2");
            
            PRINTLN;
          }              
        }
        else {
          PRINT_CSTSTR("%s","###1\n");  
          
          // if we have "only once" behavior then exit here to not have retries
          if (onlyOnce)
          	return 1;

          // wait for random number of DIFS
#ifdef ARDUINO                
          uint8_t w = random(1,8);
#else
          uint8_t w = rand() % 8 + 1;
#endif
          
          PRINT_CSTSTR("%s","--> waiting for ");
          PRINT_VALUE("%d",w);
          PRINT_CSTSTR("%s"," DIFS (DIFS=3SIFS) = ");
          PRINT_VALUE("%d",SIFS_value[loraMode]*3*w);
          PRINTLN;
          
          delay(SIFS_value[loraMode]*3*w);
          
          PRINT_CSTSTR("%s","--> retry\n");
        }

      } while (e);
    
      // CAD is OK, but need to check RSSI
      if (RSSIonSend) {
    
          e=sx1272.getRSSI();
          
          uint8_t rssi_retry_count=10;
          
          if (!e) {
          
            PRINT_CSTSTR("%s","--> RSSI ");
            PRINT_VALUE("%d", sx1272._RSSI);
            PRINTLN;
            
            while (sx1272._RSSI > -90 && rssi_retry_count) {
              
              delay(1);
              sx1272.getRSSI();
              PRINT_CSTSTR("%s","--> RSSI ");
              PRINT_VALUE("%d",  sx1272._RSSI);       
              PRINTLN; 
              rssi_retry_count--;
            }
          }
          else
            PRINT_CSTSTR("%s","--> RSSI error\n"); 
        
          if (!rssi_retry_count)
            carrierSenseRetry=true;  
          else
	    carrierSenseRetry=false;            
      }
      
    } while (carrierSenseRetry);  
  }

  return 0;
}
  
void loop(void)
{ 
  int i=0, e;
  int cmdValue;
  
/////////////////////////////////////////////////////////////////// 
// ONLY FOR TESTING CAD
#ifdef CAD_TEST

  startDoCad=millis();
  e = sx1272.doCAD(SIFS_cad_number);
  endDoCad=millis();
  
  PRINT_CSTSTR("%s","--> SIFS duration ");
  PRINT_VALUE("%ld", endDoCad-startDoCad);
  PRINTLN;

  if (!e) 
    PRINT_CSTSTR("%s","OK");
  else
    PRINT_CSTSTR("%s","###");

  PRINTLN;
  
  delay(200);
  
  startDoCad=millis();
  e = sx1272.doCAD(SIFS_cad_number*3);
  endDoCad=millis();
  
  PRINT_CSTSTR("%s","--> DIFS duration ");
  PRINT_VALUE("%ld", endDoCad-startDoCad);
  PRINTLN;

  if (!e) 
    PRINT_CSTSTR("%s","OK");
  else
    PRINT_CSTSTR("%s","###");
  
  PRINTLN;
    
  delay(200);
#endif
// ONLY FOR TESTING CAD
///END/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////// 
// START OF PERIODIC TASKS

  receivedFromSerial=false;
  receivedFromLoRa=false;
  
#ifdef LORA_LAS  
  // call periodically to be able to detect the start of a new cycle
  loraLAS.checkCycle();
#endif

#if defined ARDUINO && not defined GW_RELAY
  // check if we received data from the input serial port
  if (Serial.available()) {

    i=0;  

    while (Serial.available() && i<80) {
      cmd[i]=Serial.read();
      i++;
      delay(50);
    }
    
    cmd[i]='\0';

    PRINT_CSTSTR("%s","Rcv serial: ");
    PRINT_STR("%s",cmd);
    PRINTLN;
    
    receivedFromSerial=true; 
  }
#endif
  
// handle keyboard input from a UNIX terminal
// quick & dirty way to provide command to a running gateway
// for test & debug purposes mainly
#if not defined ARDUINO && defined WINPUT
  
  while (unistd::read(0, &ch, 1)) {
    
	if (ch == '\n') {

		strcpy(cmd,keyPressBuff);
                PRINT_CSTSTR("%s","Cmd from keyboard: ");
                PRINT_STR("%s",cmd);
                PRINTLN;
		
		keyIndex=0;	
                receivedFromSerial=true;
	}
	else {
        	// backspace
        	if (ch == 127 || ch==8) {
        		keyIndex--;
        	}
        	else {
        	
        		keyPressBuff[keyIndex]=(char)ch;
        		keyIndex++;
        	}
        }
	
	keyPressBuff[keyIndex]='\0';

        PRINT_CSTSTR("%s","keyboard input : ");
        PRINT_STR("%s",keyPressBuff);
        PRINTLN;    
  }
#endif

/////////////////////////////////////////////////////////////////// 
// THE MAIN PACKET RECEPTION LOOP
//
  if (radioON && !receivedFromSerial) {
        
      e=1;
#ifndef CAD_TEST

      if (status_counter==60 || status_counter==0) {
         PRINT_CSTSTR("%s","^$Low-level gw status ON");
         PRINTLN;
         FLUSHOUTPUT; 
         status_counter=0;
      }
      
      // check if we received data from the receiving LoRa module
#ifdef RECEIVE_ALL
      e = sx1272.receiveAll(10000);
#else
#ifdef GW_AUTO_ACK  

      e = sx1272.receivePacketTimeout(10000);

      status_counter++;

      if (e!=0 && e!=3) {
         PRINT_CSTSTR("%s","^$Receive error ");
         PRINT_VALUE("%d", e);
         PRINTLN;

         if (e==2) {
             // Power OFF the module
             sx1272.OFF();
             radioON=false;
             PRINT_CSTSTR("%s","^$Resetting radio module");
             PRINTLN;
             e = sx1272.ON();
             PRINT_CSTSTR("%s","^$Setting power ON: state ");
             PRINT_VALUE("%d",e);
             PRINTLN;

             if (!e) {
               radioON=true;
               startConfig();
             }
             // to start over
             status_counter=0;
             e=1;
         }
         FLUSHOUTPUT;         
      }
      
      if (!e && sx1272._requestACK_indicator) {
         PRINT_CSTSTR("%s","^$ACK requested by ");
         PRINT_VALUE("%d", sx1272.packet_received.src);
         PRINTLN;
         FLUSHOUTPUT;      
      }
#else
      // OBSOLETE normally we always use GW_AUTO_ACK
      // Receive message
      if (withAck)
        e = sx1272.receivePacketTimeoutACK(10000);
      else      
        e = sx1272.receivePacketTimeout(10000);
#endif          
#endif
#endif
/////////////////////////////////////////////////////////////////// 

      if (!e) {
        
         int a=0, b=0;
         uint8_t tmp_length;

         receivedFromLoRa=true;

#if not defined ARDUINO && defined DOWNLINK        
		 // set timer, gw will check for downlink request (downlink.txt) after
		 // interDownlinkCheckTime 			
         lastDownlinkCheckTime=millis();
         // only if we enabled downlink check after packet reception
         if (!optNDL)
         	enableDownlinkCheck=true;
#endif

///////////////////////////////////////////////////////////////////         
// for Linux-based gateway only
// provide reception timestamp

#if not defined ARDUINO && not defined GW_RELAY
         char time_buffer[30];
         int millisec;
         struct tm* tm_info;
         struct timeval tv;
        
         gettimeofday(&tv, NULL);
        
         millisec = lrint(tv.tv_usec/1000.0); // Round to nearest millisec
         
         if (millisec>=1000) { // Allow for rounding up to nearest second
            millisec -=1000;
            tv.tv_sec++;
         }
        
         tm_info = localtime(&tv.tv_sec);
#endif       

         tmp_length=sx1272._payloadlength;
         
#if not defined GW_RELAY

         sx1272.getSNR();
         sx1272.getRSSIpacket();
         
         // we split in 2 parts to use a smaller buffer size
         sprintf(cmd, "--- rxlora. dst=%d type=0x%.2X src=%d seq=%d", 
                   sx1272.packet_received.dst,
                   sx1272.packet_received.type, 
                   sx1272.packet_received.src,
                   sx1272.packet_received.packnum);
                   
         PRINT_STR("%s", cmd);

         sprintf(cmd, " len=%d SNR=%d RSSIpkt=%d BW=%d CR=4/%d SF=%d\n",
                   tmp_length, 
                   sx1272._SNR,
                   sx1272._RSSIpacket,
                   (sx1272._bandwidth==BW_125)?125:((sx1272._bandwidth==BW_250)?250:500),
                   sx1272._codingRate+4,
                   sx1272._spreadingFactor);
                   
         PRINT_STR("%s", cmd);         

         // provide a short output for external program to have information about the received packet
         // ^psrc_id,seq,len,SNR,RSSI
         sprintf(cmd, "^p%d,%d,%d,%d,%d,%d,%d\n",
                   sx1272.packet_received.dst,
                   sx1272.packet_received.type,                   
                   sx1272.packet_received.src,
                   sx1272.packet_received.packnum, 
                   tmp_length,
                   sx1272._SNR,
                   sx1272._RSSIpacket);
                   
         PRINT_STR("%s", cmd);          

         // ^rbw,cr,sf
         sprintf(cmd, "^r%d,%d,%d\n", 
                   (sx1272._bandwidth==BW_125)?125:((sx1272._bandwidth==BW_250)?250:500),
                   sx1272._codingRate+4,
                   sx1272._spreadingFactor);
                   
         PRINT_STR("%s", cmd);  
#endif         
///////////////////////////////////////////////////////////////////

#if not defined ARDUINO && not defined GW_RELAY        
         strftime(time_buffer, 30, "%Y-%m-%dT%H:%M:%S", tm_info);
         sprintf(cmd, "^t%s.%03d\n", time_buffer, millisec);
         PRINT_STR("%s", cmd);
#endif
            
#ifdef LORA_LAS        
         if (loraLAS.isLASMsg(sx1272.packet_received.data)) {
           
           //tmp_length=sx1272.packet_received.length-OFFSET_PAYLOADLENGTH;
           tmp_length=sx1272._payloadlength;
           
           int v=loraLAS.handleLASMsg(sx1272.packet_received.src,
                                      sx1272.packet_received.data,
                                      tmp_length);
           
           if (v==DSP_DATA) {
             a=LAS_DSP+DATA_HEADER_LEN+1;
#ifdef WITH_DATA_PREFIX
             PRINT_STR("%c",(char)DATA_PREFIX_0);      
             PRINT_STR("%c",(char)DATA_PREFIX_1);
#endif             
           }
           else
             // don't print anything
             a=tmp_length; 
         }
         else
           PRINT_CSTSTR("%s","No LAS header. Write raw data\n");
#else
#if defined WITH_DATA_PREFIX && not defined GW_RELAY
         PRINT_STR("%c",(char)DATA_PREFIX_0);        
         PRINT_STR("%c",(char)DATA_PREFIX_1);
#endif
#endif

#if defined ARDUINO && defined GW_RELAY

         // here we resend the received data to the next gateway
         //
         // set correct header information
         sx1272._nodeAddress=sx1272.packet_received.src;
         sx1272._packetNumber=sx1272.packet_received.packnum;
         sx1272.setPacketType(sx1272.packet_received.type);

         CarrierSense();
               
         e = sx1272.sendPacketTimeout(1, sx1272.packet_received.data, tmp_length, 10000);
    
         PRINT_CSTSTR("%s","Packet re-sent, state ");
         PRINT_VALUE("%d",e);
         PRINTLN;
                  
         // set back the gateway address
         sx1272._nodeAddress=loraAddr;
         
#else
         // print to stdout the content of the packet
         //
         for ( ; a<tmp_length; a++,b++) {
           PRINT_STR("%c",(char)sx1272.packet_received.data[a]);

           if (b<MAX_CMD_LENGTH)
              cmd[b]=(char)sx1272.packet_received.data[a];
         }
         
         // strlen(cmd) will be correct as only the payload is copied
         cmd[b]='\0';    
         PRINTLN;
         FLUSHOUTPUT;
         
#if not defined ARDUINO && defined WINPUT
        // if we received something, display again the current input 
        // that has still not be terminated
        if (keyIndex) {
              PRINT_CSTSTR("%s","keyboard input : ");
              PRINT_STR("%s",keyPressBuff);
              PRINTLN;
        }
#endif   
#endif       
      }  
  }  
  
  if (receivedFromSerial || receivedFromLoRa) {
    
    i=0;
    
    if (cmd[i]=='/' && cmd[i+1]=='@') {

      PRINT_CSTSTR("%s","^$Parsing command\n");      
      i=2;
      
      PRINT_CSTSTR("%s","^$");
      PRINT_STR("%s",cmd);
      PRINTLN;
      
      if ( (receivedFromLoRa && cmd[i]!='U' && !unlocked) || !unlocked_try) {
        PRINT_CSTSTR("%s","^$Remote config locked\n");
        // just assign an unknown command
        cmd[i]='*';  
      }
      
      switch (cmd[i]) {

//uncomment if you don't want your device to be remotely configured
//#ifdef 0
            case 'U':
            
              if (unlocked_try) {
                i++;
                cmdValue=getCmdValue(i);
  
                if (cmdValue==UNLOCK_PIN) {
                  
                  unlocked=!unlocked;
                  
                  if (unlocked)
                    PRINT_CSTSTR("%s","^$Unlocked\n");
                  else
                    PRINT_CSTSTR("%s","^$Locked\n");
                }
                else
                  unlocked_try--;
                  
                if (unlocked_try==0)
                  PRINT_CSTSTR("%s","^$Bad pin\n");
              }
            break;
//#endif

            case 'S': 

              if (cmd[i+1]=='F') {
                  i=i+2;
                  cmdValue=getCmdValue(i);

                  if (cmdValue > 5 && cmdValue < 13) {
                      PRINT_CSTSTR("%s","^$set SF: ");
                      PRINT_VALUE("%d",cmdValue);  
                      PRINTLN;                    
                      // Set spreading factor
                      e = sx1272.setSF(cmdValue);
                      PRINT_CSTSTR("%s","^$set SF: state ");
                      PRINT_VALUE("%d",e);  
                      PRINTLN;   
                  }
              }                  
            break;  
            
            case 'M':
              i++;
              cmdValue=getCmdValue(i);
              // cannot set mode greater than 11 (11 being the LoRaWAN test mode)
              if (cmdValue > 11)
                      cmdValue = 4;
              // cannot set mode lower than 0
              if (cmdValue < 0)
                      cmdValue = 4;
              // set dest addr        
              loraMode=cmdValue; 
              
              PRINT_CSTSTR("%s","^$Set LoRa mode to ");
              PRINT_VALUE("%d",loraMode);
              PRINTLN;
              // Set transmission mode and print the result
              e = sx1272.setMode(loraMode);
              PRINT_CSTSTR("%s","^$LoRa mode: state ");
              PRINT_VALUE("%d",e);  
              PRINTLN;              

#ifdef LORA_LAS
              loraLAS.setSIFS(loraMode);
#endif
              // get preamble length
              e = sx1272.getPreambleLength();
              PRINT_CSTSTR("%s","Get Preamble Length: state ");
              PRINT_VALUE("%d",e);
              PRINTLN;
              PRINT_CSTSTR("%s","Preamble Length: ");
              PRINT_VALUE("%d",sx1272._preamblelength);
              PRINTLN;                
            break;

            case 'W':
              i++;
              cmdValue=getCmdValue(i);

              // we expect an HEX format value
              cmdValue = (cmdValue / 10)*16 + (cmdValue % 10);
              
              // cannot set sync word greater than 255
              if (cmdValue > 255)
                      cmdValue = 0x12;
              // cannot set sync word lower than 0
              if (cmdValue <= 0)
                      cmdValue = 0x12;
              
              PRINT_CSTSTR("%s","^$Set sync word to 0x");
              PRINT_HEX("%X", cmdValue);
              PRINTLN;
              
              e = sx1272.setSyncWord(cmdValue);
              PRINT_CSTSTR("%s","^$LoRa sync word: state ");
              PRINT_VALUE("%d",e);  
              PRINTLN;                
            break;
            
            case 'C': 
             
              if (cmd[i+1]=='A' && cmd[i+2]=='D') {
                
                  if (cmd[i+3]=='O' && cmd[i+4]=='N') {
                       i=i+5;
                       cmdValue=getCmdValue(i);
                       // cannot set send_cad_number greater than 255
                       if (cmdValue > 255)
                              cmdValue = 255;
                              
                       send_cad_number=cmdValue; 
                      
                       PRINT_CSTSTR("%s","Set send_cad_number to ");
                       PRINT_VALUE("%d",send_cad_number);
                       PRINTLN;
                       break;                       
                  } 
                    
                  if (cmd[i+3]=='O' && cmd[i+4]=='F' && cmd[i+5]=='F' ) {
                       send_cad_number=0;
                       break;
                  }
                  
                  startDoCad=millis();
                  e = sx1272.doCAD(SIFS_cad_number);                  
                  endDoCad=millis();

                  PRINT_CSTSTR("%s","--> SIFS duration ");
                  PRINT_VALUE("%ld",endDoCad-startDoCad);
                  PRINTLN; 
                    
                  if (!e) 
                      PRINT_CSTSTR("%s","OK");
                  else
                      PRINT_CSTSTR("%s","###");
                    
                  PRINTLN;
              }
              else {      
                  i++;
                  cmdValue=getCmdValue(i);

                  if (cmdValue < STARTING_CHANNEL || cmdValue > ENDING_CHANNEL)
                    loraChannelIndex=STARTING_CHANNEL;
                  else
                    loraChannelIndex=cmdValue;
                      
                  loraChannelIndex=loraChannelIndex-STARTING_CHANNEL;  
                  loraChannel=loraChannelArray[loraChannelIndex];
                  
                  PRINT_CSTSTR("%s","^$Set LoRa channel to ");
                  PRINT_VALUE("%d",cmdValue);
                  PRINTLN;
                  
                  // Select frequency channel
                  e = sx1272.setChannel(loraChannel);
                  PRINT_CSTSTR("%s","^$Setting Channel: state ");
                  PRINT_VALUE("%d",e);  
                  PRINTLN;
                  PRINT_CSTSTR("%s","Time: ");
                  PRINT_VALUE("%d",sx1272._stoptime-sx1272._starttime);  
                  PRINTLN;                  
              }              
            break;

            case 'P': 

              if (cmd[i+1]=='L' || cmd[i+1]=='H' || cmd[i+1]=='M' || cmd[i+1]=='x' || cmd[i+1]=='X' ) {
                loraPower=cmd[i+1];

                PRINT_CSTSTR("%s","^$Set LoRa Power to ");
                PRINT_VALUE("%c",loraPower);  
                PRINTLN;                
                 
                e = sx1272.setPower(loraPower);
                PRINT_CSTSTR("%s","^$Setting Power: state ");
                PRINT_VALUE("%d",e);  
                PRINTLN; 
              }
              else
                PRINT_CSTSTR("%s","Invalid Power. L, H, M, x or X accepted.\n");          
            break;

            case 'O': 

              if (cmd[i+1]=='N') {
                
                  PRINT_CSTSTR("%s","^$Setting LoRa module to ON");
                  
                  // Power ON the module
                  e = sx1272.ON();
                  PRINT_CSTSTR("%s","^$Setting power ON: state ");
                  PRINT_VALUE("%d",e);     
                  PRINTLN;
                  
                  if (!e) {
                    radioON=true;
                    startConfig();
                  }
                  
                  delay(500);            
              }
              else
               if (cmd[i+1]=='F' && cmd[i+2]=='F') {
                PRINT_CSTSTR("%s","^$Setting LoRa module to OFF\n");
                
                // Power OFF the module
                sx1272.OFF(); 
                radioON=false;             
               }
               else
                 PRINT_CSTSTR("%s","Invalid command. ON or OFF accepted.\n");          
            break;            

#ifdef LORA_LAS
            // act as an LR-BS if IS_RCV_GATEWAY or an end-device if IS_SEND_GATEWAY
            case 'L': 
             
              if (cmd[i+1]=='A' && cmd[i+2]=='S') {
                
                if (cmd[i+3]=='S')
                  loraLAS.showLAS();
                  
                if (cmd[i+3]=='R') {
                  loraLAS.reset(); 
                  loraLAS.showLAS();
                }

                if (cmd[i+3]=='O' && cmd[i+4]=='N')
                  loraLAS.ON(LAS_ON_NORESET);

                if (cmd[i+3]=='O' && cmd[i+4]=='F' && cmd[i+5]=='F')
                  loraLAS.OFF();                  

                // only the base station can sent an INIT restart message
                // sends an init restart
                if (cmd[i+3]=='I') {
                  loraLAS.sendInit(LAS_INIT_RESTART); 
                }
              }
            break;         
#endif
            default:

              PRINT_CSTSTR("%s","Unrecognized cmd\n");       
              break;
      }
      FLUSHOUTPUT;
    }
  } // end of "if (receivedFromSerial || receivedFromLoRa)" 
  
#if not defined ARDUINO && defined DOWNLINK
  // handle downlink request
  else {
  	
  	// TODO change behavior so that the gw check for downlink when receivedFromLoRa
  	//
	bool triggerDownlinkCheck=false;
				
	// have we wrapped around?
	if (millis()-lastDownlinkCheckTime < 0) {
	        if (millis()+ULONG_MAX-lastDownlinkCheckTime > interDownlinkCheckTime)
	                triggerDownlinkCheck=true;            
	}  
	
    if ( (millis()-lastDownlinkCheckTime > interDownlinkCheckTime || triggerDownlinkCheck) && enableDownlinkCheck)  {	
    	
		char time_buffer[30];
		int millisec;
		struct tm* tm_info;
		struct timeval tv;
		
		gettimeofday(&tv, NULL);
		tm_info = localtime(&tv.tv_sec);
		strftime(time_buffer, 30, "%Y-%m-%dT%H:%M:%S", tm_info);   	

		// use rapidjson to parse all lines
		// if there is a downlink.txt file then read all lines in memory for
		// further processing
		printf("^$-----------------------------------------------------\n");
		printf("^$Check for downlink requests %s\n", time_buffer);
		
		fp = fopen("downlink/downlink.txt","r");
		
		if (fp) {
			
			fclose(fp);
			
			// remove any \r characters introduced by various OS and/or tools
			system("sed -i 's/\r//g' downlink/downlink.txt");
			// remove empty lines
			system("sed -i '/^$/d' downlink/downlink.txt");
			
			fp = fopen("downlink/downlink.txt","r");
			
			dl_total_line=1;
			hasDownlinkEntry=true;
			
			// read all lines
			while(!feof(fp)) {
				
				json_entry[dl_total_line]=(char*)malloc(json_entry_size*sizeof(char));
				
				dl_line_size=getline(&json_entry[dl_total_line], &json_entry_size, fp);
				
				printf("^$Read downlink %d: %ld\n", dl_total_line, dl_line_size);
				
				if (dl_line_size>0)
					dl_total_line++;
			}
			
			fclose(fp);
			
			char tmp_c[100];
			
			sprintf(tmp_c, "mv downlink/downlink.txt downlink/downlink-backup-%s.txt", time_buffer);
				
			system(tmp_c);
		}
		else {
			hasDownlinkEntry=false;
			printf("^$NO NEW DOWNLINK ENTRY\n");
		}	
	
		if (hasDownlinkEntry) {
	
			Document document;		
	
			printf("^$Queue all valid downlink requests\n");
	
			for (dl_line_index=1; dl_line_index < dl_total_line; dl_line_index++)
			{
				StringBuffer json_record_buffer;
				Writer<StringBuffer> writer(json_record_buffer);
			
		        printf("^$Downlink entry %d: %s", dl_line_index, json_entry[dl_line_index]);
				
				document.Parse(json_entry[dl_line_index]);
		
				if (document["status"].IsString())
					printf("^$status = %s\n", document["status"].GetString());
				
				if (document["dst"].IsInt())
					printf("^$dst = %d\n", document["dst"].GetInt());
		      	
		      	if (document["data"].IsString())
					printf("^$data = %s\n", document["data"].GetString());
				
				// check if it is a valid send request
				if (document["status"]=="send_request" && document["dst"].IsInt()) {
				
					document["status"].SetString("queued", document.GetAllocator());
					document.Accept(writer);
					printf("^$JSON record: %s\n", json_record_buffer.GetString());
					FLUSHOUTPUT;
					
					fp = fopen("downlink/downlink-queued.txt","a");
					
					if (fp) {
						
						gettimeofday(&tv, NULL);
						
						// Round to nearest millisec
						millisec = lrint(tv.tv_usec/1000.0); 
						
						// Allow for rounding up to nearest second
						if (millisec>=1000) { 
							millisec -=1000;
							tv.tv_sec++;
						}
						
						tm_info = localtime(&tv.tv_sec);
						strftime(time_buffer, 30, "%Y-%m-%dT%H:%M:%S", tm_info);
						
						fprintf(fp, "%s.%03d %s\n", time_buffer, millisec, json_record_buffer.GetString());
						
						fclose(fp);
					}
				}
				else
					printf("^$DISCARDING: not valid send_request\n");
			}
			
			// set to begining of downlink request for transmission
			dl_line_index=1;
		}
		
		//printf("^$Next downlink request check in %ld min\n", interDownlinkCheckTime/60000);
		printf("^$-----------------------------------------------------\n");    
		//lastDownlinkCheckTime=millis();
		// so that we can start transmitting the first request immediately if any
		lastDownlinkSendTime=0;
		// disable downlink check because new request will only be indicated at the next lora packet reception 
		enableDownlinkCheck=false;
		FLUSHOUTPUT;	
    }
    
    if (hasDownlinkEntry && (millis()-lastDownlinkSendTime>interDownlinkSendTime)) {
    	
    	Document document;
    	
    	printf("^$-----------------------------------------------------\n");
    	printf("^$Process downlink requests %d: %s\n", dl_line_index, json_entry[dl_line_index]);
    
		StringBuffer json_record_buffer;
		Writer<StringBuffer> writer(json_record_buffer);
		
		document.Parse(json_entry[dl_line_index]);

		if (document["status"].IsString())
			printf("^$status = %s\n", document["status"].GetString());
		
		if (document["dst"].IsInt())
			printf("^$dst = %d\n", document["dst"].GetInt());
      	
      	if (document["data"].IsString())
			printf("^$data = %s\n", document["data"].GetString());
		
		// check if it is a valid send request
		if (document["status"]=="send_request" && document["dst"].IsInt()) {
			
			// disable extended IFS behavior, just a small number of CAD
			extendedIFS=false;
			
			if (!CarrierSense(true)) {
				
				char time_buffer[30];
				int millisec;
				struct tm* tm_info;
				struct timeval tv;
						
				gettimeofday(&tv, NULL);
				
				sx1272.setPacketType(PKT_TYPE_DATA);
				
				e = sx1272.sendPacketTimeout(document["dst"].GetInt(), (uint8_t*)document["data"].GetString(), document["data"].GetStringLength(), 10000);
				
				PRINT_CSTSTR("%s","Packet sent, state ");
	      PRINT_VALUE("%d",e);
	      PRINTLN;
				
				if (!e)
					document["status"].SetString("sent", document.GetAllocator());
				else
					document["status"].SetString("sent_fail", document.GetAllocator());	
					
				document.Accept(writer);
				printf("^$JSON record: %s\n", json_record_buffer.GetString());
				FLUSHOUTPUT;
			
				// delete this request
				// even if transmission is not successful
				free(json_entry[dl_line_index]);
				
				fp = fopen("downlink/downlink-sent.txt","a");
				
				if (fp) {
					
					// Round to nearest millisec
					millisec = lrint(tv.tv_usec/1000.0); 
					
					// Allow for rounding up to nearest second
					if (millisec>=1000) { 
						millisec -=1000;
						tv.tv_sec++;
					}
					
					tm_info = localtime(&tv.tv_sec);
					strftime(time_buffer, 30, "%Y-%m-%dT%H:%M:%S", tm_info);
					
					fprintf(fp, "%s.%03d %s\n", time_buffer, millisec, json_record_buffer.GetString());
					
					fclose(fp);
				}
				// skip it in all cases
				dl_line_index++;
			}
			else {
				printf("^$DELAYED: busy channel\n");	
				// here we will retry later because of a busy channel
			}
			//set back extendedIFS
			extendedIFS=true;
		}
		else {
			printf("^$DISCARDING: not valid send_request\n");
			// skip it
			dl_line_index++;    	
		}
		
    	if (dl_line_index==dl_total_line) {
    		hasDownlinkEntry=false;
    		printf("^$-----------------------------------------------------\n");
    		printf("^$NO MORE PENDING send_request\n");
    	}
    	    	
    	lastDownlinkSendTime=millis();
    }
  }	
#endif
} 

///////////////////////////////
// for Linux-based gateway only
///////////////////////////////
#ifndef ARDUINO

#ifdef WINPUT
// when CTRL-C is pressed
// set back the correct terminal settings
void  INThandler(int sig)
{
    struct termios old = {0};
    
    if (tcgetattr(0, &old) < 0)
                perror("tcsetattr()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
                perror ("tcsetattr ~ICANON");
                
    PRINT_CSTSTR("%s","Bye.\n");            
    exit(0);
}
#endif

int main (int argc, char *argv[]){

  int opt=0;
  
  //Specifying the expected options
  static struct option long_options[] = {
      {"mode", required_argument, 0,  'a' },
      {"aes", no_argument, 0,         'b' },
      {"bw", required_argument, 0,    'c' },
      {"cr", required_argument, 0,    'd' },
      {"sf", required_argument, 0,    'e' }, 
      {"raw", no_argument, 0,         'f' },
      {"freq", required_argument, 0,  'g' },
      {"ch", required_argument, 0,    'h' },       
      {"sw", required_argument, 0,    'i' },
#ifdef DOWNLINK      
      {"ndl", no_argument, 0,    'j' },       
#endif                            
      {0, 0, 0,  0}
  };
  
  int long_index=0;
  
  while ((opt = getopt_long(argc, argv,"a:bc:d:e:fg:h:i:j", 
                 long_options, &long_index )) != -1) {
      switch (opt) {
           case 'a' : loraMode = atoi(optarg);
               break;
           case 'b' : optAESgw=true; // not used at the moment
               break;
           case 'c' : optBW = atoi(optarg); 
                      // 125, 250 or 500
                      // setBW() will correct the optBW value  
               break;
           case 'd' : optCR = atoi(optarg);
                      // 5, 6, 7 or 8
                      // setCR() will correct the optCR value
               break;
           case 'e' : optSF = atoi(optarg);
                      // 6, 7, 8, 9, 10, 11 or 12
               break;
           case 'f' : optRAW=true;
               break;               
           case 'g' : optFQ=atof(optarg);
                      // in MHz
                      // e.g. 868.1
                      loraChannel=optFQ*1000000.0*RH_LORA_FCONVERT;
               break;     
           case 'h' : optCH=true;
                      loraChannelIndex=atoi(optarg);
                      if (loraChannelIndex < STARTING_CHANNEL || loraChannelIndex > ENDING_CHANNEL)
                        loraChannelIndex=STARTING_CHANNEL;
                      loraChannelIndex=loraChannelIndex-STARTING_CHANNEL;  
                      loraChannel=loraChannelArray[loraChannelIndex]; 
               break;      
           case 'i' : { uint8_t sw=atoi(optarg);
                      // assume that sw is expressed in hex value
                      optSW = (sw / 10)*16 + (sw % 10); }
               break;
#ifdef DOWNLINK                       
			case 'j' : optNDL=true;
               break;    
#endif                                                     
           //default: print_usage(); 
           //    exit(EXIT_FAILURE);
      }
  }

#ifdef WINPUT  
  // set termios options to remove echo and to have non blocking read from
  // standard input (e.g. keyboard)
  struct termios old = {0};
  if (tcgetattr(0, &old) < 0)
              perror("tcsetattr()");
  // non-blocking noncanonical mode          
  old.c_lflag &= ~ICANON;
  old.c_lflag &= ~ECHO;
  // VMIN and VTIME are 0 for non-blocking    
  old.c_cc[VMIN] = 0;
  old.c_cc[VTIME] = 0;
  
  if (tcsetattr(0, TCSANOW, &old) < 0)
          perror("tcsetattr ICANON");   
  
  // we catch the CTRL-C key
  signal(SIGINT, INThandler);
#endif

  setup();
  
  while(1){
    loop();
  }
  
  return (0);
}
#endif

