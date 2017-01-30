/* 
 *  LoRa gateway to receive and send command
 *
 *  Copyright (C) 2015-2016 Congduc Pham
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
 * last update: Dec. 7th by C. Pham
 * 
 *  Version:                1.6
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
 *    - /@DBM10#: set power output to 10dBm. Check #define PABOOST for appropriate compilation
 *    - /@A9#: set node addr to 9
 *    - /@W34#: set sync word to 0x34
 *    - /@ON# or /@OFF#: power on/off the LoRa module
 *
 *  Use of ACK
 *    - /@ACK#hello w/ack : sends the message and request an ACK
 *    - /@ACKON# enables ACK (for all messages)
 *    - /@ACKOFF# disables ACK
 * 
 *  CAD, DIFS/SIFS mechanism, RSSI checking, extended IFS
 *    - /@CAD# performs an SIFS CAD, i.e. 3 or 6 CAD depending on the LoRa mode
 *    - /@CADON3# uses 3 CAD when sending data (normally SIFS is 3 or 6 CAD, DIFS=3SIFS)
 *    - /@CADOFF# disables CAD (IFS) when sending data
 *    - /@RSSI# toggles checking of RSSI before transmission and after CAD
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
 *      - /@D4#/@C1#Q30#: send the command string /@C1#Q30# to node 4
 *    - the S command sends a string of arbitrary size
 *      - /@S50# sends a 50B user payload packet filled with '#'. The real size is 55B with the Libelium header 
 *
 *  if compiled with LORA_LAS
 *    - add LAS support
 *      - sending message will use LAS service
 *      - /@LASS# prints LAS statistics
 *      - /@LASR# resets LAS service
 *      - /@LASON# enables LAS service
 *      - /@LASOFF# disables LAS service
 *    - if IS_SEND_GATEWAY
 *      - /@REG# sends a REG message 
 *    - if IS_RCV_GATEWAY
 *      - /@LASI# initiate the INITrestart/INIT procedure that ask end-devices to send the REG msg
 *      
 *   IMPORTANT NOTICE   
 *    - for more details on the underlying packet format used by our modified SX1272 library
 *      - refer to the SX1272.h
 *      - see http://cpham.perso.univ-pau.fr/LORA/RPIgateway.html
 *      
*/

/*  Change logs
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
// Include the SX1272 
#include "SX1272.h"

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
#elif defined SENEGAL_REGULATION
#define MAX_DBM 10
#elif defined FCC_US_REGULATION
#define MAX_DBM 14
#endif

// IMPORTANT
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// uncomment if your radio is an HopeRF RFM92W, HopeRF RFM95W, Modtronix inAir9B, NiceRF1276
// or you known from the circuit diagram that output use the PABOOST line instead of the RFO line
//#define PABOOST
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 

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

///////////////////////////////////////////////////////////////////
// CHANGE HERE FOR VARIOUS BEHAVIORS
/////////////////////////////////////////////////////////////////// 
//#define SHOW_FREEMEMORY
//#define CAD_TEST
//#define PERIODIC_SENDER 30000
//#define LORA_LAS
//#define WITH_SEND_LED
//#define WITH_AES
///////////////////////////////////////////////////////////////////

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

#ifdef WITH_SEND_LED
#define SEND_LED  44
#endif

///////////////////////////////////////////////////////////////////
// CHANGE HERE THE LORA MODE, NODE ADDRESS 
#define LORAMODE  1
#define LORA_ADDR 6
// the special mode to test BW=125MHz, CR=4/5, SF=12
// on the 868.1MHz channel
//#define LORAMODE 11
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// TO GATEWAY WHICH HAS ADDRESS 1
#define DEFAULT_DEST_ADDR 1

// to unlock remote configuration feature
#define UNLOCK_PIN 1234

#ifdef LORA_LAS
#include "LoRaActivitySharing.h"
// acting as an end-device
LASDevice loraLAS(LORA_ADDR,LAS_DEFAULT_ALPHA,DEFAULT_DEST_ADDR);
#endif

///////////////////////////////////////////////////////////////////
// CONFIGURATION VARIABLES
//
int dest_addr=DEFAULT_DEST_ADDR;

char cmd[260]="****************";
char sprintf_buf[100];
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

uint8_t loraAddr=LORA_ADDR;

unsigned long startDoCad, endDoCad;
bool extendedIFS=true;
uint8_t SIFS_cad_number;
// set to 0 to disable carrier sense based on CAD
uint8_t send_cad_number=3;
uint8_t SIFS_value[11]={0, 183, 94, 44, 47, 23, 24, 12, 12, 7, 4};
uint8_t CAD_value[11]={0, 62, 31, 16, 16, 8, 9, 5, 3, 1, 1};

unsigned int inter_pkt_time=0;
unsigned int random_inter_pkt_time=0;
unsigned long next_periodic_sendtime=0L;

// packet size for periodic sending
uint8_t MSS=40;

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
  0x00, 0x00, 0x00, LORA_ADDR
};

uint16_t Frame_Counter_Up = 0x0000;
// we use the same convention than for LoRaWAN as we will use the same AES convention
// See LoRaWAN specifications
unsigned char Direction = 0x00;

boolean with_aes=false;
boolean full_lorawan=false;
#endif
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
  
  // Select frequency channel
  if (loraMode==11) {
    // if we start with mode 11, then switch to 868.1MHz for LoRaWAN test
    // Note: if you change to mode 11 later using command /@M11# for instance, you have to use /@C18# to change to the correct channel
    e = sx1272.setChannel(CH_18_868);
    PRINT_CSTSTR("%s","^$Channel CH_18_868: state ");    
  }
  else {
    e = sx1272.setChannel(loraChannel);

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
  PRINT_VALUE("%d", e);
  PRINTLN; 

#ifdef PABOOST
  sx1272._needPABOOST=true;
  PRINT_CSTSTR("%s","^$Use PA_BOOST amplifier line");
  PRINTLN;  
  // previous way for setting output power
  // powerLevel='x';  
#else
  // previous way for setting output power
  // powerLevel='M';  
#endif
 
  PRINT_CSTSTR("%s","^$Set LoRa power dBm to ");
  PRINT_VALUE("%d",(uint8_t)MAX_DBM);  
  PRINTLN;

  // previous way for setting output power
  // e = sx1272.setPower(powerLevel);  
    
  // Select output power in dBm
  e = sx1272.setPowerDBM((uint8_t)MAX_DBM);  
                
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
  
  // Set the node address and print the result
  //e = sx1272.setNodeAddress(loraAddr);
  sx1272._nodeAddress=loraAddr;
  e=0;
  PRINT_CSTSTR("%s","^$LoRa addr ");
  PRINT_VALUE("%d", loraAddr);
  PRINT_CSTSTR("%s",": state ");
  PRINT_VALUE("%d", e);
  PRINTLN;
  
  // Print a success message
  PRINT_CSTSTR("%s","^$SX1272/76 configured ");
  PRINT_CSTSTR("%s","as device. Waiting serial input for serial-RF bridge\n");
}

void setup()
{
  int e;
  delay(3000);
  randomSeed(analogRead(14));

  // Open serial communications and wait for port to open:
#if defined __SAMD21G18A__ && not defined ARDUINO_SAMD_FEATHER_M0
  SerialUSB.begin(38400);
#else
  Serial.begin(38400);  
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

#ifdef __MK20DX256__
  PRINT_CSTSTR("%s","Teensy31/32 detected\n");
#endif

#ifdef __MKL26Z64__
  PRINT_CSTSTR("%s","TeensyLC detected\n");
#endif

#ifdef __SAMD21G18A__ 
  PRINT_CSTSTR("%s","Arduino M0/Zero detected\n");
#endif

#if defined ARDUINO && defined SHOW_FREEMEMORY && not defined __MK20DX256__ && not defined __MKL26Z64__ && not defined  __SAMD21G18A__ && not defined _VARIANT_ARDUINO_DUE_X_
    // Print a start message
  Serial.print(freeMemory());
  Serial.println(F(" bytes of free memory.")); 
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
  
  if (!e) {
    radioON=true;
    startConfig();
  }
  
  FLUSHOUTPUT;
  delay(1000);

#ifdef LORA_LAS
  loraLAS.ON(LAS_ON_WRESET);
   
  //delay(random(LAS_REG_RAND_MIN,LAS_REG_RAND_MAX));
  //loraLAS.sendReg(); 
#endif

#ifdef CAD_TEST
  PRINT_CSTSTR("%s","Do CAD test\n");
#endif 

#ifdef PERIODIC_SENDER
  inter_pkt_time=PERIODIC_SENDER;
  PRINT_CSTSTR("%s","Periodic sender ON");
  PRINTLN;
#endif
}

// we could use the CarrierSense function added in the SX1272 library, but it is more convenient to duplicate it here
// so that we could easily modify it for testing
void CarrierSense() {
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
            uint8_t w = random(1,8);

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

          // wait for random number of DIFS               
          uint8_t w = random(1,8);
       
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
}

void loop(void)
{ 
  int i=0, e;
  int cmdValue;
/////////////////////////////////////////////////////////////////// 
// ONLY FOR TESTING CAD
#ifdef CAD_TEST

  bool CadDetected=false;
  unsigned long firstDetected, lastDetected=0;
  
  while (1) {
  
      startDoCad=millis();
      e = sx1272.doCAD(6);
      endDoCad=millis();
      
      //PRINT_CSTSTR("%s","--> SIFS duration ");
      //PRINT_VALUE("%ld", endDoCad-startDoCad);
      //PRINTLN;
    
      //if (!e) 
      //  PRINT_CSTSTR("%s","OK");
      //else
      //  PRINT_CSTSTR("%s","###");

      if (e && !CadDetected) {
        PRINT_CSTSTR("%s","#########\n");
        PRINT_VALUE("%ld", endDoCad-lastDetected);  
        PRINTLN;
        PRINT_CSTSTR("%s","#########\n");                
        firstDetected=endDoCad;
        CadDetected=true;
      }

      if (!e && CadDetected)  {
        PRINT_VALUE("%ld", endDoCad);
        PRINTLN;
        PRINT_VALUE("%ld", endDoCad-firstDetected);  
        PRINTLN;             
        PRINT_VALUE("%d", sx1272._RSSI);
        PRINTLN;
        lastDetected=endDoCad;        
        CadDetected=false;
      }       
        
      if (e) {
        PRINT_VALUE("%ld", endDoCad);
        PRINT_CSTSTR("%s"," ## ");
        PRINT_VALUE("%d", sx1272._RSSI);
        PRINTLN;
      }
      
      //delay(200);
      
      //startDoCad=millis();
      //e = sx1272.doCAD(SIFS_cad_number*3);
      //endDoCad=millis();
      
      //PRINT_CSTSTR("%s","--> DIFS duration ");
      //PRINT_VALUE("%ld", endDoCad-startDoCad);
      //PRINTLN;
    
      //if (!e) 
      //  PRINT_CSTSTR("%s","OK");
      //else
      //  PRINT_CSTSTR("%s","###");
      
      //PRINTLN;
        
      delay(200);
  }
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

/////////////////////////////////////////////////////////////////// 
// THE MAIN LOOP FOR AN INETRACTIVE END-DEVICE
//
  if (radioON && !receivedFromSerial) {
 
      // periodic message sending? (mainly for tests)
      if (inter_pkt_time)
      
        if (millis() > next_periodic_sendtime) {
          
          PRINT_CSTSTR("%s","time is ");
          PRINT_VALUE("%ld",millis());  
          PRINTLN;
          
          sprintf(cmd, "msg%3d***", msg_sn++);
          for (i=strlen(cmd); i<MSS; i++)
            cmd[i]='*';
          
          cmd[i]='\0';
          
          PRINT_CSTSTR("%s","Sending : ");
          PRINT_STR("%s",cmd);  
          PRINTLN;
          
          CarrierSense();
          
          PRINT_CSTSTR("%s","Packet number ");
          PRINT_VALUE("%d",sx1272._packetNumber);
          PRINTLN;
          
          long startSend=millis();
          
#ifdef WITH_SEND_LED
          digitalWrite(SEND_LED, HIGH);
#endif       
          e = sx1272.sendPacketTimeout(dest_addr, (uint8_t*)cmd, strlen(cmd), 10000);
          
#ifdef WITH_SEND_LED
          digitalWrite(SEND_LED, LOW);
#endif           
          PRINT_CSTSTR("%s","LoRa Sent in ");
          PRINT_VALUE("%ld",millis()-startSend);  
          PRINTLN;
          PRINT_CSTSTR("%s","Packet sent, state ");
          PRINT_VALUE("%d",e);
          PRINTLN;
          //Serial.flush();
          
          if (random_inter_pkt_time) {                
            random_inter_pkt_time=random(2000,inter_pkt_time);

            next_periodic_sendtime = millis() + random_inter_pkt_time;
          }
          else
            next_periodic_sendtime = millis() + inter_pkt_time;
            
          PRINT_CSTSTR("%s","next at ");
          PRINT_VALUE("%ld", next_periodic_sendtime);
          PRINTLN;     
        }  
        
      // TODO
      // the end-device should also open a receiving window to receive 
      // INIT & UPDT messages
      e=1;
#ifndef CAD_TEST        
      // open a receive window
      uint16_t w_timer=1000;
      
      if (loraMode==1)
        w_timer=2500;
        
      e = sx1272.receivePacketTimeout(w_timer);
#endif 
///////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////// 
// IF WE RECEIVE A RADIO PACKET
//
      if (!e) {
         int a=0, b=0;
         uint8_t tmp_length;

         receivedFromLoRa=true;
         sx1272.getSNR();
         sx1272.getRSSIpacket();

         tmp_length=sx1272._payloadlength;
         
         sprintf(sprintf_buf,"--- rxlora. dst=%d type=0x%.2X src=%d seq=%d len=%d SNR=%d RSSIpkt=%d BW=%d CR=4/%d SF=%d\n", 
                   sx1272.packet_received.dst,
                   sx1272.packet_received.type, 
                   sx1272.packet_received.src,
                   sx1272.packet_received.packnum,
                   tmp_length, 
                   sx1272._SNR,
                   sx1272._RSSIpacket,
                   (sx1272._bandwidth==BW_125)?125:((sx1272._bandwidth==BW_250)?250:500),
                   sx1272._codingRate+4,
                   sx1272._spreadingFactor);
                   
         PRINT_STR("%s",sprintf_buf);

         // provide a short output for external program to have information about the received packet
         // ^psrc_id,seq,len,SNR,RSSI
         sprintf(sprintf_buf,"^p%d,%d,%d,%d,%d,%d,%d\n",
                   sx1272.packet_received.dst,
                   sx1272.packet_received.type,                   
                   sx1272.packet_received.src,
                   sx1272.packet_received.packnum, 
                   tmp_length,
                   sx1272._SNR,
                   sx1272._RSSIpacket);
                   
         PRINT_STR("%s",sprintf_buf);          

         // ^rbw,cr,sf
         sprintf(sprintf_buf,"^r%d,%d,%d\n", 
                   (sx1272._bandwidth==BW_125)?125:((sx1272._bandwidth==BW_250)?250:500),
                   sx1272._codingRate+4,
                   sx1272._spreadingFactor);
                   
         PRINT_STR("%s",sprintf_buf);  
                             
#ifdef LORA_LAS        
         if (loraLAS.isLASMsg(sx1272.packet_received.data)) {
           
           //tmp_length=sx1272.packet_received.length-OFFSET_PAYLOADLENGTH;
           tmp_length=sx1272._payloadlength;
           
           int v=loraLAS.handleLASMsg(sx1272.packet_received.src,
                                      sx1272.packet_received.data,
                                      tmp_length);
           
           if (v==DSP_DATA) {
             a=LAS_DSP+DATA_HEADER_LEN+1;           
           }
           else
             // don't print anything
             a=tmp_length; 
         }
         else
           PRINT_CSTSTR("%s","No LAS header. Write raw data\n");
#endif
         for ( ; a<tmp_length; a++,b++) {
           PRINT_STR("%c",(char)sx1272.packet_received.data[a]);
           
           cmd[b]=(char)sx1272.packet_received.data[a];
         }
         
         // strlen(cmd) will be correct as only the payload is copied
         cmd[b]='\0';    
         PRINTLN;
         FLUSHOUTPUT;         
      }  
  }  

/////////////////////////////////////////////////////////////////// 
// PROCESS INCOMING DATA
//  
  if (receivedFromSerial || receivedFromLoRa) {
    
    boolean sendCmd=false;
    boolean withTmpAck=false;
    int forTmpDestAddr=-1;
    
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

            case 'D': 
              if (cmd[i+1]=='B' && cmd[i+2]=='M') {
                  i=i+3;
                  cmdValue=getCmdValue(i);

                  if (cmdValue > 0 && cmdValue < 15) {
                      PRINT_CSTSTR("%s","^$set power dBm: ");
                      PRINT_VALUE("%d",cmdValue);  
                      PRINTLN;                    
                      // Set power dBm
#if defined RADIO_RFM92_95 || defined RADIO_INAIR9B || defined RADIO_20DBM                      
                      e = sx1272.setPowerDBM((uint8_t)cmdValue, 1);
#else
                      e = sx1272.setPowerDBM((uint8_t)cmdValue);
#endif
                      PRINT_CSTSTR("%s","^$set power dBm: state ");
                      PRINT_VALUE("%d",e);  
                      PRINTLN;   
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
                      PRINT_CSTSTR("%s","Set LoRa dest addr to ");
                      PRINT_VALUE("%d",dest_addr); 
                      PRINTLN;                
                  }
                  else {
                      // only for the following ASCII command
                      forTmpDestAddr=cmdValue;
                      PRINT_CSTSTR("%s","Set LoRa dest addr FOR THIS ASCII STRING to ");
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
                PRINT_CSTSTR("%s","Set inter-packet time to ");
                PRINT_VALUE("%ld",inter_pkt_time);
                PRINTLN;   
                next_periodic_sendtime=millis()+inter_pkt_time;
              }
              else {
                PRINT_CSTSTR("%s","Disable periodic send\n");                  
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
              
              PRINT_CSTSTR("%s","Set MSS to ");
              PRINT_VALUE("%d",MSS);
              PRINTLN;              
            break;    

            case 'R': 
#ifdef LORA_LAS             
              if (cmd[i+1]=='E' && cmd[i+2]=='G') {
                  PRINT_CSTSTR("%s","^$Send LAS REG msg\n");
                  loraLAS.sendReg();
              }
#endif              
              if (cmd[i+1]=='S' && cmd[i+2]=='S' && cmd[i+3]=='I') {
                
                RSSIonSend = !RSSIonSend;
                
                if (RSSIonSend)
                  PRINT_CSTSTR("%s","RSSI ON\n");
                else
                  PRINT_CSTSTR("%s","RSSI OFF\n");
              }
            break;                

            case 'E':          

              if (cmd[i+1]=='I' && cmd[i+2]=='F' && cmd[i+3]=='S') {
                
                extendedIFS = !extendedIFS;
                
                if (extendedIFS)
                  PRINT_CSTSTR("%s","EIFS ON\n");
                else
                  PRINT_CSTSTR("%s","EIFS OFF\n");
              }
            break;  

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
              else {            
                  i++;
                  cmdValue=getCmdValue(i);
                  i++;
                  if (cmdValue>250) {
                    PRINT_CSTSTR("%s","No more than 250B\n");
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
                char loraPower=cmd[i+1];

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
            
            case 'A': 
              
              if (cmd[i+1]=='C' && cmd[i+2]=='K') {    

                if (cmd[i+3]=='#') {
                  // point to the start of the message, skip /@ACK#
                  i=6;
                  withTmpAck=true;
                  sendCmd=true;         
                }
                else {

                  if (cmd[i+3]=='O' && cmd[i+4]=='N') {
                    withAck=true;
                    PRINT_CSTSTR("%s","^$ACK enabled\n");
                  }
                  
                  if (cmd[i+3]=='O' && cmd[i+4]=='F' && cmd[i+5]=='F') {
                    withAck=false;    
                    PRINT_CSTSTR("%s","^$ACK disabled\n");              
                  }
                }
              }
#ifdef WITH_AES              
              else
              if (cmd[i+1]=='E' && cmd[i+2]=='S') {
                with_aes = !with_aes;

                if (with_aes)
                  PRINT_CSTSTR("%s","AES ON\n");
                else
                  PRINT_CSTSTR("%s","AES OFF\n");                
              }
#endif              
              else {
              
                i++;
                cmdValue=getCmdValue(i);
                
                // cannot set addr greater than 255
                if (cmdValue > 255)
                        cmdValue = 255;
                // cannot set addr lower than 1 since 0 is broadcast
                if (cmdValue < 1)
                        cmdValue = LORA_ADDR;
                // set node addr        
                loraAddr=cmdValue; 
                
                PRINT_CSTSTR("%s","^$Set LoRa node addr to ");
                PRINT_VALUE("%d",loraAddr);  
                PRINTLN;
                // Set the node address and print the result
                e = sx1272.setNodeAddress(loraAddr);
                PRINT_CSTSTR("%s","^$Setting LoRa node addr: state ");
                PRINT_VALUE("%d",e);     
                PRINTLN;            
              }     
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


            case 'L': 
#ifdef LORA_LAS
              // act as an LR-BS if IS_RCV_GATEWAY or an end-device if IS_SEND_GATEWAY             
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
              }
#endif
#ifdef WITH_AES
              if (cmd[i+1]=='W') {
                full_lorawan = !full_lorawan;

                if (full_lorawan) {
                  PRINT_CSTSTR("%s","NATIVE LORAWAN FORMAT ON\n");
                  // indicate to SX1272 lib that raw mode at transmission is required to avoid our own packet header
                  sx1272._rawFormat=true;
                  with_aes=true;
                  PRINT_CSTSTR("%s","FOR SENDING TO A LORAWAN GW, YOU HAVE TO:\n");
                  PRINT_CSTSTR("%s","SET TO MODE 11: /@M11#\n");
                  PRINT_CSTSTR("%s","SET CHANNEL to 18: /@C18#\n");
                  PRINT_CSTSTR("%s","SET SYNC WORD to 0x34: /@W34#\n");
                }
                else {
                  PRINT_CSTSTR("%s","NATIVE LORAWAN FORMAT OFF\n"); 
                  PRINT_CSTSTR("%s","WARNING: AES IS NOW OFF\n");
                  sx1272._rawFormat=false;
                  with_aes=false;
                }
              }
#endif              
            break;         

            default:

              PRINT_CSTSTR("%s","Unrecognized cmd\n");       
              break;
      }
      FLUSHOUTPUT;
    }
    else
     sendCmd=true; 

/////////////////////////////////////////////////////////////////// 
// TRANSMIT DATA FROM INTERACTIVE SERIAL INPUT
//    
    if (sendCmd && receivedFromSerial) {
      
      uint8_t pl=strlen((char*)(&cmd[i]));
      PRINT_CSTSTR("%s","Sending. Length is ");
      PRINT_VALUE("%d",pl);
      PRINTLN;
      PRINT_STR("%s",(char*)(&cmd[i]));
      PRINTLN;
      
#ifdef LORA_LAS
      if (forTmpDestAddr>=0)
        e = loraLAS.sendData(forTmpDestAddr, (uint8_t*)(&cmd[i]), pl, 0, 
              LAS_FIRST_DATAPKT+LAS_LAST_DATAPKT, withAck | withTmpAck);
      else
        e = loraLAS.sendData(dest_addr, (uint8_t*)(&cmd[i]), pl, 0,
              LAS_FIRST_DATAPKT+LAS_LAST_DATAPKT, withAck | withTmpAck);
      
      if (e==TOA_OVERUSE) {
          PRINT_CSTSTR("%s","^$Not sent, TOA_OVERUSE\n");  
      }
      
      if (e==LAS_LBT_ERROR) {
          PRINT_CSTSTR("%s","^$LBT error\n");  
      }      
      
      if (e==LAS_SEND_ERROR || e==LAS_ERROR) {
          PRINT_CSTSTR("%s","Send error\n");  
      }      
#else  
      // only the DIFS/SIFS mechanism
      // we chose to have a complete control code insytead of using the implementation of the LAS class
      // for better debugging and tests features if needed.    
      PRINT_CSTSTR("%s","Payload size is ");  
      PRINT_VALUE("%d",pl);
      PRINTLN;
      
      uint32_t toa = sx1272.getToA(pl+OFFSET_PAYLOADLENGTH);      
      PRINT_CSTSTR("%s","ToA is w/4B header ");
      PRINT_VALUE("%d",toa);
      PRINTLN;
      
      long startSend, endSend;
      long startSendCad;
      
      startSendCad=millis();
      CarrierSense();
      startSend=millis();

#ifdef WITH_SEND_LED
      digitalWrite(SEND_LED, HIGH);
#endif

      PRINT_CSTSTR("%s","Packet number ");
      PRINT_VALUE("%d",sx1272._packetNumber);
      PRINTLN;

#ifdef WITH_AES
      if (with_aes) {

          PRINT_CSTSTR("%s","plain payload hex\n");
          for (int j=i; j<i+pl;j++) {
            if (cmd[j]<16)
              PRINT_CSTSTR("%s","0");
            PRINT_HEX("%X", cmd[j]);
            PRINT_CSTSTR("%s"," ");
          }
          PRINTLN; 
    
          PRINT_CSTSTR("%s","Encrypting\n");     
          PRINT_CSTSTR("%s","encrypted payload\n");
          Encrypt_Payload((unsigned char*)(&cmd[i]), pl, Frame_Counter_Up, Direction);
          //Print encrypted message
          for (int j=i; j<i+pl;j++) {
            if (cmd[j]<16)
              PRINT_CSTSTR("%s","0");
            PRINT_HEX("%X", cmd[j]);
            PRINT_CSTSTR("%s"," ");
          }
          PRINTLN;   
    
          // with encryption, we use for the payload a LoRaWAN packet format to reuse available LoRaWAN encryption libraries
          //
          unsigned char LORAWAN_Data[256];
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
          for(int j = i; j < i+pl; j++)
          {
            // see that we don't take the appkey, just the encrypted data that starts that message[app_key_offset]
            LORAWAN_Data[LORAWAN_Package_Length + j] = cmd[i+j];
          }
        
          //Add data Lenth to package length
          LORAWAN_Package_Length = LORAWAN_Package_Length + pl;
        
          PRINT_CSTSTR("%s","calculate MIC with NwkSKey\n");
          //Calculate MIC
          Calculate_MIC(LORAWAN_Data, MIC, LORAWAN_Package_Length, Frame_Counter_Up, Direction);
        
          //Load MIC in package
          for(int j=0; j < 4; j++)
          {
            LORAWAN_Data[j + LORAWAN_Package_Length] = MIC[j];
          }
        
          //Add MIC length to package length
          LORAWAN_Package_Length = LORAWAN_Package_Length + 4;
        
          PRINT_CSTSTR("%s","transmitted LoRaWAN-like packet:\n");
          PRINT_CSTSTR("%s","MHDR[1] | DevAddr[4] | FCtrl[1] | FCnt[2] | FPort[1] | EncryptedPayload | MIC[4]\n");
          //Print transmitted data
          for(int j = 0; j < LORAWAN_Package_Length; j++)
          {
            if (LORAWAN_Data[j]<16)
              PRINT_CSTSTR("%s","0");
            PRINT_HEX("%X", LORAWAN_Data[j]);         
            PRINT_CSTSTR("%s"," ");
          }
          PRINTLN;      
    
          // copy back to cmd
          memcpy((unsigned char *)(&cmd[i]),LORAWAN_Data,LORAWAN_Package_Length);
          pl = LORAWAN_Package_Length;     

          if (full_lorawan) {
              PRINT_CSTSTR("%s","end-device uses native LoRaWAN packet format\n");
          }
          else {
              PRINT_CSTSTR("%s","end-device uses encapsulated LoRaWAN packet format only for encryption\n");
          }
          
          // we increment Frame_Counter_Up
          // even if the transmission will not succeed
          Frame_Counter_Up++;

          // encrypted data
          sx1272.setPacketType(PKT_TYPE_DATA | PKT_FLAG_DATA_ENCRYPTED);   
      }
      else
        sx1272.setPacketType(PKT_TYPE_DATA);
#else     
      sx1272.setPacketType(PKT_TYPE_DATA); 
#endif
        
      if (forTmpDestAddr>=0) {
        if (withAck)
          e = sx1272.sendPacketTimeoutACK(forTmpDestAddr, (uint8_t*)(&cmd[i]), pl, 10000);  
        else    
          e = sx1272.sendPacketTimeout(forTmpDestAddr, (uint8_t*)(&cmd[i]), pl, 10000);
      }
      else {
        if (withAck || withTmpAck)   
          e = sx1272.sendPacketTimeoutACK(dest_addr, (uint8_t*)(&cmd[i]), pl, 10000);    
         else 
          e = sx1272.sendPacketTimeout(dest_addr, (uint8_t*)(&cmd[i]), pl, 10000);
      }

#ifdef WITH_SEND_LED
      digitalWrite(SEND_LED, LOW);
#endif

      endSend=millis();  
      
      if ((withAck || withTmpAck) && !e) {
        sx1272.getSNR();
        sx1272.getRSSIpacket();
         
        sprintf(sprintf_buf,"--- rxlora ACK. SNR=%d RSSIpkt=%d\n", 
                   sx1272._SNR,
                   sx1272._RSSIpacket);
                   
        PRINT_STR("%s",sprintf_buf);
        
        PRINT_CSTSTR("%s","LoRa (ACK) Sent in ");  
      }
      else      
        PRINT_CSTSTR("%s","LoRa Sent in ");
      
      PRINT_VALUE("%ld",endSend-startSend);
      PRINTLN;
      
      PRINT_CSTSTR("%s","LoRa Sent w/CAD in ");
      PRINT_VALUE("%ld",endSend-startSendCad);
      PRINTLN;      
#endif    
      PRINT_CSTSTR("%s","Packet sent, state ");
      PRINT_VALUE("%d",e);
      PRINTLN;
    }
  } // end of "if (receivedFromSerial || receivedFromLoRa)"
} 

