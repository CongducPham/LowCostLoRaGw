/********************************************************************************
 *  gsp_light - a small GPS library for Arduino providing universal NMEA parsing,
 *	in particulary GGA sentences (GPGGA, GNGGA) to exploit position.
 * 	
 *  Copyright (C) 2017 Mamour Diop, University of Pau, France
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************
 * last update: Oct 17th by C. Pham
 */
 
#include "gps_light.h"

gps_light::gps_light(HardwareSerial& serial) : serial(serial) {
  this->lat = 0.0;
	this->lat_direction = 'X';
	this->lgt = 0.0;
	this->lgt_direction = 'X';
	this->gps_index = 0;
	this->gps_buffer = new char[BUFFER_SIZE];
}

void gps_light::gps_init_PSM() {
  bool gps_set_sucess = false;

  // CFG-PM2
  // set PSM parameters, 
  // set to ON/OFF mode, 
  // update time is 0ms, do not get back to acquisition state after a fix
  // search time is 0ms, do not get back to acquisiation state after acquisition timeout
  // grid offset is 0ms, 
  // on time is 2s
  // minimum acquisition time is 35s   
  uint8_t GPS_CFG_PM2[] = {0xB5, 0x62, 0x06, 0x3B, 0x2C, 0x00, 0x01, 0x00, 0x00, 0x00, 
    0x00, 0x10, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 
    0x02, 0x00, 
    0x23, 0x00, 
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};  

  //Serial.println("CFG-PM2 initiated");
  
  while(!gps_set_sucess){    
    send_UBX_MSG(GPS_CFG_PM2, sizeof(GPS_CFG_PM2)/sizeof(uint8_t));
    gps_set_sucess=get_UBX_ACK(GPS_CFG_PM2);
  }      
  //Serial.println("CFG-PM2 ok");
    
  // CFG-RXM
  // set to use PSM instead of default Max performance mode
  uint8_t GPS_CFG_RXM[] = {0xB5, 0x62, 0x06, 0x11, 0x02, 0x00, 0x08, 0x01};  

  //Serial.println("CFG-RXM initiated");
  
  while(!gps_set_sucess){    
    send_UBX_MSG(GPS_CFG_RXM, sizeof(GPS_CFG_RXM)/sizeof(uint8_t));
    gps_set_sucess=get_UBX_ACK(GPS_CFG_RXM);
  }    

  //Serial.println("CFG-RXM ok");
}

void gps_light::begin(long speed) {
    return this->serial.begin(speed);
}

int gps_light::available() {
    return this->serial.available();
}

byte gps_light::read() {
    return this->serial.read();
}

void gps_light::set_latitude(double lat) {
    this->lat = lat;
}

void gps_light::set_longitude(double lgt) {
    this->lgt = lgt;
}

void gps_light::set_lat_direction(char dir) {
    this->lat_direction = dir;
}

void gps_light::set_lgt_direction(char dir) {
    this->lgt_direction = dir;
}

double gps_light::get_latitude() {
    return this->lat;
}

double gps_light::get_longitude() {
    return this->lgt;
}

char gps_light::get_lat_direction() {
    return this->lat_direction;
}

char gps_light::get_lgt_direction() {
    return this->lgt_direction;
}

HardwareSerial gps_light::get_serial() {
    return this->serial;
}

uint8_t gps_light::isValid(){
  return (this->lat != 0.0 && this->lgt != 0.0 && (this->lat_direction == 'N' || this->lat_direction=='S') && (this->lgt_direction=='E'|| this->lgt_direction=='W'));
}

void gps_light::parseNMEA() {
  char inByte;
  bool detect_new_line=false;
  
  while (!detect_new_line){
    if (this->serial.available()) {
    
      inByte = this->serial.read();
      
      if ((inByte =='$') || (this->gps_index >= 80)){
        this->gps_index = 0;
      }
    
      if (inByte != '\r' && inByte != '\n') {
        gps_buffer[this->gps_index++] = inByte;
      }
   
      if (inByte == '\n') {
        gps_buffer[this->gps_index++] = '\0';
        detect_new_line=true;
      }
    }
  } 

  // Output exactly what we read from the GPS to debug  
  //Serial.println(GPSBuffer);
  //Serial.flush();
}


uint8_t gps_light::isGGA(){
  return (gps_buffer[1] == 'G' && (gps_buffer[2] == 'P' || gps_buffer[2] == 'N') && gps_buffer[3]=='G' && gps_buffer[4]=='G' && gps_buffer[5]=='A');
}

void gps_light::decode() {
  int i, j;
  char latitude[30], longitude[30];
  byte lat_index=0, lgt_index=0;
  
  // We start at 7 so we ignore the header '$GPGGA,' or '$GNGGA,'  
  for (i=7, j=0; (i<this->gps_index) && (j<9); i++){ 
    if (gps_buffer[i] == ','){
      j++;    // Segment index
    }

    else{
      if (j == 1){   //Latitude
        latitude[lat_index++]= gps_buffer[i];
      }

      else if (j == 2){   //Latitude Direction: N=North, S=South
        //this->lat_direction = gps_buffer[i];
        set_lat_direction(gps_buffer[i]);
      }
      
      else if (j == 3){    //Longitude
        longitude[lgt_index++]= gps_buffer[i];
      }

      else if (j == 4){   //Longitude Direction: E=East, W=West
        //this->lgt_direction = gps_buffer[i];
        set_lgt_direction(gps_buffer[i]);
      }
    }
  }
  //this->lat = degree_convertor(atof(latitude), this->lat_direction);
  //this->lgt = degree_convertor(atof(longitude), this->lgt_direction);
  set_latitude(degree_convertor(atof(latitude), this->lat_direction));
  set_longitude(degree_convertor(atof(longitude), this->lgt_direction));
}

//////////////////////////////////////////////////////////////////////////////////
//Convert NMEA latitude & longitude to decimal (Degrees)
//  Notice that for latitude of 0302.78469,
//              03 ==> degrees. counted as is
//              02.78469 ==> minutes. divide by 60 before adding to degrees above
//              Hence, the decimal equivalent is: 03 + (02.78469/60) = 3.046412
//  For longitude of 10601.6986,
//              ==> 106+1.6986/60 = 106.02831 degrees
//  Location is latitude or longitude
//  In Southern hemisphere latitude should be negative
//  In Western Hemisphere, longitude degrees should be negative
//////////////////////////////////////////////////////////////////////////////////

double gps_light::degree_convertor(double loc, char direction){
  double degLoc = 0.0;
  double degWhole= loc/100; //gives the whole degree part of latitude
  double degDec = (degWhole - ((int)degWhole)) * 100 / 60; //gives fractional part of location
  degLoc = (int)degWhole + degDec; //gives complete correct decimal form of location degrees
  
  if (direction =='S' || direction =='W') {  
    degLoc = (-1)*degLoc;
  }
  return degLoc;  
}

void gps_light::send_UBX_MSG(uint8_t *msg, uint8_t len) {

  uint8_t CKA=0;
  uint8_t CKB=0;
  
  for (int i=0; i<len; i++) {
    this->serial.write(msg[i]);

    if (i>1) {
      CKA += msg[i];
      CKB += CKA;
    }
  }

  this->serial.write(CKA);
  this->serial.write(CKB);  
}

//CODE from /////
uint8_t gps_light::get_UBX_ACK(uint8_t *msg) {
  uint8_t b;
  uint8_t ackByteID = 0;
  uint8_t ackPacket[10];
  unsigned long startTime = millis();
  //PRINT_CSTSTR("%s","Reading ACK response: ");
 
  // Construct the expected ACK packet    
  ackPacket[0] = 0xB5;  // header
  ackPacket[1] = 0x62;  // header
  ackPacket[2] = 0x05;  // class
  ackPacket[3] = 0x01;  // id
  ackPacket[4] = 0x02;  // length
  ackPacket[5] = 0x00;
  ackPacket[6] = msg[2];  // ACK class
  ackPacket[7] = msg[3];  // ACK id
  ackPacket[8] = 0;   // CK_A
  ackPacket[9] = 0;   // CK_B
 
  // Calculate the checksums
  for (uint8_t i=2; i<8; i++) {
    ackPacket[8] = ackPacket[8] + ackPacket[i];
    ackPacket[9] = ackPacket[9] + ackPacket[8];
  }
 
  while (1) {
    // Test for success
    if (ackByteID > 9) {
      // All packets in order!
      //printf("Success! \n");
      return 1;
    }
 
    // Timeout if no valid response in 3 seconds
    if (millis() - startTime > 3000) { 
      //printf("Failed!  \n");
      return 0;
    }
 
    // Make sure data is available to read
    if (this->serial.available()) {
      b = this->serial.read();
 
      // Check that bytes arrive in sequence as per expected ACK packet
      if (b == ackPacket[ackByteID]) { 
        ackByteID++;
        //Serial.print(b, HEX);
      } 
      else {
        ackByteID = 0;  // Reset and look again, invalid order
      }
 
    }
  }
}

void gps_light::gps_ON() {
  bool gps_set_sucess = false;
  
  // CFG-RST
  uint8_t GPS_CFG_RST[] = {0xB5, 0x62, 0x06, 0x04, 0x04, 0x00, 0x00, 0x00, 0x09, 0x00};
  uint8_t dummy[] = {0xFF, 0xFF};

  //send_UBX_MSG(dummy, sizeof(dummy)/sizeof(uint8_t));  

  send_UBX_MSG(GPS_CFG_RST, sizeof(GPS_CFG_RST)/sizeof(uint8_t));
    
  while(!gps_set_sucess){    
    send_UBX_MSG(GPS_CFG_RST, sizeof(GPS_CFG_RST)/sizeof(uint8_t));
    gps_set_sucess=get_UBX_ACK(GPS_CFG_RST);
  }  
}

void gps_light::gps_OFF() {
  bool gps_set_sucess = false;
  // CFG-RST
  uint8_t GPS_CFG_RST[] = {0xB5, 0x62, 0x06, 0x04, 0x04, 0x00, 0x00, 0x00, 0x08, 0x00};
  // RXM-PMREQ
  uint8_t GPS_RXM_PMREQ[] = {0xB5, 0x62, 0x02, 0x41, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00};

  send_UBX_MSG(GPS_CFG_RST, sizeof(GPS_CFG_RST)/sizeof(uint8_t));
  //delay(1000);
  //send_UBX_MSG(GPS_RXM_PMREQ, sizeof(GPS_RXM_PMREQ)/sizeof(uint8_t));
  //delay(500);
  //send_UBX_MSG(GPS_RXM_PMREQ, sizeof(GPS_RXM_PMREQ)/sizeof(uint8_t));

  //Serial.println("RXM-PMREQ ok");

  //while(!gps_set_sucess){    
  //  send_UBX_MSG(GPSoff, sizeof(GPSoff)/sizeof(uint8_t));
  //  gps_set_sucess=get_UBX_ACK(GPSoff);
  //}
}

