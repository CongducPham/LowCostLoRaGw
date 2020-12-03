/* 
 *  LoRa interactive device to receive and send command
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
 * 
 *  Version:                2.0
 *  Design:                 C. Pham
 *  Implementation:         C. Pham
 *
 *  waits for command or data on serial port or from the LoRa module
 *    - command starts with /@ and ends with #
 *    - you can therefore send remote command to another interactive device to control it
 *    
 *  LoRa parameters
 *    - /@M1#: set LoRa mode 1
 *    - /@C12#: use channel 12 (in 868 band this is defined as 865.8MHz)
 *    - /@F433175#: set frequency to 433175000Hz
 *    - /@SF8#: set SF to 8
 *    - /@BW125#: set BW to 125kHz (use 125, 250 or 500 for SX126X and SX127X; and 200, 400, 800 or 1600 for SX128X)
 *    - /@DBM10#: set power output to 10dBm. Check #define PABOOST for appropriate compilation
 *    - /@A9#: set node addr to 9
 *    - /@ON# or /@OFF#: power on/off the LoRa module, /@ON# can be used to reset the radio module
 *
 *  Use of ACK
 *    - /@ACK#hello w/ack : sends the message and request an ACK
 *    - /@ACKON# enables ACK (for all messages)
 *    - /@ACKOFF# disables ACK
 * 
 *  CAD, DIFS/SIFS mechanism, RSSI checking, extended IFS
 *    - /@CS0#, /@CS1# or /@CS2# or /@CS3# choose Carrier Sense method  
 *    - /@CAD# performs an SIFS CAD, i.e. 3 or 6 CAD depending on the LoRa mode
 *    - /@CAD*# launch continous CAD mode, exit with return in Serial Monitor input
 *    - /@CADON3# uses 3 CAD when sending data (normally SIFS is 3 or 6 CAD, DIFS=3SIFS)
 *    - /@CADOFF# disables CAD (IFS) when sending data
 *    - /@EIFS# toggles for extended IFS wait
 *    
 *    - can read from serial port to send input data (ASCII format)
 *    - remote configuration needs to be allowed by unlocking the gateway with command '/@U' with an unlock pin
 *      - '/@U1234#'
 *    - accepts all the command set of a receive gateway
 *    - periodic sending of packet for range test
 *      - /@T5000#: send a message at regular time interval of 5000ms. Use /@T0# to disable periodic sending
 *      - /@TR5000#: send a message at random time interval between [2000, 5000]ms.
 *      - /@Z200# sets the packet payload size to 200
 *    - the command /@D specifies the destination node for sending remote commands
 *      - /@D56#: set the destination node to be 56, this is permanent, until the next D command
 *      - /@D58#hello: send hello to node 56, destination addr is only for this message
 *      - /@D4#/@SF10#: send the command string /@SF10# to node 4
 *    - the S command sends a string of arbitrary size
 *      - /@S50# sends a 50B user payload packet filled with '#'. The real size is 54B with the 4-byte header
 *    - add LoRaWAN-like AES encyption, with MIC which provide reliable bad data rejection mechanisms
 *      - /@AES# toggles AES ON and OFF
 *      - /@LW# toggles LoRaWAN mode. If LoRaWAN mode, set AES ON and pure LoRaWAN format, without underlying data header
 *      - if you want to use raw mode (without underlying data header), use /@LW# then /@AES# to disable AES while keeping raw mode 
 *      
 *      
 *   IMPORTANT NOTICE   
 *    - for more details on the our underlying packet format
 *      - see http://cpham.perso.univ-pau.fr/LORA/RPIgateway.html
 *      
*/

/*  Change logs
 *  Nov, 10th, 2020. v2.0 
 *        IMPORTANT. The low-level LoRa communication lib has moved from Libelium-based lib to Stuart Robinson's SX12XX lib.
 *          - https://github.com/StuartsProjects/SX12XX-LoRa
 *          - SX126X, SX127X, and SX128X related files have been ported to run on both Arduino & RaspberryPI environment
 *         - for more details see https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-SX12XX.md
 *  Feb, 6th, 2020. v1.9 
 *        Continous CAD mode can be started interactively. Use /@CAD*# command. Press return to exit continous CAD mode
 *        Add /@F command to set frequency: /@F43317500# for instance for 433.175MHz
 *  Feb, 1st, 2019. v1.8a 
 *        Add support of a small OLED screen in both CAD_TEST and PERIODIC_SENDER mode
 *  June, 29th, 2017. v1.8
 *        Add CarrierSense selection method to perform tests
 *          - CarrierSense(0) does nothing -> pure ALOHA
 *          - CarrierSense(1) is the inital proposed carrier sense mechanism described in TOS LAS paper
 *          - CarrierSense(2) is an alternative carrier sense mechanism derived from 802.11
 *          - CarrierSense(3) is the proposed carrier sense mechanism for cohabitation with long message
 *          - /@CS0#, /@CS1# or /@CS2# or /@CS3# can dynamically choose between those
 *  May, 8th, 2017. v1.7 
 *        Improve AES support
 *        AppKey can now also be used with AES encryption, compile with WITH_APPKEY
 *  Dec, 7th, 2016. v1.6
 *        Improve CAD_TEST mode
 *          - will continously perform CAD, then will notify when channel activity has been detected.
 *        Add an automatic periodic sender mode. Uncomment PERIODIC_SENDER. Change value of PERIODIC_SENDER to sending period in ms  
 *  Nov, 16th, 2016. v1.5
 *        Add /@DBM command to set output power in dbm. 0 < dbm < 15
 *  Oct, 21st, 2016. v1.4S
 *        Split the lora_gateway sketch into 2 parts:   
 *          - lora_gateway: for gateway, similar to previous IS_RCV_GATEWAY
 *          - lora_interactivedevice, similar to previous IS_SEND_GATEWAY   
 *        Add support for 4 channels in the 433MHz band. Default is CH_00_433 = 0x6C5333 for 433.3MHz. See SX1272.h.
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
 
#include <SPI.h>
//this is the standard behaviour of library, use SPI Transaction switching
#define USE_SPI_TRANSACTION         
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// please uncomment only 1 choice 
//#define SX126X
//#define SX127X
#define SX128X
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SX126X
#include <SX126XLT.h>                                          
#include "SX126X_RadioSettings.h"
#endif

#ifdef SX127X
#include <SX127XLT.h>                                          
#include "SX127X_RadioSettings.h"
#endif

#ifdef SX128X
#include <SX128XLT.h>                                          
#include "SX128X_RadioSettings.h"
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

///////////////////////////////////////////////////////////////////
// CHANGE HERE FOR VARIOUS BEHAVIORS
////////////////////////////
//#define SHOW_FREEMEMORY
////////////////////////////
//when uncommenting PERIODIC_SENDER, the device only does continous periodic sending
#define PERIODIC_SENDER 8000
////////////////////////////
//start in continous Cad mode
//#define CAD_TEST
////////////////////////////
//this will enable AES & LoRaWAN packet format
//#define WITH_AES
////////////////////////////
//this will enable a receive window to act as a simple gateway
//#define WITH_RCVW
////////////////////////////
#define SEND_LED 9
////////////////////////////
//normal behavior is to invert IQ for RX, the normal behavior at gateway is also to invert its IQ setting, only valid with WITH_RCVW
#define INVERTIQ_ON_RX
///////////////////////////////////////////////////////////////////
//new small OLED screen, mostly based on SSD1306 
//#define OLED
//#define OLED_GND234
#define OLED_9GND876
//#define OLED_7GND654
//#define OLED_GND13_12_11
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// CHANGE HERE THE NODE ADDRESS 
uint8_t node_addr=6;
//////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// CHANGE HERE THE APPKEY, BUT IF GW CHECKS FOR APPKEY, MUST BE
// IN THE APPKEY LIST MAINTAINED BY GW.
uint8_t my_appKey[4]={5, 6, 7, 8};
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// default packet size for periodic sending
uint8_t MSS=220;
//////////////////////////////////////////////////////////////////

#ifdef WITH_AES
///////////////////////////////////////////////////////////////////
// ENCRYPTION CONFIGURATION AND KEYS FOR LORAWAN
#include "local_lorawan.h"

//ENTER HERE your Device Address from the TTN device info (same order, i.e. msb). Example for 0x12345678
//unsigned char DevAddr[4] = { 0x12, 0x34, 0x56, 0x78 };
///////////////////////////////////////////////////////////////////

//Danang
//unsigned char DevAddr[4] = { 0x26, 0x04, 0x1F, 0x24 };

//Pau
unsigned char DevAddr[4] = { 0x26, 0x01, 0x17, 0x21 };
#endif

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

/*****************************
 _____           _      
/  __ \         | |     
| /  \/ ___   __| | ___ 
| |    / _ \ / _` |/ _ \
| \__/\ (_) | (_| |  __/
 \____/\___/ \__,_|\___|
*****************************/ 

///////////////////////////////////////////////////////////////////
// TRANSMIT BUFFER
uint8_t message[255];
///////////////////////////////////////////////////////////////////

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

//create a library class instance called LT
//to handle LoRa radio communications

#ifdef SX126X
SX126XLT LT;
#endif

#ifdef SX127X
SX127XLT LT;
#endif

#ifdef SX128X
SX128XLT LT;
#endif

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

// to unlock remote configuration feature
#define UNLOCK_PIN 1234

///////////////////////////////////////////////////////////////////
// VARIABLES
//

// number of retries to unlock remote configuration feature
uint8_t unlocked_try=3;
boolean unlocked=true;
boolean receivedFromSerial=false;
boolean receivedFromLoRa=false;
boolean withAck=false;
boolean withAes=false;
boolean withAppKey=false;
boolean fullLorawan=false;
boolean radioOn=true;
boolean runLocalCommand=true;

//input command buffer
char cmd[260]="**********";
char sprintf_buf[80];

uint8_t savedSpreadingFactor=SpreadingFactor;
uint8_t savedBandwidth=Bandwidth;
uint8_t loraMode=1;
uint32_t my_frequency;
uint8_t dest_addr=DEFAULT_DEST_ADDR;
uint8_t carrier_sense_method=1;
bool extendedIFS=false;
// set to 0 to disable carrier sense based on CAD
uint8_t cad_number=3;
uint8_t SIFS_cad_number=cad_number*3;
uint32_t msg_sn=0;

unsigned int inter_pkt_time=0;
unsigned int random_inter_pkt_time=0;
unsigned long next_periodic_sendtime=0L;

uint8_t TXPacketL;
uint32_t TXPacketCount, startmS, endmS;

///////////////////////////////////////////////////////////////////

#if defined ARDUINO && defined SHOW_FREEMEMORY && not defined __MK20DX256__ && not defined __MKL26Z64__ && not defined  __SAMD21G18A__ && not defined ARDUINO_SAM_DUE
int freeMemory () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
#endif

long getCmdValue(int &i, char* strBuff=NULL) {
        
        char seqStr[10]="******";

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
/*******************************************************************************************************
  Based from SX12XX example - Stuart Robinson 
*******************************************************************************************************/

  //The function call list below shows the complete setup for the LoRa device using the
  // information defined in the Settings.h file.
  //The 'Setup LoRa device' list below can be replaced with a single function call;
  //LT.setupLoRa(Frequency, Offset, SpreadingFactor, Bandwidth, CodeRate, Optimisation);

  //***************************************************************************************************
  //Setup LoRa device
  //***************************************************************************************************
  //got to standby mode to configure device
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
  LT.setRfFrequency(my_frequency, Offset);
//run calibration after setting frequency
#ifdef SX127X
  LT.calibrateImage(0);
#endif
  //set LoRa modem parameters
#if defined SX126X || defined SX127X
  LT.setModulationParams(savedSpreadingFactor, savedBandwidth, CodeRate, Optimisation);
#endif
#ifdef SX128X
  LT.setModulationParams(savedSpreadingFactor, savedBandwidth, CodeRate);
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
#if defined SX126X || defined SX127X 
  //syncword, LORA_MAC_PRIVATE_SYNCWORD = 0x12, or LORA_MAC_PUBLIC_SYNCWORD = 0x34 
  LT.setSyncWord(LORA_MAC_PRIVATE_SYNCWORD);
  //set for highest sensitivity at expense of slightly higher LNA current
  LT.setHighSensitivity();  //set for maximum gain
#endif
#ifdef SX126X
  //set for IRQ on TX done and timeout on DIO1
  LT.setDioIrqParams(IRQ_RADIO_ALL, (IRQ_TX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);
#endif
#ifdef SX127X
  //set for IRQ on RX done
  LT.setDioIrqParams(IRQ_RADIO_ALL, IRQ_TX_DONE, 0, 0);
  LT.setPA_BOOST(PA_BOOST);
  PRINTLN_CSTSTR("Use PA_BOOST amplifier line");  
#endif
#ifdef SX128X
  LT.setDioIrqParams(IRQ_RADIO_ALL, (IRQ_TX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);
#endif   

  if (IQ_Setting==LORA_IQ_INVERTED) {
    LT.invertIQ(true);
    PRINT_CSTSTR("Invert I/Q on RX\n");
  }
  else {
    LT.invertIQ(false);
    PRINT_CSTSTR("Normal I/Q\n");
  }  
  //***************************************************************************************************  

  PRINT_CSTSTR("Set LoRa txpower dBm to ");
  PRINTLN_VALUE("%d", MAX_DBM); 
  
#ifdef SX127X   
  LT.setTxParams(MAX_DBM, RADIO_RAMP_DEFAULT);            
#endif
#if defined SX126X || defined SX128X
  LT.setTxParams(MAX_DBM, RAMP_TIME);
#endif

  PRINTLN;
  //reads and prints the configured LoRa settings, useful check
  LT.printModemSettings();                               
  PRINTLN;
  //reads and prints the configured operating settings, useful check
  LT.printOperatingSettings();                           
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
}

void ContinousCad() 
{
  bool CadDetected=false;
  unsigned long firstDetected, lastDetected=0;
  int8_t e;
  unsigned long startDoCad, endDoCad;

  PRINT_CSTSTR("Start CAD test mode\n");
  PRINT_CSTSTR("SF: ");
  PRINT_VALUE("%d", LT.getLoRaSF());
  PRINTLN;
  
#ifdef OLED
  u8x8.clearDisplay();  
  u8x8.drawString(0, 0, "CAD TEST");
  sprintf(oled_msg,"LoRa mode %d", loraMode );
  u8x8.drawString(0, 1, oled_msg); 
#endif 
  
  while (!Serial.available()) {
      
      startDoCad=millis();
      // for energy testing, use 100  
      e = LT.doCAD(1);
      endDoCad=millis();
      
      //e<0 means we detected activity and no activity was detected before
      if (e<0 && !CadDetected) {      
        PRINT_CSTSTR("###########################\n");
        PRINT_CSTSTR("no activity for (ms): ");
        PRINT_VALUE("%ld", endDoCad-lastDetected);  
        PRINTLN;
        PRINT_CSTSTR("SF: ");
        PRINT_VALUE("%d", LT.getLoRaSF());
        PRINTLN;        
        PRINT_CSTSTR("###########################\n");                
        firstDetected=endDoCad;
        CadDetected=true;
#ifdef OLED
        u8x8.clearLine(0);
        sprintf(oled_msg,"#### SF%d ###", LT.getLoRaSF());  
        u8x8.drawString(0, 0, oled_msg);
        u8x8.clearLine(1);
        sprintf(oled_msg,"%ld", endDoCad-lastDetected);
        u8x8.drawString(0, 1, oled_msg);        
        u8x8.drawString(0, 2, "#############");  
#endif            
      }

      //we detected but now CAD returns no activity
      if (e==0 && CadDetected)  {     
        PRINT_CSTSTR("+++++++++++++++++++++++++++\n");
        PRINT_CSTSTR("time (ms): ");
        PRINT_VALUE("%ld", endDoCad);
        PRINTLN;
        PRINT_CSTSTR("duration (ms): ");
        PRINT_VALUE("%ld", endDoCad-firstDetected);  
        PRINTLN;
        lastDetected=endDoCad;        
        CadDetected=false;
#ifdef OLED
        u8x8.clearLine(6);
        sprintf(oled_msg,"time %ld", endDoCad);
        u8x8.drawString(0, 6, oled_msg);
        u8x8.clearLine(7);        
        sprintf(oled_msg,"duration %d", endDoCad-firstDetected);
        u8x8.drawString(0, 7, oled_msg);
#endif          
      }       

      //we detected activity, and had detected activity before  
      if (e<0) {
        PRINT_VALUE("%ld", endDoCad);
        PRINT_CSTSTR(" + ");
        PRINT_VALUE("%d", e);
        PRINT_CSTSTR(" ");
        PRINT_VALUE("%ld", endDoCad-startDoCad);
        PRINTLN;
#ifdef OLED
        u8x8.clearLine(3);  
        sprintf(oled_msg,"%ld", endDoCad);
        u8x8.drawString(0, 3, oled_msg);
        
        u8x8.clearLine(4);
        sprintf(oled_msg,"%d", e);
        u8x8.drawString(0, 4, oled_msg);        
        
        u8x8.clearLine(5);
        sprintf(oled_msg,"%d", endDoCad-startDoCad);
        u8x8.drawString(0, 5, oled_msg);         
#endif          
      }

      // for energy testing, use 12000
      //delay(12000);
      // you can also use 100ms to test CAD frequency impact
      delay(100);      
  }

  FLUSHOUTPUT;
  
  PRINT_CSTSTR("Exit CAD test mode\n");
#ifdef PERIODIC_SENDER
  inter_pkt_time=PERIODIC_SENDER;
  PRINT_CSTSTR("Periodic sender ON");
  PRINTLN;
#ifdef OLED
  u8x8.clearDisplay();
  u8x8.drawString(0, 0, "PERIODIC SEND");
  sprintf(oled_msg,"SF%dBW%ld", LT.getLoRaSF(), LT.returnBandwidth()/1000);
  u8x8.drawString(0, 1, oled_msg);  
  sprintf(oled_msg,"period=%d", inter_pkt_time);
  u8x8.drawString(0, 2, oled_msg);  
#endif    
#endif   
}

void setup()
{
  int e;
  delay(3000);
  //randomSeed(analogRead(14));

  // Open serial communications and wait for port to open:
#if defined __SAMD21G18A__ && not defined ARDUINO_SAMD_FEATHER_M0
  SerialUSB.begin(38400);
#else
  Serial.begin(38400);  
#endif  
  // Print a start message
  PRINT_CSTSTR("LoRa interactive device\n");

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
  digitalWrite(9, LOW);
#endif
#endif
#endif

#ifdef SEND_LED
  pinMode(SEND_LED, OUTPUT);
  digitalWrite(SEND_LED, LOW);
#endif  

#ifdef WITH_RCVW
  PRINT_CSTSTR("With received window\n");
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

  //start SPI bus communication
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
    PRINT_CSTSTR("LoRa Device found\n");                                  
    delay(1000);
  }
  else
  {
    PRINT_CSTSTR("No device responding\n");
    while (1){ }
  }

  my_frequency=DEFAULT_CHANNEL;

  savedSpreadingFactor=SpreadingFactor;
  savedBandwidth=Bandwidth;
  
  loraConfig();          

  LT.setDevAddr(node_addr);
  PRINT_CSTSTR("node addr: ");
  PRINT_VALUE("%d", node_addr);
  PRINTLN;               

#ifdef OLED
  u8x8.begin();
  //u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.setFont(u8x8_font_pxplustandynewtv_r);
  u8x8.drawString(0, 0, "Simple IntDev");
  sprintf(oled_msg,"SF%dBW%d", LT.getLoRaSF(), LT.returnBandwidth()/1000);
  u8x8.drawString(0, 1, oled_msg); 
#endif 

#ifdef PERIODIC_SENDER
  inter_pkt_time=PERIODIC_SENDER;
  PRINT_CSTSTR("Periodic sender ON");
  PRINTLN;
#ifdef OLED  
  u8x8.drawString(0, 0, "PERIODIC SEND");
  sprintf(oled_msg,"SF%dBW%ld", LT.getLoRaSF(), LT.returnBandwidth()/1000);
  u8x8.drawString(0, 1, oled_msg);  
  sprintf(oled_msg,"period=%d", inter_pkt_time);
  u8x8.drawString(0, 2, oled_msg);  
#endif    
#endif

#ifdef SX126X
  PRINT_CSTSTR("SX126X");
#endif
#ifdef SX127X
  PRINT_CSTSTR("SX127X");
#endif
#ifdef SX128X
  PRINT_CSTSTR("SX128X");
#endif 

  // Print a success message
  PRINT_CSTSTR(" successfully configured ");
  
#ifdef CAD_TEST
  PRINT_CSTSTR(" as CAD device - ready");
  PRINTLN;
  
  ContinousCad();
  // flush as we do not want to send data
  while (Serial.available())
    Serial.read();  
#else
  PRINT_CSTSTR(" as device. Waiting serial input for serial-RF bridge\n");
#endif
}

void packet_is_OK()
{
  uint16_t localCRC;

  PRINT_CSTSTR("  BytesSent,");
  PRINT_VALUE("%d", TXPacketL);                             //print transmitted packet length
  localCRC = LT.CRCCCITT(message, TXPacketL, 0xFFFF);
  PRINT_CSTSTR("  CRC,");
  PRINT_HEX("%X",localCRC);                         //print CRC of transmitted packet
  PRINT_CSTSTR("  TransmitTime,");
  PRINT_VALUE("%ld", endmS - startmS);                       //print transmit time of packet
  PRINT_CSTSTR("mS");
  PRINT_CSTSTR("  PacketsSent,");
  PRINT_VALUE("%d", TXPacketCount);                         //print total of packets sent OK
  PRINTLN;
  FLUSHOUTPUT;
#ifdef OLED
  u8x8.clearLine(5);          
  u8x8.clearLine(7);
  sprintf(oled_msg,"sent ok");
  u8x8.drawString(0, 7, oled_msg);
#endif   
}


void packet_is_Error()
{
  uint16_t IRQStatus;
  IRQStatus = LT.readIrqStatus();                   //read the LoRa device IRQ status register

  PRINT_CSTSTR(" SendError");
  PRINT_CSTSTR("Length,");
  PRINT_VALUE("%d",TXPacketL);               //get the device packet length
  PRINT_CSTSTR(",IRQreg,");
  PRINT_HEX("%X",IRQStatus);
  LT.printIrqStatus();                            //print the names of the IRQ registers set
  PRINTLN;
  FLUSHOUTPUT;
#ifdef OLED
  u8x8.clearLine(5);          
  u8x8.clearLine(7);
  sprintf(oled_msg,"sent error");
  u8x8.drawString(0, 7, oled_msg);
#endif  
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
  int i=0;
  long cmdValue;
  uint8_t app_key_offset=0;

/////////////////////////////////////////////////////////////////// 
// START OF PERIODIC TASKS

  receivedFromSerial=false;
  receivedFromLoRa=false;

  // check if we received data from the input serial port
  if (Serial.available()) {

    i=0;  

    while (Serial.available() && i<80) {
      cmd[i]=Serial.read();
      i++;
      delay(50);
    }
    
    cmd[i]='\0';

    PRINT_CSTSTR("Rcv serial: ");
    PRINT_STR("%s",cmd);
    PRINTLN;
    
    receivedFromSerial=true; 
  }

/////////////////////////////////////////////////////////////////// 
// THE MAIN LOOP FOR AN INTERACTIVE END-DEVICE
//
  if (radioOn && !receivedFromSerial) {
		// periodic message sending?
		// periodic message are sent in non-LoRaWAN mode
		if (inter_pkt_time) {
			if (millis() > next_periodic_sendtime) {
				
				PRINT_CSTSTR("time is ");
				PRINT_VALUE("%ld",millis());  
				PRINTLN;
				
				sprintf((char*)message, "msg%3d***", msg_sn++);
				for (i=strlen((char*)message); i<MSS; i++)
					message[i]='*';
				
				message[i]='\0';
				
				PRINT_CSTSTR("Sending : ");
				PRINT_STR("%s", (char*)message);  
				PRINTLN;

				PRINT_CSTSTR("Packet sequence number ");
				PRINT_VALUE("%d",LT.readTXSeqNo());
				PRINTLN;

				PRINT_CSTSTR("Payload size is ");  
				PRINT_VALUE("%d",strlen((char*)message));
				PRINTLN;
#ifdef OLED
				u8x8.clearLine(5);
				u8x8.clearLine(6);
				u8x8.clearLine(7);
				sprintf(oled_msg,"Sending %d Bytes", strlen((char*)message));
				u8x8.drawString(0, 5, oled_msg);
#endif            
				uint32_t toa = LT.getToA(strlen((char*)message)+(fullLorawan?0:HEADER_SIZE));      
				PRINT_CSTSTR("ToA is ");
				if (!fullLorawan)
					PRINT_CSTSTR("w/4B header ");
				PRINT_VALUE("%d",toa);
				PRINTLN; 
#ifdef OLED
				sprintf(oled_msg,"ToA : %ld", toa);
				u8x8.drawString(0, 6, oled_msg);
#endif               
									
				startmS=millis();

      	LT.CarrierSense(carrier_sense_method, extendedIFS);            

        uint8_t p_type=PKT_TYPE_DATA;
        
        if (withAck)
          p_type=p_type | PKT_FLAG_ACK_REQ;

#ifdef SEND_LED
        digitalWrite(SEND_LED, HIGH);
#endif          
        //txpower can be set to -1 if we want to keep the same power setting PROVIDED THAT setTxParams() has been called before
        TXPacketL=LT.transmitAddressed(message, strlen((char*)message), p_type, dest_addr, LT.readDevAddr(), 10000, -1, WAIT_TX);      

#ifdef SEND_LED
        digitalWrite(SEND_LED, LOW);
#endif 
				if (TXPacketL) 
				{
					endmS = millis();                                          
					TXPacketCount++;
					packet_is_OK();
				}
				else
				{
					packet_is_Error();                                 
				}       
				
				if (random_inter_pkt_time) {                
					random_inter_pkt_time=random(2000,inter_pkt_time);

					next_periodic_sendtime = millis() + random_inter_pkt_time;
				}
				else
					next_periodic_sendtime = millis() + inter_pkt_time;
					
				PRINT_CSTSTR("next at ");
				PRINT_VALUE("%ld", next_periodic_sendtime);
				PRINTLN;     
			}  
		}
#ifdef WITH_RCVW        
    // open a receive window

    uint8_t RXPacketL = LT.receiveAddressed(cmd, 255, 2000, WAIT_RX);

    if (RXPacketL) {
      
      cmd[RXPacketL]='\0';
  
      PRINT_CSTSTR("--> Rcv LoRa: ");
      PRINT_STR("%s",cmd);
      PRINTLN;
  
      PRINT_CSTSTR("From: ");
      PRINT_VALUE("%d",LT.readRXSource());
  
      PRINT_CSTSTR("  For: ");
      PRINT_VALUE("%d",LT.readRXDestination());
  
      if (LT.readRXDestination()==node_addr) {
        PRINT_CSTSTR(" - Packet is for me");
        receivedFromLoRa=true;      
      }
      else
        PRINT_CSTSTR(" - Packet is NOT for me");
        
      PRINTLN;   
    }
#endif  
	}

/////////////////////////////////////////////////////////////////// 
// PROCESS INCOMING DATA
//  
  if (receivedFromSerial || receivedFromLoRa || runLocalCommand) {
    
    boolean sendCmd=false;
    boolean withTmpAck=false;
    int forTmpDestAddr=-1;

    if (runLocalCommand)
      runLocalCommand=false;

    i=0;
    
    if (cmd[i]=='/' && cmd[i+1]=='@') {

      PRINT_CSTSTR("Parsing command\n");      
      i=i+2;
      
      PRINT_STR("%s",cmd);
      PRINTLN;
      
      if ( (receivedFromLoRa && cmd[i]!='U' && !unlocked) || !unlocked_try) {
        PRINT_CSTSTR("Remote config locked\n");
        // just assign an unknown command
        cmd[i]='*';  
      }
      
      switch (cmd[i]) {
	
				case 'D': 
					if (cmd[i+1]=='B' && cmd[i+2]=='M') {
						i=i+3;
						cmdValue=getCmdValue(i);

						if (cmdValue > 0 && cmdValue < 15) {
							PRINT_CSTSTR("Set power dBm: ");
							PRINT_VALUE("%d",cmdValue);  
							PRINTLN;                    
							// Set power dBm
#ifdef SX127X 	
							LT.setTxParams(MAX_DBM, RADIO_RAMP_DEFAULT);            
#endif
#if defined SX126X || defined SX128X
							LT.setTxParams(MAX_DBM, RAMP_TIME);
#endif   
						}                                
					}
					else {
						i++;
						cmdValue=getCmdValue(i);
						
						i++;
						// cannot set dest addr greater than 255
						if (cmdValue > 255)
							cmdValue = 255;
						// cannot set dest addr lower than 0, 0 is broadcast
						if (cmdValue < 0)
							cmdValue = 0;
										
						// only a D command        
						if (i==strlen(cmd)) {
							// set dest addr permanently       
							dest_addr=cmdValue; 
							PRINT_CSTSTR("Set permanent dest addr to ");
							PRINT_VALUE("%d",dest_addr); 
							PRINTLN;                
						}
						else {
							// only for the following ASCII command
							forTmpDestAddr=cmdValue;
							PRINT_CSTSTR("Set dest addr FOR THIS ASCII STRING to ");
							PRINT_VALUE("%d",forTmpDestAddr);
							PRINTLN;    
							sendCmd=true;           
						}
					}
				break;

				case 'T': 
					i++;
					
					if (cmd[i]=='R') {
						random_inter_pkt_time=1;
						i++;
					}
					else
						random_inter_pkt_time=0;
					
					cmdValue=getCmdValue(i);
					
					inter_pkt_time=cmdValue;
					
					if (inter_pkt_time) {
						PRINT_CSTSTR("Set inter-packet time to ");
						PRINT_VALUE("%ld",inter_pkt_time);
						PRINTLN;   
						next_periodic_sendtime=millis()+inter_pkt_time;
					}
					else {
						PRINT_CSTSTR("Disable periodic send\n");                  
					}         

					if (random_inter_pkt_time) {  
						random_inter_pkt_time=random(2000,inter_pkt_time); 
						next_periodic_sendtime=millis()+random_inter_pkt_time;  
					}          
																			
				break;        
				
				// set the pkt size default is 40
				// "Z250#"
				case 'Z':    
					i++;
					cmdValue=getCmdValue(i);
					// cannot set pkt size greater than MAX_PKT_SIZE
					if (cmdValue > 250)
						cmdValue = 250;
					// cannot set pkt size smaller than MAX_PKT_SIZE
					if (cmdValue < 10)
						cmdValue = 10;
					// set new pkt size        
					MSS=cmdValue; 
					
					PRINT_CSTSTR("Set MSS to ");
					PRINT_VALUE("%d",MSS);
					PRINTLN;              
				break;                   

				case 'E':          
					if (cmd[i+1]=='I' && cmd[i+2]=='F' && cmd[i+3]=='S') {
						
						extendedIFS = !extendedIFS;
						
						if (extendedIFS)
							PRINT_CSTSTR("EIFS ON\n");
						else
							PRINT_CSTSTR("EIFS OFF\n");
					}
				break;  

				case 'U':
					if (unlocked_try) {
						i++;
						cmdValue=getCmdValue(i);

						if (cmdValue==UNLOCK_PIN) {
							
							unlocked=!unlocked;
							
							if (unlocked)
								PRINT_CSTSTR("Unlocked\n");
							else
								PRINT_CSTSTR("Locked\n");
						}
						else
							unlocked_try--;
							
						if (unlocked_try==0)
							PRINT_CSTSTR("Bad pin\n");
					}
				break;

				case 'S': 
					if (cmd[i+1]=='F') {
						i=i+2;
						cmdValue=getCmdValue(i);
								 
#ifdef SX126X
						if (cmdValue > 4 && cmdValue < 13) {
							LT.setModulationParams(cmdValue, Bandwidth, CodeRate, Optimisation);
              savedSpreadingFactor=cmdValue;
							PRINT_CSTSTR("Set SF to ");
							PRINT_VALUE("%d",cmdValue);  
							PRINTLN;    
						}							
#endif
#ifdef SX127X
						if (cmdValue > 5 && cmdValue < 13) {
							LT.setModulationParams(cmdValue, Bandwidth, CodeRate, Optimisation);
              savedSpreadingFactor=cmdValue;              		
							PRINT_CSTSTR("Set SF to ");
							PRINT_VALUE("%d",cmdValue);  
							PRINTLN;    
						}												
#endif
#ifdef SX128X
						if (cmdValue > 4 && cmdValue < 13) {
							LT.setModulationParams((cmdValue<<4), Bandwidth, CodeRate);
              savedSpreadingFactor=(cmdValue<<4);
							PRINT_CSTSTR("Set SF to ");
							PRINT_VALUE("%d",cmdValue);  
							PRINTLN;    
						}							
#endif
					}
					else {            
						i++;
						cmdValue=getCmdValue(i);
						i++;
						if (cmdValue>250) {
							PRINT_CSTSTR("No more than 250B\n");
						}
						else {
							int k=0;
							// fill the message to be sent
							for (k=0; k<cmdValue; k++)
								cmd[k+i]='#';
							cmd[k+i]='\0';  
							sendCmd=true; 
						}
					}                    
				break;  

        case 'B': 
          if (cmd[i+1]=='W') {
            i=i+2;
            cmdValue=getCmdValue(i);

            uint8_t tbw;
                 
#if defined SX126X || defined SX127X
            if (cmdValue==125)
              tbw = LORA_BW_125;
            else if (cmdValue==250)
              tbw = LORA_BW_250;
            else if (cmdValue==500)
              tbw = LORA_BW_500;
            else {
              PRINT_CSTSTR("not supported BW value");
              PRINTLN;
              break;  
            }
                                          
            LT.setModulationParams(SpreadingFactor, tbw, CodeRate, Optimisation);
            savedBandwidth=tbw;
            PRINT_CSTSTR("Set BW to ");
            PRINT_VALUE("%d",cmdValue);  
            PRINTLN;                 
#endif
#ifdef SX128X
            if (cmdValue==200 || cmdValue==203)
              tbw = LORA_BW_0200;
            else if (cmdValue==400 || cmdValue==406)
              tbw = LORA_BW_0400;
            else if (cmdValue==800 || cmdValue==812)
              tbw = LORA_BW_0800;
            else if (cmdValue==1600 || cmdValue==1625)
              tbw = LORA_BW_1600;              
            else {
              PRINT_CSTSTR("not supported BW value");
              PRINTLN;
              break;  
            }         

            LT.setModulationParams(SpreadingFactor, tbw, CodeRate);
            savedBandwidth=tbw; 
            PRINT_CSTSTR("Set BW to ");
            PRINT_VALUE("%d", cmdValue); 
            PRINTLN;            
#endif
          }
        break;
                  
				case 'M':
					i++;
					cmdValue=getCmdValue(i);
					// cannot set mode greater than 10 (11 being the LoRaWAN mode is triggered by command /@LW#)
					if (cmdValue > 10)
						cmdValue = 1;
					// cannot set mode lower than 0
					if (cmdValue < 0)
						cmdValue = 1;

					loraMode=cmdValue; 
					PRINT_CSTSTR("Set LoRa mode to ");
					PRINT_VALUE("%d",loraMode);
					PRINTLN;

          PRINT_CSTSTR("Keep current frequency\n");
          
#if defined SX126X || defined SX127X
					LT.setSyncWord(LORA_MAC_PRIVATE_SYNCWORD);
          LT.setRfFrequency(my_frequency, Offset);					
					if (loraMode==1)
						LT.setModulationParams(LORA_SF12, LORA_BW_125, CodeRate, Optimisation);
					else if	(loraMode==2)
						LT.setModulationParams(LORA_SF12, LORA_BW_250, CodeRate, Optimisation);
					else if	(loraMode==3)
						LT.setModulationParams(LORA_SF10, LORA_BW_125, CodeRate, Optimisation);
					else if	(loraMode==4)
						LT.setModulationParams(LORA_SF12, LORA_BW_500, CodeRate, Optimisation);
					else if	(loraMode==5)
						LT.setModulationParams(LORA_SF10, LORA_BW_250, CodeRate, Optimisation);
					else if	(loraMode==6)
						LT.setModulationParams(LORA_SF11, LORA_BW_500, CodeRate, Optimisation);																								
					else if	(loraMode==7)
						LT.setModulationParams(LORA_SF9, LORA_BW_250, CodeRate, Optimisation);
					else if	(loraMode==8)
						LT.setModulationParams(LORA_SF9, LORA_BW_500, CodeRate, Optimisation);						
					else if	(loraMode==9)
						LT.setModulationParams(LORA_SF8, LORA_BW_500, CodeRate, Optimisation);						
					else if	(loraMode==10)
						LT.setModulationParams(LORA_SF7, LORA_BW_500, CodeRate, Optimisation);											
#endif						

#if defined SX128X
          LT.setRfFrequency(my_frequency, Offset);		
					if (loraMode==1)
						LT.setModulationParams(LORA_SF12, LORA_BW_0200, CodeRate);
					else if	(loraMode==2)
						LT.setModulationParams(LORA_SF12, LORA_BW_0400, CodeRate);
					else if	(loraMode==3)
						LT.setModulationParams(LORA_SF10, LORA_BW_0200, CodeRate);
					else if	(loraMode==4)
						LT.setModulationParams(LORA_SF12, LORA_BW_0800, CodeRate);
					else if	(loraMode==5)
						LT.setModulationParams(LORA_SF10, LORA_BW_0400, CodeRate);
					else if	(loraMode==6)
						LT.setModulationParams(LORA_SF11, LORA_BW_0800, CodeRate);																								
					else if	(loraMode==7)
						LT.setModulationParams(LORA_SF9, LORA_BW_0400, CodeRate);
					else if	(loraMode==8)
						LT.setModulationParams(LORA_SF9, LORA_BW_0800, CodeRate);						
					else if	(loraMode==9)
						LT.setModulationParams(LORA_SF8, LORA_BW_0800, CodeRate);						
					else if	(loraMode==10)
						LT.setModulationParams(LORA_SF7, LORA_BW_0800, CodeRate);						
#endif
          PRINTLN;
          //reads and prints the configured LoRa settings, useful check
          LT.printModemSettings();                               
          PRINTLN;
          //reads and prints the configured operating settings, useful check
          LT.printOperatingSettings();                           
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
											
							cad_number=cmdValue; 
							
							PRINT_CSTSTR("Set cad_number to ");
							PRINT_VALUE("%d", cad_number);
							PRINTLN;
							break;                       
						} 
								
						if (cmd[i+3]=='O' && cmd[i+4]=='F' && cmd[i+5]=='F' ) {
							cad_number=0;
							break;
						}
							
						if (cmd[i+3]=='*') {
              loraConfig();
							ContinousCad();
             // flush as we do not want to send data
              while (Serial.available())
                Serial.read();   
						}
						else {
							startmS=millis();
							//return RSSI if e<0
							int e = LT.doCAD(SIFS_cad_number);                  
							endmS=millis();

							PRINT_CSTSTR("--> SIFS duration ");
							PRINT_VALUE("%ld",endmS-startmS);
							PRINTLN; 
							
							if (e==0) 
									PRINT_CSTSTR("OK");
							else
									PRINT_VALUE("%d", e);
								
							PRINTLN;
							}
					}
					else if (cmd[i+1]=='S') {
						i=i+2;
						cmdValue=getCmdValue(i);

						if (cmdValue > 3) {
							PRINT_CSTSTR("CS method must be 0, 1, 2 or 3.\n");
							cmdValue=1;    
						}

						carrier_sense_method=cmdValue;

						PRINT_CSTSTR("CS method is ");  
						PRINT_VALUE("%d",carrier_sense_method);
						PRINTLN;
					}
					else {      
						i++;
						cmdValue=getCmdValue(i);
						
						if (cmdValue < STARTING_CHANNEL || cmdValue > ENDING_CHANNEL)
							cmdValue=STARTING_CHANNEL;
								
						cmdValue=cmdValue-STARTING_CHANNEL;  
						
						PRINT_CSTSTR("Set LoRa channel index to ");
						PRINT_VALUE("%d", cmdValue);
						PRINTLN;
						PRINT_CSTSTR("which correspond to frequency of ");
						PRINT_VALUE("%ld", loraChannelArray[cmdValue]);
						PRINTLN;						
						LT.setRfFrequency(loraChannelArray[cmdValue], Offset);              
					}              
				break;
				
				case 'A': 
					if (cmd[i+1]=='C' && cmd[i+2]=='K') {    
						if (cmd[i+3]=='#') {
							// point to the start of the message, skip ACK#
							i=i+4;
							withTmpAck=true;
							sendCmd=true;         
						}
						else {
							if (cmd[i+3]=='O' && cmd[i+4]=='N') {
								withAck=true;
								PRINT_CSTSTR("ACK enabled\n");
							}
							
							if (cmd[i+3]=='O' && cmd[i+4]=='F' && cmd[i+5]=='F') {
								withAck=false;    
								PRINT_CSTSTR("ACK disabled\n");              
							}
						}
					}       
#ifdef WITH_AES               
					else
         
					if (cmd[i+1]=='E' && cmd[i+2]=='S') {
						withAes = !withAes;

						if (withAes)
							PRINT_CSTSTR("AES ON\n");
						else {
              if (fullLorawan)
                PRINT_CSTSTR("WARNING: AES OFF in LoRaWAN mode, deactivating 4-byte header\n");   
							PRINT_CSTSTR("AES OFF\n");   
						}             
					}  
#endif                      
         else
          if (cmd[i+1]=='P' && cmd[i+2]=='P' && cmd[i+3]=='K') {
            withAppKey = !withAppKey;

            if (withAppKey)
              if (fullLorawan) {
                PRINT_CSTSTR("AppKey can not be used in LoRaWAN mode\n");
                withAppKey=false;
              }
              else
                PRINT_CSTSTR("AppKey ON\n");
            else
              PRINT_CSTSTR("AppKey OFF\n");                
          }    
					else {
						i++;
						cmdValue=getCmdValue(i);
						
						// cannot set addr greater than 255
						if (cmdValue > 255)
							cmdValue = 255;
						// cannot set addr lower than 1 since 0 is broadcast
						if (cmdValue < 1)
							cmdValue = 1;
						// set node addr        
						node_addr=cmdValue; 
						
						PRINT_CSTSTR("Set node addr to ");
						PRINT_VALUE("%d",node_addr);  
						PRINTLN;
						// Set the node address 
						LT.setDevAddr(node_addr);
					}     
				break;

				case 'O': 

					if (cmd[i+1]=='N') {						
						PRINT_CSTSTR("Resetting LoRa module and set to ON\n");
            PRINT_CSTSTR("Keep current frequency\n");
            
						radioOn=true;

            LT.resetDevice();
            savedSpreadingFactor=SpreadingFactor;
            savedBandwidth=Bandwidth;
						loraConfig();			
					}
					else
						if (cmd[i+1]=='F' && cmd[i+2]=='F') {
							PRINT_CSTSTR("LoRa module OFF\n");
							radioOn=false;             
					 	}
					 	else
						 	PRINT_CSTSTR("only ON/OFF accepted.\n");          
				break;            

#ifdef WITH_AES
				case 'L': 
					if (cmd[i+1]=='W') {
						fullLorawan = !fullLorawan;

						if (fullLorawan) {
							PRINT_CSTSTR("LORAWAN mode\n");
							PRINT_CSTSTR("AES IS ON\n");
							withAes=true;

#if defined SX126X || defined SX127X
              LT.setModulationParams(LORA_SF12, LORA_BW_125, CodeRate, Optimisation);
              LT.setSyncWord(LORA_MAC_PUBLIC_SYNCWORD);
#endif
#ifdef SX128X
              LT.setModulationParams(LORA_SF12, LORA_BW_0200, CodeRate);
#endif
              //normally 868.1MHz
              LT.setRfFrequency(LORAWAN_UPFQ, Offset);

              PRINTLN;
              //reads and prints the configured LoRa settings, useful check
              LT.printModemSettings();                               
              PRINTLN;
              //reads and prints the configured operating settings, useful check
              LT.printOperatingSettings();                           
              PRINTLN;              
						}
						else {
							PRINT_CSTSTR("LORAWAN mode OFF\n"); 
							PRINT_CSTSTR("AES IS OFF\n");
							withAes=false;
                            
              PRINT_CSTSTR("Set frequency to ");
              PRINT_VALUE("%ld",my_frequency);              
              PRINTLN;   
              LT.setRfFrequency(my_frequency, Offset);

              PRINT_CSTSTR("Set back LoRa mode to "); 
              PRINT_VALUE("%d", loraMode);
              PRINTLN;
              
              //get back to previous LoRa mode
              sprintf(cmd,"/@M%d#", loraMode);
              runLocalCommand=true;
						}
					}            
				break;     
#endif

				// set the frequency 
				// "F433175#" -> 433175000Hz
				case 'F':    
					i++;
					cmdValue=getCmdValue(i);
					
					my_frequency=cmdValue*1000;
														
					PRINT_CSTSTR("Set frequency (Hz) to ");
					PRINT_VALUE("%lu",my_frequency);              
					PRINTLN;   
					LT.setRfFrequency(my_frequency, Offset);     
				break;                 

				default:
					PRINT_CSTSTR("Unrecognized cmd\n");       
					break;
			}
			
			FLUSHOUTPUT;
		}
    else
     sendCmd=true; 

		/////////////////////////////////////////////////////////////////// 
		// TRANSMIT DATA FROM INTERACTIVE SERIAL INPUT
		//    
    if (radioOn && sendCmd && receivedFromSerial) {
      
      uint8_t pl=strlen((char*)(&cmd[i]));

      PRINT_CSTSTR("Sending: ");
      PRINT_STR("%s",(char*)(&cmd[i]));
      PRINTLN;
      PRINT_CSTSTR("Length is ");
      PRINT_VALUE("%d",pl);
      PRINTLN;

      PRINT_CSTSTR("Packet sequence number ");
      PRINT_VALUE("%d",LT.readTXSeqNo());
      PRINTLN;
           
      uint8_t p_type=PKT_TYPE_DATA;

      if (withAppKey && !fullLorawan) {
        app_key_offset = sizeof(my_appKey);
        p_type = p_type | PKT_FLAG_DATA_WAPPKEY;
        //copy the appkey
        memcpy(message,my_appKey,sizeof(my_appKey));
      }
      else
        app_key_offset = 0;

      //copy the command
      memcpy(message+app_key_offset,(char*)(&cmd[i]), pl);
          
      pl+=app_key_offset;  

#ifdef WITH_AES
      if (withAes) {
          p_type = p_type | PKT_FLAG_DATA_ENCRYPTED;

          if (fullLorawan)
            PRINT_CSTSTR("end-device uses native LoRaWAN packet format\n");
          else
            PRINT_CSTSTR("end-device uses encapsulated LoRaWAN packet format only for encryption\n");

          pl=local_aes_lorawan_create_pkt(message, pl, app_key_offset);
      }
#endif

      PRINT_CSTSTR("Payload size is ");  
      PRINT_VALUE("%d",pl);
      PRINTLN;
#ifdef OLED
      u8x8.clearLine(5);
      u8x8.clearLine(6);
      u8x8.clearLine(7);
      sprintf(oled_msg,"Sending %d Bytes", pl);
      u8x8.drawString(0, 5, oled_msg);
#endif
      
      uint32_t toa = LT.getToA(pl+(fullLorawan?0:HEADER_SIZE));
      PRINT_CSTSTR("ToA is ");
      if (!fullLorawan)
        PRINT_CSTSTR("w/4B header ");
      PRINT_VALUE("%d",toa);
      PRINTLN;        
#ifdef OLED
      sprintf(oled_msg,"ToA : %ld", toa);
      u8x8.drawString(0, 6, oled_msg);
#endif               
                    
      startmS=millis();

			if (carrier_sense_method==1)
      	LT.CarrierSense1(cad_number, extendedIFS);
			if (carrier_sense_method==2)
      	LT.CarrierSense2(cad_number, extendedIFS); 
			if (carrier_sense_method==3)
      	LT.CarrierSense3(cad_number);       	      	          

      if ( (withAck || withTmpAck) && !fullLorawan)
        p_type=p_type | PKT_FLAG_ACK_REQ;

      uint8_t tmp_dest_addr=dest_addr;
      
      if (forTmpDestAddr>=0)
        tmp_dest_addr=forTmpDestAddr;      

#ifdef SEND_LED
      digitalWrite(SEND_LED, HIGH);
#endif 
      //txpower can be set to -1 if we want to keep the same power setting PROVIDED THAT setTxParams() has been called before
      if (fullLorawan)
        TXPacketL=LT.transmit(message, pl, 10000, -1, WAIT_TX);
      else          
        TXPacketL=LT.transmitAddressed(message, pl, p_type, tmp_dest_addr, LT.readDevAddr(), 10000, -1, WAIT_TX);

#ifdef SEND_LED
      digitalWrite(SEND_LED, LOW);
#endif 
        
      if (TXPacketL) 
      {
        endmS = millis();                                          
        TXPacketCount++;
        packet_is_OK();
        
        if (LT.readAckStatus()) {
          PRINTLN;
          PRINT_CSTSTR("Received ACK from ");
          PRINTLN_VALUE("%d", LT.readRXSource());
          PRINT_CSTSTR("SNR of transmitted pkt is ");
          PRINTLN_VALUE("%d", LT.readPacketSNRinACK());   
          
          sprintf(sprintf_buf,"--- rxlora ACK. SNR=%d RSSIpkt=%d\n", LT.readPacketSNR(), LT.readPacketRSSI());           
          PRINT_STR("%s",sprintf_buf);                 
        }         
      }
      else
      {
        packet_is_Error();                                 
      }
    }
  } // end of "if (receivedFromSerial || receivedFromLoRa)"
} 
