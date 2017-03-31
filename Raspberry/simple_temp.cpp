/*
 *  LoRa gateway to receive and send command
 *
 *  Copyright (C) 2016 Rainer 'rei' Schuth
 *
 */

// Include the SX1272
#include "SX1272.h"

#include <iostream>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include  <signal.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>


// IMPORTANT
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
// please uncomment only 1 choice
#define ETSI_EUROPE_REGULATION
//#define FCC_US_REGULATION
//#define SENEGAL_REGULATION

// please uncomment only 1 choice
#define BAND868
//#define BAND900
//#define BAND433

#ifdef ETSI_EUROPE_REGULATION
  #define MAX_DBM 14
#elif defined SENEGAL_REGULATION
  #define MAX_DBM 10
#endif

#ifndef MAX_DBM
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

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
// CHANGE HERE THE LORA MODE, NODE ADDRESS
#define LORAMODE  1
// the special mode to test BW=125MHz, CR=4/5, SF=12
// on the 868.1MHz channel
//#define LORAMODE 11
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
#define DEFAULT_NODE_ADDR 8
#define DEFAULT_DEST_ADDR 1
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//#define WITH_APPKEY

#ifdef WITH_APPKEY
  // CHANGE HERE THE APPKEY, BUT IF GW CHECKS FOR APPKEY, MUST BE
  // IN THE APPKEY LIST MAINTAINED BY GW.
  uint8_t my_appKey[4]={5, 6, 7, 8};
#endif
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
#define WITH_ACK

#ifdef WITH_ACK
  #define NB_RETRIES 3
#endif
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
// use the dynamic ACK feature of our modified SX1272 lib
#define GW_AUTO_ACK
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
#define WITH_CARRIER_SENSE
//////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// CHANGE HERE THE THINGSPEAK FIELD BETWEEN 1 AND 4
#define THINGSPEAK_FIELD_INDEX 4
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
#define WITH_SEND_LED

#ifdef WITH_SEND_LED
  /***

    RPI vs arduPi GPIO Pin Layout

    | Raspbeery Pi  v2,3 GPIO Pin | arduPi IO Pin | Alternativ Function |
    | --------------------------- |---------------| --------------------|
    |  2                          |               |  I2C:SDA            |
    |  3                          |               |  I2C:SCL            |
    |  4                          |  6            |                     |
    |  8                          |  10           |  SPI:CE0            |
    |  9                          |  12           |  SPI:CE0            |
    |  10                         |  11           |  SPI:CE0            |
    |  11                         |  13           |  SPI:CE0            |
    |  14                         |  1            |  UART:TX            |
    |  15                         |  0            |  UART:RX            |
    |  17                         |  7            |                     |
    |  18                         |  2            |                     |
    |  21/27                      |  8            |                     |
    |  22                         |  9            |                     |
    |  23                         |  3            |                     |
    |  24                         |  4            |                     |
    |  25                         |  5            |                     |

   ***/
  // Define the LED pin based on the arduPi IO Pin column
  #define SEND_LED  3
#endif
///////////////////////////////////////////////////////////////////

#ifdef BAND868
  #define MAX_NB_CHANNEL 9
  #define STARTING_CHANNEL 10
  #define ENDING_CHANNEL 18
  uint8_t loraChannelIndex=0;
  uint32_t loraChannelArray[MAX_NB_CHANNEL]={
          CH_10_868,
          CH_11_868,
          CH_12_868,
          CH_13_868,
          CH_14_868,
          CH_15_868,
          CH_16_868,
          CH_17_868,
          CH_18_868
  };
#elif defined BAND900
  #define MAX_NB_CHANNEL 13
  #define STARTING_CHANNEL 0
  #define ENDING_CHANNEL 12
  uint8_t loraChannelIndex=5;
  uint32_t loraChannelArray[MAX_NB_CHANNEL]={
        CH_00_900,
        CH_01_900,
        CH_02_900,
        CH_03_900,
        CH_04_900,
        CH_05_900,
        CH_06_900,
        CH_07_900,
        CH_08_900,
        CH_09_900,
        CH_10_900,
        CH_11_900,
        CH_12_900
  };
#endif

#define PRINTLN                   printf("\n")
#define PRINT_CSTSTR(fmt,param)   printf(fmt,param)
#define PRINT_STR(fmt,param)      PRINT_CSTSTR(fmt,param)
#define PRINT_VALUE(fmt,param)    PRINT_CSTSTR(fmt,param)
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


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

int node_addr = DEFAULT_NODE_ADDR;
double temp;
uint8_t message[100];


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

void setup()
{
    int e;
    srand (time(NULL));

    // Power ON the module
    e = sx1272.ON();

    PRINT_CSTSTR("%s","^$**********Power ON: state ");
    PRINT_VALUE("%d", e);
    PRINTLN;

    // Set transmission mode and print the result
    e = sx1272.setMode(LORAMODE);
    PRINT_CSTSTR("%s","Setting Mode: state ");
    PRINT_VALUE("%d", e);
    PRINTLN;

    // enable carrier sense
    sx1272._enableCarrierSense=true;

    // Select frequency channel
    e = sx1272.setChannel(DEFAULT_CHANNEL);
    PRINT_CSTSTR("%s","Setting Channel: state ");
    PRINT_VALUE("%d", e);
    PRINTLN;

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

    FLUSHOUTPUT;
    delay(1000);
}

void loop(void)
{
    long startSend;
    long endSend;
    uint8_t app_key_offset=0;
    int e;

    temp = rand();

    PRINT_CSTSTR("%s","Temp is ");
    PRINT_VALUE("%f", temp);
    PRINTLN;

#ifdef WITH_APPKEY
    app_key_offset = sizeof(my_appKey);
      // set the app key in the payload
      memcpy(message,my_appKey,app_key_offset);
#endif

    uint8_t r_size;

    // then use app_key_offset to skip the app key

    r_size=sprintf((char*)message+app_key_offset, "\\!#%d#TC/%.2f", THINGSPEAK_FIELD_INDEX, temp);

    PRINT_CSTSTR("%s","Sending ");
    PRINT_STR("%s",(char*)(message+app_key_offset));
    PRINTLN;

    PRINT_CSTSTR("%s","Real payload size is ");
    PRINT_VALUE("%d", r_size);
    PRINTLN;

    int pl=r_size+app_key_offset;

#ifdef WITH_CARRIER_SENSE
    sx1272.CarrierSense();
#endif

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
#ifdef WITH_SEND_LED
    digitalWrite(SEND_LED, HIGH);
#endif

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

#ifdef WITH_SEND_LED
    digitalWrite(SEND_LED, LOW);
#endif

    endSend=millis();

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

}

int main (int argc, char *argv[]){

    if(argc < 2) {
        PRINT_CSTSTR("%s", "Need the node address as a parameter!");
        PRINTLN;
        PRINT_CSTSTR("Example: %s <nodeAddr>", argv[0]);
        PRINTLN;
        return 1;
    }
    node_addr = (uint8_t)atoi(argv[1]);
    PRINT_CSTSTR("%s", "My node address is: ");
    PRINT_VALUE("%d", node_addr);
    PRINTLN;

    setup();

    while(1){
        loop();
        delay(1000);
    }

    return 0;
}
