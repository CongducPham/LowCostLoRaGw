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
 * last update: April 8th by M. Diop
 */

#ifndef __gps_light_h
#define __gps_light_h

#include <Arduino.h>
#include <HardwareSerial.h>


#define BUFFER_SIZE 82

class gps_light {

public:
   gps_light(HardwareSerial&);  // uses hardware serial
   void gps_init_PSM(); // init power save mode
   void begin(long);  // initialize, set baudrate, listen
   
   uint8_t isValid();  // Check if location is valid
   void parseNMEA(); // Parsing NMEA sentences
   uint8_t isGGA(); // Check if a NMEA sentence is GGA format : GPAGGA, GNGGA(for the Ublox 8 series)
   void decode();   // Decode GPGGA/GNGGA sentences content
	 double degree_convertor(double, char); // Convert NMEA latitude & longitude to decimal (Degrees)
	 
  
	// Getters & Setters
	void set_latitude(double);
	void set_longitude(double);
	void set_lat_direction(char);
	void set_lgt_direction(char);
	
	double get_latitude();
	double get_longitude();
	char get_lat_direction();
	char get_lgt_direction();
  HardwareSerial get_serial();

	//Some methods for exploiting UBX protocol : in our case forcing saving mode?????? 
	void send_UBX_MSG(uint8_t *, uint8_t); //Send a UBX protocol message (a byte array) to the GPS
	uint8_t get_UBX_ACK(uint8_t *); //Calculate expected UBX ACK packet and parse UBX response from GPS
  void gps_ON();  //Switching on GPS RF
  void gps_OFF(); //Switching off GPS RF
	  
private:
	double lat; // Latitude
	char lat_direction; // Latitude direction: North, South
	double lgt; // Longitude
	char lgt_direction; // Longitude direction:  East, West
	byte gps_index;
	char* gps_buffer;
	int available(); // returns number of characters in buffer
  byte read();     // get one character from buffer
	/* Class Properties */
  HardwareSerial& serial;
	
};

#endif // def(__gps_light_h)
