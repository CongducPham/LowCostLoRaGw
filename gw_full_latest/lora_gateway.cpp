/* 
 *  LoRa low-level gateway to receive and send command
 *
 *  Copyright (C) 2015-2019 Congduc Pham
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
 *  Version:                2.1a
 *  Design:                 C. Pham
 *  Implementation:         C. Pham
 *
 *      
 *   IMPORTANT NOTICE   
 *    - the gateway use data prefix to indicate data received from radio. The prefix is 2 bytes: 0xFF0xFE 
 *    - the post-processing stage looks for this sequence to dissect the data, according to the format adopted by the sender 
 *    - if you use our LoRa_Temp example, then the packet format as expected by the post-processing script is as follows:
 *      - without application key and without encryption, payload starts immediately: [payload]
 *      - if application key is used, without encryption: [AppKey(4B)][payload]
 *      - with encryption: the payload is encrypted using LoRaWAN algorithm
 *    - for more details on the underlying packet format used by our modified SX1272 library
 *      - refer to the SX1272.h
 *      - see http://cpham.perso.univ-pau.fr/LORA/RPIgateway.html
*/

/*  Change logs
 *  May, 19th, 2020. v2.1a
 *	      uplink and downlink feequency
 *	        - define LORAWAN_UPFQ, LORAWAN_D2FQ and LORAWAN_D2SF to better adapt the custom frequency plan
 *          - change these parameters for each band definition (BAND868, BAND900 and BAND433)
 *  Jan, 29th, 2020. v2.1
 *        lora_gateway in raw mode now checks 4 times for downlink.txt after a packet reception at t_r (to work with Network Server sending PULL_RESP in just-in-time mode)
 *			- first, after t_r+DELAY_DNWFILE (delay of 900ms, no reception possible) to target RX1
 *			- second, after t_r+DELAY_DNWFILE+DELAY_EXTDNW2 (additional delay of 1000ms, no reception possible) to target RX2
 *			- third, after t_r+DELAY_JACC1-DELAY_DNWFILE (radio in reception mode) to target RX1 for join-accept only
 *				* if a new packet is received during the last wait period then it has priority
 *				* lora_gateway then starts a new cycle of downlink attempts
 *			- last, after t_r+DELAY_JACC1+DELAY_EXTDNW2-DELAY_DNWFILE (additional delay of 1000ms, no reception possible) to target RX2 for join-accept only
 *  Jan, 20th, 2020. v2.0
 *        add LoRaWAN downlink from LoRaWAN Network Server
 *			- can use RX1 (same datarate than uplink) or RX2 (869.525MHz and SF12BW125) depending on downlink timestamp
 *        remove old and unused code and only targets RPI-based gateway
 *			- removing the Arduino support (those who want to use an Arduino as a simple radio bridge should use Arduino_LoRa_Gateway v1.9)
 *			- removing remote configuration feature
 *			- removing keyboard input option
 *  Nov, 22nd, 2019. v1.9a 
 *		  handle getopt issue on newer Linux distrib, compile with -DGETOPT_ISSUE
 *	March 23rd, 2019. v1.9
 *		  improve suport for LoRaWAN
 *		  the radio info string has a frequency information, e.g. 125,5,12,868100
 *			- divide the frequency by 1000.0 to get in MHz	
 *  Feb, 28th, 2018. v1.8 
 *        add a 4-byte MIC when sending downlink packet: uncomment #define INCLUDE_MIC_IN_DOWNLINK
 *		  	- packet type is then set to PKT_TYPE_DATA | PKT_FLAG_DATA_ENCRYPTED | PKT_FLAG_DATA_DOWNLINK
 *		    - the 4-byte MIC comes right after the payload		
 *  July, 4th, 2017. v1.75 
 *        receive window set to MAX_TIMEOUT (10000ms defined in SX1272.h)
 *  June, 19th, 2017. v1.74
 *        interDownlinkCheckTime is set to 5s
 *  May, 8th, 2017. v1.73
 *        Default behavior is to disable the remote configuration features
 *  Mar, 29th, 2017. v1.72
 *        Add periodic status (every 10 minutes) from the low-level gateway
 *        Add reset of radio module when receive error from radio is detected
 *        Change receive windows from 1000ms, or 2500ms in mode 1, to 10000ms for all modes, reducing overheads of receive loop 
 *  Mar, 2nd, 2017. v1.71
 *        Add preamble length verification and correction to the default value of 8 if it is not the case
 *        Note that in most cases, a wrong preamble usually means more than one running instance of the low-level program
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
 *			- the lora_gateway.cpp program will check for a downlink.txt file after each LoRa packet reception
 *			- this behavior can be disable with --ndl option
 *			- after a LoRa packet reception, lora_gateway.cpp will wait for interDownlinkCheckTime before checking for downlink.txt file
 *			- downlink.txt will be normally generated by post_processing_gw.py
 *			- post_processing_gw.py periodically check for downlink-post.txt and will build a queue of downlink requests
 *			- when a lora packet from device i is processed by post_processing_gw.py, it will check if there is a pending request for device i
 *			- if it is the case, then post_processing_gw.py generates the corresponding downlink.txt which will contain in most cases only 1 entry
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef GETOPT_ISSUE
int getopt (int argc, char * const argv[], const char *optstring);
extern char *optarg;
extern int optind, opterr, optopt;
#endif
#include <getopt.h>
#include <termios.h> 
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>

#define PRINTLN                   printf("\n")
#define PRINT_CSTSTR(fmt,param)   printf(fmt,param)
#define PRINT_STR(fmt,param)      PRINT_CSTSTR(fmt,param)
#define PRINT_VALUE(fmt,param)    PRINT_CSTSTR(fmt,param)
#define PRINT_HEX(fmt,param)      PRINT_VALUE(fmt,param)
#define FLUSHOUTPUT               fflush(stdout);

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

// IMPORTANT
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// it is better to use radio.makefile to indicate the default frequency band
//#define BAND868
//#define BAND900
//#define BAND433
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// For a Raspberry-based gateway the distribution uses a radio.makefile file that can define MAX_DBM
//
#ifndef MAX_DBM
#define MAX_DBM 14
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// FOR DOWNLINK FEATURES
//
//comment if you want to disable downlink at compilation
#define DOWNLINK
#if defined DOWNLINK

//use same definitions from LMIC, but in ms
enum { DELAY_JACC1       =  5000 }; // in millisecs
enum { DELAY_DNW1        =  1000 }; // in millisecs down window #1
enum { DELAY_EXTDNW2     =  1000 }; // in millisecs
enum { DELAY_JACC2       =  DELAY_JACC1+(int)DELAY_EXTDNW2 }; // in millisecs
enum { DELAY_DNW2        =  DELAY_DNW1 +(int)DELAY_EXTDNW2 }; // in millisecs down window #1

// need to give some time for the post-processing stage to generate a downlink.txt file if any
// for LoRaWAN, need to give some time to lorawan_downlink.py to get PULL_RESP and generate downlink.txt
// as the receive window 1 is at 1s after packet transmission at device side, it does not give much margin
enum { DELAY_DNWFILE     =  900 }; // in millisecs
enum { MARGIN_DNW        =  20 }; // in millisecs

#define DEBUG_DOWNLINK_TIMING
//#define KEEP_DOWNLINK_BACKUP_FILE
//#define KEEP_DOWNLINK_SENT_FILE

#include "base64.h"
#include "rapidjson/reader.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <iostream>

using namespace rapidjson;
using namespace std;

bool enableDownlinkCheck=true;
bool checkForLateDownlinkRX2=false;
bool checkForLateDownlinkJACC1=false;
bool checkForLateDownlinkJACC2=false;

//only for non-LoRaWAN downlink
//#define INCLUDE_MIC_IN_DOWNLINK

int xtoi(const char *hexstring);
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//BAND868 by default
#if not defined BAND868 && not defined BAND900 && not defined BAND433
#define BAND868
#endif 

///////////////////////////////////////////////////////////////////
// BAND868
#ifdef BAND868

//first freq of EU863-870
#define LORAWAN_UPFQ 868.1
#define LORAWAN_D2FQ 869.525
#define LORAWAN_D2SF 12

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

///////////////////////////////////////////////////////////////////
// BAND900
#elif defined BAND900 

//first freq of AS923
#define LORAWAN_UPFQ 923.2
#define LORAWAN_D2FQ 923.2
#define LORAWAN_D2SF 10

#define MAX_NB_CHANNEL 13
#define STARTING_CHANNEL 0
#define ENDING_CHANNEL 12
uint8_t loraChannelIndex=5;
uint32_t loraChannelArray[MAX_NB_CHANNEL]={CH_00_900,CH_01_900,CH_02_900,CH_03_900,CH_04_900,CH_05_900,CH_06_900,CH_07_900,CH_08_900,
                                            CH_09_900,CH_10_900,CH_11_900,CH_12_900};

///////////////////////////////////////////////////////////////////
// BAND433
#elif defined BAND433

//first freq of EU433
#define LORAWAN_UPFQ 433.175
#define LORAWAN_D2FQ 434.665
#define LORAWAN_D2SF 12

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
// on the 868.1MHz channel for BAND868, 923.2MHz for BAND900 and 433.175 for BAND433
//#define LORAMODE 11
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// GATEWAY HAS ADDRESS 1
#define LORA_ADDR 1
///////////////////////////////////////////////////////////////////

// will use 0xFF0xFE to prefix data received from LoRa, so that post-processing stage can differenciate
// data received from radio
#define WITH_DATA_PREFIX

#ifdef WITH_DATA_PREFIX
#define DATA_PREFIX_0 0xFF
#define DATA_PREFIX_1 0xFE
#endif

///////////////////////////////////////////////////////////////////
// CONFIGURATION VARIABLES
//
bool radioON=false;
bool RSSIonSend=true;
uint8_t loraMode=LORAMODE;
uint16_t rcv_timeout=MAX_TIMEOUT;
uint32_t loraChannel=loraChannelArray[loraChannelIndex];
uint8_t loraAddr=LORA_ADDR;
int status_counter=0;
unsigned long startDoCad, endDoCad;
bool extendedIFS=true;
uint8_t SIFS_cad_number;
// set to 0 to disable carrier sense based on CAD
uint8_t send_cad_number=0;
uint8_t SIFS_value[11]={0, 183, 94, 44, 47, 23, 24, 12, 12, 7, 4};
uint8_t CAD_value[11]={0, 62, 31, 16, 16, 8, 9, 5, 3, 1, 1};

uint16_t optBW=0; 
uint8_t optSF=0;
uint8_t optCR=0;
uint8_t optCH=0;
bool  optRAW=false;
double optFQ=-1.0;
uint8_t optSW=0x12;
bool  optHEX=false;
bool optIIQ=false;

///////////////////////////////////////////////////////////////////
// TIMER-RELATED VARIABLES
//
uint32_t rcv_time_ms;
uint32_t rcv_time_tmst;
char time_str[100];
///////////////////////////////////////////////////////////////////


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
    //loraMode=0;
  
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
  
    if (loraMode>7)
      SIFS_cad_number=6;
    else 
      SIFS_cad_number=3;
  }
  
  // Select frequency channel
  
  // LoRaWAN
  if (loraMode==11) {
  	PRINT_CSTSTR("%s","^$Configuring for LoRaWAN\n");
  	
  	if (optIIQ)
    	PRINT_CSTSTR("%s","^$Invert I/Q for downlink\n");  	
    
    if (optFQ<0.0) {
    	optFQ=LORAWAN_UPFQ;	
		loraChannel=optFQ*1000000.0*RH_LORA_FCONVERT;
    	e = sx1272.setChannel(loraChannel);    
	}
	else {
      e = sx1272.setChannel(loraChannel);
	}

	PRINT_CSTSTR("%s","^$Set frequency to ");
	PRINT_VALUE("%f", optFQ);
	PRINT_CSTSTR("%s",": state "); 
			
	//set raw mode for LoRaWAN
	//overriding existing configuration
	optRAW=true;
	
	//set sync word for LoRaWAN
	optSW=0x34;
  }
  else {
    e = sx1272.setChannel(loraChannel);

    if (optFQ>0.0) {
      PRINT_CSTSTR("%s","^$Set frequency to ");
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
		optFQ=loraChannel/(1000000.0*RH_LORA_FCONVERT);
    }
  }  
  PRINT_VALUE("%d", e);
  PRINTLN; 

  // Select amplifier line; PABOOST or RFO
#ifdef PABOOST
  sx1272._needPABOOST=true;
  PRINT_CSTSTR("%s","^$Use PA_BOOST amplifier line");
  PRINTLN;   
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
  sx1272._nodeAddress=loraAddr;
  e=0;
  PRINT_CSTSTR("%s","^$LoRa addr ");
  PRINT_VALUE("%d", loraAddr);
  PRINT_CSTSTR("%s",": state ");
  PRINT_VALUE("%d", e);
  PRINTLN;

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
}

uint32_t getGwDateTime()
{
  char time_buffer[30];
  struct tm* tm_info;
  struct timeval tv;
  
  gettimeofday(&tv, NULL);
  tm_info = localtime(&tv.tv_sec);
  strftime(time_buffer, 30, "%Y-%m-%dT%H:%M:%S", tm_info);
  sprintf(time_str, "%s.%06ld", time_buffer, tv.tv_usec);
  
  return (tv.tv_usec+tv.tv_sec*1000000UL);
}

void setup()
{
  int e;

  srand (time(NULL));

  // Power ON the module
  e = sx1272.ON();
  
  PRINT_CSTSTR("%s","^$**********Power ON: state ");
  PRINT_VALUE("%d", e);
  PRINTLN;
  
  if (!e) {
    radioON=true;
    startConfig();
  }

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
    
  FLUSHOUTPUT;
  delay(1000);

#if defined DOWNLINK
  PRINT_CSTSTR("%s","^$Low-level gateway has downlink support");
  PRINTLN;
  if (!enableDownlinkCheck) {
  	PRINT_CSTSTR("%s","^$But downlink is disabled in gateway_conf.json");
  	PRINTLN;
  }
#endif
  
  getGwDateTime();
  PRINT_STR("%s\n", time_str);
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
        PRINT_VALUE("%lu",endDoCad-startDoCad);
        PRINTLN;
        
        if (!e) {
          PRINT_CSTSTR("%s","OK1\n");
          
          if (extendedIFS)  {          
            // wait for random number of CAD

            uint8_t w = rand() % 8 + 1;
  
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
            PRINT_VALUE("%lu",endDoCad-startDoCad);
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
          uint8_t w = rand() % 8 + 1;
          
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
    char print_buf[100];
    uint32_t current_time_tmst;    

    /////////////////////////////////////////////////////////////////// 
    // START OF PERIODIC TASKS
	
    bool receivedFromLoRa=false;

    /////////////////////////////////////////////////////////////////// 
    // THE MAIN PACKET RECEPTION LOOP
    //
    if (radioON && !checkForLateDownlinkRX2 && !checkForLateDownlinkJACC2) {
        
		e=1;

		if (status_counter==60 || status_counter==0) {
			PRINT_CSTSTR("%s","^$Low-level gw status ON");
			PRINTLN;
			FLUSHOUTPUT; 
			status_counter=0;
		} 

		e = sx1272.receivePacketTimeout(rcv_timeout);

		if (e==0) {
			//get timestamps only when we actually receive something
			rcv_time_tmst=getGwDateTime();
			rcv_time_ms=millis();
			
			//we actually received something when we were "waiting" for a JACC1 downlink 
			//we give priority to the latest reception
			if (rcv_timeout!=MAX_TIMEOUT) {
				//remove any downlink.txt file that may have been generated late because it was not for the new packet reception
				remove("downlink/downlink.txt");
				//set back to normal reception mode
				checkForLateDownlinkJACC1=false;
				rcv_timeout=MAX_TIMEOUT;
			}
		}
		
		status_counter++;
		
		//e=0 : packet reception
		//e=3 : no packet reception
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

		if (e==0 && sx1272._requestACK_indicator) {
		 	PRINT_CSTSTR("%s","^$ACK requested by ");
		 	PRINT_VALUE("%d", sx1272.packet_received.src);
		 	PRINTLN;
		 	FLUSHOUTPUT;      
		}         

		/////////////////////////////////////////////////////////////////// 
		// LoRa reception       
    
		if (e==0) {

#ifdef DEBUG_DOWNLINK_TIMING
		 	//this is the time (millis()) at which the packet has been completely received at the radio module
		 	PRINT_CSTSTR("%s","^$rcv_time_ms:"); 		 
      	 	PRINT_VALUE("%lu",rcv_time_ms);
      	 	PRINTLN;
#endif          
         	uint8_t tmp_length;

         	receivedFromLoRa=true;
			//remove any downlink.txt file that may have been generated late and will therefore be out-dated
			remove("downlink/downlink.txt");         	       

			///////////////////////////////////////////////////////////////////         
			// provide reception timestamp

			//tmp_length=sx1272._payloadlength;
			tmp_length=sx1272.getPayloadLength();

			sx1272.getSNR();
			sx1272.getRSSIpacket();

			sprintf(print_buf, "--- rxlora. dst=%d type=0x%02X src=%d seq=%d", 
				sx1272.packet_received.dst,
				sx1272.packet_received.type, 
				sx1272.packet_received.src,
				sx1272.packet_received.packnum);
	   
			PRINT_STR("%s", print_buf);

			sprintf(print_buf, " len=%d SNR=%d RSSIpkt=%d BW=%d CR=4/%d SF=%d\n", 
				tmp_length, 
				sx1272._SNR,
				sx1272._RSSIpacket,
				(sx1272._bandwidth==BW_125)?125:((sx1272._bandwidth==BW_250)?250:500),
				sx1272._codingRate+4,
				sx1272._spreadingFactor);     

			PRINT_STR("%s", print_buf);              

			// provide a short output for external program to have information about the received packet
			// ^psrc_id,seq,len,SNR,RSSI
			sprintf(print_buf, "^p%d,%d,%d,%d,",
				sx1272.packet_received.dst,
				sx1272.packet_received.type,                   
				sx1272.packet_received.src,
				sx1272.packet_received.packnum);
	   
			PRINT_STR("%s", print_buf);       

			sprintf(print_buf, "%d,%d,%d\n",
				tmp_length,
			 	sx1272._SNR,
			 	sx1272._RSSIpacket);

			PRINT_STR("%s", print_buf); 

			// ^rbw,cr,sf,fq
			sprintf(print_buf, "^r%d,%d,%d,%lu\n", 
				(sx1272._bandwidth==BW_125)?125:((sx1272._bandwidth==BW_250)?250:500),
				sx1272._codingRate+4,
				sx1272._spreadingFactor,
				(uint32_t)(optFQ*1000.0));

			PRINT_STR("%s", print_buf);

			//print the reception date and time
			sprintf(print_buf, "^t%s*%lu\n", time_str, rcv_time_tmst);
			PRINT_STR("%s", print_buf);

#if defined WITH_DATA_PREFIX
        	PRINT_STR("%c",(char)DATA_PREFIX_0);        
         	PRINT_STR("%c",(char)DATA_PREFIX_1);
#endif

         	// print to stdout the content of the packet
         	//
         	FLUSHOUTPUT;
         
         	for ( int a=0; a<tmp_length; a++) {
         	
				if (optHEX) {
			  		if ((uint8_t)sx1272.packet_received.data[a]<16)
			  			PRINT_CSTSTR("%s","0");
			  		PRINT_HEX("%X",	(uint8_t)sx1272.packet_received.data[a]);
			  		PRINT_CSTSTR("%s"," ");	
			  	}
			  	else
			  		PRINT_STR("%c",(char)sx1272.packet_received.data[a]);
         	}
           
         	PRINTLN;
         	FLUSHOUTPUT;      
      	}  
	}  
  
  	if (receivedFromLoRa || checkForLateDownlinkRX2 || checkForLateDownlinkJACC1 || checkForLateDownlinkJACC2) {
    
#ifdef DOWNLINK
    	// handle downlink request
		bool hasDownlinkEntry=false;    	
		char* downlink_json_entry=NULL;
		
#ifdef DEBUG_DOWNLINK_TIMING
		//this is the time (millis()) at which we start waiting for checking downlink requests
		printf("^$downlink wait: %lu\n", millis());
#endif
    		    	
		if (enableDownlinkCheck)  {	

			//wait until it is the (approximate) right time to check for downlink requests
			//because the generation of downlink.txt file takes some time
			//only for RX1, after that, we simply pass
			while (millis()-rcv_time_ms < DELAY_DNWFILE)
				;

#ifdef DEBUG_DOWNLINK_TIMING
			//this is the time (millis()) at which we start checking downlink requests
			printf("^$downlink check: %lu\n", millis());	
#endif
    		//to set time_str 
    		current_time_tmst=getGwDateTime();   	
    
    		// use rapidjson to parse all lines
    		// if there is a downlink.txt file then read all lines in memory for
    		// further processing
    		if (checkForLateDownlinkRX2)
    			printf("^$----delayRX2-----------------------------------------\n");
    		else if (checkForLateDownlinkJACC1)	
    			printf("^$----retryJACC1---------------------------------------\n");    			
    		else if (checkForLateDownlinkJACC2)	
    			printf("^$----retryJACC2---------------------------------------\n");
    		else	
    			printf("^$-----------------------------------------------------\n");
    			
    		printf("^$Check for downlink requests %s*%lu\n", time_str, current_time_tmst);
    		
    		FILE *fp = fopen("downlink/downlink.txt","r");
    		
    		if (fp) {
    			
    			fclose(fp);
    			
    			// remove any \r characters introduced by various OS and/or tools
    			system("sed -i 's/\r//g' downlink/downlink.txt");
    			// remove empty lines
    			system("sed -i '/^$/d' downlink/downlink.txt");
    			
    			fp = fopen("downlink/downlink.txt","r");
    			
    			size_t len=0;
    			
    			ssize_t dl_line_size=getline(&downlink_json_entry, &len, fp);
    				
    			printf("^$Read downlink: %lu\n", dl_line_size);
    				
    			if (dl_line_size>0) {
    				hasDownlinkEntry=true;
    				//printf("^$Downlink entry: %s", downlink_json_entry);
    			}
    			
    			fclose(fp);
    			
#ifdef KEEP_DOWNLINK_BACKUP_FILE 
				char tmp_c[100];   			
				sprintf(tmp_c, "mv downlink/downlink.txt downlink/downlink-backup-%s.txt", time_str);
				system(tmp_c);
#else
				remove("downlink/downlink.txt");
#endif

    			//got a downlink, clear all indicators and back to normal reception for next time
    			checkForLateDownlinkRX2=false;
    			checkForLateDownlinkJACC1=false;
    			checkForLateDownlinkJACC2=false;
    			rcv_timeout=MAX_TIMEOUT;					
    		}
    		else {
    			hasDownlinkEntry=false;
    			printf("^$NO NEW DOWNLINK ENTRY\n");

    			if (checkForLateDownlinkRX2) {
					checkForLateDownlinkRX2=false;
					// retry later, last retry
					checkForLateDownlinkJACC1=true;
					//will however wait for new incoming messages up to rcv_time_ms+DELAY_JACC1-(DELAY_DNW1-DELAY_DNWFILE)
					rcv_timeout=rcv_time_ms+DELAY_JACC1-millis()-(DELAY_DNW1-DELAY_DNWFILE);    			
    			}
				else
				//it was already the last retry for late downlink
				if (checkForLateDownlinkJACC2) {
					checkForLateDownlinkJACC2=false;
					rcv_timeout=MAX_TIMEOUT;
				}   
				else
				if (checkForLateDownlinkJACC1) { 
					// retry later, for RX2 (join)
					delay(DELAY_EXTDNW2);
					checkForLateDownlinkJACC1=false;
					checkForLateDownlinkJACC2=true;
				}				 			
				else {
					//we initiate the entire downlink check only for LoRaWAN, i.e. when raw mode is enabled
					if (optRAW) { 
						// retry later, for RX2 (data)
						delay(DELAY_EXTDNW2);
						checkForLateDownlinkRX2=true;
					}
				}    			

				FLUSHOUTPUT;
    		}	   		
    	}

#ifdef DEBUG_DOWNLINK_TIMING
		//this is the time (millis()) at which we start processing the downlink requests
		printf("^$downlink process: %lu\n", millis());
#endif
        
    	if (hasDownlinkEntry) {
			Document document;
		  
			bool is_lorawan_downlink=false;
		
			printf("^$Process downlink request\n");
			//printf("^$Process downlink requests: %s\n",  downlink_json_entry);
		
			document.Parse(downlink_json_entry);
					
			if (document.IsObject()) {
				if (document.HasMember("txpk"))								
					if (document["txpk"].HasMember("modu"))
						if (document["txpk"]["modu"].IsString())
							if (document["txpk"]["modu"]=="LORA") {
								is_lorawan_downlink=true;    
								//printf("^$data = %s\n", document["txpk"]["data"].GetString());
							}
						
				if (is_lorawan_downlink) {		
					printf("^$LoRaWAN downlink request\n");				

					//use the tmst field provided by the Network Server in the downlink message
					uint32_t send_time_tmst=document["txpk"]["tmst"].GetUint();
					//to get the waiting delay
					uint32_t rx_wait_delay=send_time_tmst-rcv_time_tmst;

					//uncomment to test re-scheduling for RX2
					//delay(500);	
					
					bool go_for_downlink=false;
									
					// just in case
					if (send_time_tmst < rcv_time_tmst)
						printf("^$WARNING: downlink tmst in the past, abort\n");	
					else 
					if (send_time_tmst - rcv_time_tmst > ((uint32_t)DELAY_JACC2+(uint32_t)DELAY_EXTDNW2)*1000)
						printf("^$WARNING: downlink tmst too much in future, abort\n");	
					else
						{
							uint32_t wmargin;
							
							//determine if we have some margin to re-schedule
							if (rx_wait_delay/1000 == (uint32_t)DELAY_DNW1 || rx_wait_delay/1000 == (uint32_t)DELAY_JACC1)
								wmargin=(uint32_t)DELAY_EXTDNW2;
							else
								wmargin=0;
							
							//are we behind scheduled 	
							if (millis() > rcv_time_ms + rx_wait_delay/1000 - MARGIN_DNW) {
								//
								if (wmargin) {
									rx_wait_delay+=(uint32_t)wmargin*1000;
									go_for_downlink=true;
									printf("^$WARNING: too late for RX1, try RX2\n");	
								}
								else
									printf("^$WARNING: too late to send downlink, abort\n");
							}
							else
								go_for_downlink=true;				
						}
								
					if (go_for_downlink) {												
						uint8_t downlink_payload[256];
				
						//invert I/Q
						//TODO: currently, invert I/Q on the gateway is not working. Why?
  						if (optIIQ) {
							e = sx1272.invertIQ(true);
							printf("^$Invert I/Q: state %d\n", e);	
						}	
						
						//set raw mode in sending
						sx1272._rawFormat_send=true;
						//save current freq
						uint32_t currentFreq=sx1272._channel;
						//save current SF
						uint8_t currentSF=sx1272._spreadingFactor;				
				
						//convert base64 data into binary buffer
						int downlink_size = b64_to_bin(document["txpk"]["data"].GetString(), document["txpk"]["data"].GetStringLength(), downlink_payload, sizeof downlink_payload);
				
						if (downlink_size != document["txpk"]["size"].GetInt()) {
							printf("^$WARNING: mismatch between .size and .data size once converter to binary\n");
						}
								
						//uncomment to test for RX2
						//rx_wait_delay+=(uint32_t)DELAY_EXTDNW2*1000;

						bool useRX2=false;
				
						if (rx_wait_delay/1000 == (uint32_t)DELAY_DNW2 || rx_wait_delay/1000 == (uint32_t)DELAY_JACC2)
							useRX2=true;
				
						// should wait for RX2
						if (useRX2) {           

							printf("^$Target RX2\n");
							//set frequency according to RX2 window
							e = sx1272.setChannel(LORAWAN_D2FQ*1000000.0*RH_LORA_FCONVERT);
							//change to SF according to RX2 window
							e = sx1272.setSF(LORAWAN_D2SF);					
						}
						else
							printf("^$Target RX1\n");
					
						//wait until it is the (approximate) right time to send the downlink data
						//downlink data can use DELAY_DNW1 or DELAY_DNW2
						//join-request-accept can use DELAY_JACC1 or DELAY_JACC2
						while (millis()-rcv_time_ms < (rx_wait_delay/1000-MARGIN_DNW /* take a margin*/))
							;
			
						//	send the packet
#ifdef DEBUG_DOWNLINK_TIMING						
						printf("^$downlink send: %lu\n", millis());
#endif									
						e = sx1272.sendPacketTimeout(0, (uint8_t*)downlink_payload, downlink_size, 10000);    

						printf("^$Packet sent, state %d\n", e);	
				
						//remove I/Q inversion
  						if (optIIQ) {							
							e = sx1272.invertIQ(false);
							printf("^$Normal I/Q: state %d\n", e);	
						}
						
						//remove raw mode in sending
						sx1272._rawFormat_send=false;

						if (useRX2) {					
							//set back to the reception frequency
							e = sx1272.setChannel(currentFreq);
							printf("^$Set back frequency: state %d\n", e);	
							//set back the SF
							e = sx1272.setSF(currentSF);
							printf("^$Set back SF: state %d\n", e);
						}
					}
				} 
				else {			
					// non-LoRaWAN downlink		    
					if (document["dst"].IsInt())
						printf("^$dst = %d\n", document["dst"].GetInt());

					e=1;
							
					// check if it is a valid send request
					if (document["status"]=="send_request" && document["dst"].IsInt()) {
			
						// disable extended IFS behavior, just a small number of CAD
						extendedIFS=false;
			
						if (!CarrierSense(true)) {
				
							sx1272.setPacketType(PKT_TYPE_DATA | PKT_FLAG_DATA_DOWNLINK);
						
							uint8_t l=document["data"].GetStringLength();
							uint8_t* pkt_payload=(uint8_t*)document["data"].GetString();
							uint8_t downlink_message[256];
						 
#ifdef INCLUDE_MIC_IN_DOWNLINK    				
							// we test if we have MIC data in the json entry
							if (document["MIC0"].IsString() && document["MIC0"]!="") {
						
								// indicate a downlink packet with a 4-byte MIC after the payload
								sx1272.setPacketType(PKT_TYPE_DATA | PKT_FLAG_DATA_ENCRYPTED | PKT_FLAG_DATA_DOWNLINK);
					
								memcpy(downlink_message, (uint8_t*)document["data"].GetString(), l);
					
								// set the 4-byte MIC after the payload
								downlink_message[l]=(uint8_t)xtoi(document["MIC0"].GetString());
								downlink_message[l+1]=(uint8_t)xtoi(document["MIC1"].GetString());
								downlink_message[l+2]=(uint8_t)xtoi(document["MIC2"].GetString());
								downlink_message[l+3]=(uint8_t)xtoi(document["MIC3"].GetString());
							
								// here we sent the downlink packet with the 4-byte MIC
								// at the device, the expected behavior is
								// - to test for the packet type, then remove 4 bytes from the payload length to get the real payload
								// - use AES encryption on the clear payload to compute the MIC and compare with the MIC sent in the downlink packet
								// - if both MIC are equal, then accept the downlink packet as a valid downlink packet
								l += 4;
								pkt_payload=downlink_message;
							}			
#endif

							//wait until it is the (approximate) right time to send the downlink date
							//the end-device approximately waits for 1s
							while (millis()-rcv_time_ms < (DELAY_DNW1-MARGIN_DNW) /* take a margin*/)
								;
						
#ifdef DEBUG_DOWNLINK_TIMING						
							printf("^$downlink send: %lu\n", millis());
#endif							
							e = sx1272.sendPacketTimeout(document["dst"].GetInt(), pkt_payload, l, 10000);

							printf("^$Packet sent, state %d\n", e); 
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
					}
				}							

#ifdef KEEP_DOWNLINK_SENT_FILE

				if (document.HasMember("status")) {

					StringBuffer json_record_buffer;
					Writer<StringBuffer> writer(json_record_buffer);
						
					getGwDateTime();

					//transmission successful		
					if (!e)
						document["status"].SetString("sent", document.GetAllocator());
					else
						document["status"].SetString("sent_fail", document.GetAllocator());

					document.Accept(writer);
					printf("^$JSON record: %s\n", json_record_buffer.GetString());

					FILE *fp = fopen("downlink/downlink-sent.txt","a");
		
					if (fp) {
						fprintf(fp, "%s %s\n", time_str, json_record_buffer.GetString());
						fclose(fp);
					}					
				}  		
#endif			

				if (downlink_json_entry)
					free(downlink_json_entry);
				
				printf("^$-----------------------------------------------------\n");
			}
			else { 
				printf("^$WARNING: not valid JSON format, abort\n");
			}			
			FLUSHOUTPUT;
		}	
		
		//remove any downlink.txt file that may have been generated late and will therefore be out-dated
		if (!checkForLateDownlinkRX2 && !checkForLateDownlinkJACC2)
			remove("downlink/downlink.txt");
#endif
	}
} 


int
xtoi(const char *hexstring)
{
	int	i = 0;
	
	if ((*hexstring == '0') && (*(hexstring+1) == 'x'))
		  hexstring += 2;
	while (*hexstring)
	{
		char c = toupper(*hexstring++);
		if ((c < '0') || (c > 'F') || ((c > '9') && (c < 'A')))
			break;
		c -= '0';
		if (c > 9)
			c -= 7;
		i = (i << 4) + c;
	}
	return i;
}

int main (int argc, char *argv[]){

  int opt=0;
  
  //Specifying the expected options
  static struct option long_options[] = {
      {"mode", required_argument, 0,  'a' },
      {"bw", required_argument, 0,    'b' },
      {"cr", required_argument, 0,    'c' },
      {"sf", required_argument, 0,    'd' }, 
      {"raw", no_argument, 0,         'e' },
      {"freq", required_argument, 0,  'f' },
      {"ch", required_argument, 0,    'g' },       
      {"sw", required_argument, 0,    'h' },
#ifdef DOWNLINK      
      {"ndl", no_argument, 0,    'i' },       
#endif                            
      {"hex", no_argument, 0,    'j' },
      {"iiq", no_argument, 0,    'k' },      
      {0, 0, 0,  0}
  };
  
  int long_index=0;
  
  while ((opt = getopt_long(argc, argv,"a:b:c:d:ef:g:h:ij", 
                 long_options, &long_index )) != -1) {
      switch (opt) {
           case 'a' : loraMode = atoi(optarg);
               break;
           case 'b' : optBW = atoi(optarg); 
                      // 125, 250 or 500
                      // setBW() will correct the optBW value  
               break;
           case 'c' : optCR = atoi(optarg);
                      // 5, 6, 7 or 8
                      // setCR() will correct the optCR value
               break;
           case 'd' : optSF = atoi(optarg);
                      // 6, 7, 8, 9, 10, 11 or 12
               break;
           case 'e' : optRAW=true;
               break;               
           case 'f' : optFQ=atof(optarg);
                      // in MHz
                      // e.g. 868.1
                      loraChannel=optFQ*1000000.0*RH_LORA_FCONVERT;
               break;     
           case 'g' : optCH=true;
                      loraChannelIndex=atoi(optarg);
                      if (loraChannelIndex < STARTING_CHANNEL || loraChannelIndex > ENDING_CHANNEL)
                        loraChannelIndex=STARTING_CHANNEL;
                      loraChannelIndex=loraChannelIndex-STARTING_CHANNEL;  
                      loraChannel=loraChannelArray[loraChannelIndex]; 
               break;      
           case 'h' : { uint8_t sw=atoi(optarg);
                      // assume that sw is expressed in hex value
                      optSW = (sw / 10)*16 + (sw % 10); }
               break;
#ifdef DOWNLINK                       
			case 'i' : enableDownlinkCheck=false;
               break;    
#endif
           case 'j' : optHEX=true;
               break;
           case 'k' : optIIQ=true;
               break;                                                                  
           //default: print_usage(); 
           //    exit(EXIT_FAILURE);
      }
  }

  setup();
  
  while(1){
    loop();
  }
  
  return (0);
}
