/*
 *  uCAMII takes picture, performs intrusion detection. If intrusion then encode and sent
 *
 *  Copyright (C) 2014-2017 Congduc Pham
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
 *  The current configuration makes the image node entirely autonomous on startup. Current default configuration:
 *    - use an XBEE module connected at 125000 baud, attached on UART3 (Due & Teensy32); or a LoRa module connected on SPI bus
 *    - 1 camera connected at 115200 baud, attached on UART1 (Due & Teensy32) 
 *    - at startup, can send the reference image
 *    - the node source address is derived from the 64-bit MAC addr as WITH_SRC_ADDR and SRC_ADDR_FROM_64MAC are defined (XBee only)
 *    - no power saving of the XBee module
 *    - criticality-based scheduling is implemented but not activated, can be activated by issuing "CS#" command
 *    - intrusion detection can be performed every 30s as criticality-based scheduling is not activated
 *
 *  IMPORTANT: change FINAL_DEST_MAC_ADDR (XBee 802.15.4) or LORA_DEST_ADDR (default is 1) according to the address of your receiving gateway
 *             or change it at run time by issuing a "/@D" command
 *
 *  Version:                1.8
 *  Design:                 C. Pham
 *  Implementation:         C. Pham
 *
 *  Command starts with /@ and ends with #
 *
 *  "T130#" -> immediately captures, encodes and transmits with inter pkt time of 130ms
 *  "CO3#" -> sets the number of cover-sets to 3
 *  "CL8#" -> sets the criticality level to 0.8
 *  "CM8#" -> sets the maximum number of cover-sets to 8
 *  "CB#" -> starts the criticality-based scheduling
 *  "CT10#" -> sets the high-criticality period after intrusion detection
 *  "C1#" -> select camera 1 for next T/S/I command
 *  "F30000#" -> sets inter-snapshot time to 30000ms, i.e. 30s, fps is then 1/30 
 *  "S0#"/"S1#" -> starts a snapshot, make the comparison with the reference image and transmit the image if S1
 *  "I#" -> makes a snapshot and stores a new reference image
 *  "I0#I1#/" -> disables/enables(sets) whether an intrusion defines a new reference image or not, default is true
 *  "Z40#" -> sets the MSS size, default is 64
 *  "Q40#" -> sets the quality factor, default is 50
 *  "D0013A2004086D828#" -> sets the destination mac addr, use D000000000000FFFF for broadcast again
 *  "Y0#"/"Y1#" -> disables/enables(sets) the 16-bit node's short address
 *  "L0#" -> sets flow id (start at 0x50 in image mode)
 *  "R0#"/"R1#" -> disables/enables(set) the MAC layer ack mechanism
 *
 *  LoRa module commands
 *
 *    - LORAM1#: set LoRa mode 1
 *    - LORAC12#: use channel 12 (in 868MHz band)
 *    - LORAPL/H/M/x/X#: set power to Low, High, Max, extreme (PA_BOOST 14dBm, eXtreme (PA_BOOST 20dBm)
 *    - LORAA9#: set node addr to 9  
 *
 *  Need to increase the serial buffer size to 512 for instance, see RingBuffer.h
 *  
 *  packet structure : 0xFF 0x50-0x54 src_addr(16b) SN(8b) Q(8b) len (8b)
 *
 *  put a red led on pin 44 (Teensy: 18) and a green led on pin 45 (Teensy: 19)
 *  
 * Sync led (green led)
 *    - The green led shows that the ucam has successfully syncked
 * Capture led (red led)
 *    - When device is initiating an image capture the red led is ON
 *    - The red led will blink once when transmitting each packet
 *    - When device is receiving a command, the red led will blink 5 times
 * On startup
 *    - The green led will blink 5 times then the red led will be ON during the sync attempts
 *    - When ucam has synched, green led will be ON for 1s
 *    - If the red led stays ON, it means that the ucam is not ready, 
 *      - reset the board by unplugging and plugging again the VCC wire; 
 *      - repeat the process until the green led turns to ON
 * Taking first reference image
 *    - The red led will blink 5 times then the green led will blink 3 times to indicate that raw image is encoded and for each transmitted image packet the red led will blink
 * Waking-up for image change detection
 *    - The red led will be ON and once camera has synched the green led will blink 3 times
 *    - The red led will then be OFF and if there have been changes, will blink for each image packet transmission
 *
 *  if connected, a led for each camera can be switched on each time the camera is active
 *    - pin 30 for camera 0
 *    - pin 31 for camera 1
 *    - pin 32 for camera 2
 *
 */

/*  Change logs
 *  June, 29th, 2017. v1.8
 *        Add CarrierSense selection method to perform tests
 *          - CarrierSense0 does nothing -> pure ALOHA
 *          - CarrierSense1 is the inital proposed carrier sense mechanism described in TOS LAS paper
 *          - CarrierSense2 is an alternative carrier sense mechanism derived from 802.11
 *          - CarrierSense3 is the proposed carrier sense mechanism 
 *          - /@CS0#, /@CS1# or /@CS2# or /@CS3# can dynamically choose between those
 *        Change previous CS command to CB to begin criticality scheduling, see Apr, 17th, 2015. v1.1
 *  June, 8th, 2017. v1.7 
 *        Fix bugs to still support an XBee module on the Teensy32
 *        Clean/correct some code portion, especially uniformizing usage of UART ports
 *            - uCam is connected to Serial1. On the Teensy, it is (RX1:pin0; TX1:pin1)
 *            - XBee 802.15.4 can still be used and should connected to Serial3. On the Teensy it is (RX3:pin7; TX3:pin8)
 *            - On the Due, use the same settings but hardware pin differ
 *  May, 23rd, 2016. v1.6
 *        Teensy + LoRa becomes the target development board. Due and 802.15.4 support will not be maintained.
 *        Added low power with Snooze library for the Teensy.
 *        Bug fixes
 *            - when receiving ctrl packets from Serial and from LoRa
 *  Apr, 12th, 2016. v1.5
 *        Add the simple behavior to periodically transmit an image for situation-awareness applications
 *            - uncomment #define PERIODIC_IMG_TRANSMIT and comment #define USEREFIMAGE
 *            - for the moment DEFAULT_INTER_SNAPSHOT_TIME is then set 1 hour
 *  Apr, 3rd, 2016. v1.4
 *        Add Teensy3.2 support. Only one uCam is connected to Serial3 (RX3:pin7; TX3:pin8)
 *        The current distribution is configured for the Teensy with a LoRa radio module:
 *            - LORA_UCAM && USEREFIMAGE && CRAN_NEW_CODING && GET_PICTURE_ON_SETUP  
 *  Jun, 29th, 2015. v1.3
 *        Add CAD support, preliminary test for LoRa LAS integration
 *  Apr, 28th, 2015. v1.2
 *        Accepts command string from serial input for direct configuration without wireless gateway
 *        Must have UART0 free for PC communication, so CAM3 nor XEEE_UART0 cannot be used 
 *  Apr, 17th, 2015. v1.1
 *        Add possibility to configure remotely the LoRa operation mode
 *          - LORAM1#: set LoRa mode 1
 *          - LORAC12#: use channel 12 (868MHz)
 *          - LORAPL/H/M#: set power to Low, High or Max
 *          - LORAA9#: set node addr to 9       
 *        Add CS and CT command to handle criticality-based scheduling
 *          - CS now starts the criticality scheduling, use F to disable the criticality scheduling
 *          - CT sets the high criticality period after an intrusion
 *  Apr, 13th, 2015. v1.0
 *        Add support for LoRa transmission with Libelium SX1272 module
 *          comment XBEE_UCAM and uncomment LORA_UCAM
 *          address of the image sensor is defined by UCAM_ADDR
 *          the default LoRa dest addr is 8, you can change it but need to have the corresponding LoRa receiver
 *  Mar, 30th, 2015. v0.9b
 *        Source node addr can be derived from the 64-bit MAC addr (last 16 bits): uncomment #define SRC_ADDR_FROM_64MAC
 *        The node id (node addr) and/or the cam id can be made random for tests: uncomment  #define RANDOM_NODE_ID or RANDOM_CAM_ID
 *          node id can be between 0x000A (10) and 0x000F (15). Cam id is between 0 and 2
 *        if SRC_ADDR_FROM_64MAC and only RANDOM_CAM_ID, only the cam id will be random, emulate multi-camera system
 *  Mar, 24th, 2015. v0.9a
 *        Change the packet structure to have both SN and quality factor: 0xFF 0x50-0x54 SN(8b) Q(8b)
 *        16-bit source address of node can be stored in packet to identify source at display node: uncomment #define WITH_SRC_ADDR
 *          structure is then 0xFF 0x50-0x54 src_addr(16b) SN(8b) Q(8b)
 *          Note that this is the behavior that display_multi_image is expecting
 *          in case of multiple camera, the current camera index is indicated in the flow id. display_multi_image used it
 *  Mar, 13th, 2015. v0.9
 *        Improve intrusion detection by computing and taking into account the mean image luminosity: uncomment #define LUM_HIST0
 *  Fev, 20th, 2015. v0.8
 *        Add energy test support for performing measures with appropriate devices: uncomment #define ENERGY_TEST
 *          on startup, will capture, encode and send an image 40s after capture led blinked 10 times
 *          capture led will be on 500ms before and 500ms after to delimit the cycle duration
 *        Can insert additional led energy pattern for easy identification of processing phases: uncomment #define ENERGY_TEST_TIMING
 *          capture led is on 200ms after cam sync, 100ms after getting data from ucam
 *        Add XBee radio power saving: uncomment #def/ine XBEE_POWER_SAVING
 *          power off and wakeup xbee when necessary
 *          if power saving is used, connect the XBee VCC pin to xbee_power_pin = 8
 *          Note: XBee may not be able to receive command if it is put in sleep mode
 *                TODO: periodically wakeup XBee to receive some commands
 *  Fev, 2nd, 2015. v0.7
 *        Add multi-camera support
 *          "C0/1/2" set the current camera for T/S/I commands
 *          EX: use "/@C1#T30#" to select camera 1 and indicate an immediate sending of image
 *              use "/@Q40#C1#T30#" to set quality factor to 40 for camera 1, prior to sending the image
 *        Add led rule for multi-camera:
 *          1/ on initial sync: sync_led (green) will blink 5+cam*3 (0=0,1,2) then capture_led (red) will be on during the sync tries
 *          2/ when cam has synched, sync_led will be on for 1s, then off for 1s, then go to step 1 again. Do this for all cams
 *          3/ on initial reference image: capture_led will blink 5+cam*3 (0=0,1,2) then off for 1s. Then capture_led will be on during the reference image process
 *          4/ after the reference image process, capture_led is off for 3s before going to step 3 for next cam 
 *        Add support for one led per camera to indicate active status
 *  Jan, 15th, 2015. v0.6
 *        Add the new coding scheme from V. Lecuire which performs the packetization on the fly for lower memory needs
 *  Dec, 25th, 2014. v0.5
 *        Add dynamic setting of inter-snapshot time
 *          "F30000#" -> sets inter-snapshot time to 30000ms, i.e. 30s, fps is then 1/30
 *        Add criticality-based scheduling. Specify maximum number of cover-sets, number of cover-sets and criticality level
 *          "CO3#" -> sets the number of cover-sets to 3
 *          "CL8#" -> sets the criticality level to 0.8
 *          "CM8#" -> sets the maximum number of cover-sets to 8        
 *  Dec, 13th, 2014. v0.4
 *        Add short-based computation for the output image, can then divide by 2 the amount of memory used (32768B instead of 65536B)
 *  Dec, 10th, 2014. v0.3
 *        Add simple image difference processing to detect intrusion
 *          "S0#"/"S1#" -> starts a snapshot, make the comparison with the reference image and transmit the image if S1
 *          "I#" -> makes a snapshot and define a new reference image
 *  Dec, 3rd, 2014. v0.2
 *        Stable version with ASCII command to control the image capture and transmission features
 *        Transmission with XBee module (custom baud rate of 125000 is recommended)
 *  Nov, 16th, 2014. v0.1
 *        First test with uCam
*/

// have an XBee attached
//#define XBEE_UCAM

//have a LoRa module attached
#define LORA_UCAM
//#define LORA_LAS

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// change this value for each image sensor
// address will be 0x0004 if UCAM_ADDR is 0x04
// For XBee module: if SRC_ADDR_FROM_64MAC is defined, we instead use the last 16 bits of the 64-bit MAC address 
// For SX1272 LoRa module: this is the address of the image sensor node
// Note that for the display program, the address is converted into 16-bit - 0x0004
///////////////////////////////////////////////////////////////////////////////////////////////////////////
#define UCAM_ADDR                         0x04
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef XBEE_UCAM
    #include <XBee.h>
#elif defined LORA_UCAM
    // Include the SX1272 and SPI library: 
    #include "SX1272.h"
    #include <SPI.h>

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
    char loraPower='M';
    #elif defined SENEGAL_REGULATION
    #define MAX_DBM 10
    // previous way for setting output power
    // 'H' is actually 6dBm, so better to use the new way to set output power
    char loraPower='H';
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
    
    #ifdef LORA_LAS
    #include "LoRaActivitySharing.h"
    #endif
    
    ///////////////////////////////////////////////////////////////////
    // CHANGE HERE THE LORA MODE
    #define LORAMODE  1
    ///////////////////////////////////////////////////////////////////
#endif

#include <uCam.h>
#include "mqc.h"

// BEGIN of compilation #define statements
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#define LOW_POWER
#define LOW_POWER_HIBERNATE
// the 4 following define statements are for test purposes only
//#define RANDOM_NODE_ID
//#define RANDOM_CAM_ID
//#define ENERGY_TEST
//#define ENERGY_TEST_TIMING
// when using 3 camera with reference image, must use CRAN_NEW_CODING otherwise not enough memory
// or set useRefImage to false so that only the initial images are sent
//#define CAM_3
//#define CAM_2
// it is better to use the 4D system software on a Window machine to set focus of the ucamII camera
//#define TEST_FOCUS_CAM0
#define USEREFIMAGE
// use the new image encoding scheme, DO NOT CHANGE
#define CRAN_NEW_CODING
//#define QUALITY_TEST
#define DISPLAY_PKT
//#define DISPLAY_FILLPKT
//#define DISPLAY_BLOCK
//#define DISPLAY_PGM
// get and print stats for image encoding
#define DISPLAY_ENCODE_STATS
// get and print stats for each packet sent
//#define DISPLAY_PACKETIZATION_SEND_STATS
// get and print stats for eack packet packetized
//#define DISPLAY_PACKETIZATION_STATS
// will periodically take a snapshot and transmit the image
//#define PERIODIC_IMG_TRANSMIT
// will periodically take a new reference image
//#define PERIODIC_REF_UPDATE
// this define will enable a snapshot at startup that will be used as reference image
// in this case, the behavior is fully automatic
#define GET_PICTURE_ON_SETUP
// take into account light change between snapshot, recommended.
#define LUM_HISTO
// with periodic sync, not compatible with LOW_POWER
//#define PERIODIC_SYNC
// uses an additional LCD display for more information
//#define WITH_LCD
// actually, it does not change the data read time at all
//#define UCAM_CUSTOM_BAURATE
// criticality-based scheduling, for tests mainly because difficult to have large-scale test-beds
//#define CRITICALITY_SCHEDULING
// will insert the source addr in the image packet
#define WITH_SRC_ADDR, DO NOT CHANGE

// FOR XBEE
#ifdef XBEE_UCAM
// use custom baud rate
#define XBEE_CUSTOM_BAUDRATE
#define SRC_ADDR_FROM_64MAC
//#define XBEE_POWER_SAVING
//#define XBEE_UART0
#endif

// SHORT_COMPUTATION should actually always been defined
////////////////////////////////////////////////////////
#define SHORT_COMPUTATION

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// END of compilation #define statements

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

// BEGIN of constant #define statements
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#define ULONG_MAX                         4294967295UL

#ifdef WITH_SRC_ADDR 
#define PREAMBLE_SIZE            7
#else
#define PREAMBLE_SIZE            5
#endif

#define MIN_PKT_SIZE                      32
#define DEFAULT_MSS                       90

#ifdef LORA_UCAM
    #define MAX_PKT_SIZE                  238
    #define DEFAULT_LORA_MSS              235 //MAX_PKT_SIZE
    #define DEFAULT_QUALITY_FACTOR        10
    #define DEFAULT_LORA_QUALITY_FACTOR   DEFAULT_QUALITY_FACTOR
    #define LORA_SRC_ADDR                 UCAM_ADDR
#endif

#ifdef XBEE_UCAM
    #define MAX_PKT_SIZE                  100-PREAMBLE_SIZE // in bytes, all the XBee radio payload
    #define DEFAULT_QUALITY_FACTOR        50

    #define BROADCAST_MAC_ADDR            "000000000000FFFF"
    #define SHORT_BROADCAST_MAC_ADDR      "FFFF"
    #define FINAL_DEST_MAC_ADDR           "0013A20040762191"

    #define SHORT_ADDR_FROM_64MAC         1
    #define SHORT_ADDR_FFFF               2
    #ifdef XBEE_CUSTOM_BAUDRATE
        // can use 125000 for faster UART operation but need an XBee module configured for this baud rate
        #define MY_XBEE_BAUDRATE          125000
    #else
        #define MY_XBEE_BAUDRATE          38400
    #endif
#endif

#define MIN_INTER_PKT_TIME       10 // in ms
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// if the inter-pkt time is set to MIN_INTER_PKT_TIME, then packets are sent back-to-back, as fast as possible
///////////////////////////////////////////////////////////////////////////////////////////////////////////
#define DEFAULT_INTER_PKT_TIME   MIN_INTER_PKT_TIME+5500
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MIN_INTER_SNAPSHOT_TIME  1712 // in ms

#ifdef PERIODIC_IMG_TRANSMIT
// 1 image / hour
#define DEFAULT_INTER_SNAPSHOT_TIME  3600 // in s
#else
// here it means that image will be sent only on intrusion detection
#define DEFAULT_INTER_SNAPSHOT_TIME  60 // in s
#endif

// BEWARE, we limit the size of the received cmd to avoid strange packets arriving. 
// So avoid to have to long cmd, use multiple cmd is necessary
#define MAX_CMD_LENGTH           60

// maximum of flow id 0->4
#define MAX_FLOW_ID              4

#ifdef UCAM_CUSTOM_BAUDRATE
// can use 921600 for faster UART operation
#define MY_UCAM_BAUDRATE         921600
#else
#define MY_UCAM_BAUDRATE         115200 
#endif

#define MAX_NB_UCAM              3

#ifdef CAM_3
#define NB_UCAM                  3
#else
#ifdef CAM_2
#define NB_UCAM                  2
#else
#define NB_UCAM                  1
#endif
#endif

// the size of data to store one line
#define CAMDATA_PGM_LINE_SIZE     128
#define CAMDATA_LINE_NUMBER       1
#define DISPLAY_CAM_DATA_SIZE     12

// CHANGE HERE THE THRESHOLD TO TUNE THE INTRUSION DETECTION PROCESS
///////////////////////////////////////////////////////////////////////////////////////////////////////////
#define PIX_THRES                 55
#define INTRUSION_THRES           500
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#define DEFAULT_PGM_FILE         "ucam.pgm"

// //////////////////////////////////
// end of constant #define statements

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// low power is only efficient for the Teensy32
///////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef LOW_POWER
// this is for the Teensy36, Teensy35, Teensy31/32 & TeensyLC
// need v6 of Snooze library
#if defined __MK20DX256__ || defined __MKL26Z64__ || defined __MK64FX512__ || defined __MK66FX1M0__
#define LOW_POWER_PERIOD 60 // in s
#include <Snooze.h>
SnoozeTimer timer;
SnoozeBlock sleep_config(timer);
#else
#define LOW_POWER_PERIOD 8 // in s
#endif
unsigned int nCycle = DEFAULT_INTER_SNAPSHOT_TIME/LOW_POWER_PERIOD;
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// XBEE VARIABLES
///////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef XBEE_UCAM
XBee xbee = XBee();

// useful AT commands
// get serial high
uint8_t shCmd[] = {'S','H'};
// get serial low
uint8_t slCmd[] = {'S','L'};
// get ack failures low
uint8_t eaCmd[] = {'E','A'};
// get cca failures low
uint8_t ecCmd[] = {'E','C'};
// get/set mac mode
uint8_t mmCmd[] = {'M','M'};
// get/set MY short address
uint8_t myCmd[] = {'M','Y'};

AtCommandRequest atRequest = AtCommandRequest();
AtCommandResponse atResponse = AtCommandResponse();

XBeeResponse response = XBeeResponse();

// create reusable response objects for responses we expect to handle 
Rx64Response rx64 = Rx64Response();
Rx16Response rx16 = Rx16Response();
TxStatusResponse txStatus = TxStatusResponse();

XBeeAddress64 destAddr64;
uint16_t destAddr16;
int myMacMode=-1;

String macAddr="";
unsigned long inter_binary_pkt=DEFAULT_INTER_PKT_TIME+5;

// need to reserve 1 byte more for the NULL-terminated character
// broadcast by default
//char destMACAddr[17]=BROADCAST_MAC_ADDR;
char destMACAddr[17]=FINAL_DEST_MAC_ADDR;

// if you want to try xbee power saving, connect the VCC pin of the xbee to this digital pin
int xbee_power_pin = 8;

#ifdef XBEE_POWER_SAVING
void wakeup_xbee();
void sleep_xbee();
#endif

unsigned int MSS = DEFAULT_MSS;
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// LORA VARIABLES
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef LORA_UCAM
unsigned long inter_binary_pkt=50;
uint8_t LORA_DEST_ADDR=1;
uint8_t loraAddr=LORA_SRC_ADDR;
uint8_t loraMode=LORAMODE;
uint32_t loraChannelArray[9]={CH_10_868,CH_11_868,CH_12_868,CH_13_868,CH_14_868,CH_15_868,CH_16_868,CH_17_868,CH_18_868};
uint32_t loraChannel=DEFAULT_CHANNEL;
bool loraRadioOn=false;

unsigned long startDoCad, endDoCad;
bool extendedIFS=true;
uint8_t SIFS_cad_number;
// set to 0 to disable carrier sense based on CAD
uint8_t send_cad_number=9;
uint8_t SIFS_value[11]={0, 183, 94, 44, 47, 23, 24, 12, 12, 7, 4};
uint8_t CAD_value[11]={0, 62, 31, 16, 16, 8, 9, 5, 3, 1, 1};
uint8_t carrier_sense_method=2;
bool RSSIonSend=false;

#ifdef LORA_LAS
LASDevice loraLAS(LORA_SRC_ADDR,LAS_DEFAULT_ALPHA,LORA_DEST_ADDR);
#endif

unsigned int MSS = DEFAULT_LORA_MSS;
#endif // LORA_UCAM

uint8_t option = 0;
uint8_t* rcv_data = NULL;
uint8_t* rcv_rawData = NULL;
uint8_t rcv_data_len=0;

#ifdef CRITICALITY_SCHEDULING

#define MAX_HIGH_CRITICALITY_PERIOD 60

uint8_t MAX_COVER_SET = 8;
uint8_t MIN_COVER_SET = 1;
uint8_t DEFAULT_COVER_SET = MIN_COVER_SET;
uint8_t numberOfCoverSets = DEFAULT_COVER_SET;           /////////
float MIN_CRITICALITY_LEVEL = 0.1;
float MAX_CRITICALITY_LEVEL = 0.8;
float DEFAULT_CRITICALITY_LEVEL = 0.2;

// 1 frame every 1.512s + 0.440s=1.952s 
float MAX_CAPTURE_RATE = 0.52;
// 1 frame every 300s=5min
float MIN_CAPTURE_RATE = 0.0033;

float criticalityLevel = DEFAULT_CRITICALITY_LEVEL;
// high criticality period of 15s
unsigned long highCriticalityPeriod=15000L;
unsigned long startHighCriticalityPeriod=0L;
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// multi-purpose buffer for various information display
char banner[100];
// used mainly for the LCD display, typically remind the packet size and the generation frequency
char TGInfoStr[80];

// will wrap at 255
uint8_t nbSentPackets=0;
unsigned long lastSentTime=0;

// use flow id of image mode 0x50
#ifdef WITH_SRC_ADDR 
// the 3rd and 4th byte are the src addr
uint8_t pktPreamble[PREAMBLE_SIZE] = {0xFF,0x50,0x00,UCAM_ADDR,0x00,0x00};
#else
uint8_t pktPreamble[PREAMBLE_SIZE] = {0xFF,0x50,0x00,0x00,0x00};
#endif
uint8_t flowId = 0x00;

boolean have_short_address=false;
// default behavior is to use framing bytes
boolean with_framing_bytes=true;

#ifdef __MK20DX256__
UCAM529 myCam(&Serial1, UCAM_RAW, UCAM_RAW_128_128);
// will store the ucam pointers to serial port
// but will only use up to NB_CAM
HardwareSerial *ucamArray[MAX_NB_UCAM]={&Serial1, &Serial1, &Serial1};
#else
UCAM529 myCam(&Serial1, UCAM_RAW, UCAM_RAW_128_128);
// will store the ucam pointers to serial port
// but will only use up to NB_CAM
HardwareSerial *ucamArray[MAX_NB_UCAM]={&Serial1, &Serial2, &Serial3};
#endif

uint8_t currentCam=0;
#ifdef USEREFIMAGE
// if true, try to allocate memory space to store the reference image, if not enough memory then stop
boolean useRefImage=true;
#else
boolean useRefImage=false;
#endif

#ifdef __MK20DX256__
// change your pin assignment if necessary
int capture_led = 18;
int cam_sync_led = 19;
// change your pin assignment if necessary
// pin 15 for ucam0, pin 16 for ucam1 and pin 17 for ucam2
uint8_t ucamLedArray[MAX_NB_UCAM]={15, 16, 17};
#else
// change your pin assignment if necessary
int capture_led = 44;
int cam_sync_led = 45;
// change your pin assignment if necessary
// pin 30 for ucam0, pin 31 for ucam1 and pin 32 for ucam2
uint8_t ucamLedArray[MAX_NB_UCAM]={30, 31, 32};
#endif

// when run_test is true, we encode twice, but the first time without transmission to know the time for only encoding, then the time for encoding and transmission 
boolean run_test=false;
// when useCriticalityScheduling is true and CRITICALITY_SCHEDULING is defined, then the node will compute the capture rate
// according to the criticality level and its number of cover-sets and then set the inter-scnapshot time accordingly
boolean useCriticalityScheduling=false;
boolean send_image_on_intrusion=true;
boolean camDataReady=false;
boolean camHasSync=false;
boolean transmitting_data=false;
boolean new_ref_on_intrusion=true;

// for the incoming data from uCam
InImageStruct inImage;
// for the reference images
InImageStruct refImage[NB_UCAM];

#ifdef CRAN_NEW_CODING
InImageStruct outImage;
#else
OutImageStruct outImage;
#endif

int nbIntrusion=0;
bool detectedIntrusion=false;

#if defined RANDOM_NODE_ID || defined RANDOM_CAM_ID 
uint8_t randNodeId;
uint8_t randCamId;
#endif

#ifdef LUM_HISTO
// to store the image histogram
short histoRefImage[NB_UCAM][255];
short histoInImage[255];
long refImageLuminosity[NB_UCAM];
long inImageLuminosity;
#endif

unsigned long totalPacketizationTime=0;

unsigned long lastCamSyncTime=0;
unsigned long lastCamRefImageTime=0;
unsigned long lastCamIntrusionDetectionTime=0;

// 12s between 2 cam sync
unsigned long interCamSyncTime=12000L;
// 5min between 2 new reference image
unsigned long interCamRefImageTime=300000L;
// 30s between 2 intrusion detection. Can be linked to our criticality-based scheduling
unsigned long interCamIntrusionDetectionTime=DEFAULT_INTER_SNAPSHOT_TIME*1000; // in ms
//unsigned long interCamIntrusionDetectionTime=50000L;

#ifdef WITH_LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x20,16,2);  // set the LCD address to 0x20 for a 16 chars and 2 line display
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////////////
// IMAGE ENCODING METHOD FROM CRAN LABORATORY
///////////////////////////////////////////////////////////////////////////////////////////////////////////

struct position { uint8_t row;  uint8_t col;
  } ZigzagCoordinates[8*8]= // Matrice Zig-Zag
  {0, 0, 0, 1, 1, 0, 2, 0, 1, 1, 0, 2, 0, 3, 1, 2, 2, 1, 3, 0,
   4, 0, 3, 1, 2, 2, 1, 3, 0, 4, 0, 5, 1, 4, 2, 3, 3, 2, 4, 1,
   5, 0, 6, 0, 5, 1, 4, 2, 3, 3, 2, 4, 1, 5, 0, 6, 0, 7, 1, 6,
   2, 5, 3, 4, 4, 3, 5, 2, 6, 1, 7, 0, 7, 1, 6, 2, 5, 3, 4, 4,
   3, 5, 2, 6, 1, 7, 2, 7, 3, 6, 4, 5, 5, 4, 6, 3, 7, 2, 7, 3,
   6, 4, 5, 5, 4, 6, 3, 7, 4, 7, 5, 6, 6, 5, 7, 4, 7, 5, 6, 6,
   5, 7, 6, 7, 7, 6, 7, 7};
   
unsigned int packetcount = 0; 
long count = 0L;
unsigned int QualityFactor[NB_UCAM];

float **AllocateFloatMemSpace(int Horizontal, int Vertical)
{
	float **Array;
	
	if ((Array = (float **) calloc(Vertical,sizeof(float*)))==NULL) return NULL;

	for (int i = 0; i < Vertical; i++) {
		Array[i] = (float *) calloc(Horizontal,sizeof(float));
                if (Array[i]==NULL)
                    return NULL;
        }

	return Array;
}

uint8_t **AllocateUintMemSpace(int Horizontal, int Vertical)
{
	uint8_t **Array;
	
	if ((Array = (uint8_t **) calloc(Vertical,sizeof(uint8_t*)))==NULL) return NULL;

	for (int i = 0; i < Vertical; i++) {
		Array[i] = (uint8_t *) calloc(Horizontal,sizeof(uint8_t));
                if (Array[i]==NULL)
                    return NULL;
        }

	return Array;
}	

#ifdef SHORT_COMPUTATION
short **AllocateShortMemSpace(int Horizontal, int Vertical)
{
	short **Array;
	
	if ((Array = (short **) calloc(Vertical,sizeof(short*)))==NULL) return NULL;

	for (int i = 0; i < Vertical; i++) {
		Array[i] = (short *) calloc(Horizontal,sizeof(short));
                if (Array[i]==NULL)
                    return NULL;
        }

	return Array;
}	
#endif


#ifdef LORA_UCAM

void CarrierSense0() {
  // this carrier sense perform nothing -> pure ALOHA
}

// we could use the CarrierSense function added in the SX1272 library, but it is more convenient to duplicate it here
// so that we could easily modify it for testing
void CarrierSense1() {
  int e;
  bool carrierSenseRetry=false;
  uint8_t n_collision=0;
  
  PRINT_CSTSTR("%s","--> CarrierSense1: do CAD for DIFS=9CAD");
  PRINTLN;  

  if (send_cad_number) {
    do { 
      do {
        
        // check for free channel (SIFS/DIFS)        
        startDoCad=millis();
        e = sx1272.doCAD(send_cad_number);
        endDoCad=millis();
        
        PRINT_CSTSTR("%s","--> SIFS/DIFS duration ");
        PRINT_VALUE("%ld",endDoCad-startDoCad);
        PRINTLN;
        
        if (!e) {
          PRINT_CSTSTR("%s","OK1\n");

          extendedIFS=true;
          
          if (extendedIFS)  {          
            // wait for random number of CAD         
            uint8_t w = random(1,8);

            PRINT_CSTSTR("%s","--> extended wait for ");
            PRINT_VALUE("%d",w);
            PRINT_CSTSTR("%s"," CAD = ");
            PRINT_VALUE("%d",CAD_value[loraMode]*w);
            PRINTLN;
            
            delay(CAD_value[loraMode]*w);
            
            // check for free channel (SIFS/DIFS) once again
            startDoCad=millis();
            e = sx1272.doCAD(send_cad_number);
            endDoCad=millis();
 
            PRINT_CSTSTR("%s","--> SIFS/DIFS duration ");
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
          n_collision++;
          PRINT_CSTSTR("%s","###");  
          PRINT_VALUE("%d",n_collision);
          PRINTLN;

          // wait for random number of DIFS               
          uint8_t w = random(1,8);
       
          PRINT_CSTSTR("%s","--> Channel busy. Wait for ");
          PRINT_VALUE("%d",w);
          PRINT_CSTSTR("%s"," DIFS (DIFS=3SIFS) = ");
          PRINT_VALUE("%d",SIFS_value[loraMode]*3*w);
          PRINTLN;
          
          delay(SIFS_value[loraMode]*3*w);
          
          // to perform a new DIFS          
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

void CarrierSense2() {
  int e;
  bool carrierSenseRetry=false;  
  uint8_t n_collision=0;

  PRINT_CSTSTR("%s","--> CarrierSense2: do CAD for DIFS=9CAD");
  PRINTLN;  
  
  if (send_cad_number) {
    do { 
      do {
        
        // check for free channel (SIFS/DIFS)        
        startDoCad=millis();
        e = sx1272.doCAD(send_cad_number);
        endDoCad=millis();
        
        PRINT_CSTSTR("%s","--> DIFS duration ");
        PRINT_VALUE("%ld",endDoCad-startDoCad);
        PRINTLN;

        // successull SIFS/DIFS
        if (!e) {

          // previous collision detected
          if (n_collision) {
                
              PRINT_CSTSTR("%s","--> counting for ");
              // count for random number of CAD/SIFS/DIFS?   
              // SIFS=3CAD
              // DIFS=9CAD            
              uint8_t w = random(3,24);

              PRINT_VALUE("%d", w);
              PRINTLN;         
                   
              int busyCount=0;
              bool nowBusy=false;
              
              do {

                  if (nowBusy)
                    e = sx1272.doCAD(send_cad_number);
                  else
                    e = sx1272.doCAD(1);

                  if (nowBusy && e) {
                    PRINT_CSTSTR("%s","#");
                    busyCount++;                    
                  }
                  else
                  if (nowBusy && !e) {
                    PRINT_CSTSTR("%s","|");
                    nowBusy=false;                    
                  }                  
                  else
                  if (!e) {
                    w--;
                    PRINT_CSTSTR("%s","-");
                  }  
                  else {
                    PRINT_CSTSTR("%s","*");
                    nowBusy=true;
                    busyCount++;  
                  }
                  
              } while (w);   
              
              // if w==0 then we exit and 
              // the packet will be sent  
              PRINTLN;
              PRINT_CSTSTR("%s","--> found busy during ");
              PRINT_VALUE("%d", busyCount);
              PRINTLN;              
          }
          else {
              PRINT_CSTSTR("%s","OK1\n");
    
              extendedIFS=false;
              
              if (extendedIFS)  {          
                // wait for random number of CAD         
                uint8_t w = random(1,8);
    
                PRINT_CSTSTR("%s","--> extended waiting for ");
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
        }
        else {
          n_collision++;
          PRINT_CSTSTR("%s","###");  
          PRINT_VALUE("%d",n_collision);
          PRINTLN;
          
          PRINT_CSTSTR("%s","--> Channel busy. Waiting until free channel\n");

          startDoCad=millis();
          do {

            PRINT_CSTSTR("%s","R");

            e = sx1272.doCAD(1);
                         
          } while (e);

          endDoCad=millis();

          PRINTLN;
          PRINT_CSTSTR("%s","--> wait duration ");
          PRINT_VALUE("%ld",endDoCad-startDoCad);
          PRINTLN;

          // to perform a new DIFS
          PRINT_CSTSTR("%s","--> retry\n");
          e=1;
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

void CarrierSense3() {
  int e;
  bool carrierSenseRetry=false;
  uint8_t n_collision=0;
  uint8_t n_cad=9;
  
  uint32_t max_toa = sx1272.getToA(255);

  //unsigned long end_carrier_sense=0;
  
  if (send_cad_number) {
    do { 

      PRINT_CSTSTR("%s","--> CarrierSense3: do CAD for MaxToa=");
      PRINT_VALUE("%ld", max_toa);
      PRINTLN;  
        
      //end_carrier_sense=millis()+(max_toa/n_cad)*(n_cad-1);
      
      for (int i=0; i<n_cad; i++) {      
        startDoCad=millis();
        e = sx1272.doCAD(1);
        endDoCad=millis();

        if (!e) {
          PRINT_VALUE("%ld", endDoCad);
          PRINT_CSTSTR("%s"," 0 ");
          PRINT_VALUE("%d", sx1272._RSSI);
          PRINT_CSTSTR("%s"," ");
          PRINT_VALUE("%ld", endDoCad-startDoCad);
          PRINTLN;
        }
        else
          continue;
          
        // wait in order to have n_cad CAD operations during max_toa
        delay(max_toa/(n_cad-1)-(millis()-startDoCad));
      }

      if (e) {
        n_collision++;
        PRINT_CSTSTR("%s","###");  
        PRINT_VALUE("%d",n_collision);
        PRINTLN;
                  
        PRINT_CSTSTR("%s","Channel busy. Wait for MaxToA=");
        PRINT_VALUE("%ld", max_toa);
        PRINTLN;        
        delay(max_toa);
        // to perform a new max_toa waiting
        PRINT_CSTSTR("%s","--> retry\n");        
        carrierSenseRetry=true;
      }
      else
        carrierSenseRetry=false;
      
    } while (carrierSenseRetry);  
  }
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW CRAN ENCODING, WITH PACKET CREATION ON THE FLY
///////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef CRAN_NEW_CODING

float CordicLoefflerScalingFactor[8]={0.35355339, 0.35355339, 0.31551713, 0.5, 0.35355339, 0.5, 0.31551713, 0.35355339};

short OriginalLuminanceJPEGTable[8][8] = {
  16,  11,  10,  16,  24,  40,  51,  61,
  12,  12,  14,  19,  26,  58,  60,  55,
  14,  13,  16,  24,  40,  57,  69,  56,
  14,  17,  22,  29,  51,  87,  80,  62,
  18,  22,  37,  56,  68, 109, 103,  77,
  24,  35,  55,  64,  81, 104, 113,  92,
  49,  64,  78,  87, 103, 121, 120, 101,
  72,  92,  95,  98, 112, 100, 103,  99
};

short LuminanceJPEGTable[8][8] = {
  16,  11,  10,  16,  24,  40,  51,  61,
  12,  12,  14,  19,  26,  58,  60,  55,
  14,  13,  16,  24,  40,  57,  69,  56,
  14,  17,  22,  29,  51,  87,  80,  62,
  18,  22,  37,  56,  68, 109, 103,  77,
  24,  35,  55,  64,  81, 104, 113,  92,
  49,  64,  78,  87, 103, 121, 120, 101,
  72,  92,  95,  98, 112, 100, 103,  99
};

opj_mqc_t mqobjet, mqbckobjet, *objet=NULL;
uint8_t buffer[MQC_NUMCTXS], bckbuffer[MQC_NUMCTXS];
uint8_t packet[MQC_NUMCTXS];
int packetsize, packetoffset, buffersize;

void QTinitialization(int Quality)
{
 float Qs, scale;

 if (Quality <= 0)  Quality = 1;
 if (Quality > 100) Quality = 100;
 if (Quality < 50)   Qs = 50.0 / (float) Quality;
 	else	     Qs = 2.0 - (float) Quality/50.0;

 // Calcul des coefficients de la table de quantification
 for (int u=0; u<8; u++)
   for (int v=0; v<8; v++)
	{
	 scale = (float) OriginalLuminanceJPEGTable[u][v] * Qs;

	 if (scale < 1.0) scale=1.0;

	 LuminanceJPEGTable[u][v] = (short) round (scale / (CordicLoefflerScalingFactor[u]*CordicLoefflerScalingFactor[v]));
	}

 return;
}

void JPEGencoding(int Block[8][8])
{
 int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
 int tmp10, tmp11, tmp12, tmp13, tmp20, tmp23;
 int z11, z12, z21, z22;
 
    // On calcule la DCT, puis on quantifie
    for (int u=0; u<8; u++)
      {
       tmp0=Block[u][0]+Block[u][7];
       tmp7=Block[u][0]-Block[u][7];
       tmp1=Block[u][1]+Block[u][6];
       tmp6=Block[u][1]-Block[u][6];
       tmp2=Block[u][2]+Block[u][5];
       tmp5=Block[u][2]-Block[u][5];
       tmp3=Block[u][3]+Block[u][4];
       tmp4=Block[u][3]-Block[u][4];
       
       tmp10=tmp0+tmp3;
       tmp13=tmp0-tmp3;
       tmp11=tmp1+tmp2;
       tmp12=tmp1-tmp2;

       Block[u][0]=tmp10+tmp11;
       Block[u][4]=tmp10-tmp11;
       z11=tmp13+tmp12;	
       z12=tmp13-tmp12;
       z21=z11+(z12>>1);
       z22=z12-(z11>>1);
       Block[u][2]=z21-(z22>>4);
       Block[u][6]=z22+(z21>>4);

       z11=tmp4+(tmp7>>1);
       z12=tmp7-(tmp4>>1);
       z21=z11+(z12>>3);
       z22=z12-(z11>>3);
       z21=z21-(z21>>3);
       z22=z22-(z22>>3);
       tmp10=z21+(z21>>6);
       tmp13=z22+(z22>>6);
       z11=tmp5+(tmp6>>3);
       z12=tmp6-(tmp5>>3);
       tmp11=z11+(z12>>4);
       tmp12=z12-(z11>>4);

       tmp20=tmp10+tmp12;
       Block[u][5]=tmp10-tmp12;
       tmp23=tmp13+tmp11;
       Block[u][3]=tmp13-tmp11;
       Block[u][1]=tmp23+tmp20;
       Block[u][7]=tmp23-tmp20;
    }		

    // On attaque ensuite colonne par colonne
    for (int v=0; v<8; v++)
      {
       // 1Ëre Ètape
       tmp0= Block[0][v]+Block[7][v];
       tmp7= Block[0][v]-Block[7][v];
       tmp1= Block[1][v]+Block[6][v];
       tmp6= Block[1][v]-Block[6][v];
       tmp2= Block[2][v]+Block[5][v];
       tmp5= Block[2][v]-Block[5][v];
       tmp3= Block[3][v]+Block[4][v];
       tmp4= Block[3][v]-Block[4][v];

       tmp10=tmp0+tmp3;
       tmp13=tmp0-tmp3;
       tmp11=tmp1+tmp2;
       tmp12=tmp1-tmp2;

       Block[0][v]=tmp10+tmp11;
       Block[4][v]=tmp10-tmp11;
       z11=tmp13+tmp12;	
       z12=tmp13-tmp12;
       z21=z11+(z12>>1);
       z22=z12-(z11>>1);
       Block[2][v]=z21-(z22>>4);
       Block[6][v]=z22+(z21>>4);

       z11=tmp4+(tmp7>>1);
       z12=tmp7-(tmp4>>1);
       z21=z11+(z12>>3);
       z22=z12-(z11>>3);
       z21=z21-(z21>>3);
       z22=z22-(z22>>3);

       tmp10=z21+(z21>>6);
       tmp13=z22+(z22>>6);
       z11=tmp5+(tmp6>>3);
       z12=tmp6-(tmp5>>3);
       tmp11=z11+(z12>>4);
       tmp12=z12-(z11>>4);

       tmp20=tmp10+tmp12;
       Block[5][v]=tmp10-tmp12;
       tmp23=tmp13+tmp11;
       Block[3][v]=tmp13-tmp11;
       Block[1][v]=tmp23+tmp20;
       Block[7][v]=tmp23-tmp20;
    }

    // on centre sur l'interval [-128, 127]
    Block[0][0]-=8192;

    // Quantification
#ifdef DISPLAY_BLOCK
    Serial.println(F("JPEGencoding:"));
    
    for (int u=0; u<8; u++) {
       for (int v=0; v<8; v++) {
          Block[u][v] = (int)round((float)Block[u][v] / (float)LuminanceJPEGTable[u][v]);

          Serial.print(Block[u][v]);
          Serial.print(F("\t"));
       }   
       Serial.println("");
    }   
#else

    for (int u=0; u<8; u++)
       for (int v=0; v<8; v++)
	   Block[u][v] = (int)round((float)Block[u][v] / (float)LuminanceJPEGTable[u][v]);

#endif

   return;
}

void CreateNewPacket(unsigned int BlockOffset)
{
   // On initialise le codeur MQ
   objet=&mqobjet;
   for (int x=0; x< MQC_NUMCTXS; x++) 
     buffer[x]=0;

   mqc_init_enc(objet, buffer);
   mqc_resetstates(objet);
   packetoffset = BlockOffset;
   packetsize = 0;
   mqc_backup(objet, &mqbckobjet, bckbuffer);
   mqc_flush(objet);
}

void SendPacket()
{
   if (packetsize == 0) 
     return;

#ifdef DISPLAY_PKT     
   Serial.print(F("00"));
   Serial.print(packetsize + 2, HEX);
   Serial.print(F(" "));
   Serial.print(F("00 "));
   if (packetoffset<0x10)
       Serial.print(F("0"));
   Serial.println(packetoffset, HEX);
   
   for (int x=0; x<packetsize; x++) {
        Serial.print(F(" "));
        
        if (packet[x]<0x10)
            Serial.print(F("0"));
   	
        Serial.print(packet[x], HEX);
   }
   Serial.println(F(" "));
#endif  

   if (transmitting_data) {
     
        digitalWrite(capture_led, LOW);
        // here we transmit data  
        /*
        Serial.print(F("LS "));
        Serial.print(lastSentTime);
        Serial.print(F(" CT "));
        Serial.println(millis());
        */
        
        if (inter_binary_pkt!=MIN_INTER_PKT_TIME)
            while ( (millis()-lastSentTime) < inter_binary_pkt)
              ; 
              
        // just the maximum pkt size plus some more bytes
#ifdef LORA_UCAM
        uint8_t myBuff[260]; 
#else
        uint8_t myBuff[110];      
#endif            
        uint8_t dataSize=0;
        long startSendTime, stopSendTime, previousLastSendTime;
        
        if (with_framing_bytes) {
                myBuff[0]=pktPreamble[0];
#ifdef RANDOM_CAM_ID
                myBuff[1]=0x50+randCamId;
#else
                myBuff[1]=0x50+currentCam;
#endif 

#ifdef WITH_SRC_ADDR
#ifdef RANDOM_NODE_ID
                // set the src addr
                myBuff[2]=0;
                myBuff[3]=randNodeId;
#else
                myBuff[2]=pktPreamble[2];
                myBuff[3]=pktPreamble[3];
#endif                
                // set the sequence number
                myBuff[4] = (uint8_t)packetcount;
                // set the Quality Factor
                myBuff[5] = (uint8_t)QualityFactor[currentCam];
                // set the packet size
                myBuff[6] = (uint8_t)(packetsize+2);
#else
                // set the sequence number
                myBuff[2] = (uint8_t)packetcount;
                // set the Quality Factor
                myBuff[3] = (uint8_t)QualityFactor[currentCam];
                // set the packet size
                myBuff[4] = (uint8_t)(packetsize+2);
#endif 
                dataSize += sizeof(pktPreamble);
        }
#ifdef DISPLAY_PKT         
        Serial.print(F("Building packet : "));
        Serial.println(packetcount);
#endif        
        // high part
        myBuff[dataSize++]=packetoffset >> 8 & 0xff; 
        // low part
        myBuff[dataSize++]=packetoffset & 0xff;
        
        for (int x=0; x<packetsize; x++) {
            myBuff[dataSize+x]=(uint8_t)packet[x];
        }

#ifdef DISPLAY_PKT
        for (int x=0; x<dataSize+packetsize; x++) {
                if (myBuff[x]<0x10)
                    Serial.print(F("0"));
            Serial.print(myBuff[x], HEX);
        }
        Serial.println("");

        Serial.print(F("Sending packet : "));
        Serial.println(packetcount);
#endif        
        
        previousLastSendTime=lastSentTime;
        
        startSendTime=millis();
                
        // we set lastSendTime before the call to the sending procedure in order to be able to send packets back to back since 
        // the sending procedure can introduce a delay 
        lastSentTime=(unsigned long)startSendTime;

#ifdef XBEE_UCAM        
        // 16-bit destination address?        
        if (strlen(destMACAddr)==4) {
                // set the frame id to the pkt sequence number to keep track of ACK failures when mac mode is 0/2 with unicast address
                //Tx16Request tx = Tx16Request(destAddr16, ACK_OPTION, myBuff, dataSize+packetsize, nbSentPackets);
                Tx16Request tx = Tx16Request(destAddr16, myBuff, dataSize+packetsize);
                // send the frame
                xbee.send(tx); 
        }
        else {
                // set the frame id to the pkt sequence number to keep track of ACK failures when mac mode is 0/2 with unicast address
                //Tx64Request tx = Tx64Request(destAddr64, ACK_OPTION, myBuff, dataSize+packetsize, nbSentPackets);  
                Tx64Request tx = Tx64Request(destAddr64, myBuff, dataSize+packetsize);   
                // send the frame
                xbee.send(tx);    
        }  
#endif        

#ifdef LORA_UCAM
        int sx1272_return_code;

        //sx1272.CarrierSense();
        if (carrier_sense_method==1)
            CarrierSense1();
  
        if (carrier_sense_method==2)
            CarrierSense2();

        if (carrier_sense_method==3)
            CarrierSense3();  
                      
        sx1272.setPacketType(PKT_TYPE_DATA);
        sx1272_return_code=sx1272.sendPacketTimeout(LORA_DEST_ADDR, myBuff, dataSize+packetsize, 10000);
#endif    
        stopSendTime=millis();

#ifndef ENERGY_TEST        
        digitalWrite(capture_led, HIGH);
#endif
        nbSentPackets++;

#ifdef DISPLAY_PKT     

        Serial.print(F("Packet Sent in "));
        Serial.println(stopSendTime-startSendTime);

#ifdef LORA_UCAM
        Serial.print(F("LoRa Sent w/CAD in "));
        Serial.println(stopSendTime-sx1272._startDoCad);

        Serial.print(F("LoRa sending: state = "));
        Serial.println(sx1272_return_code);
#endif    

#ifdef XBEE_UCAM
        // read incoming pkt, to get the transmit status, for 5ms max
        xbee.readPacket(5);
        
        if (xbee.getResponse().isAvailable()) {
  
              // is it a response to the previously sent packet?            	
	      if (xbee.getResponse().getApiId() == TX_STATUS_RESPONSE) {
                      
	            xbee.getResponse().getTxStatusResponse(txStatus);	
  
                    if (txStatus.isSuccess()) {
                          Serial.print(F(" Success "));        
                    }                        
                    else {                     
                          Serial.print(F(" Error: 0x"));
                         
                          Serial.print(txStatus.getStatus(), HEX);
                          Serial.print(F(". Pkt "));
                          // get back the frame id
                          Serial.print(txStatus.getFrameData()[0]);

                          Serial.print(F(". "));     
                    }
              }
              else
                    Serial.print(F(" Not a TX status "));  
        }
        else
              Serial.print(F(" Timeout "));       
#endif  
#endif

#ifdef DISPLAY_PACKETIZATION_SEND_STATS
        Serial.print(packetsize);
        Serial.print(" ");  
        Serial.println(stopSendTime-startSendTime);
        //Serial.println("ms. ");  
#endif        
   }   

   count += packetsize;
   packetcount++;
}

int FillPacket(int Block[8][8], boolean *full)
{
   unsigned int index, q, r, K;

   mqc_restore(objet, &mqbckobjet, bckbuffer);
   
   long startFillPacket=millis();
   
   // On cherche où se trouve le dernier coef <> 0 selon le zig-zag
   K=63;   
   
   while ((Block[ZigzagCoordinates[K].row][ZigzagCoordinates[K].col]==0) && (K>0)) 
     K--;
   
   K++;

   // On code la valeur de K, nombre de coefs encodé dans le bloc
   q=K / 2;	
   r=K % 2;
   
   for (int x=0; x<q; x++) 
     mqc_encode(objet, 1);
     
   mqc_encode(objet, 0);
   mqc_encode(objet, r);

   // On code chaque coef significatif par Golomb-Rice puis par MQ
   for (int x=0; x<K; x++) {

      if (Block[ZigzagCoordinates[x].row][ZigzagCoordinates[x].col]>=0) { 
        index=2*Block[ZigzagCoordinates[x].row][ZigzagCoordinates[x].col]; 
      }
      else { 
        index=2*abs(Block[ZigzagCoordinates[x].row][ZigzagCoordinates[x].col])-1; 
      }

      // Golomb
      q=index / 2;
      r=index % 2;
      
      for (int x=0; x<q; x++) 
        mqc_encode(objet, 1);

      mqc_encode(objet, 0);
      mqc_encode(objet, r);
   } 

   // On regarde si le paquet est plein
   mqc_backup(objet, &mqbckobjet, bckbuffer);
   mqc_flush(objet);
   buffersize=mqc_numbytes(objet);
   
   // On déborde (il faut tenir compte du champ offset (2 octets) dans le paquet
   if (buffersize > (MSS-2)) {
     totalPacketizationTime+=millis()-startFillPacket;
     return -1;  
   } 

#ifdef DISPLAY_FILLPKT
   Serial.println(F("filling pkt\n"));
#endif   
   packetsize = buffersize;
   
   for (int x=0; x<packetsize; x++) {
#ifdef DISPLAY_FILLPKT
     if (buffer[x]<0x10)
       Serial.print(F("0"));
            
     Serial.print(buffer[x], HEX);  
     Serial.print(F(" "));
#endif       
     packet[x]=buffer[x];
   }

#ifdef DISPLAY_FILLPKT   
   Serial.println("");
#endif 

   if (buffersize < (MSS - 6)) {  
     *full = false; 
   }  
   else { 
     *full = true; 
   }

   totalPacketizationTime+=millis()-startFillPacket;
   
   return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// OLD CRAN ENCODING, NEED MORE MEMORY
///////////////////////////////////////////////////////////////////////////////////////////////////////////
#else 

#define a4   1.38703984532215
#define a7  -0.275899379282943
#define a47  0.831469612302545
#define a5   1.17587560241936
#define a6  -0.785694958387102
#define a56  0.98078528040323
#define a2   1.84775906502257
#define a3   0.765366864730179
#define a23  0.541196100146197
#define a32  1.306562964876376
#define rc2  1.414213562373095

float OriginalLuminanceJPEGTable[8][8] = {
  16.0,  11.0,  10.0,  16.0,  24.0,  40.0,  51.0,  61.0,
  12.0,  12.0,  14.0,  19.0,  26.0,  58.0,  60.0,  55.0,
  14.0,  13.0,  16.0,  24.0,  40.0,  57.0,  69.0,  56.0,
  14.0,  17.0,  22.0,  29.0,  51.0,  87.0,  80.0,  62.0,
  18.0,  22.0,  37.0,  56.0,  68.0, 109.0, 103.0,  77.0,
  24.0,  35.0,  55.0,  64.0,  81.0, 104.0, 113.0,  92.0,
  49.0,  64.0,  78.0,  87.0, 103.0, 121.0, 120.0, 101.0,
  72.0,  92.0,  95.0,  98.0, 112.0, 100.0, 103.0,  99.0
};

float LuminanceJPEGTable[8][8] = {
  16.0,  11.0,  10.0,  16.0,  24.0,  40.0,  51.0,  61.0,
  12.0,  12.0,  14.0,  19.0,  26.0,  58.0,  60.0,  55.0,
  14.0,  13.0,  16.0,  24.0,  40.0,  57.0,  69.0,  56.0,
  14.0,  17.0,  22.0,  29.0,  51.0,  87.0,  80.0,  62.0,
  18.0,  22.0,  37.0,  56.0,  68.0, 109.0, 103.0,  77.0,
  24.0,  35.0,  55.0,  64.0,  81.0, 104.0, 113.0,  92.0,
  49.0,  64.0,  78.0,  87.0, 103.0, 121.0, 120.0, 101.0,
  72.0,  92.0,  95.0,  98.0, 112.0, 100.0, 103.0,  99.0
};

void QTinitialization(int Quality)
{
 float Qs;

 if (Quality <= 0)  Quality = 1;
 if (Quality > 100) Quality = 100;
 if (Quality < 50)   Qs = 50.0 / (float) Quality;
 	else	     Qs = 2.0 - (float) Quality/50.0;


 // Calcul des coefficients de la table de quantification
 for (int u=0; u<8; u++)
   for (int v=0; v<8; v++)
	{
	 LuminanceJPEGTable[u][v] = OriginalLuminanceJPEGTable[u][v] * Qs;
	 if (LuminanceJPEGTable[u][v] < 1.0) LuminanceJPEGTable[u][v]=1.0;
	 if (LuminanceJPEGTable[u][v] > 255.0) LuminanceJPEGTable[u][v]=255.0;
	}

 return;
}

void JPEGencoding(InImageStruct *InputImage , OutImageStruct *OutputImage)
{
   float Block[8][8];
   float tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
   float tmp10, tmp11, tmp12, tmp13, tmp20, tmp23;
   float tmp;

   // Encodage bloc par bloc
   for (int i=0; i<InputImage->imageVsize; i=i+8)
     for (int j=0; j<InputImage->imageHsize; j=j+8) {
       
	  for (int u=0; u<8; u++)
	   for (int v=0; v<8; v++) Block[u][v]=InputImage->data[i+u][j+v];

          // On calcule la DCT, puis on quantifie
          for (int u=0; u<8; u++)
            {
             // 1ère étape
             tmp0=Block[u][0]+Block[u][7];
             tmp7=Block[u][0]-Block[u][7];
             tmp1=Block[u][1]+Block[u][6];
             tmp6=Block[u][1]-Block[u][6];		// soit 8 ADD
             tmp2=Block[u][2]+Block[u][5];
             tmp5=Block[u][2]-Block[u][5];
             tmp3=Block[u][3]+Block[u][4];
             tmp4=Block[u][3]-Block[u][4];
             // 2ème étape: Partie paire
             tmp10=tmp0+tmp3;
             tmp13=tmp0-tmp3;			// soit 4 ADD
             tmp11=tmp1+tmp2;
             tmp12=tmp1-tmp2;
             // 3ème étape: Partie paire
             Block[u][0]=tmp10+tmp11;
             Block[u][4]=tmp10-tmp11;
             tmp=(tmp12+tmp13)*a23;		// soit 3 MULT et 5 ADD
             Block[u][2]=tmp+(a3*tmp13);
             Block[u][6]=tmp-(a2*tmp12);
             // 2ème étape: Partie impaire
             tmp=(tmp4+tmp7)*a47;
             tmp10=tmp+(a7*tmp7);
             tmp13=tmp-(a4*tmp4);
             tmp=(tmp5+tmp6)*a56;		// soit 6 MULT et 6 ADD
             tmp11=tmp+(a6*tmp6);
             tmp12=tmp-(a5*tmp5);
             // 3ème étape: Partie impaire
             tmp20=tmp10+tmp12;
             tmp23=tmp13+tmp11;
             Block[u][7]=tmp23-tmp20;		// soit 2 MULT et 6 ADD
             Block[u][1]=tmp23+tmp20;
             Block[u][3]=(tmp13-tmp11)*rc2;
             Block[u][5]=(tmp10-tmp12)*rc2;
            }                

          for (int v=0; v<8; v++)
            {
             // 1ère étape
             tmp0=Block[0][v]+Block[7][v];
             tmp1=Block[1][v]+Block[6][v];
             tmp2=Block[2][v]+Block[5][v];
             tmp3=Block[3][v]+Block[4][v];
             tmp4=Block[3][v]-Block[4][v];
             tmp5=Block[2][v]-Block[5][v];
             tmp6=Block[1][v]-Block[6][v];
             tmp7=Block[0][v]-Block[7][v];
             // 2ème étape: Partie paire
             tmp10=tmp0+tmp3;
             tmp13=tmp0-tmp3;
             tmp11=tmp1+tmp2;
             tmp12=tmp1-tmp2;
             // 3ème étape: Partie paire
             Block[0][v]=tmp10+tmp11;
             Block[4][v]=tmp10-tmp11;
             tmp=(tmp12+tmp13)*a23;
             Block[2][v]=tmp+(a3*tmp13);
             Block[6][v]=tmp-(a2*tmp12);
             // 2ème étape: Partie impaire
             tmp=(tmp4+tmp7)*a47;
             tmp10=tmp+(a7*tmp7);
             tmp13=tmp-(a4*tmp4);
             tmp=(tmp5+tmp6)*a56;
             tmp11=tmp+(a6*tmp6);
             tmp12=tmp-(a5*tmp5);
             // 3ème étape: Partie impaire
             tmp20=tmp10+tmp12;
             tmp23=tmp13+tmp11;
             Block[7][v]=tmp23-tmp20;
             Block[1][v]=tmp23+tmp20;
             Block[3][v]=(tmp13-tmp11)*rc2;
             Block[5][v]=(tmp10-tmp12)*rc2;
            }

          // on centre sur l'interval [-128, 127]
          Block[0][0]-=8192.0;
      
          // Quantification
          for (int u=0; u<8; u++)
             for (int v=0; v<8; v++)
      	       Block[u][v] = round(Block[u][v] / (LuminanceJPEGTable[u][v] * 8.0));
          
          // On range le résultat dans l'image de sortie
      	  for (int u=0; u<8; u++)
      	     for (int v=0; v<8; v++)
#ifdef SHORT_COMPUTATION
      		OutputImage->data[i+u][j+v]=(short)Block[u][v];
#else
      		OutputImage->data[i+u][j+v]=Block[u][v];
#endif      
   }
   return;
}

unsigned int JPEGpacketization(OutImageStruct *InputImage, unsigned int BlockOffset)
{
   int Block[8][8], row, col, row_mix, col_mix;
   short packetsize, packetoffset, buffersize;
   unsigned int index, q, r, K;
   opj_mqc_t mqobjet, mqbckobjet, *objet=NULL;
   unsigned char buffer[MQC_NUMCTXS], bckbuffer[MQC_NUMCTXS];
   unsigned char packet[MQC_NUMCTXS];
   bool loop = true, overhead = true;

#ifdef LORA_LAS   
   uint8_t packetPos = (BlockOffset==0)?LAS_FIRST_DATAPKT:LAS_NO_FIRST_DATAPKT;
#endif   
   
   // On crée et on initialise le codeur MQ
   objet=&mqobjet;
   for (int x=0; x< MQC_NUMCTXS; x++) 
     buffer[x]=0;
   mqc_init_enc(objet, buffer);
   mqc_resetstates(objet);
   packetoffset = BlockOffset;

   while ((loop == true) && (BlockOffset != (InputImage->imageHsize * InputImage->imageVsize /64))) {
	// On lit le bloc
	row = (BlockOffset * 8) / InputImage->imageHsize * 8;
	col = (BlockOffset * 8) % InputImage->imageHsize;
	row_mix = ((row * 5) + (col *  8)) % (InputImage->imageHsize);
	col_mix = ((row * 8) + (col * 13)) % (InputImage->imageVsize);
	
	//printf("(%d %d) ", row_mix, col_mix);

	for (int u=0; u<8; u++)
	   for (int v=0; v<8; v++)
#ifdef SHORT_COMPUTATION
              Block[u][v]=(short)InputImage->data[row_mix+u][col_mix+v];
#else
              Block[u][v]=(int)round(InputImage->data[row_mix+u][col_mix+v]);
#endif
	// On cherche où se trouve le dernier coef <> 0 selon le zig-zag
	K=63;

	while ((Block[ZigzagCoordinates[K].row][ZigzagCoordinates[K].col]==0) && (K>0)) 
          K--; 
          
        K++;

	// On code la valeur de K, nombre de coefs encodé dans le bloc
	q=K / 2;	
        r=K % 2;
        
	for (int x=0; x<q; x++) 
          mqc_encode(objet, 1);
	
        mqc_encode(objet, 0);
	mqc_encode(objet, r);

	// On code chaque coef significatif par Golomb-Rice puis par MQ
	for (int x=0; x<K; x++)
	  {
	   if (Block[ZigzagCoordinates[x].row][ZigzagCoordinates[x].col]>=0)
		{ index=2*Block[ZigzagCoordinates[x].row][ZigzagCoordinates[x].col]; }
	   else { index=2*abs(Block[ZigzagCoordinates[x].row][ZigzagCoordinates[x].col])-1; }

	 // Golomb
	 q=index / 2;
	 r=index % 2;
	 for (int x=0; x<q; x++) mqc_encode(objet, 1);
	 mqc_encode(objet, 0);
	 mqc_encode(objet, r);
	}
	mqc_backup(objet, &mqbckobjet, bckbuffer);
	mqc_flush(objet);
	// On compte combien il y a de bits dans le code (octets entiers).
	buffersize=mqc_numbytes(objet);
	if (buffersize < (MSS - 2)) {
		overhead = false;
		packetsize = buffersize;
		for (int x=0; x<packetsize; x++) 
                  packet[x]=buffer[x];
		BlockOffset++;
		mqc_restore(objet, &mqbckobjet, bckbuffer);
        }
	else { 
          loop = false;
          if (overhead == true) { 
            BlockOffset++;
            //break; 
            return (BlockOffset); 
          }
	}
   }

#ifdef DISPLAY_PKT   
   Serial.print(F("00"));
   Serial.print(packetsize + 2, HEX);
   Serial.print(F(" "));
   Serial.print(F("00 "));
   if (packetoffset<0x10)
       Serial.print(F("0"));
   Serial.print(packetoffset, HEX);
 
   for (int x=0; x<packetsize; x++) {
        Serial.print(F(" "));
        
        if (packet[x]<0x10)
            Serial.print(F("0"));
   	
        Serial.print(packet[x], HEX);
   }
   Serial.println(F(" "));
#endif  

   if (BlockOffset == (InputImage->imageHsize * InputImage->imageVsize /64)) {
     Serial.println(F("last packet"));
#ifdef LORA_LAS
     packetPos=LAS_LAST_DATAPKT; 
#endif
   }
   
   if (transmitting_data) {
     
        digitalWrite(capture_led, LOW);
        // here we transmit data  

        /*
        Serial.print(F("LS "));
        Serial.print(lastSentTime);
        Serial.print(F(" CT "));
        Serial.println(millis());
        */
        
        if (inter_binary_pkt!=MIN_INTER_PKT_TIME)
            while ( (millis()-lastSentTime) < inter_binary_pkt)
              ;
              
        // just the maximum pkt size plus some more bytes
#ifdef LORA_UCAM
        uint8_t myBuff[260]; 
#else
        uint8_t myBuff[110];      
#endif
        uint8_t dataSize=0;
        long startSendTime, stopSendTime, previousLastSendTime;
              
        if (with_framing_bytes) {
                myBuff[0]=pktPreamble[0];
#ifdef RANDOM_CAM_ID
                // cam between 0 and 2
                myBuff[1]=0x50+randCamId;
#else
                myBuff[1]=0x50+currentCam;
#endif 

#ifdef WITH_SRC_ADDR
#ifdef RANDOM_NODE_ID
                myBuff[2]=0;
                // node id between 10 and 16
                myBuff[3]=randNodeId;
#else
                myBuff[2]=pktPreamble[2];
                myBuff[3]=pktPreamble[3];
#endif     
                // set the sequence number
                myBuff[4] = (uint8_t)packetcount;
                // set the Quality Factor
                myBuff[5] = (uint8_t)QualityFactor[currentCam];
                // set the packet size
                myBuff[6] = (uint8_t)(packetsize+2);
#else
                // set the sequence number
                myBuff[2] = (uint8_t)packetcount;
                // set the Quality Factor
                myBuff[3] = (uint8_t)QualityFactor[currentCam];
                // set the packet size
                myBuff[4] = (uint8_t)(packetsize+2);
#endif                
                dataSize += sizeof(pktPreamble);
        }
#ifdef DISPLAY_PKT         
        Serial.print(F("Building packet : "));
        Serial.println(packetcount);
#endif        
        // high part
        myBuff[dataSize++]=packetoffset >> 8 & 0xff; 
        // low part
        myBuff[dataSize++]=packetoffset & 0xff;
        
        for (int x=0; x<packetsize; x++) {
            myBuff[dataSize+x]=(uint8_t)packet[x];
        }

#ifdef DISPLAY_PKT
        for (int x=0; x<dataSize+packetsize; x++) {
                if (myBuff[x]<0x10)
                    Serial.print("0");
            Serial.print(myBuff[x], HEX);
        }
        Serial.println("");

        Serial.print(F("Sending packet : "));
        Serial.println(packetcount);
#endif         
        
        previousLastSendTime=lastSentTime;
        
        startSendTime=millis();
                
        // we set lastSendTime before the call to the sending procedure in order to be able to send packets back to back since 
        // the sending procedure can introduce a delay 
        lastSentTime=(unsigned long)startSendTime;

#ifdef XBEE_UCAM        
        // 16-bit destination address?        
        if (strlen(destMACAddr)==4) {
                // set the frame id to the pkt sequence number to keep track of ACK failures when mac mode is 0/2 with unicast address
                //Tx16Request tx = Tx16Request(destAddr16, ACK_OPTION, myBuff, dataSize+packetsize, nbSentPackets);
                Tx16Request tx = Tx16Request(destAddr16, myBuff, dataSize+packetsize);
                // send the frame
                xbee.send(tx); 
        }
        else {
                // set the frame id to the pkt sequence number to keep track of ACK failures when mac mode is 0/2 with unicast address
                //Tx64Request tx = Tx64Request(destAddr64, ACK_OPTION, myBuff, dataSize+packetsize, nbSentPackets);  
                Tx64Request tx = Tx64Request(destAddr64, myBuff, dataSize+packetsize);   
                // send the frame
                xbee.send(tx);    
        }                                    
#endif

#ifdef LORA_UCAM
        int e;
        
#ifdef LORA_LAS
        // packetPos will indicate the last packet to trigger UPDT message from LR-BS
        e=loraLAS.sendData(LORA_DEST_ADDR, myBuff, dataSize+packetsize, 0, packetPos);
        
        if (e==TOA_OVERUSE) {
            Serial.println(F("LoRa packet not sent, TOA_OVERUSE"));  
        }
#else
        //sx1272.CarrierSense();
        if (carrier_sense_method==1)
            CarrierSense1();
  
        if (carrier_sense_method==2)
            CarrierSense2();

        if (carrier_sense_method==3)
            CarrierSense3();       
               
        sx1272.setPacketType(PKT_TYPE_DATA);
        e=sx1272.sendPacketTimeout(LORA_DEST_ADDR, myBuff, dataSize+packetsize, 10000);
#endif
#endif
              
#ifndef ENERGY_TEST        
        digitalWrite(capture_led, HIGH);
#endif
        nbSentPackets++;

#ifdef DISPLAY_PKT

        Serial.print(F("Packet Sent in "));
        Serial.println(stopSendTime-startSendTime);

#ifdef LORA_UCAM
        Serial.print(F("LoRa Sent w/CAD in "));
        Serial.println(stopSendTime-sx1272._startDoCad);

        Serial.print(F("LoRa sending: state = "));
        Serial.println(e);
#endif

#ifdef XBEE_UCAM
        // read incoming pkt, to get the transmit status, for 5ms max
        xbee.readPacket(5);
        
        if (xbee.getResponse().isAvailable()) {
  
              // is it a response to the previously sent packet?            	
	      if (xbee.getResponse().getApiId() == TX_STATUS_RESPONSE) {
                      
	            xbee.getResponse().getTxStatusResponse(txStatus);	
  

                    if (txStatus.isSuccess()) {
                          Serial.print(F(" Success "));        
                    }                        
                    else {                     
                          Serial.print(F(" Error: 0x"));
                         
                          Serial.print(txStatus.getStatus(), HEX);
                          Serial.print(". Pkt ");
                          // get back the frame id
                          Serial.print(txStatus.getFrameData()[0]);

                          Serial.print(". ");     
                    }
              }
              else
                    Serial.print(F(" Not a TX status "));  
        }
        else
              Serial.print(F(" Timeout "));
#endif        
#endif  

#ifdef DISPLAY_PACKETIZATION_SEND_STATS
        Serial.print(packetsize);
        Serial.print(" ");  
        Serial.println(stopSendTime-startSendTime);
        //Serial.println("ms. ");   
#endif        
   }

   count += packetsize;
   packetcount++;

   return (BlockOffset);
}

#endif // #ifdef CRAN_NEW_CODING

//End CRAN ENCODING

void set_led(int led, int n) {
  
      digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(n);                  // wait for n millisecond
      digitalWrite(led, LOW);
}

void blink_led(int led, int count) {
  
  for (int i=0; i<count; i++) {
      digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(70);               // wait for a second
      digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
      delay(70);               // wait for a second
  }
}

void getTGInfo(int index=0) {

        switch (index) {
          
          case 0:
              sprintf(TGInfoStr,"uCam raw 128x128"); 
              break;
              
          case 1:
        
              sprintf(TGInfoStr,"L%.2X#", 0x50+flowId);
              break;
              
          default:
              break;
        }                     
}

#ifdef WITH_LCD

void printLCD(int theLine, const char* theInfo, boolean clearFirst=true) {
        if (clearFirst)
              lcd.clear();
        lcd.setCursor(0, theLine);
        lcd.print(theInfo);
}

void printInfoLCD(int index=0) {
        lcd.clear();
        getTGInfo(index);
        lcd.home();
        lcd.print(TGInfoStr);
        lcd.setCursor(0,1);
        printLCD(1, destMACAddr, false);
}

#endif

#ifdef LUM_HISTO

void clearHistogram(short histo_data[]) {
  
        for (int i=0; i<255; i++)
              histo_data[i]=0;
}
              
void computeHistogram(short histo_data[], uint8_t **data) {

        clearHistogram(histo_data);
              
        for (int x=0; x<CAMDATA_PGM_LINE_SIZE; x++)
              for (int y=0; y<CAMDATA_PGM_LINE_SIZE; y++)
                    histo_data[data[x][y]]++; 
  
}

long computeMeanLuminosity(short histo_data[]) {

        double luminosity=0;
        
        for (int i=0; i<255; i++)     
            luminosity+=histo_data[i]*i;
        
        luminosity = luminosity / (CAMDATA_PGM_LINE_SIZE*CAMDATA_PGM_LINE_SIZE);
        
        return (long)luminosity;
}

#endif

// only compare with already stored in outImage
long compare_raw_picture_data() {

      uint8_t x=0;
      uint8_t y=0;
      uint8_t pixV;
      uint8_t pixDiff;
      
      long nbPixDiff=0;
      int totalBytes=0;
      long startCamCompTime=0;
      long stopCamCompTime=0;

#ifdef WITH_LCD
      printLCD(1, F("Comp. RAW Image "), false);
#endif

      Serial.println(F("Waiting for image raw data (compare)"));

#ifdef LUM_HISTO
      clearHistogram(histoInImage);   
#endif      

      startCamCompTime=millis();
      
      // we have to read the raw data here and send it over serial for instance	
      while (ucamArray[currentCam]->available()) {

          pixV = ucamArray[currentCam]->read();

          inImage.data[x][y]=pixV;
          
#ifdef LUM_HISTO
          // create the histogram of the incoming image
          histoInImage[pixV]++;
#endif
          
          //compare with the reference image previously stored in the outImage array
          if (pixV > (uint8_t)refImage[currentCam].data[x][y])
               pixDiff = pixV - (uint8_t)refImage[currentCam].data[x][y];
          else
               pixDiff = (uint8_t)refImage[currentCam].data[x][y] - pixV;

          if (pixDiff > PIX_THRES)
               nbPixDiff++;
           
          x++;
          totalBytes++;
          
          // new ucam line or buffer full
          if (x==CAMDATA_PGM_LINE_SIZE) {              

              x=0;
              // next line
              y++;
          }
          
          // in this way, we always have the last timestamp
          stopCamCompTime=millis();
              
          while (!ucamArray[currentCam]->available() && millis()-stopCamCompTime < 100)
           ; 
      }

      Serial.print(F("\nTotal bytes compared: "));
      Serial.println(totalBytes);
      
      Serial.print(F("Time to read and process from uCAM: "));
      Serial.println(stopCamCompTime-startCamCompTime);
      
      myCam.send_ack_picture(); 

#ifdef LUM_HISTO

      long startComputeLum=millis();
      
      // don't need to compute the histogram because this is done on-the-fly
      inImageLuminosity=computeMeanLuminosity(histoInImage);

      Serial.println(millis()-startComputeLum);
      
      Serial.print(F("Incoming image mean luminosity is "));
      Serial.println(inImageLuminosity); 
        
      Serial.print(F("Reference mean luminosity is "));
      Serial.println(refImageLuminosity[currentCam]); 
      
      
      if (nbPixDiff > INTRUSION_THRES) {

        Serial.print(F("#pix difference "));
        Serial.println(nbPixDiff);
      
        int nbPixDiff2=0;
        uint8_t new_pix_thres = PIX_THRES + abs(refImageLuminosity[currentCam]-inImageLuminosity);
        
        for (x=0; x<CAMDATA_PGM_LINE_SIZE; x++)
              for (y=0; y<CAMDATA_PGM_LINE_SIZE; y++) {
                
                    pixDiff = abs(inImage.data[x][y]-(uint8_t)refImage[currentCam].data[x][y]);
                    
                    if (pixDiff > new_pix_thres)
                        nbPixDiff2++;          
              }
              
        nbPixDiff=nbPixDiff2;
        
        Serial.println(millis()-startComputeLum);
        Serial.print(F("Updated #pix difference "));
        Serial.println(nbPixDiff);        
      }
#endif

#ifdef ENERGY_TEST     
      return 0;
#else
      return nbPixDiff;
#endif      
}

void get_raw_picture_data() {

      uint8_t x=0;
      uint8_t y=0;
      int totalBytes=0;
      long startCamDataTime=0;
      long stopCamDataTime=0;

#ifdef WITH_LCD
      printLCD(1, F("Get RAW Image   "), false);
#endif

      Serial.println(F("Waiting for image raw data"));
      
      startCamDataTime=millis();
      
      // we have to read the raw data here and send it over serial for instance	
      while (ucamArray[currentCam]->available()) {
	  inImage.data[x][y] = ucamArray[currentCam]->read();

          x++;
          totalBytes++;
          
          // new ucam line or buffer full
          if (x==CAMDATA_PGM_LINE_SIZE) {              

              x=0;
              // next line
              y++;
          }
          
          // in this way, we always have the last timestamp
          stopCamDataTime=millis();
              
          while (!ucamArray[currentCam]->available() && millis()-stopCamDataTime < 100)
           ; 
      }

      Serial.print(F("\nTotal bytes read: "));
      Serial.println(totalBytes);
      
      Serial.print(F("Time to read data from uCAM: "));
      Serial.println(stopCamDataTime-startCamDataTime);
      
      myCam.send_ack_picture(); 
}

void display_ucam_data() {
      int x=0;
      int y=0;
      int totalBytes=0;
      
      // print PGM header
      Serial.println(F("P2"));
      Serial.println(F("# test ucam in RAW mode, 128x128 8-bit gray scale"));
      Serial.println(F("128 128"));
      Serial.println(F("255"));
      
      for (y=0; y<inImage.imageHsize; y++)
          for (x=0; x<inImage.imageVsize; x++) {
              Serial.print(inImage.data[x][y]);
              Serial.print(" ");
          
              totalBytes++;
          
              if ( (x+1) % DISPLAY_CAM_DATA_SIZE==0) {
                  Serial.print("\n"); 
              }        
          }
          
      Serial.print(F("\nTotal bytes read and displayed: "));
      Serial.println(totalBytes);
}

#ifdef CRAN_NEW_CODING

int encode_ucam_file_data() {
    unsigned int err, offset;
    boolean RTS;
    float CompressionRate;
    int Block[8][8];
    int row, col, row_mix, col_mix, N;
    int i, j, w;    
    
    long startCamGlobalEncodeTime=0;
    long stopCamGlobalEncodeTime=0;
    long startEncodeTime=0;
    long totalEncodeTime=0;
    totalPacketizationTime=0;    

    Serial.print(F("Encoding picture data, Quality Factor is : "));
    Serial.println(QualityFactor[currentCam]);
  
    Serial.print(F("MSS for packetization is : "));
    Serial.println(MSS);   

#ifdef XBEE_POWER_SAVING   
    if (transmitting_data) 
        wakeup_xbee();
#endif 

    startCamGlobalEncodeTime=millis();

    // Initialisation de la matrice de quantification
    QTinitialization(QualityFactor[currentCam]);
    
#ifdef DISPLAY_ENCODE_STATS
    Serial.print(F("Q: "));
    Serial.print(millis()-startCamGlobalEncodeTime);   
#endif

    offset=0;
    CreateNewPacket(offset);

    Serial.println(F("QT ok"));

#if defined RANDOM_NODE_ID || defined RANDOM_CAM_ID
     // node id between 10 and 16
     randNodeId=random(10,16);
     // cam between 0 and 2
     randCamId=random(3);
#endif

    // N=16 for 128x128 image
    N = CAMDATA_PGM_LINE_SIZE / 8;
    
    for (row = 0; row < N; row++)
        // for a given row, we will have 2 main row_mix*8 values separated by 64 lines
        // then, for a row_mix value, we need to be able to browse 8 lines
        // Therefore we need to read 2x8lines in memory from SD file
        //read_image_block=true;
        
	for (col = 0; col < N; col++) {
		// determine starting point
		row_mix = ((row * 5) + (col *  8)) % N;
		col_mix = ((row * 8) + (col * 13)) % N;

                row_mix = row_mix*8;
                col_mix = col_mix*8;

#ifdef DISPLAY_BLOCK                
		for (i=0; i<8; i++) {
		  for (j=0; j<8; j++) {
                    Block[i][j] = (int)inImage.data[row_mix+i][col_mix+j]; 
                    
                    if (Block[i][j]<0x10)
                         Serial.print(F("0"));
       
                    Serial.print(Block[i][j], HEX);
                    Serial.print(F(" "));
                  }
                  Serial.println("");
                }
#else
                startEncodeTime=millis();
                
		for (i=0; i<8; i++) 
		  for (j=0; j<8; j++) 
                    Block[i][j] = (int)inImage.data[row_mix+i][col_mix+j];  
#endif
		// Encodage JPEG du bloc 8x8
		JPEGencoding(Block);

                totalEncodeTime+=millis()-startEncodeTime;

#ifdef DISPLAY_BLOCK
                Serial.println(F("encode_ucam_file_data():"));
                
                for (int u=0; u<8; u++) {
                   for (int v=0; v<8; v++) { 
                      Serial.print(Block[u][v]);
                      Serial.print(F("\t"));
                   }               
                   Serial.println("");
                }   

#endif
	        err = FillPacket(Block, &RTS);

	        if (err == -1) {
#ifdef DISPLAY_FILLPKT  
                        Serial.println(F("err"));
#endif                        
			SendPacket();
			CreateNewPacket(offset);
			FillPacket(Block, &RTS);
		}

	        offset++;

	        if (RTS == true) {
			SendPacket();
			CreateNewPacket(offset);
			RTS = false;
		}
	}

     SendPacket();

     stopCamGlobalEncodeTime=millis();

#ifdef XBEE_POWER_SAVING   
    if (transmitting_data) 
        sleep_xbee();
#endif 

     Serial.print(F("Time to encode : "));
     Serial.println(stopCamGlobalEncodeTime-startCamGlobalEncodeTime);	
     Serial.print(F("Total encode time : "));
     Serial.println(totalEncodeTime);	
     Serial.print(F("Total pkt time : "));
     Serial.println(totalPacketizationTime);
     
     CompressionRate = (float) count * 8.0 / (CAMDATA_PGM_LINE_SIZE * CAMDATA_PGM_LINE_SIZE);
     Serial.print(F("Compression rate (bpp) : "));
     Serial.println(CompressionRate);
     Serial.print(F("Packets : "));
     Serial.print(packetcount);
     Serial.print(" ");   
     Serial.println(packetcount, HEX);
     Serial.print(F("Q : "));
     Serial.print(QualityFactor[currentCam]);
     Serial.print(" ");   
     Serial.println(QualityFactor[currentCam], HEX);
     Serial.print(F("H : "));
     Serial.print(CAMDATA_PGM_LINE_SIZE);
     Serial.print(" ");   
     Serial.println(CAMDATA_PGM_LINE_SIZE, HEX);
     Serial.print(F("V : "));
     Serial.print(CAMDATA_PGM_LINE_SIZE);
     Serial.print(" ");   
     Serial.println(CAMDATA_PGM_LINE_SIZE, HEX);     
     Serial.print(F("Real encoded image file size : "));
     Serial.println(count);	
    
     // reset
     packetcount=0L;
     count=0L;  
     return 0;
}

#else

int encode_ucam_file_data() {
  
    unsigned int offset = 0;
    float CompressionRate;
    long startCamGlobalEncodeTime=0;
    long stopCamGlobalEncodeTime=0;
    long startCamEncodeTime=0;
    long startCamPktEncodeTime=0;
    long stopCamPktEncodeTime=0;
    long stopCamQuantizatioTime=0;
    
#ifdef WITH_LCD
    printLCD(1, F("Encode Image    "), false);
#endif

    Serial.print(F("Encoding picture data, Quality Factor is : "));
    Serial.println(QualityFactor[currentCam]);
  
    Serial.print(F("MSS for packetization is : "));
    Serial.println(MSS);   

#ifdef XBEE_POWER_SAVING   
    if (transmitting_data) 
        wakeup_xbee();
#endif 

    startCamGlobalEncodeTime=millis();
    
#ifdef DISPLAY_ENCODE_STATS
    
    // Initialisation de la matrice de quantification
    QTinitialization(QualityFactor[currentCam]);

    startCamEncodeTime=millis();

    // Encodage JPEG et fin
    JPEGencoding(&inImage, &outImage);

    Serial.print(F("Q: "));
    Serial.print(startCamEncodeTime-startCamGlobalEncodeTime);
    
    Serial.print(F(" E: "));
    Serial.println(millis()-startCamEncodeTime);    
#else
    // Initialisation de la matrice de quantification
    QTinitialization(QualityFactor[currentCam]);
    
    // Encodage JPEG et fin
    JPEGencoding(&inImage, &outImage);
#endif

#if defined RANDOM_NODE_ID || defined RANDOM_CAM_ID
     // node id between 10 and 16
     randNodeId=random(10,16);
     // cam between 0 and 2
     randCamId=random(3);
#endif
     // so that we can know the packetisation time without displaying stats for each packets
     startCamPktEncodeTime=millis();

     do { 
#ifdef DISPLAY_PACKETIZATION_STATS      
         startCamPktEncodeTime=millis();
         offset = JPEGpacketization(&outImage, offset); 
         stopCamPktEncodeTime=millis();
         Serial.print(offset);
         Serial.print("|");  
         Serial.println(stopCamPktEncodeTime-startCamPktEncodeTime); 	
#else
         offset = JPEGpacketization(&outImage, offset);
#endif
     } while (offset != (outImage.imageHsize * outImage.imageVsize / 64));

     stopCamGlobalEncodeTime=millis();

#ifdef XBEE_POWER_SAVING   
    if (transmitting_data) 
        sleep_xbee();
#endif 

     Serial.print(F("Global time to encode : "));
     Serial.println(stopCamGlobalEncodeTime-startCamGlobalEncodeTime);	

     Serial.print(F("Time for packetization : "));
     Serial.println(stopCamGlobalEncodeTime-startCamPktEncodeTime);
     
     CompressionRate = (float) count * 8.0 / (outImage.imageHsize * outImage.imageVsize);
     Serial.print(F("Compression rate (bpp) : "));
     Serial.println(CompressionRate);
     Serial.print(F("Packets : "));
     Serial.print(packetcount);
     Serial.print(" ");   
     Serial.println(packetcount, HEX);
     Serial.print(F("Q : "));
     Serial.print(QualityFactor[currentCam]);
     Serial.print(" ");   
     Serial.println(QualityFactor[currentCam], HEX);
     Serial.print(F("H : "));
     Serial.print(inImage.imageHsize);
     Serial.print(" ");   
     Serial.println(inImage.imageHsize, HEX);
     Serial.print(F("V : "));
     Serial.print(inImage.imageVsize);
     Serial.print(" ");   
     Serial.println(inImage.imageVsize, HEX);     
     Serial.print(F("Real encoded image file size : "));
     Serial.println(count);	
     
     // reset
     packetcount=0L;
     count=0L;
     return 0;
}

#endif

long getCmdValue(int &i, char* strBuff=NULL) {
      
      // a maximum of 16 characters plus the null-terminited character, can store a 64-bit MAC address
      char seqStr[17]="****************";

      int j=0;
      // character '#' will indicate end of cmd value
      while ((char)rcv_data[i]!='#' && (i < rcv_data_len) && j<strlen(seqStr)) {
              seqStr[j]=(char)rcv_data[i];
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

#ifdef XBEE_UCAM

// used here to send AT command to the XBee to get information such as the MAC address
int sendAtCommand() {
#ifdef DEBUGPRINT
        Serial.println(F("Sending command to the Serial"));
#endif
        // send the command
        xbee.send(atRequest);

        // wait up to 5 seconds for the status response
        if (xbee.readPacket(5000)) {
                // got a response!
                // should be an AT command response
                if (xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {
                        xbee.getResponse().getAtCommandResponse(atResponse);
            
                        if (atResponse.isOk()) {
#ifdef DEBUGPRINT        
                                Serial.print("Command [");
                                Serial.print((char)atResponse.getCommand()[0]);
                                Serial.print((char)atResponse.getCommand()[1]);
                                Serial.println("] was successful!");
#endif
                                if (atResponse.getValueLength() > 0) {
#ifdef DEBUGPRINT          
                                        Serial.print("Command value length is ");
                                        Serial.println(atResponse.getValueLength(), DEC);
      
                                        Serial.print("Command value: 0x");
                                        
                                        for (int i = 0; i < atResponse.getValueLength(); i++) {
                                          Serial.print(atResponse.getValue()[i], HEX);
                                          Serial.print(" ");
                                        }
                              
                                        Serial.println("");
#endif          
                                }
                                return 1;
                        } 
                        else {
                                Serial.print("Command return error code: 0x");
                                Serial.println(atResponse.getStatus(), HEX);
                        }
                } 
                else {
                        Serial.print("Expected AT response but got 0x");
                        Serial.print(xbee.getResponse().getApiId(), HEX);
                }   
                
                
        } 
        else {
              // at command failed
              if (xbee.getResponse().isError()) {
                    Serial.print("Error reading packet.  Error code: 0x");  
                    Serial.println(xbee.getResponse().getErrorCode(), HEX);
              } 
              else {
                    Serial.print("No response from radio. ");  
              }
        }
        return 0;
}

void setMYAddr(int addrMode) {

        // set command to MY to set 16-bit node's short address
        atRequest.setCommand(myCmd); 
        uint16_t myShortAddr=0xFFFF;    
        
        Serial.print(F("Set node's short 16-bit short address to 0x"));
        
        // we set the 16-bit addr by taking the last 4-digit of the 64-bit MAC address
        if (addrMode==SHORT_ADDR_FROM_64MAC) {

                Serial.println((const char*)&macAddr[12]);

                myShortAddr=(uint16_t)strtol((const char*)&macAddr[12], NULL, 16);     
                  
                uint8_t tmpValue;
                
                // need to swap byte order
                tmpValue = *((uint8_t*)&myShortAddr);
                *((uint8_t*)&myShortAddr) = *((uint8_t*)&myShortAddr+1);
                *((uint8_t*)&myShortAddr+1) = tmpValue;
                
                have_short_address = true;
        }
        else {   
                // otherwise, set to 0xFFFF to disable 16-bit address packet reception 
                Serial.print(SHORT_BROADCAST_MAC_ADDR);
                have_short_address = false;
        }
                
        atRequest.setCommandValue((uint8_t*)&myShortAddr);  
        
        atRequest.setCommandValueLength(sizeof(myShortAddr)); 

        while (!sendAtCommand()) {
                Serial.println(F("retry set command ATMY"));
                delay(500);
        }
        
        atRequest.clearCommandValue();

        // set command to WR to write in EEPROM
        //atRequest.setCommand(wrCmd); 
        
        //while (!sendAtCommand()) {
        //        Serial.println("retry command ATWR");
        //        delay(500);
        //}
        
        // set command to read back MY 16-bit node's short address
        atRequest.setCommand(myCmd);    
        while (!sendAtCommand()) {
                Serial.println(F("retry read command ATMY"));
                delay(500);
        }
        
        char my_part[4];
        String myAddr="";

        // get each value, convert it to string and append to the MY addr
        for (int i = 0; i < atResponse.getValueLength(); i++) {
              sprintf(my_part, "%lX", (unsigned long)(atResponse.getValue()[i]));
    
              if (atResponse.getValue()[i]<=0xF) 
                myAddr += 0;
        
              myAddr += my_part;
        }                                                         
        
        Serial.print(F("Node's short address is now: "));
        Serial.println((char*)&myAddr[0]);                        
}

void setAddresses() {
        boolean setShortAddress=false;
        
        // 16-bit destination address
        if (strlen(destMACAddr)==4) {
                // to indicate that we need to set a short address for the node because 16-bit destination needs a node's short address as well
                if (have_short_address==false)
                        setShortAddress=true;
                
                if (strcmp(destMACAddr, SHORT_BROADCAST_MAC_ADDR)==0) {
                        // 16-bit addressing: This is the SH + SL address of remote XBee
                        // unless you have MY on the receiving radio set to FFFF, this will be received as a RX16 packet
                        destAddr16 = 0xFFFF;    
                } 
                else {
                        destAddr16=(uint16_t)strtol(destMACAddr, NULL, 16);
                }                                 
        }
        else {
                // we can have a 64-bit destination address and have a short address to enable 16-bit address addressing
                // this is why we do not set haveShortAddress to false
                if (strcmp(destMACAddr, BROADCAST_MAC_ADDR)==0) {
                        // 64-bit addressing: This is the SH + SL address of remote XBee
                        // unless you have MY on the receiving radio set to FFFF, this will be received as a RX16 packet
                        destAddr64 = XBeeAddress64(0x00000000, 0x0000FFFF);   
                }
                else {
                        // need to transform the MAC addr string into 2 uint32_t
                        char myMacHigh[9], myMacLow[9];
                        
                        strncpy(myMacHigh, destMACAddr, 8);
                        strncpy(myMacLow, destMACAddr+8, 8);
                        myMacHigh[8]='\0';
                        myMacLow[8]='\0';
                        
                        destAddr64 = XBeeAddress64((uint32_t)strtoul(myMacHigh, NULL, 16), (uint32_t)strtoul(myMacLow, NULL, 16));     
                }           
        }              

        if (setShortAddress) 
                setMYAddr(SHORT_ADDR_FROM_64MAC);                                                           
}

void setMMMode(uint8_t mmMode) {

        // set command to MY to set 16-bit node's short address
        atRequest.setCommand(mmCmd);     
        uint8_t myMMMode=mmMode;
        
        Serial.print(F("Set MM mode to "));
        Serial.println(mmMode);
                
        atRequest.setCommandValue((uint8_t*)&myMMMode);  
        
        atRequest.setCommandValueLength(sizeof(myMMMode)); 
        
        while (!sendAtCommand()) {
                Serial.println(F("retry set command ATMM"));
                delay(500);
        }
        
        atRequest.clearCommandValue();
  
        // set command to MM to get the XBee mac mode
        atRequest.setCommand(mmCmd);
        
        int counter = 4;
        while (!sendAtCommand() && counter > 0) {
              Serial.println(F("retry command ATMM"));
              counter--;
        }
        
        if (counter > 0) {
                myMacMode=(int)atResponse.getValue()[0];
        }
        else
                Serial.print(F("ERROR AT+MM"));            
        
        Serial.print(F("MAC mode is now: "));
        Serial.println(myMacMode);                        
}

#ifdef XBEE_POWER_SAVING  
void wakeup_xbee() {

        Serial.println(F("Wakeup XBee radio"));
        digitalWrite(xbee_power_pin, HIGH);
        delay(500);
#ifdef XBEE_CUSTOM_BAUDRATE        
        // if we have an xbee module at 125000, it means that it has mac mode 0 for remote AT command
        // then switch it to MM2. Note that this is not definitive since ATWR is not issued
        setMMMode(2);        
#endif        
        setAddresses();
}

void sleep_xbee() {
        Serial.println(F("Sleeping XBee radio"));
        digitalWrite(xbee_power_pin, LOW);
}
#endif
#endif

void display_ucam_stats() {
  
       Serial.print(F("Time for get snapshop : "));
       Serial.println(myCam.stopCamSnapshotTime-myCam.startCamSnapshotTime);
       
       Serial.print(F("Time for get picture : "));
       Serial.println(myCam.stopCamGetPictureTime-myCam.startCamGetPictureTime);
}

void immediateSendPicture(int delay_before_send, int delay_after_send) {      
        
        if (transmitting_data) {
            Serial.print(F("Immediate send picture with inter-pkt "));
            Serial.print(inter_binary_pkt);
            Serial.print(" to ");
#ifdef XBEE_UCAM
            Serial.print(destMACAddr);
#else
            Serial.print(F("gw"));
#endif
            Serial.print(" in ");
            Serial.print(delay_before_send);
            Serial.println("ms");
        }
        
        delay(delay_before_send);        

#ifdef WITH_LCD
        printLCD(1, F("Send Image      "), false);
#endif 

        Serial.println(F("Getting new picture")); 
        
        camDataReady=false;
        
        if (myCam.send_initial()) {
                if (myCam.do_snapshot()) {
                    if (myCam.get_picture(PICTURE_SNAPSHOT_TYPE)) {
                        digitalWrite(capture_led, HIGH);
                        myCam.get_data();
                        camDataReady=true;
                    }
                }
        }    
        
        display_ucam_stats();
        
        if (camDataReady) {
            get_raw_picture_data();
#ifdef DISPLAY_PGM            
            display_ucam_data();
#endif            
            nbSentPackets=0;
            
            if (transmitting_data && run_test) {
            
                Serial.println(F("ENCODE WITHOUT TRANSMISSION"));
                transmitting_data = false;
                encode_ucam_file_data();
                transmitting_data = true;
                Serial.println(F("ENCODE WITH TRANSMISSION"));
            }
            
            encode_ucam_file_data();
            
            if (transmitting_data) {
                Serial.print("Sent packets : ");
                Serial.println(nbSentPackets);
            }
        }
        else
          Serial.println(F("Error: will try again"));
        
        digitalWrite(capture_led, LOW);

        delay(delay_after_send);
                
#ifdef WITH_LCD     
        printInfoLCD();
#endif  
}

void copy_in_refImage() {

        if (useRefImage) {
              for (int x=0; x<CAMDATA_PGM_LINE_SIZE; x++)
                    for (int y=0; y<CAMDATA_PGM_LINE_SIZE; y++)
                          refImage[currentCam].data[x][y]=inImage.data[x][y];  
                          
#ifdef LUM_HISTO
              computeHistogram(histoRefImage[currentCam], refImage[currentCam].data);
              refImageLuminosity[currentCam]=computeMeanLuminosity(histoRefImage[currentCam]); 
#endif                           
        }
}

#ifdef CRITICALITY_SCHEDULING

float getCaptureRate(float r0, int sizeofCoverset) {

      float minCaptureRate = MIN_CAPTURE_RATE;
      float maxCaptureRate = MAX_CAPTURE_RATE;
      int maxDefinedCoverSetNumber = MAX_COVER_SET;     

      float captureRate;
      
      int X = sizeofCoverset;         
	
      if (X >= maxDefinedCoverSetNumber) X=maxDefinedCoverSetNumber; 	
	
      if (sizeofCoverset>=0) {
            float hx = (float)maxDefinedCoverSetNumber;           
	    float hy = maxCaptureRate;                   
         
	    if (r0 < 0) r0=0;               		// r0 => criticality level.

	    float bx = -hx*r0+hx;  			// = hx(1-r0)           

	    float by = hy*r0;                              

	    float aX = (-bx+sqrt(bx*bx-2*bx*X+hx*X))/(hx-2*bx);      

	    if ( (hx-2*bx)==0 ) {
	          captureRate = ((hy-2*by)/(4*bx*bx))*X*X+(by/bx)*X;
	    }
	    else {
		  captureRate = (hy-2*by)*aX*aX+2*by*aX;
		}

	    if (captureRate<minCaptureRate) captureRate=minCaptureRate;
	}

	return captureRate;
}

void setCaptureRate() {

       Serial.print(F("Maximum number if cover-sets is "));
       Serial.println(MAX_COVER_SET);  
       Serial.print(F("Criticality level is "));
       Serial.println(criticalityLevel);
       Serial.print(F("Number of cover-sets is "));
       Serial.println(numberOfCoverSets);
       
       // get the capture rate from default values for criticality level and number of cover-sets
       float myCaptureRate;
      
       if (detectedIntrusion && highCriticalityPeriod && startHighCriticalityPeriod==0) {
         Serial.print(F("Enters high criticality period for "));
         Serial.println(highCriticalityPeriod);
         myCaptureRate = getCaptureRate(MAX_CRITICALITY_LEVEL, numberOfCoverSets);
         startHighCriticalityPeriod = millis();
         Serial.println(startHighCriticalityPeriod);
       }
       else
         myCaptureRate = getCaptureRate(criticalityLevel, numberOfCoverSets);

       Serial.print(F("Computed capture rate is "));
       Serial.println(myCaptureRate);       
       
       long interFrame = (long)round(1/myCaptureRate*1000.0);
       
       if (useCriticalityScheduling) {
         interCamIntrusionDetectionTime = interFrame;
       
         Serial.print(F("Set inter-snapshot time to "));
         Serial.println(interCamIntrusionDetectionTime);
       }  
       else {
         Serial.print(F("Inter-snapshot time would be "));
         Serial.println(interFrame);         
       }
}

void setCaptureRateLight() {
       // get the capture rate from default values for criticality level and number of cover-sets
       float myCaptureRate;
      
       // if we have detected an intrusion and have a highCriticalityPeriod
       if (detectedIntrusion && highCriticalityPeriod && startHighCriticalityPeriod==0) {
           Serial.print(F("Enters high criticality period for "));
           Serial.println(highCriticalityPeriod);
           myCaptureRate = getCaptureRate(MAX_CRITICALITY_LEVEL, numberOfCoverSets);
           startHighCriticalityPeriod = millis();
           Serial.println(startHighCriticalityPeriod);
       }
       else {
           // still in highCriticalityPeriod?
           if (startHighCriticalityPeriod && (millis() - startHighCriticalityPeriod < highCriticalityPeriod)) {
             Serial.println(F("Still in high criticality period"));
             // keep the current interCamIntrusionDetectionTime
             return;
           }
           
           if (startHighCriticalityPeriod && (millis() - startHighCriticalityPeriod > highCriticalityPeriod)) {
              Serial.println(F("Exits high criticality period, was for "));
              Serial.println(millis()-startHighCriticalityPeriod);
              startHighCriticalityPeriod=0L;
           }
            
           myCaptureRate = getCaptureRate(criticalityLevel, numberOfCoverSets);  
       }
       
       long interFrame = (long)round(1/myCaptureRate*1000.0);
       
       if (useCriticalityScheduling) {
           interCamIntrusionDetectionTime = interFrame;  
       }
}

#endif

void immediateComparePicture(int delay_before_send, int delay_after_send) {
        
        Serial.print(F("Immediate compare picture"));
        Serial.print(" in ");
        Serial.print(delay_before_send);
        Serial.println("ms");
        delay(delay_before_send);        
        
#ifdef WITH_LCD
        printLCD(1, F("Compare Image   "), false);
#endif 
        Serial.println(F("Getting new picture")); 
        
        camDataReady=false;
        
        if (myCam.send_initial()) {
                if (myCam.do_snapshot()) {
                    if (myCam.get_picture(PICTURE_SNAPSHOT_TYPE)) {
#ifdef ENERGY_TEST                      
                        digitalWrite(capture_led, HIGH);
#endif                        
                        myCam.get_data();
                        camDataReady=true;
                    }
                }
        }    
        
        // to save energy as the picture has already been taken
        digitalWrite(ucamLedArray[currentCam], LOW);

#ifdef ENERGY_TEST_TIMING                  
        set_led(capture_led, 100);
#endif        

        display_ucam_stats();
        
        if (camDataReady) {
#ifdef PERIODIC_IMG_TRANSMIT
            // we always set nbPixDiff so that an intrusion is detected
            long nbPixDiff=INTRUSION_THRES+1;
#else
            long nbPixDiff=compare_raw_picture_data();
#endif

#ifdef ENERGY_TEST_TIMING                  
        set_led(capture_led, 100);
#endif

            Serial.print(F("nb diff. pixel : "));
            Serial.println(nbPixDiff);
            
            if (nbPixDiff > INTRUSION_THRES) {
                  Serial.println(F("POTENTIAL intrusion"));
                  
                  nbIntrusion++;
                  detectedIntrusion=true;
#ifdef CRITICALITY_SCHEDULING                  
                  setCaptureRate();
#endif                  
                  if (send_image_on_intrusion) {
                        // activate encoding and transmission on inImage
                        transmitting_data = true;
                        encode_ucam_file_data();
                        transmitting_data = false;
                        // we have to define a new reference image
                        if (new_ref_on_intrusion)
                          copy_in_refImage();   
                        
                  }
            } else 
                  Serial.println(F("Maybe NO intrusion"));      
        }
        else
          Serial.println(F("Error: will try again"));
          
        Serial.print(nbIntrusion); 
        Serial.println(F(" intrusions detected so far"));
            
        digitalWrite(capture_led, LOW);

        delay(delay_after_send);
        
#ifdef WITH_LCD       
        printInfoLCD();
#endif  
}

void sync_cam(int cam_index) {
        camHasSync=false;
        
        if (cam_index==-1) {
                for (int cam=0; cam<NB_UCAM; cam++) {
                  
                      Serial.print(F("--->>> Synching cam "));
                      Serial.println(cam);
                      
                      myCam.set_camPort(ucamArray[cam]);
                      
                      myCam.cam_sync=false;
                      
                      digitalWrite(cam_sync_led, LOW);
                      
                      while (!myCam.cam_sync) {
                        myCam.init();

                        //ucamArray[cam]->begin(38400);
                      }
                      
                      myCam.send_ack();
                      
                      digitalWrite(cam_sync_led, HIGH);
                      
                      camHasSync=true;

                      //ucamArray[cam]->begin(MY_UCAM_BAUDRATE);
                }
        }
        else {
                      Serial.print(F("--->>> Synching cam "));
                      Serial.println(cam_index);
                      
                      myCam.set_camPort(ucamArray[cam_index]);
                      
                      myCam.cam_sync=false;
                      
                      digitalWrite(cam_sync_led, LOW);
                      
                      while (!myCam.cam_sync) {
                        myCam.init();

                        //ucamArray[cam_index]->begin(38400);
                      }
                  
                      myCam.send_ack();
                      
                      digitalWrite(cam_sync_led, HIGH);
                      
                      camHasSync=true;  

                      //ucamArray[cam_index]->begin(MY_UCAM_BAUDRATE);
        }
}

#ifdef XBEE_UCAM

void setSrcNodeAddr() {
        uint16_t mySrcNodeAddr;    
        
        // we set the 16-bit src node addr by taking the last 4-digit of the 64-bit MAC address
        mySrcNodeAddr=(uint16_t)strtol((const char*)&macAddr[12], NULL, 16);  
        // high part
        pktPreamble[2]=mySrcNodeAddr >> 8 & 0xff; 
        Serial.print(pktPreamble[2], HEX);
        // low part
        pktPreamble[3]=mySrcNodeAddr & 0xff;
        Serial.println(pktPreamble[3], HEX);
}
#endif

#ifdef LORA_UCAM

void startLoraConfig() {

    int e;
            
    // Set transmission mode and print the result
    e = sx1272.setMode(loraMode);
    
    Serial.print(F("Setting LoRa mode: state "));
    Serial.println(e, DEC);

#ifdef LORA_LAS
    loraLAS.setSIFS(loraMode);
#else
    // enable carrier sense
    sx1272._enableCarrierSense=true;  
#ifdef LOW_POWER
    // TODO: with low power, when setting the radio module in sleep mode
    // there seem to be some issue with RSSI reading
    sx1272._RSSIonSend=false;
#endif  
#endif

    // Select frequency channel
    e = sx1272.setChannel(loraChannel);
    Serial.print(F("Setting Channel: state "));
    Serial.println(e, DEC);
  
    // Select amplifier line; PABOOST or RFO
#ifdef PABOOST
    sx1272._needPABOOST=true;
    // previous way for setting output power
    // powerLevel='x';
#else
    // previous way for setting output power
    // powerLevel='M';  
#endif

    Serial.print(F("Set LoRa Power (dBm) to "));
    Serial.println(MAX_DBM);  
    
    e = sx1272.setPowerDBM((uint8_t)MAX_DBM);
    
    Serial.print(F("Setting Power: state "));
    Serial.println(e);
    
    // get preamble length
    e = sx1272.getPreambleLength();
    Serial.print(F("Getting Preamble Length: state "));
    Serial.println(e); 
    Serial.print(F("Preamble Length: "));
    Serial.println(sx1272._preamblelength);
    
    // Set the node address and print the result
    e = sx1272.setNodeAddress(loraAddr);
    Serial.print(F("Setting LoRa addr "));
    Serial.print(loraAddr);
    Serial.print(F(" : state "));
    Serial.println(e, DEC);
}
#endif

void setup() {                  
        boolean ok_to_read_picture_data=true;
        boolean ok_to_encode_picture_data=true;

        delay(2000);
        randomSeed(analogRead(14));
                    
        Serial.begin(115200);
        Serial.println(F("Init uCam test."));

#ifdef XBEE_POWER_SAVING  
        pinMode(xbee_power_pin, OUTPUT);
        digitalWrite(xbee_power_pin, HIGH);
        delay(500);
#endif
        pinMode(capture_led, OUTPUT);
        digitalWrite(capture_led, LOW);
        pinMode(cam_sync_led, OUTPUT);
        digitalWrite(cam_sync_led, LOW);
        
        for (int k=0; k<NB_UCAM; k++) {
            pinMode(ucamLedArray[k], OUTPUT);
            digitalWrite(ucamLedArray[k], LOW);          
        }
        
#ifdef WITH_LCD
        lcd.init();
        lcd.backlight();
        lcd.home();
        lcd.print(F("Image Sensor"));      
#endif         

#ifdef XBEE_UCAM 
        Serial.println(F("Init XBee 802.15.4")); 
#ifdef WITH_LCD
        printLCD(1, F("Init802"), false);
#endif 

#if defined CAM_3 || defined XBEE_UART0
        // with 3 camera, we have to use Serial (UART0) for xbee, so can not have serial monitor for print
        xbee.setSerial(Serial);
        // Inits the XBee 802.15.4 library
        Serial.begin(MY_XBEE_BAUDRATE); 
#else
        // The XBee on the Due or the Teensy32 should be connected to Serial3
        // Serial3 (RX,TX)=(7,8) for Teensy32
        xbee.setSerial(Serial3);
        // Inits the XBee 802.15.4 library
        Serial3.begin(MY_XBEE_BAUDRATE);      
#endif
        delay(100);
        
        // set command to SH to get high part of MAC address
        atRequest.setCommand(shCmd);  
        while (!sendAtCommand()) {Serial.println(F("retry command ATSH"));};
 
        char adr_part[8];

        // get each value, convert it to string and append to the adr
        for (int i = 0; i < atResponse.getValueLength(); i++) {
              sprintf(adr_part, "%lX", (unsigned long)(atResponse.getValue()[i]));
    
              if (atResponse.getValue()[i]<=0xF) 
                macAddr += 0;
        
              macAddr += adr_part;
        } 
        
        // set command to SL to get low part of MAC address
        atRequest.setCommand(slCmd);  
        while (!sendAtCommand()) {Serial.println(F("retry command ATSL"));};
  
        for (int i = 0; i < atResponse.getValueLength(); i++) {
              sprintf(adr_part, "%lX", (unsigned long)(atResponse.getValue()[i]));
    
              if (atResponse.getValue()[i]<=0xF) 
              macAddr += 0;
        
              macAddr += adr_part;    
        } 
#ifdef XBEE_CUSTOM_BAUDRATE        
        // if we have an xbee module at 125000, it means that it has mac mode 0 for remote AT command
        // then switch it to MM2. Note that this is not definitive since ATWR is not issued
        setMMMode(2);
#ifdef WITH_LCD
        printLCD(1, "Set XBee in MM2 ", false);
        delay(1000);
#endif         
#endif  

        // set command to MM to get the XBee mac mode
        atRequest.setCommand(mmCmd);
        
        int counter = 4;
        while (!sendAtCommand() && counter > 0) {
              Serial.println(F("retry command ATMM"));
              counter--;
        }
        
        if (counter > 0) {
                myMacMode=(int)atResponse.getValue()[0];
        }
        else
                Serial.print(F("ERROR AT+MM"));

        sprintf(banner,"-mac:%s WAITING for command from 802.15.4 interface. XBee mac mode %d", (const char*)&macAddr[0], myMacMode);
        
        Serial.println(banner);        
        Serial.println(F("Wait for command \@D0013A20040762053#T60# to capture and send image with an inter-pkt time of 60ms to 0013A20040762053"));
        Serial.print(F("Current destination: "));
        Serial.println(destMACAddr);

        // set the destination addr and set the node's 16-bit addr if the destination addr is also in 16-bit format  
        setAddresses();          
        
#ifdef SRC_ADDR_FROM_64MAC
        setSrcNodeAddr();   
#endif
        
#ifdef WITH_LCD
        printLCD(1, (const char*)&macAddr[0], false);
        delay(2000);
        
        printInfoLCD();
#endif         

#endif// XBEE_UCAM

#ifdef LORA_UCAM

        // Print a start message
        Serial.println(F("Init SX1272 LoRa module: send packets without ACK"));
        int e;
        
        // Power ON the module
        e=sx1272.ON();

        Serial.print(F("Setting power ON: state "));
        Serial.println(e, DEC);
        
        if (!e) {
          loraRadioOn=true;
          startLoraConfig();
        }
        
        MSS=DEFAULT_LORA_MSS;
       
        Serial.print(F("SX1272 configured "));
        Serial.println(F("Init UARTs for uCam board"));

#ifdef CAM_3
        Serial1.begin(MY_UCAM_BAUDRATE);
        Serial2.begin(MY_UCAM_BAUDRATE);
        Serial3.begin(MY_UCAM_BAUDRATE);
#else
#ifdef CAM_2
        Serial1.begin(MY_UCAM_BAUDRATE);
        Serial2.begin(MY_UCAM_BAUDRATE);
#else
        Serial1.begin(MY_UCAM_BAUDRATE);
#endif        
#endif     
        
        for (int cam=0; cam<NB_UCAM; cam++) {

                QualityFactor[cam] = DEFAULT_QUALITY_FACTOR;

                digitalWrite(ucamLedArray[cam], HIGH); 
                blink_led(cam_sync_led, 5+cam*3);
                
                Serial.print(F("--->>> Initializing cam "));
                Serial.println(cam);
                
                myCam.set_camPort(ucamArray[cam]);
                
                digitalWrite(cam_sync_led, LOW);
                digitalWrite(capture_led, HIGH);
                
                while (!myCam.cam_sync) {
                  myCam.init();

                  if (!myCam.cam_sync) {
                    ucamArray[cam]->end();
                    ucamArray[cam]->begin(MY_UCAM_BAUDRATE);
                    delay(100);
                    myCam.send_reset();
                  }
                }
            
                myCam.send_ack();                
                camHasSync=true;  
                //ucamArray[cam]->begin(MY_UCAM_BAUDRATE);
                
                digitalWrite(capture_led, LOW);
                digitalWrite(cam_sync_led, HIGH);                
                delay(1000);
                digitalWrite(cam_sync_led, LOW);
                delay(1000);
                digitalWrite(ucamLedArray[cam], LOW);
        }        

        if (NB_UCAM>1)
            // indicates that all cams have sync
            digitalWrite(cam_sync_led, HIGH);

        Serial.println(F("Cams have synched"));
        
        inImage.imageVsize=inImage.imageHsize=CAMDATA_PGM_LINE_SIZE;
        outImage.imageVsize=outImage.imageHsize=CAMDATA_PGM_LINE_SIZE;
        
        // allocate memory to store the image from ucam
        if ((inImage.data = AllocateUintMemSpace(inImage.imageHsize, inImage.imageVsize))==NULL) {
              Serial.println(F("Error calloc inImage"));
              ok_to_read_picture_data=false;
        }
        
        for (int k=0; k<NB_UCAM; k++)
              if (useRefImage) {
                    if ((refImage[k].data = AllocateUintMemSpace(CAMDATA_PGM_LINE_SIZE, CAMDATA_PGM_LINE_SIZE))==NULL) {
                          Serial.println(F("Error calloc refImage"));
                          ok_to_read_picture_data=false;                      
                          blink_led(cam_sync_led, 5);
                          blink_led(capture_led, k+1);
                    }
              }
              else
                 refImage[k].data=NULL;     

        Serial.println(F("InImage memory allocation passed"));

#ifndef CRAN_NEW_CODING
#ifdef SHORT_COMPUTATION
       Serial.println(F("OutImage using short"));
       if ((outImage.data = AllocateShortMemSpace(outImage.imageHsize, outImage.imageVsize))==NULL) {
#else
       Serial.println(F("OutImage using float"));
       if ((outImage.data = AllocateFloatMemSpace(outImage.imageHsize, outImage.imageVsize))==NULL) {
#endif

           Serial.println(F("Error calloc outImage"));
           ok_to_encode_picture_data=false;
       }
       else {
           Serial.println(F("OutImage memory allocation passed"));
       }        
#endif

       if (!ok_to_read_picture_data || !ok_to_encode_picture_data) {
           Serial.println(F("Sorry, stop process"));
           digitalWrite(cam_sync_led, HIGH); 
           digitalWrite(capture_led, HIGH);
           while (1)
           ;
       }
       else
          Serial.println(F("Ready to encode picture data"));

#ifdef XBEE_POWER_SAVING                  
        sleep_xbee();
#endif

#ifdef ENERGY_TEST
        // ENERGY1
        blink_led(capture_led, 10);
        digitalWrite(cam_sync_led, LOW);
        // after ENERGY1, wait for 36s before starting the energy capture
        delay(40000);
        set_led(capture_led, 500);
#endif

#ifdef GET_PICTURE_ON_SETUP

#ifdef WITH_LCD
        printLCD(1, F("Get first image "), false);
#endif

#ifdef TEST_FOCUS_CAM0
        // we capture and send picture contineously to set focus of the camera
        // only use camera 0
        int cam=0;
        
        while (1) {
#else          
        for (int cam=0; cam<NB_UCAM; cam++) {
#endif
                Serial.print(F("--->>> Get reference image from uCam "));
                Serial.println(cam);
                
                digitalWrite(ucamLedArray[cam], HIGH);
                // we have to re-sync the cam in case the cam went to sleep mode because of delays
                sync_cam(cam);

#ifdef ENERGY_TEST_TIMING
                set_led(capture_led, 200);
#endif

#ifndef ENERGY_TEST                
                blink_led(capture_led, 5+cam*3);
                
                delay(1000);
#endif                
                currentCam=cam;
                
                myCam.set_camPort(ucamArray[currentCam]);
                
                if (myCam.send_initial()) {
                    if (myCam.do_snapshot()) {
                        if (myCam.get_picture(PICTURE_SNAPSHOT_TYPE)) {
#ifndef ENERGY_TEST                          
                            digitalWrite(capture_led, HIGH);
#endif
                            myCam.get_data();
                            camDataReady=true;
                        }
                    }
                }    

#ifdef ENERGY_TEST_TIMING                  
                set_led(capture_led, 100);
#endif                
                display_ucam_stats();
                
                if (camDataReady && myCam.camMode == UCAM_RAW) {
                   
                    if (ok_to_read_picture_data) {
#ifdef ENERGY_MEASURE_VOLTMETER                    
                        digitalWrite(cam_sync_led, LOW);
                        set_led(capture_led, 3000);
                        delay(2000);
#endif                        
                        get_raw_picture_data();
#ifdef ENERGY_MEASURE_VOLTMETER                        
                        set_led(capture_led, 3000);
                        delay(2000);
#endif                                       
                    }
        	}

                digitalWrite(ucamLedArray[currentCam], LOW);

#ifdef ENERGY_TEST_TIMING                  
                set_led(capture_led, 100);
#endif

#ifdef DISPLAY_PGM      
               if (ok_to_read_picture_data)
                   display_ucam_data();
#endif

               if (ok_to_encode_picture_data) {

                   blink_led(cam_sync_led, 3);
#ifndef QUALITY_TEST   
                   transmitting_data=true;                
                   encode_ucam_file_data();
                   // copy in the refImage to serve as a reference image
                   copy_in_refImage();                    
#endif                   
                   transmitting_data=false;

#ifdef ENERGY_TEST
                  // ENERGY2
                  set_led(capture_led, 500);
#endif

#ifdef QUALITY_TEST            
                  delay(500);
#ifdef ENERGY_MEASURE_VOLTMETER                  
                  digitalWrite(cam_sync_led, LOW);
                  set_led(capture_led, 5000);
#endif                  
                  
                  for (int i=100; i>5; i=i-10) {  
#ifdef LORA_UCAM                 
                    // we can compare with an MSS of 90
                    //MSS=DEFAULT_MSS;
#endif                   
                    QualityFactor[currentCam]=i;     
                    //encode_ucam_file_data();
                    delay(500);
#ifdef LORA_UCAM                    
                    MSS=DEFAULT_LORA_MSS;
                    encode_ucam_file_data();
#endif                   
                  }
#ifdef LORA_UCAM                    
                  //MSS=DEFAULT_MSS;
#endif                  
                  QualityFactor[currentCam]=5;
                  //encode_ucam_file_data();
#ifdef LORA_UCAM
                  delay(500);
                  MSS=DEFAULT_LORA_MSS;
                  encode_ucam_file_data();
#endif                  
                  // copy in the outImage memory array to serve as a reference image
                  copy_in_refImage();
                  // set back the quality factor for normal operation
#ifdef LORA_UCAM                  
                  QualityFactor[currentCam]=DEFAULT_LORA_QUALITY_FACTOR;
#else
                  QualityFactor[currentCam]=DEFAULT_QUALITY_FACTOR
#endif

#ifdef ENERGY_MEASURE_VOLTMETER               
              set_led(capture_led, 5000);
#endif 

#endif//QUALITY_TEST             
               }

              digitalWrite(capture_led, LOW);
              blink_led(cam_sync_led, 3);
              digitalWrite(cam_sync_led, LOW);

#ifdef LUM_HISTO
              long startComputeHisto=millis();      
              
              computeHistogram(histoRefImage[currentCam], inImage.data);
              
              long startComputeLum=millis();
                    
              refImageLuminosity[currentCam]=computeMeanLuminosity(histoRefImage[currentCam]);

              Serial.println(startComputeLum-startComputeHisto);
              Serial.println(millis()-startComputeLum);
              
              Serial.print(F("Reference mean luminosity is "));
              Serial.println(refImageLuminosity[currentCam]);              
#endif

              lastCamSyncTime=millis();
#ifdef TEST_FOCUS_CAM0
              delay(10000);
#else              
              delay(1000);
#endif              
              digitalWrite(ucamLedArray[cam], LOW);   
        }
#endif
       // just in case, set to use cam 0
       currentCam=0; 
       
#ifdef CRITICALITY_SCHEDULING
       setCaptureRate();       
#endif

#ifdef WITH_LCD         
       printInfoLCD();
#endif       
       delay(1000);
       
       lastCamRefImageTime=lastCamIntrusionDetectionTime=millis();
       
#ifdef LORA_LAS
        loraLAS.ON(LAS_ON_WRESET);
        //delay(random(1000,3000));
        //loraLAS.sendReg();
#endif        
#endif       
}

void loop() 
{
        boolean triggerCamSync=false;
        boolean triggerRefImage=false;
        boolean triggerIntrusionDetection=false;
        boolean receivedCmd=false;

#ifdef LORA_LAS
        loraLAS.checkCycle();
#endif

#ifdef XBEE_UCAM        
        // read incoming pkt, to get cmd
        xbee.readPacket();

        if (xbee.getResponse().isAvailable()) {

                if (xbee.getResponse().getApiId() == RX_64_RESPONSE || xbee.getResponse().getApiId() == RX_16_RESPONSE) {

                        if (xbee.getResponse().getApiId() == RX_64_RESPONSE) {
                          
                                xbee.getResponse().getRx64Response(rx64);
                                option = rx64.getOption();
                                rcv_rawData = rx64.getFrameData();
                                rcv_data = rx64.getData();
                                rcv_data_len = rx64.getDataLength();
                        }
                        
                        if (xbee.getResponse().getApiId() == RX_16_RESPONSE) {
                          
                                xbee.getResponse().getRx16Response(rx16);
                                option = rx16.getOption();
                                rcv_rawData = rx16.getFrameData();
                                rcv_data = rx16.getData();
                                rcv_data_len = rx16.getDataLength();
                        }                        
 
                        Serial.print(F("RCVCMD802:"));
                        receivedCmd=true;     
                }
        }
#endif

#ifdef LORA_UCAM

        // Receive message
        int e = sx1272.receivePacketTimeout(1000);
        
        if (!e) { 

           int a=0, b=0;
           uint8_t tmp_length;
           
           Serial.print("Received from LoRa: ");
           receivedCmd=true;

           tmp_length=sx1272.packet_received.length-OFFSET_PAYLOADLENGTH;
           
#ifdef LORA_LAS        
           if (loraLAS.isLASMsg(sx1272.packet_received.data)) {
             
             int v=loraLAS.handleLASMsg(sx1272.packet_received.src,
                                      sx1272.packet_received.data,
                                      tmp_length);
           
             if (v==DSP_DATA) {
               a=LAS_DSP+DATA_HEADER_LEN+1;
               b=LAS_FRAMING_LEN+DSP_LEN+DATA_HEADER_LEN;
             }
             else
               receivedCmd=false; 
           }
           else
             Serial.println("No LAS header detected. Taking raw data");
#endif           
           // make rdv_data to point to the received data from the SX1272 module       
           rcv_data = sx1272.packet_received.data+a;
           rcv_data_len = tmp_length-b;
        }
#endif

#if not defined CAM_3 && not defined XBEE_UART0

        uint8_t scmd[80];
                  
        if (Serial.available()) {
      
          int sb=0;  
      
          while (Serial.available() && sb<80) {
            scmd[sb]=Serial.read();
            sb++;
            delay(50);
          }
          
          scmd[sb]='\0';
            
          Serial.print("Received from serial: ");

          rcv_data = scmd;
          rcv_data_len = sb;    
          receivedCmd=true;     
        }
#endif

        if (receivedCmd) {

                        blink_led(capture_led, 5);
                        
                        for (int f=0; f < rcv_data_len; f++) {
                                Serial.print((char)rcv_data[f]);
                        }
                          
                        int i=0;                        
                        int cmdValue;
                        long longCmdValue;
                        
                        // we use prefix '/@' to indicate a command string
                        if ((char)rcv_data[i]=='/' && (char)rcv_data[i+1]=='@') {
                        
                                Serial.println(F("\nParsing command string"));
                                
                                i=2;
                        
                                // parse the received command
                                // typical cmd is "C1#Q40#S1#" for instance
                                while (i < rcv_data_len-1 && rcv_data_len<MAX_CMD_LENGTH) {
#ifdef WITH_LCD                                                
                                        printLCD(1, (char*)(rcv_data+2), true);
                                        delay(1000);
#endif                                                
                                        switch ((char)rcv_data[i]) {
                                                                                  
                                        // T130# -> start sending with inter pkt time of 130ms
                                        case 'T': {
                                                                
                                                i++;
                                                longCmdValue=getCmdValue(i);
                                                // cannot set an inter pkt time smaller than MIN_INTER_PKT_TIME
                                                if (longCmdValue < MIN_INTER_PKT_TIME)
                                                        longCmdValue = MIN_INTER_PKT_TIME;
                                                // set new inter packet time        
                                                inter_binary_pkt=longCmdValue; 
                                                
                                                transmitting_data=true;
                                                immediateSendPicture(500,3500); 
                                                transmitting_data=false;
                                                // copy into outImage since reference image has changed
                                                copy_in_refImage();                              
                                        }      
                                        break;  
                                        
                                        // S0/1# -> start getting snapshot and compare with stored image, if 1, send on intrusion
                                        case 'S': {
                                                i++;
                                                cmdValue=getCmdValue(i);
                                                
                                                if (cmdValue==0)
                                                      send_image_on_intrusion=false;
                                                else 
                                                      send_image_on_intrusion=true; 
                                                
                                                if (useRefImage)
                                                    immediateComparePicture(500,3500);
                                                // only for this one
                                                send_image_on_intrusion=false;                              
                                        }      
                                        break;  
                                        
                                        // I# -> start getting new reference image
                                        case 'I': 

                                                if ((char)rcv_data[i+1]=='#') {

                                                    transmitting_data=false;
                                                    immediateSendPicture(500,3500);
                                                    // copy into outImage
                                                    copy_in_refImage();  
                                                }
                                                else {
                                                      if ((char)rcv_data[i+1]=='0') {
                                                          Serial.print(F("Disable "));
                                                          new_ref_on_intrusion=false;
                                                      }
                                                      else {
                                                          Serial.print(F("Enable "));
                                                          new_ref_on_intrusion=true;
                                                      }
                                                      
                                                      Serial.println(F("new ref image on intrusion")); 
                                                }
                                                
                                                break;                                          
                                        
                                        // set the pkt size for binary mode, default is 100
                                        // "Z50#"
                                        case 'Z':
                                        
                                                i++;
                                                cmdValue=getCmdValue(i);
                                                // cannot set pkt size greater than MAX_PKT_SIZE
                                                if (cmdValue > MAX_PKT_SIZE)
                                                        cmdValue = MAX_PKT_SIZE;
                                                // cannot set pkt size smaller than MAX_PKT_SIZE
                                                if (cmdValue < MIN_PKT_SIZE)
                                                        cmdValue = MIN_PKT_SIZE;
                                                // set new pkt size        
                                                MSS=cmdValue; 
                                                
                                                Serial.print(F("Set MSS to "));
                                                Serial.println(MSS);
                                        break;      
                                        
                                        // set the quality factor, default is 50
                                        // "Q40#"
                                        case 'Q':
                                        
                                                i++;
                                                cmdValue=getCmdValue(i);
                                                // cannot set pkt size greater than MAX_PKT_SIZE
                                                if (cmdValue > 100)
                                                        cmdValue = 100;
                                                // cannot set pkt size smaller than MAX_PKT_SIZE
                                                if (cmdValue < 5)
                                                        cmdValue = 5;
                                                // set new pkt size        
                                                QualityFactor[currentCam]=cmdValue; 
                                                
                                                Serial.print(F("Set Q to "));
                                                Serial.println(QualityFactor[currentCam]);
                                                
                                                break;  
                                                                                                                                                   
                                        // command to set the destination, override the default destination "D0013A20040762053#" will send to 0013A20040762053 
                                        // use D000000000000FFFF for broadcast again 
                                        case 'D':  
#ifdef XBEE_UCAM                                                             
                                                if (  ((char)rcv_data[i+1] >= '0' && (char)rcv_data[i+1] <= '9') || ((char)rcv_data[i+1] >= 'A' && (char)rcv_data[i+1] <= 'F')) {
                                                        Serial.print(F("Set dest MAC addr to "));    
        
                                                        i++;
                                                        cmdValue=getCmdValue(i, destMACAddr);
                                                        
                                                        if (strlen(destMACAddr)!=16 && strlen(destMACAddr)!=4) {
                                                                Serial.print(F("bad address, need 16 or 4 bytes in hex format. Restore default destination "));  
                                                                strcpy(destMACAddr, FINAL_DEST_MAC_ADDR);
                                                        }                                                                
                                                            
                                                        Serial.println(destMACAddr);
                                                        
                                                        setAddresses();
                                                }     
#endif

#ifdef LORA_UCAM
                                                i++;
                                                cmdValue=getCmdValue(i);
                                                // cannot set dest addr greater than 255
                                                if (cmdValue > 255)
                                                        cmdValue = 255;
                                                // cannot set dest addr lower than 0, 0 is broadcast
                                                if (cmdValue < 0)
                                                        cmdValue = 0;
                                                // set dest addr        
                                                LORA_DEST_ADDR=cmdValue; 
                                                
                                                Serial.print(F("Set LoRa dest addr to "));
                                                Serial.println(LORA_DEST_ADDR);
#endif
                                                break;

#ifdef XBEE_UCAM
                                        // command to set the node's short address to the last 4 digit of the 64-bit MAC address, "Y0#" or "Y1#" 
                                        case 'Y':  
                                              
                                                if ((char)rcv_data[i+1]=='0')
                                                        setMYAddr(SHORT_ADDR_FFFF);
                                                else 
                                                        setMYAddr(SHORT_ADDR_FROM_64MAC);                                                     
                                                                           
                                                break;                                        
#endif                                        
                                        // set inter-frame time, in ms, default is 30s                                                                                                                                      
                                        case 'F':

                                                i++;
                                                longCmdValue=getCmdValue(i);
                                                // cannot set an inter snapshot time smaller than MIN_INTER_SNAPSHOT_TIME
                                                if (longCmdValue < MIN_INTER_SNAPSHOT_TIME)
                                                        longCmdValue = MIN_INTER_SNAPSHOT_TIME;
                                                // set new inter snapshot time        
                                                interCamIntrusionDetectionTime=longCmdValue; 
                                                
                                                Serial.print(F("Set inter-snapshot time to "));
                                                Serial.println(interCamIntrusionDetectionTime);

                                                useCriticalityScheduling=false;
                                                        
                                                break;

                                                
                                        // CL or CM or CO or CT or CB or C command        
                                        case 'C':
#ifdef CRITICALITY_SCHEDULING                                        
                                                // CL command to set the criticality level
                                                if ((char)rcv_data[i+1]=='L') {
                                                        i++;
                                                        cmdValue=getCmdValue(++i);
                                                        
                                                        float theCriticalityLevel=(float)cmdValue/10.0;
                                                         
                                                        if (theCriticalityLevel < MIN_CRITICALITY_LEVEL || theCriticalityLevel > MAX_CRITICALITY_LEVEL) {
                                                            Serial.println(F("Criticality level out of range, keep last value"));
                                                        }
                                                        else
                                                           criticalityLevel=theCriticalityLevel;
                                                           
                                                        setCaptureRate();       
                                                }
                                                else
                                                // CM command to set the maximum number of cover-sets
                                                if ((char)rcv_data[i+1]=='M') {
                                                        i++;
                                                        cmdValue=getCmdValue(++i);
                                                        
                                                         
                                                        if (cmdValue < 6 || cmdValue > 12) {
                                                            Serial.println(F("Maximum number of cover-sets out of range, keep last value"));
                                                        }
                                                        else
                                                           MAX_COVER_SET=cmdValue;
                                                           
                                                        setCaptureRate();       
                                                }                                                                                             
                                                else 
                                                // CO command to set the number of cover-sets
                                                if ((char)rcv_data[i+1]=='O'){
                                                        i++;
                                                        cmdValue=getCmdValue(++i);
                                                         
                                                        if (cmdValue < MIN_COVER_SET || cmdValue > MAX_COVER_SET) {
                                                            Serial.println(F("Number of cover-set out of range, keep last value"));
                                                        }
                                                        else
                                                           numberOfCoverSets=cmdValue;
                                                        
                                                        setCaptureRate();                                                        
                                                }
                                                else
                                                // CT command to set the high criticality period
                                                if ((char)rcv_data[i+1]=='T'){
                                                        i++;
                                                        cmdValue=getCmdValue(++i);
                                                         
                                                        if (cmdValue > MAX_HIGH_CRITICALITY_PERIOD) {
                                                            Serial.println(F("High criticality period out of range, keep last value"));
                                                        }
                                                        else
                                                           // in ms
                                                           highCriticalityPeriod=cmdValue*1000;                                                   
                                                }
                                                else                  
                                                // CB command to start the criticality-based scheduling                              
                                                if ((char)rcv_data[i+1]=='B'){
                                                        useCriticalityScheduling=true;
                                                        setCaptureRate();                                                  
                                                }
#endif
#ifdef LORA_UCAM
#ifdef CRITICALITY_SCHEDULING
                                                else
#endif                                                
                                                if ((char)rcv_data[i+1]=='S') {
                                                        i=i+2;
                                                        cmdValue=getCmdValue(i);
                                      
                                                        if (cmdValue > 3) {
                                                          PRINT_CSTSTR("%s","^$Carrier Sense method must be 0, 1, 2 or 3.\n");
                                                          cmdValue=2;    
                                                        }
                                      
                                                        carrier_sense_method=cmdValue;
                                      
                                                        PRINT_CSTSTR("%s","^$Selected carrier Sense method is ");  
                                                        PRINT_VALUE("%d",carrier_sense_method);
                                                        PRINTLN;
                                                }
#endif
                                                              
                                                // C command to set the current camera 0/1/2
                                                if ((char)rcv_data[i+1]>='0' && (char)rcv_data[i+1]<='2') { 
                                                        i++;
                                                        cmdValue=getCmdValue(i);
                                                        // cannot set current cam greater than NB_UCAM-1
                                                        if (cmdValue > NB_UCAM-1)
                                                                cmdValue = 0;
                                                        // set currentCam        
                                                        currentCam=cmdValue; 
                                                        
                                                        myCam.set_camPort(ucamArray[currentCam]);
                                                        
                                                        Serial.print(F("Set current cam to "));
                                                        Serial.println(currentCam);          
                                                }
                                                break;                                                

                                        case 'L':
#ifdef LORA_UCAM
                                              if ((char)rcv_data[i+1]=='O' &&
                                                  (char)rcv_data[i+2]=='R' &&         
                                                  (char)rcv_data[i+3]=='A') {
                                                  
                                                  i+=4;
                                                
                                                  switch ((char)rcv_data[i]) {  
                                                    case 'M': {
                                                      i++;
                                                      cmdValue=getCmdValue(i);
                                                      
                                                      // cannot set mode greater than 10
                                                      if (cmdValue > 10)
                                                              cmdValue = 4;
                                                      // cannot set mode lower than 0
                                                      if (cmdValue < 0)
                                                              cmdValue = 4;
                                                      // set dest addr        
                                                      loraMode=cmdValue; 
                                                      
                                                      Serial.print(F("Set LoRa mode to "));
                                                      Serial.println(loraMode);
                                        
                                                      // Set transmission mode and print the result
                                                      e = sx1272.setMode(loraMode);
                                                      Serial.print(F("Setting LoRa mode: state "));
                                                      Serial.println(e, DEC);     
                                                    }
                                                    break;
                                                    
                                                    case 'C': {
                                                      i++;
                                                      cmdValue=getCmdValue(i);
                                                      
                                                      // cannot set channel greater than 17
                                                      if (cmdValue > 17)
                                                              cmdValue = 10;
                                                      // cannot set channel lower than 10
                                                      if (cmdValue < 10)
                                                              cmdValue = 10;
                                                      // set dest addr        
                                                      loraChannel=loraChannelArray[cmdValue-10]; 
                                                      
                                                      Serial.print(F("Set LoRa channel to "));
                                                      Serial.println(cmdValue);             
                                        
                                                      // Select frequency channel
                                                      e = sx1272.setChannel(loraChannel);
                                                      Serial.print(F("Setting Channel: state "));
                                                      Serial.println(e, DEC);               
                                                    }
                                                    break;
                                        
                                                    case 'P': {
                                        
                                                      if (rcv_data[i+1]=='H' || rcv_data[i+1]=='L' || rcv_data[i+1]=='M' || rcv_data[i+1]=='x' || rcv_data[i+1]=='X') {
                                                        loraPower=rcv_data[i+1];
                                        
                                                        Serial.print(F("Set LoRa Power to "));
                                                        Serial.println(loraPower);                  

                                                        e = sx1272.setPower(loraPower);
                                                        Serial.print(F("Setting Power: state "));
                                                        Serial.println(e, DEC); 
                                                      }
                                                      else
                                                        Serial.println(F("Invalid Power. L, H, M, x or X accepted."));          
                                                    }
                                                    break;
                                                    
                                                    case 'A': {
                                                      i++;
                                                      cmdValue=getCmdValue(i);
                                                      
                                                      // cannot set addr greater than 255
                                                      if (cmdValue > 255)
                                                              cmdValue = 255;
                                                      // cannot set addr lower than 1 since 0 is broadcast
                                                      if (cmdValue < 1)
                                                              cmdValue = LORA_SRC_ADDR;
                                                      // set node addr        
                                                      loraAddr=cmdValue; 
                                                      
                                                      Serial.print(F("Set LoRa node addr to "));
                                                      Serial.println(loraAddr);  
                                                      
                                                      // Set the node address and print the result
                                                      e = sx1272.setNodeAddress(loraAddr);
                                                      Serial.print(F("Setting LoRa node addr: state "));
                                                      Serial.println(e, DEC);                  
                                                    }            
                                                    break;
                                                    
                                                    default:
                                        
                                                      Serial.println(F("Sorry, unrecognized command: waits for M, C, P, A, D commands only"));       
                                                      break;                                             
                                                  }                                                  
                                                  break;
                                                }
#endif
                                                i++;
                                                cmdValue=getCmdValue(i);
                                                // cannot set flow id greater than MAX_FLOW_ID
                                                if (cmdValue > MAX_FLOW_ID)
                                                        cmdValue = 0;
                                                        
                                                flowId=cmdValue; 
                                                
                                                Serial.print(F("Set flow id to "));
                                                Serial.println(flowId);
                                                
#ifdef WITH_LCD                                                        
                                                printInfoLCD(1);
#endif                                                        
                                                delay(1000);
                                                break;          
                                        
                                        case 'R':
                                                i++;
                                                cmdValue=getCmdValue(i);
                                                
                                                if (cmdValue==0)
#ifdef XBEE_UCAM                                                
                                                      // MAC mode 1, without ack
                                                      setMMMode(1);
                                                else 
                                                      // MAC mode 2, with ack
                                                      setMMMode(2);
#endif                                                      
                                                break;                                               
        
                                        default:
        
                                                Serial.println(F("Invalid cmd, skiping"));
                                                // skip everything until the '#' character
                                                i++;
                                                while ((char)rcv_data[i]!='#' && (i < rcv_data_len))
                                                        i++;                                                    
                                                         
                                                break;
                                        }
                                        
                                        // go to next command if any.
                                        i++;                                    
                                }
                        }
                        else {
                                Serial.println(F("\nHave not recognized a command prefix /@"));
                        }
                        
#ifdef WITH_LCD         
                        printInfoLCD();
#endif     
                }

        // have we wrapped around?
        if (millis()-lastCamSyncTime < 0) {
                if (millis()+ULONG_MAX-lastCamSyncTime > interCamSyncTime)
                        triggerCamSync=true;            
        }
        
        // have we wrapped around?
        if (millis()-lastCamRefImageTime < 0) { 
                // beacon every 5mins        
                if (millis()+ULONG_MAX-lastCamRefImageTime > interCamRefImageTime)
                        triggerRefImage=true;          
        }
        
        // have we wrapped around?
        if (millis()-lastCamIntrusionDetectionTime < 0) { 
                // beacon every 5mins        
                if (millis()+ULONG_MAX-lastCamIntrusionDetectionTime > interCamIntrusionDetectionTime)
                        triggerIntrusionDetection=true;          
        }        

#ifdef PERIODIC_REF_UPDATE

        if ( (millis()-lastCamRefImageTime > interCamRefImageTime || triggerRefImage) && camHasSync && useRefImage) {
            Serial.println(F("UPDATING REFERENCE IMAGE")); 
                
            transmitting_data=false;
            
            for (int cam=0; cam<NB_UCAM; cam++) {
              
                  Serial.print(F("--->>> Get reference image with ucam "));
                  Serial.println(cam);
                  
                  digitalWrite(ucamLedArray[cam], HIGH);
                  
                  currentCam=cam;
                  
                  myCam.set_camPort(ucamArray[currentCam]);
                  immediateSendPicture(500,3500);
                  
                  // copy into refImage
                  copy_in_refImage();
                  
                  digitalWrite(ucamLedArray[cam], LOW);
            }

            // just in case, set to use cam 0            
            currentCam=0;            
            lastCamRefImageTime=millis();
        }
#endif

#ifdef LOW_POWER 
        if (1) {

            Serial.println(F("Switch to power saving mode"));
        
#ifdef LORA_UCAM        
        
            int e = sx1272.setSleepMode();
        
            if (!e)
              Serial.println(F("Successfully switch LoRa module in sleep mode"));
            else  
              Serial.println(F("Could not switch LoRa module in sleep mode"));
          
            Serial.flush();
            delay(200);
#endif  

#if defined __MK20DX256__ || defined __MKL26Z64__ || defined __MK64FX512__ || defined __MK66FX1M0__
            timer.setTimer(LOW_POWER_PERIOD*1000);// milliseconds
#endif
            for (int i=0; i<nCycle; i++) {      

#if defined __MK20DX256__ || defined __MKL26Z64__ || defined __MK64FX512__ || defined __MK66FX1M0__                        

#ifdef LOW_POWER_HIBERNATE
                Snooze.hibernate(sleep_config);
#else            
                Snooze.deepSleep(sleep_config);
#endif
#else
                // use the delay function
                delay(LOW_POWER_PERIOD*1000);
#endif                         
            }

#if defined __MK20DX256__ || defined __MKL26Z64__ || defined __MK64FX512__ || defined __MK66FX1M0__
            // on the teensy, we need to wait to get the USB port to be detected
            delay(5000);
#endif            
            
#ifdef PERIODIC_IMG_TRANSMIT
            Serial.println(F("START PERIODIC IMAGE TRANSMIT"));
#else
            Serial.println(F("START INTRUSION DETECTION"));
            detectedIntrusion=false;
#endif

#else //LOW_POWER 

#ifdef PERIODIC_IMG_TRANSMIT
        if ( (millis()-lastCamIntrusionDetectionTime > interCamIntrusionDetectionTime || triggerIntrusionDetection) && camHasSync) {
            Serial.println(F("START PERIODIC IMAGE TRANSMIT"));
#else
        if ( (millis()-lastCamIntrusionDetectionTime > interCamIntrusionDetectionTime || triggerIntrusionDetection) && camHasSync && useRefImage) {
            Serial.println(F("START INTRUSION DETECTION"));
            detectedIntrusion=false;
#endif
#endif
            
#ifdef CRITICALITY_SCHEDULING
            setCaptureRateLight();
#endif            
            send_image_on_intrusion=true;

#ifdef ENERGY_TEST  
            // after ENERGY2, wait 30s to start the energy capture
            set_led(capture_led, 500);
#else
            digitalWrite(capture_led, HIGH);
#endif           
            for (int cam=0; cam<NB_UCAM; cam++) {
              
                  Serial.print(F("--->>> Intrusion detection with ucam "));
                  Serial.println(cam);
                  
                  digitalWrite(ucamLedArray[cam], HIGH);
                  sync_cam(cam);

#ifdef ENERGY_TEST_TIMING                  
                  set_led(capture_led, 200);
#endif                  
                  currentCam=cam;
                  
                  myCam.set_camPort(ucamArray[currentCam]);

                  blink_led(cam_sync_led, 3);
#ifdef ENERGY_TEST                  
                  immediateComparePicture(0,0);
#else
                  immediateComparePicture(500,3500);
#endif                  
                  digitalWrite(ucamLedArray[cam], LOW);
            }

            // just in case, set to use cam 0
            currentCam=0;
            // only for this one
            send_image_on_intrusion=false;
            // if the last operation was ok, then re-init timer
            if (camDataReady) {
                  lastCamIntrusionDetectionTime=millis();
            }
                  
#ifdef ENERGY_TEST                  
            set_led(capture_led, 500);
#else
            digitalWrite(capture_led, LOW);
#endif
            blink_led(cam_sync_led, 3);
            digitalWrite(cam_sync_led, LOW);
        }

#if defined PERIODIC_SYNC && not defined LOW_POWER
        // keep the camera awake
        if ( (millis()-lastCamSyncTime > interCamSyncTime || triggerCamSync)  && camHasSync) {

            camHasSync=false;
            
            for (int cam=0; cam<NB_UCAM; cam++) {
              
                  Serial.print(F("--->>> Synching cam "));
                  Serial.println(cam);
                  
                  digitalWrite(ucamLedArray[cam], HIGH);
                  
                  currentCam=cam;
                  
                  myCam.set_camPort(ucamArray[currentCam]);
                  
                  myCam.cam_sync=false;
                  
                  digitalWrite(cam_sync_led, LOW);
                  
                  while (!myCam.cam_sync)
                    myCam.init();
              
                  myCam.send_ack();
                  
                  digitalWrite(cam_sync_led, HIGH);
                  
                  camHasSync=true;
                  
                  digitalWrite(ucamLedArray[cam], LOW);
            }
            
            // just in case, set to use cam 0            
            currentCam=0;
            lastCamSyncTime=millis();
        }
#endif
}


