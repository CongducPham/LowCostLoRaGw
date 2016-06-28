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

#define DEFAULT_DEST_ADDR 1
#define LORAMODE  1
#define node_addr 8

uint8_t message[100];

int loraMode=LORAMODE;

void setup()
{
  int e;
  
  // Open serial communications and wait for port to open:
  Serial.begin(38400);

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

  // Power ON the module
  sx1272.ON();
  
  // Set transmission mode and print the result
  e = sx1272.setMode(loraMode);
  Serial.print(F("Setting Mode: state "));
  Serial.println(e, DEC);

  // enable carrier sense
  sx1272._enableCarrierSense=true;
    
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
