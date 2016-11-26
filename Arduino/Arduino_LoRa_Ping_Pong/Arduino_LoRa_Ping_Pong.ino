/*
 *  simple ping-pong test by requesting an ACK from the gateway
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
 * last update: Nov. 16th by C. Pham
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
//
// uncomment if your radio is an HopeRF RFM92W, HopeRF RFM95W, Modtronix inAir9B, NiceRF1276
// or you known from the circuit diagram that output use the PABOOST line instead of the RFO line
//#define PABOOST
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
// CHANGE HERE THE LORA MODE, NODE ADDRESS 
#define LORAMODE  1
#define node_addr 8
//////////////////////////////////////////////////////////////////

#define DEFAULT_DEST_ADDR 1

uint8_t message[100];

int loraMode=LORAMODE;

void setup()
{
  int e;
  
  // Open serial communications and wait for port to open:
#ifdef __SAMD21G18A__  
  SerialUSB.begin(38400);
#else
  Serial.begin(38400);  
#endif 

  // Print a start message
  Serial.println(F("Simple LoRa ping-pong with the gateway"));

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

#ifdef __SAMD21G18A__ 
  SerialUSB.println(F("Arduino M0/Zero detected"));
#endif

  // Power ON the module
  sx1272.ON();
  
  // Set transmission mode and print the result
  e = sx1272.setMode(loraMode);
  Serial.print(F("Setting Mode: state "));
  Serial.println(e, DEC);

  // enable carrier sense
  sx1272._enableCarrierSense=true;
    
  // Select frequency channel
  e = sx1272.setChannel(DEFAULT_CHANNEL);
  Serial.print(F("Setting Channel: state "));
  Serial.println(e, DEC);
  
  // Select amplifier line; PABOOST or RFO
#ifdef PABOOST
  sx1272._needPABOOST=true;
#endif   

  e = sx1272.setPowerDBM((uint8_t)MAX_DBM); 

  Serial.print(F("Setting Power: state "));
  Serial.println(e);
  
  // Set the node address and print the result
  e = sx1272.setNodeAddress(node_addr);
  Serial.print(F("Setting node addr: state "));
  Serial.println(e, DEC);
  
  // Print a success message
  Serial.println(F("SX1272/76 successfully configured"));

  delay(500);
}


void loop(void)
{
  uint8_t r_size;
  int e;

  sx1272.CarrierSense();

  sx1272.setPacketType(PKT_TYPE_DATA);

  r_size=sprintf((char*)message, "Ping");
      
  while (1) {

      Serial.println(F("Sending Ping"));  
      
      e = sx1272.sendPacketTimeoutACK(DEFAULT_DEST_ADDR, message, r_size);

      // this is the no-ack version
      // e = sx1272.sendPacketTimeout(DEFAULT_DEST_ADDR, message, r_size);
            
      Serial.print(F("Packet sent, state "));
      Serial.println(e);
      
      if (e==3)
          Serial.println(F("No Pong!"));
        
      if (e==0)
          Serial.println(F("Pong received from gateway!"));      

      delay(10000);    
  }          
}
