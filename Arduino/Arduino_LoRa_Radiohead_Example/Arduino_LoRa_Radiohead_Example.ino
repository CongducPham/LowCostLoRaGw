/*
 *  Simple example using the RadioHead LoRa RH_RF95 library
 *
 *  Copyright (C) 2017 Congduc Pham, University of Pau, France
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

/* 
 *  We show here how a device using the RadioHead LoRa (RH_RF95) library can communicate with our gateway  
 *  Note that our gateway still uses our dedicated library, referered to as SX1272 lib
 *  
 *  The RadioHead lib can be downloaded from http://www.airspayce.com/mikem/arduino/RadioHead/
 *  
 *  Our library uses the following 4-byte header 
 *    dst(1B) ptype(1B) src(1B) seq(1B)
 *    ptype is decomposed in 2 parts type(4bits) flags(4bits)
 *      type can take current value of DATA=0001 and ACK=0010
 *      the flags are from left to right: ack_requested|encrypted|with_appkey|is_binary
 *      
 *  Fortunately the RH lib also use a 4-byte header: to, from, id, flags    
 *  
 *  It is therefore easy to have the following correspondance: to <-> dst and from <-> src. 
 *  
 *  Then we can use id for the ptype and flags for seq
 *  
 *  However, in RH_RF95, the field insertion order is: to, from, id, flags 
 *  so to avoid changing the RH lib, we chose to use the following mapping:
 *  
 *  to <-> dst ; from <-> ptype ; id <-> src ; flags <-> seq 
 *  
 *  These variables will be set using the RHDatagram.h function: setHeaderTo, setHeaderfrom, setHeaderId, setHeaderFlags
 *    
 */
 
#include <SPI.h>

#include <RH_RF95.h>
#include <RHDatagram.h>

// the CS pin
#define RFM95_CS 10
// the reset pin, not mandatory
#define RFM95_RST 9
// the interrupt pin, it is necessary as RadioHead uses the TxDone/RxDone from interrupt
#define RFM95_INT 2

// Change to 434.0 or other frequency, must match RX's freq!
// here we match CH_10_868
#define RF95_FREQ 865.20

// the instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// our packet type definition
#define PKT_TYPE_DATA   0x10
#define PKT_TYPE_ACK    0x20
#define PKT_FLAG_ACK_REQ            0x08
#define PKT_FLAG_DATA_ENCRYPTED     0x04
#define PKT_FLAG_DATA_WAPPKEY       0x02

// So called LoRa mode 1 (BW125, CR45, SF12) with CRC on
RH_RF95::ModemConfig my_Bw125Cr45Sf4096 = {0x72, 0xc4, 0x08};

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
//#define WITH_APPKEY

///////////////////////////////////////////////////////////////////
// CHANGE HERE THE NODE ADDRESS 
#define node_addr 6
//////////////////////////////////////////////////////////////////

#ifdef WITH_APPKEY
///////////////////////////////////////////////////////////////////
// CHANGE HERE THE APPKEY, BUT IF GW CHECKS FOR APPKEY, MUST BE
// IN THE APPKEY LIST MAINTAINED BY GW.
uint8_t my_appKey[4]={5, 6, 7, 8};
///////////////////////////////////////////////////////////////////
#endif

// we use the simple RHDatagram class
RHDatagram manager(rf95, node_addr);

// the sequence number
uint8_t seq=0;

void setup()
{
  delay(3000);
  pinMode(RFM95_RST, OUTPUT);

  Serial.begin(38400);

  delay(100);
  
  Serial.println("Arduino LoRa RX Test!");
  // manual reset
  //digitalWrite(RFM95_RST, LOW);
  //delay(10);
  //digitalWrite(RFM95_RST, HIGH);
  //delay(10);

  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
  }
  
  Serial.println("LoRa radio init OK!");
  
  // set the frequency to work with the default gateway configuration
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }

  Serial.print("Set Freq to: "); 
  Serial.println(RF95_FREQ);
  
  // set to so-called LoRa mode 1
  rf95.setModemRegisters(&my_Bw125Cr45Sf4096);
  
  // The default transmitter power is 14dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin (means RFO = false), then
  // you can set transmitter powers from 5 to 23 dBm:
#ifdef PABOOST
  rf95.setTxPower(14, false);
#else
  // RFO = true; for inAir9 module for instance
  rf95.setTxPower(14, true);
#endif  
}

void loop() {

  uint8_t message[20];
  uint8_t data[] = "hello";
  
  uint8_t app_key_offset=0;  

#ifdef WITH_APPKEY
  app_key_offset = sizeof(my_appKey);
  // set the app key in the payload
  memcpy(message,my_appKey,app_key_offset);
#endif  
  
  memcpy(message+app_key_offset,data,sizeof(data)-1);

  // the destination, i.e. the gateway
  manager.setHeaderTo(1);
  // We use the from field to indicate the packet type for the gateway  
#ifdef WITH_APPKEY
  manager.setHeaderFrom(PKT_TYPE_DATA | PKT_FLAG_DATA_WAPPKEY);
#else
  manager.setHeaderFrom(PKT_TYPE_DATA);
#endif  
  // We use the Id field to indicate the source, i.e. to device's address
  manager.setHeaderId(node_addr);
  // we use the flag field to indicate the packet sequence number
  manager.setHeaderFlags(seq++,0xFF);
  
  Serial.println("Sending");  
  manager.sendto(message,sizeof(data)-1+app_key_offset,1);

  //This is the simple version using directly the driver
  //rf95.send(message, sizeof(data)-1+app_key_offset);
  
  delay(5000);
}


