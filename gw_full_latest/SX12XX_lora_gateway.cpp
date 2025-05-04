/* 
 *  LoRa low-level gateway to receive and send command
 *
 *  Copyright (C) 2015-2020 Congduc Pham
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
 *  Version:                2.2
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
 *  October 28th, 2020. v2.2 
 *				IMPORTANT. The low-level LoRa communication lib has moved from Libelium-based lib to Stuart Robinson's SX12XX lib.
 *					- https://github.com/StuartsProjects/SX12XX-LoRa
 *					- SX126X, SX127X, and SX128X related files have been ported to run on both Arduino & RaspberryPI environment
 *					- currently, the code has been tested for SX127X, still require testing for SX126X and SX128X	
  *					- for more details see https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-SX12XX.md
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
 *					- can use RX1 (same datarate than uplink) or RX2 (869.525MHz and SF12BW125) depending on downlink timestamp
 *        IMPORTANT. Remove old and unused code and only targets RPI-based gateway
 *					- removing the Arduino support (those who want to use an Arduino as a simple radio bridge should use Arduino_LoRa_Gateway v1.9)
 *					- removing remote configuration feature
 *					- removing keyboard input option
 *  Nov, 22nd, 2019. v1.9a 
 *		  	handle getopt issue on newer Linux distrib, compile with -DGETOPT_ISSUE
 *	March 23rd, 2019. v1.9
 *		  	improve suport for LoRaWAN
 *		  	the radio info string has a frequency information, e.g. 125,5,12,868100
 *					- divide the frequency by 1000.0 to get in MHz	
 *  Feb, 28th, 2018. v1.8 
 *        add a 4-byte MIC when sending downlink packet: uncomment #define INCLUDE_MIC_IN_DOWNLINK
 *		  		- packet type is then set to PKT_TYPE_DATA | PKT_FLAG_DATA_ENCRYPTED | PKT_FLAG_DATA_DOWNLINK
 *		    	- the 4-byte MIC comes right after the payload		
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
 *		  	Change the downlink strategy
 *					- the lora_gateway.cpp program will check for a downlink.txt file after each LoRa packet reception
 *					- this behavior can be disable with --ndl option
 *					- after a LoRa packet reception, lora_gateway.cpp will wait for interDownlinkCheckTime before checking for downlink.txt file
 *					- downlink.txt will be normally generated by post_processing_gw.py
 *					- post_processing_gw.py periodically check for downlink-post.txt and will build a queue of downlink requests
 *					- when a lora packet from device i is processed by post_processing_gw.py, it will check if there is a pending request for device i
 *					- if it is the case, then post_processing_gw.py generates the corresponding downlink.txt which will contain in most cases only 1 entry
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

//LoRa chip version will be defined by makefile
//
#ifdef SX126X
#include <SX126XLT.h>                                          
#include "SX126X_lora_gateway.h"
#endif

#ifdef SX127X
#include <SX127XLT.h>                                          
#include "SX127X_lora_gateway.h"
#endif

#ifdef SX128X
#include <SX128XLT.h>                                          
#include "SX128X_lora_gateway.h"
#endif

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
#define PRINT_CSTSTR(param)       printf(param)
#define PRINTLN_CSTSTR(param)			do {printf(param);printf("\n");} while(0)
#define PRINT_STR(fmt,param)      printf(fmt,param)
#define PRINTLN_STR(fmt,param)		{printf(fmt,param);printf("\n");}
#define PRINT_VALUE(fmt,param)    printf(fmt,param)
#define PRINTLN_VALUE(fmt,param)	do {printf(fmt,param);printf("\n");} while(0)
#define PRINT_HEX(fmt,param)      printf(fmt,param)
#define PRINTLN_HEX(fmt,param)		do {printf(fmt,param);printf("\n");} while(0)
#define FLUSHOUTPUT               fflush(stdout)

//#define DEBUG

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
// ADDITIONAL FEATURES
//
//reconfigure radio module periodically, every 100 pkt received
#define PERIODIC_RESET100
//usually set for LoRaWAN
#define INVERTIQ_ON_TX

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

//#define DEBUG_DOWNLINK_TIMING
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
#if not defined BAND868 && not defined BAND900 && not defined BAND433 && not defined BAND2400
#define BAND868
#endif 

///////////////////////////////////////////////////////////////////
// BAND868
#ifdef BAND868

//first freq of EU863-870
#define LORAWAN_UPFQ 868100000
#define LORAWAN_D2FQ 869525000
#define LORAWAN_D2SF 12

#define MAX_NB_CHANNEL 15
#define STARTING_CHANNEL 4
#define ENDING_CHANNEL 18
uint32_t loraChannelArray[MAX_NB_CHANNEL]={CH_04_868,CH_05_868,CH_06_868,CH_07_868,CH_08_868,CH_09_868, CH_10_868,CH_11_868,CH_12_868,CH_13_868,CH_14_868,CH_15_868,CH_16_868,CH_17_868,CH_18_868};

///////////////////////////////////////////////////////////////////
// BAND900
#elif defined BAND900 

//first freq of AS923
#define LORAWAN_UPFQ 923200000
#define LORAWAN_D2FQ 923200000
#define LORAWAN_D2SF 10

#define MAX_NB_CHANNEL 13
#define STARTING_CHANNEL 0
#define ENDING_CHANNEL 12
uint32_t loraChannelArray[MAX_NB_CHANNEL]={CH_00_900,CH_01_900,CH_02_900,CH_03_900,CH_04_900,CH_05_900,CH_06_900,CH_07_900,CH_08_900,CH_09_900,CH_10_900,CH_11_900,CH_12_900};

///////////////////////////////////////////////////////////////////
// BAND433
#elif defined BAND433

//first freq of EU433
#define LORAWAN_UPFQ 433175000
#define LORAWAN_D2FQ 434665000
#define LORAWAN_D2SF 12

#define MAX_NB_CHANNEL 4
#define STARTING_CHANNEL 0
#define ENDING_CHANNEL 3
uint32_t loraChannelArray[MAX_NB_CHANNEL]={CH_00_433,CH_01_433,CH_02_433,CH_03_433};             

///////////////////////////////////////////////////////////////////
// BAND2400
#elif defined BAND2400
#define LORAWAN_UPFQ CH_00_2400
#define LORAWAN_D2FQ CH_00_2400
#define LORAWAN_D2SF 12            

#define MAX_NB_CHANNEL 3
#define STARTING_CHANNEL 0
#define ENDING_CHANNEL 2
uint32_t loraChannelArray[MAX_NB_CHANNEL]={CH_00_2400,CH_01_2400,CH_02_2400};                     
#endif

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
#define LORAWAN_MODE 11
uint8_t loraMode=-1;
const uint16_t MAX_TIMEOUT = 10000;	
uint16_t rcv_timeout=MAX_TIMEOUT;
int status_counter=0;
uint8_t SIFS_cad_number;
// set to 0 to disable carrier sense based on CAD
uint8_t cad_number=3;

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

//create a library class instance called LT
#ifdef SX126X
SX126XLT LT;
#endif

#ifdef SX127X
SX127XLT LT;
#endif

#ifdef SX128X
SX128XLT LT;
#endif

//create the buffer that received packets are copied into
uint8_t RXBUFFER[RXBUFFER_SIZE];

uint32_t RXpacketCount;
uint32_t RXpacketCountReset=0;
uint32_t errors;

//stores length of packet received
uint8_t RXPacketL;                     
//stores RSSI of received packet          
int8_t  PacketRSSI;      
//stores signal to noise ratio (SNR) of received packet                        
int8_t  PacketSNR;                               
                                   
///////////////////////////////////////////////////////////////////

void loraConfig() {

  // has customized LoRa settings
    // has customized LoRa settings    
  if (optBW!=0 || optCR!=0 || optSF!=0) {
		if (optCR!=0) {
		
			PRINT_CSTSTR("^$LoRa CR4/");
			if (optCR<5)
				optCR=optCR+4;
			PRINTLN_VALUE("%d", optCR);    
			if (optCR==5)
				CodeRate = LORA_CR_4_5;
			else if (optCR==6)
				CodeRate = LORA_CR_4_6;		
			else if (optCR==7)
				CodeRate = LORA_CR_4_7;				
			else if (optCR==8)
				CodeRate = LORA_CR_4_8;
			else {
				PRINT_CSTSTR("^$Unknown LoRa CR, taking CR4/5\n");
				CodeRate = LORA_CR_4_5;
			}	
		}
			
		if (optSF!=0) {
			PRINT_CSTSTR("^$LoRa SF ");
			PRINTLN_VALUE("%d", optSF);
#if defined SX126X || defined SX128X			
			if (optSF==5)
				SpreadingFactor = LORA_SF5;
			else
#endif			
			if (optSF==6)
				SpreadingFactor = LORA_SF6;
			else if (optSF==7)
				SpreadingFactor = LORA_SF7;
			else if (optSF==8)
				SpreadingFactor = LORA_SF8;
			else if (optSF==9)
				SpreadingFactor = LORA_SF9;
			else if (optSF==10)
				SpreadingFactor = LORA_SF10;
			else if (optSF==11)
				SpreadingFactor = LORA_SF11;
			else if (optSF==12)
				SpreadingFactor = LORA_SF12;				
			else {
				PRINT_CSTSTR("^$Unknown LoRa SF, taking SF12\n");
				SpreadingFactor = LORA_SF12;
			}																	
		}

		if (optBW!=0) {    
			PRINT_CSTSTR("^$LoRa BW ");
			PRINTLN_VALUE("%ld", optBW*1000);
#if defined SX126X || defined SX127X			
			if (optBW==500)
				Bandwidth = LORA_BW_500;			
			else if (optBW==250)
				Bandwidth = LORA_BW_250;
			else if (optBW==125)
				Bandwidth = LORA_BW_125;							    
			else if (optBW==62)
				Bandwidth = LORA_BW_062; //actual  62500hz 			
			else if (optBW==41)
				Bandwidth = LORA_BW_041; //actual  41670hz
			else if (optBW==31)
				Bandwidth = LORA_BW_031; //actual  31250hz
			else if (optBW==20)
				Bandwidth = LORA_BW_020; //actual  20830hz			
			else if (optBW==15)
				Bandwidth = LORA_BW_015; //actual  15630hz
			else if (optBW==10)
				Bandwidth = LORA_BW_010; //actual  10420hz
			else if (optBW==7)
				Bandwidth = LORA_BW_007; //actual   7810hz	
			else {
				PRINT_CSTSTR("^$Unknown LoRa BW, taking BW125\n");
				Bandwidth = LORA_BW_125;
			}												
#endif
#ifdef SX128X
			if (optBW==125 || optBW==203 || optBW==200)
				Bandwidth = LORA_BW_0200;	//actually 203125hz		
			else if (optBW==250 || optBW==406 || optBW==400)
				Bandwidth = LORA_BW_0400; //actually 406250hz
			else if (optBW==500 || optBW==812 || optBW==800)
				Bandwidth = LORA_BW_0800;	//actually 812500hz						    
			else if (optBW==1625 || optBW==1600)
				Bandwidth = LORA_BW_1600; //actually 1625000hz
			else {
				PRINT_CSTSTR("^$Unknown LoRa BW, taking BW203\n");
				Bandwidth = LORA_BW_0200;
			}							
#endif				
		}

		if (optSF<10)
			SIFS_cad_number=6;
		else 
			SIFS_cad_number=3;
  }
  
  // Select frequency channel
	if (optFQ>0.0)
		DEFAULT_CHANNEL=optFQ;
		  
  // LoRaWAN
	if (loraMode==LORAWAN_MODE) {
		PRINT_CSTSTR("^$Configuring for LoRaWAN\n");
    	
		if (optFQ<0.0) {
			DEFAULT_CHANNEL=optFQ=LORAWAN_UPFQ;
		}
		//set raw mode for LoRaWAN
		//overriding existing configuration
		optRAW=true;		
	}

#ifdef MY_FREQUENCY
	DEFAULT_CHANNEL=MY_FREQUENCY;
#endif
	
	//otherwise DEFAULT_CHANNEL is set in SX12XX_lora_gateway.h
	PRINT_CSTSTR("^$Frequency ");
	PRINTLN_VALUE("%lu", DEFAULT_CHANNEL);

	PRINT_CSTSTR("^$LoRa addr ");
	PRINTLN_VALUE("%d", GW_ADDR);
	
  if (optRAW) {
		PRINT_CSTSTR("^$Raw format, not assuming any header in reception\n");  
		// when operating in raw format, the gateway will not decode the packet header but will pass all the payload to stdout
		// note that in this case, the gateway may process packet that are not addressed explicitly to it as the dst field is not checked at all
		// this would be similar to a promiscuous sniffer, but most of real LoRa gateway works this way 
  } 

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
  LT.calibrateImage(DEFAULT_CHANNEL);
  LT.setDIO2AsRfSwitchCtrl();
#endif
#ifdef SX128X
  LT.setRegulatorMode(USE_LDO);
#endif
  //set for LoRa transmissions                              
  LT.setPacketType(PACKET_TYPE_LORA);
  //set the operating frequency                 
  LT.setRfFrequency(DEFAULT_CHANNEL, Offset);                   
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
	if (loraMode==LORAWAN_MODE)
  	LT.setSyncWord(LORA_MAC_PUBLIC_SYNCWORD);              
	else
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
  PRINTLN_CSTSTR("^$Use PA_BOOST amplifier line");
#endif
#endif
#ifdef SX128X
  LT.setDioIrqParams(IRQ_RADIO_ALL, (IRQ_RX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);
#endif  
  
  //***************************************************************************************************

	//the gateway needs to know its address to verify that a packet requesting an ack is for him
	//this is done in the SX12XX library
	//
	LT.setDevAddr(GW_ADDR);
	//TX power and ramp time
	//this is required because the gateway can have to transmit ACK messages prior to send any downlink packet
	//so txpower needs to be set before hand.
	PRINT_CSTSTR("^$Set LoRa txpower dBm to ");
	PRINTLN_VALUE("%d",(uint8_t)MAX_DBM); 
#ifdef SX127X 	
	LT.setTxParams(MAX_DBM, RADIO_RAMP_DEFAULT);            
#endif
#if defined SX126X || defined SX128X
	LT.setTxParams(MAX_DBM, RAMP_TIME);
#endif
	//as the "normal" behavior is to invert I/Q on TX, the option here is only to set invert I/Q on RX
	//to test inversion with reception when a device has been set to invert I/Q on TX
	//of course this is only valid if proposed by the radio module which is the case for SX127X
  if (optIIQ) {
		PRINT_CSTSTR("^$Invert I/Q on RX\n");  
		LT.invertIQ(true);
  }

  // get preamble length
  PRINT_CSTSTR("^$Preamble Length: ");
  PRINTLN_VALUE("%d", LT.getPreamble());

#if defined SX126X || defined SX127X
    while (LT.getPreamble() != 8) {
      PRINTLN_CSTSTR("^$Bad Preamble Length: set back to 8");
  		LT.setPacketParams(8, LORA_PACKET_VARIABLE_LENGTH, 255, LORA_CRC_ON, LORA_IQ_NORMAL);
  }    
#endif

#ifdef SX128X
    while (LT.getPreamble() != 12) {
      PRINTLN_CSTSTR("^$Bad Preamble Length: set back to 12");
  		LT.setPacketParams(12, LORA_PACKET_VARIABLE_LENGTH, 255, LORA_CRC_ON, LORA_IQ_NORMAL, 0, 0);
  } 
#endif

#if defined SX126X || defined SX127X  
  PRINT_CSTSTR("^$Sync word: 0x"); 
  PRINTLN_HEX("%X", LT.getSyncWord());
#endif  

  //reads and prints the configured LoRa settings, useful check
  PRINT_CSTSTR("^$");
  LT.printModemSettings();                                     
  PRINTLN;
  //reads and prints the configured operting settings, useful check
  PRINT_CSTSTR("^$");
  LT.printOperatingSettings();                                
  PRINTLN;
#ifdef DEBUG  
  //print contents of device registers, normally 0x00 to 0x4F
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
#endif    

#ifdef SX126X
	PRINT_CSTSTR("^$SX126X ");
#endif
#ifdef SX127X
	PRINT_CSTSTR("^$SX127X ");
#endif
#ifdef SX128X
	PRINT_CSTSTR("^$SX128X ");
#endif 	    
  // Print a success message
  PRINTLN_CSTSTR("configured as LR-BS. Waiting RF input for transparent RF-serial bridge");
#ifdef PERIODIC_RESET100
	PRINTLN_CSTSTR("^$Will safely reset radio module every 100 packet received");
#endif   
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
  srand (time(NULL));
  
  SPI.begin();
  //Set Most significant bit first
  SPI.setBitOrder(MSBFIRST);
  //Divide the clock frequency
  SPI.setClockDivider(SPI_CLOCK_DIV64);
  //Set data mode
  SPI.setDataMode(SPI_MODE0); 

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
    PRINTLN_CSTSTR("^$**********Power ON");
  }
  else
  {
    PRINTLN_CSTSTR("^$No device responding");
    while (1)
    {
    }
  }

  loraConfig(); 

#if defined DOWNLINK
  PRINTLN_CSTSTR("^$Low-level gateway has downlink support");
  if (!enableDownlinkCheck)
  	PRINTLN_CSTSTR("^$But downlink is disabled");
#endif

  getGwDateTime();
  PRINT_STR("%s\n", time_str);
  
  FLUSHOUTPUT;
  delay(500);
}

void packet_is_Error()
{
  uint16_t IRQStatus;
  IRQStatus = LT.readIrqStatus();                   //read the LoRa device IRQ status register

  if (IRQStatus & IRQ_RX_TIMEOUT)                   //check for an RX timeout
  {
  	//here if we don't receive anything, we normally print nothing
    //PRINT_CSTSTR("RXTimeout");
  }
  else
  {
    errors++;
    PRINT_CSTSTR("^$Receive error");
    PRINT_CSTSTR(",RSSI,");
    PRINT_VALUE("%d",PacketRSSI);
    PRINT_CSTSTR("dBm,SNR,");
    PRINT_VALUE("%d",PacketSNR);
    PRINT_CSTSTR("dB,Length,");
    PRINT_VALUE("%d",LT.readRXPacketL());               //get the device packet length
    PRINT_CSTSTR(",Packets,");
    PRINT_VALUE("%d",RXpacketCount);
    PRINT_CSTSTR(",Errors,");
    PRINT_VALUE("%d",errors);
    PRINT_CSTSTR(",IRQreg,");
    PRINT_HEX("%X",IRQStatus);
    LT.printIrqStatus();                            //print the names of the IRQ registers set
    PRINTLN;
  }

  FLUSHOUTPUT;
}
  
void loop(void) 
{ 
	int i=0;
	int RXPacketL=0;
	char print_buf[100];
	uint32_t current_time_tmst;

	/////////////////////////////////////////////////////////////////// 
	// START OF PERIODIC TASKS

	bool receivedFromLoRa=false;

	/////////////////////////////////////////////////////////////////// 
	// THE MAIN PACKET RECEPTION LOOP
	//
	if (!checkForLateDownlinkRX2 && !checkForLateDownlinkJACC2) {
	
		if (status_counter==60 || status_counter==0) {
			PRINT_CSTSTR("^$Low-level gw status ON");
			PRINTLN;
			FLUSHOUTPUT; 
			status_counter=0;
		} 
		
		if (optRAW)
			RXPacketL = LT.receive(RXBUFFER, RXBUFFER_SIZE, 10000, WAIT_RX);
		else {	
			RXPacketL = LT.receiveAddressed(RXBUFFER, RXBUFFER_SIZE, 10000, WAIT_RX);
			
			//for packet with our header, destination must be GW_ADDR (i.e. 1) otherwise we reject it
			if (LT.readRXDestination()!=GW_ADDR)
				RXPacketL=-1;
		}
		
		if (RXPacketL>0) {
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
	
		if (RXPacketL==0) {
			packet_is_Error();

			if (errors>10) {
				errors=0;
				PRINTLN_CSTSTR("^$Resetting radio module, too many errors");
				LT.resetDevice();
				loraConfig();
				// to start over
				status_counter=0;
			}
		}
		
		FLUSHOUTPUT;
	
		/////////////////////////////////////////////////////////////////// 
		// LoRa reception       
	
		if (RXPacketL>0) {                 
		
#ifdef DEBUG_DOWNLINK_TIMING
		 	//this is the time (millis()) at which the packet has been completely received at the radio module
		 	PRINT_CSTSTR("^$rcv_time_ms:"); 		 
      PRINT_VALUE("%lu",rcv_time_ms);
      PRINTLN;
#endif
		
			RXpacketCount++;
			
			receivedFromLoRa=true;       

			//remove any downlink.txt file that may have been generated late and will therefore be out-dated
			remove("downlink/downlink.txt");

			PacketRSSI = LT.readPacketRSSI();              //read the recived RSSI value
			PacketSNR = LT.readPacketSNR();                //read the received SNR value

			sprintf(print_buf, "--- rxlora[%lu]. dst=%d type=0x%02X src=%d seq=%d", 
				RXpacketCount,
				LT.readRXDestination(),
				LT.readRXPacketType(), 
				LT.readRXSource(),
				LT.readRXSeqNo());
	 
			PRINT_STR("%s", print_buf);

			sprintf(print_buf, " len=%d SNR=%d RSSIpkt=%d BW=%d CR=4/%d SF=%d\n", 
				RXPacketL, 
				PacketSNR,
				PacketRSSI,
				LT.returnBandwidth()/1000,
#if defined SX126X || defined SX128X
				LT.getLoRaCodingRate()+4,
#else				
				LT.getLoRaCodingRate(),
#endif				
				LT.getLoRaSF());     

			PRINT_STR("%s", print_buf);              

			// provide a short output for external program to have information about the received packet
			// ^psrc_id,seq,len,SNR,RSSI
			sprintf(print_buf, "^p%d,%d,%d,%d,",
				LT.readRXDestination(),
				LT.readRXPacketType(), 
				LT.readRXSource(),
				LT.readRXSeqNo());
	 
			PRINT_STR("%s", print_buf);       

			sprintf(print_buf, "%d,%d,%d\n",
				RXPacketL, 
				PacketSNR,
				PacketRSSI);

			PRINT_STR("%s", print_buf); 

			// ^rbw,cr,sf,fq
			sprintf(print_buf, "^r%d,%d,%d,%ld\n", 
				LT.returnBandwidth()/1000,
#if defined SX126X || defined SX128X
				LT.getLoRaCodingRate()+4,
#else				
				LT.getLoRaCodingRate(),
#endif	
				LT.getLoRaSF(),
				(uint32_t)(DEFAULT_CHANNEL/1000.0));

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
	 
			for (int a=0; a<RXPacketL; a++) {
		
				if (optHEX) {
					if ((uint8_t)RXBUFFER[a]<16)
						PRINT_CSTSTR("0");
					PRINT_HEX("%X",	(uint8_t)RXBUFFER[a]);
					PRINT_CSTSTR(" ");	
				}
				else
					PRINT_STR("%c",(char)RXBUFFER[a]);
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
		uint8_t TXPacketL;
		
#ifdef DEBUG_DOWNLINK_TIMING
		//this is the time (millis()) at which we start waiting for checking downlink requests
		PRINT_CSTSTR("^$downlink wait: ");
		PRINTLN_VALUE("%lu", millis());
#endif
    		    	
		if (enableDownlinkCheck)  {	

			//wait until it is the (approximate) right time to check for downlink requests
			//because the generation of downlink.txt file takes some time
			//only for RX1, after that, we simply pass
			while (millis()-rcv_time_ms < DELAY_DNWFILE)
				;

#ifdef DEBUG_DOWNLINK_TIMING
			//this is the time (millis()) at which we start checking downlink requests
		PRINT_CSTSTR("^$downlink check: ");
		PRINTLN_VALUE("%lu", millis());
#endif
			//to set time_str 
			current_time_tmst=getGwDateTime();   	
	
			// use rapidjson to parse all lines
			// if there is a downlink.txt file then read all lines in memory for
			// further processing
			if (checkForLateDownlinkRX2)
				PRINT_CSTSTR("^$----delayRX2-----------------------------------------\n");
			else if (checkForLateDownlinkJACC1)	
				PRINT_CSTSTR("^$----retryJACC1---------------------------------------\n");    			
			else if (checkForLateDownlinkJACC2)	
				PRINT_CSTSTR("^$----retryJACC2---------------------------------------\n");
			else	
				PRINT_CSTSTR("^$-----------------------------------------------------\n");
				
			PRINT_CSTSTR("^$Check for downlink requests ");
			PRINT_STR("%s", time_str);
			PRINTLN_VALUE("%lu", current_time_tmst);
			
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

				PRINT_CSTSTR("^$Read downlink: ");
				PRINTLN_VALUE("%lu", dl_line_size);
							
				if (dl_line_size>0) {
					hasDownlinkEntry=true;
					//PRINT_CSTSTR("^$Downlink entry: ");
					//PRINTLN_STR("%d", downlink_json_entry);
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
				PRINT_CSTSTR("^$NO NEW DOWNLINK ENTRY\n");

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
		PRINT_CSTSTR("^$downlink process: ");
		PRINTLN_VALUE("%lu", millis());
#endif
        
    if (hasDownlinkEntry) {
			Document document;
		  
			bool is_lorawan_downlink=false;
		
			PRINT_CSTSTR("^$Process downlink request\n");
			//PRINT_CSTSTR("^$Process downlink requests: ");
			//PRINTLN_STR("%s", downlink_json_entry);
		
			document.Parse(downlink_json_entry);
					
			if (document.IsObject()) {
				if (document.HasMember("txpk"))								
					if (document["txpk"].HasMember("modu"))
						if (document["txpk"]["modu"].IsString())
							if (document["txpk"]["modu"]=="LORA") {
								is_lorawan_downlink=true;    
								//PRINT_CSTSTR("^$data = ");
								//PRINTLN_STR("%s", document["txpk"]["data"].GetString());
							}
						
				if (is_lorawan_downlink) {		
					PRINT_CSTSTR("^$LoRaWAN downlink request\n");				

					//use the tmst field provided by the Network Server in the downlink message
					uint32_t send_time_tmst=document["txpk"]["tmst"].GetUint();
					//to get the waiting delay
					uint32_t rx_wait_delay=send_time_tmst-rcv_time_tmst;

					//uncomment to test re-scheduling for RX2
					//delay(500);	
					
					bool go_for_downlink=false;
									
					// just in case
					if (send_time_tmst < rcv_time_tmst)
						PRINT_CSTSTR("^$WARNING: downlink tmst in the past, abort\n");	
					else 
					if (send_time_tmst - rcv_time_tmst > ((uint32_t)DELAY_JACC2+(uint32_t)DELAY_EXTDNW2)*1000)
						PRINT_CSTSTR("^$WARNING: downlink tmst too much in future, abort\n");	
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
									PRINT_CSTSTR("^$WARNING: too late for RX1, try RX2\n");	
								}
								else
									PRINT_CSTSTR("^$WARNING: too late to send downlink, abort\n");
							}
							else
								go_for_downlink=true;				
						}
								
					if (go_for_downlink) {												
						uint8_t downlink_payload[256];										
				
						//convert base64 data into binary buffer
						int downlink_size = b64_to_bin(document["txpk"]["data"].GetString(), document["txpk"]["data"].GetStringLength(), downlink_payload, sizeof downlink_payload);
				
						if (downlink_size != document["txpk"]["size"].GetInt()) {
							PRINT_CSTSTR("^$WARNING: mismatch between .size and .data size once converter to binary\n");
						}

#ifdef INVERTIQ_ON_TX
						PRINTLN_CSTSTR("^$invert IQ on TX");
						LT.invertIQ(true);
#endif								
						//uncomment to test for RX2
						//rx_wait_delay+=(uint32_t)DELAY_EXTDNW2*1000;

						bool useRX2=false;
				
						if (rx_wait_delay/1000 == (uint32_t)DELAY_DNW2 || rx_wait_delay/1000 == (uint32_t)DELAY_JACC2)
							useRX2=true;
				
						// should wait for RX2
						if (useRX2) {           

							PRINT_CSTSTR("^$Target RX2\n");
							//set frequency according to RX2 window
							LT.setRfFrequency(LORAWAN_D2FQ, Offset);
							//change to SF according to RX2 window
#if defined SX126X || defined SX127X
  						LT.setModulationParams(LORAWAN_D2SF, Bandwidth, CodeRate, Optimisation);
#endif
#ifdef SX128X
  						LT.setModulationParams(LORAWAN_D2SF, Bandwidth, CodeRate);
#endif											
						}
						else
							PRINT_CSTSTR("^$Target RX1\n");
					
						//wait until it is the (approximate) right time to send the downlink data
						//downlink data can use DELAY_DNW1 or DELAY_DNW2
						//join-request-accept can use DELAY_JACC1 or DELAY_JACC2
						while (millis()-rcv_time_ms < (rx_wait_delay/1000-MARGIN_DNW /* take a margin*/))
							;
			
						//	send the packet
#ifdef DEBUG_DOWNLINK_TIMING						
						PRINT_CSTSTR("^$downlink send: ");
						PRINTLN_VALUE("%lu", millis());						
#endif
						//LoRaWAN downlink so no header in communication lib								
						TXPacketL=LT.transmit((uint8_t*)downlink_payload, downlink_size, 10000, MAX_DBM, WAIT_TX);    

#ifdef INVERTIQ_ON_TX
						PRINTLN_CSTSTR("^$IQ back normal");
						LT.invertIQ(false);
#endif

						if (TXPacketL>0)
							PRINTLN_CSTSTR("^$Packet sent");
						else	
							PRINT_CSTSTR("^$Send error\n");

						if (useRX2) {					
							//set back to the reception frequency
							LT.setRfFrequency(DEFAULT_CHANNEL, Offset);
							PRINT_CSTSTR("^$Set back frequency\n");	
							//set back the SF
#if defined SX126X || defined SX127X
  						LT.setModulationParams(SpreadingFactor, Bandwidth, CodeRate, Optimisation);
#endif
#ifdef SX128X
  						LT.setModulationParams(SpreadingFactor, Bandwidth, CodeRate);
#endif
							PRINT_CSTSTR("^$Set back SF\n");
						}
					}
				} 
				else {			
					// non-LoRaWAN downlink		    
					if (document["dst"].IsInt()) {
						PRINT_CSTSTR("^$dst = ");
						PRINTLN_VALUE("%d", document["dst"].GetInt());
					}
							
					// check if it is a valid send request
					if (document["status"]=="send_request" && document["dst"].IsInt()) {
						
						//TODO CarrierSense?
						if (1) {
				
							uint8_t p_type=PKT_TYPE_DATA | PKT_FLAG_DATA_DOWNLINK;
						
							uint8_t l=document["data"].GetStringLength();
							uint8_t* pkt_payload=(uint8_t*)document["data"].GetString();
							uint8_t downlink_message[256];
						 
#ifdef INCLUDE_MIC_IN_DOWNLINK    				
							// we test if we have MIC data in the json entry
							if (document["MIC0"].IsString() && document["MIC0"]!="") {
						
								// indicate a downlink packet with a 4-byte MIC after the payload
								p_type = p_type | PKT_FLAG_DATA_ENCRYPTED;
					
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
							PRINT_CSTSTR("^$downlink send: ");
							PRINTLN_VALUE("%lu", millis());			
#endif
#ifdef INVERTIQ_ON_TX
						PRINTLN_CSTSTR("^$invert IQ on TX");
						LT.invertIQ(true);
#endif										
							TXPacketL=LT.transmitAddressed(pkt_payload, l, p_type, document["dst"].GetInt(), GW_ADDR, 10000, MAX_DBM, WAIT_TX);

#ifdef INVERTIQ_ON_TX
						PRINTLN_CSTSTR("^$IQ back normal");
						LT.invertIQ(false);
#endif
							
							if (TXPacketL>0)
								PRINT_CSTSTR("^$Packet sent\n");
							else	
								PRINT_CSTSTR("^$Send error\n");	
						}
						else {
							PRINT_CSTSTR("^$DELAYED: busy channel\n");	
							// here we will retry later because of a busy channel
						}
					}
					else {
						PRINT_CSTSTR("^$DISCARDING: not valid send_request\n");   	
					}
				}							

#ifdef KEEP_DOWNLINK_SENT_FILE

				if (document.HasMember("status")) {

					StringBuffer json_record_buffer;
					Writer<StringBuffer> writer(json_record_buffer);
						
					getGwDateTime();

					//transmission successful		
					if (TXPacketL>0)
						document["status"].SetString("sent", document.GetAllocator());
					else
						document["status"].SetString("sent_fail", document.GetAllocator());

					document.Accept(writer);
					PRINT_CSTSTR("^$JSON record: ");
					PRINTLN_STR("%s", json_record_buffer.GetString());

					FILE *fp = fopen("downlink/downlink-sent.txt","a");
		
					if (fp) {
						fprintf(fp, "%s %s\n", time_str, json_record_buffer.GetString());
						fclose(fp);
					}					
				}  		
#endif			

				if (downlink_json_entry)
					free(downlink_json_entry);
				
				PRINT_CSTSTR("^$-----------------------------------------------------\n");
			}
			else { 
				PRINT_CSTSTR("^$WARNING: not valid JSON format, abort\n");
			}			
			FLUSHOUTPUT;
		}	
		
		//remove any downlink.txt file that may have been generated late and will therefore be out-dated
		if (!checkForLateDownlinkRX2 && !checkForLateDownlinkJACC2)
			remove("downlink/downlink.txt");
#endif
	} 

#ifdef PERIODIC_RESET100	
	//test periodic reconfiguration
	if ( RXpacketCount!=0 && !(RXpacketCount % 100) && RXpacketCount!=RXpacketCountReset)
	{	
		PRINTLN_CSTSTR("^$****Periodic reset");
		RXpacketCountReset=RXpacketCount;
		LT.resetDevice();
		loraConfig();	
	}	
#endif	
}

int xtoi(const char *hexstring) {
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
           						// from 1 to 10 (see https://github.com/CongducPham/LowCostLoRaGw#annexa-lora-mode-and-predefined-channels)
           						// for SX128X, BW 125, 250 and 500 are replaced respectively by BW 203, 406 and 812
           						// keep loraMode == 11 == LoRaWAN for the moment
               break;
           case 'b' : optBW = atoi(optarg); 
                      // 7, 10, 15, 20, 31, 41, 62, 125, 250 or 500 for SX126X and SX127X_lora_gateway
                      // 125/200/203, 250/400/406, 500/800/812 or 1600/1625 for SX128X
               break;
           case 'c' : optCR = atoi(optarg);
                      // 5, 6, 7 or 8
               break;
           case 'd' : optSF = atoi(optarg);
                      // 5, 6, 7, 8, 9, 10, 11 or 12
               break;
           case 'e' : optRAW=true;
           						// no header
           						// required for LoRaWAN
               break;               
           case 'f' : optFQ=atof(optarg)*1000000.0;
                      // optFQ in MHz e.g. 868.1
               break;     
           case 'g' : optCH=atoi(optarg);
                      if (optCH < STARTING_CHANNEL || optCH > ENDING_CHANNEL)
                        optCH=STARTING_CHANNEL;
                      optCH=optCH-STARTING_CHANNEL;  
                      DEFAULT_CHANNEL=loraChannelArray[optCH]; 
                      // pre-defined channels (see https://github.com/CongducPham/LowCostLoRaGw#annexa-lora-mode-and-predefined-channels)
               break;      
           case 'h' : { 
           							uint8_t sw=atoi(optarg);
                      	// assume that sw is expressed in hex value
                      	optSW = (sw / 10)*16 + (sw % 10);
                      	// optSW is no longer used, it is not taken into account anymore
                      	// we only differentiate between LoRaWAN_mode and non-LoRaWAN mode
                      }
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
