//////////////////////////////////////////////////////////////////////
// Simple GPS Sensor : parsing GGA NMEA sentence (GPGGA or GNGGA) 
// On the Ublox 8 series GPGGA is replaced by GNGGA by default
// 
// $GNGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
//
// Copyright (C) 2018 Mamour Diop & Congduc Pham, University of Pau, France
/////////////////////////////////////////////////////////////////////

//#define HARDWARE_SERIAL_GPS
#define SOFTWARE_SERIAL_GPS

#ifdef HARDWARE_SERIAL_GPS

///////////////////////////////
// USING HARDWARE SERIAL
//
// On Arduino with only 1 serial port (TX/RX), it is necessary to perform the following actions 
// as the serial port is use for both the GPS and the programming. To see GPS message:

// be sure that serial monitor is not open
// connect only GPS_GND<->GND and GPS_TX<>RX
// after uploading the sketch, connect GPS_VCC<>3.3V
// then open Serial Monitor (set baud rate to 9600)
// do not connect GPS_TX
// 
// you should see NMEA messages from the GPS module
// to understand NMEA message: http://aprs.gids.nl/nmea/

// if you are using an Arduino board with several serial port (such as the MEGA)
// then define here the one that is connected to the GPS module
//#define gps_serial Serial3
#define gps_serial Serial

#endif

#ifdef SOFTWARE_SERIAL_GPS

#include <SoftwareSerial.h> 
// connect GPS_TX <> 4 and GPS_RX <> 3
SoftwareSerial gps_serial(4,3); //rx,tx for Arduino side

#endif

char GPSBuffer[150];
byte GPSIndex=0;
double gps_latitude = 0.0;
double gps_longitude = 0.0;
char lat_direction, lgt_direction;

void parseNMEA(){
  char inByte;
  bool detect_new_line=false;
  
  while (!detect_new_line)
  {
    if (gps_serial.available()) {
      inByte = gps_serial.read();
    
      //Serial.print(inByte); // Output exactly what we read from the GPS to debug
    
      if ((inByte =='$') || (GPSIndex >= 150)){
        GPSIndex = 0;
      }
      
      if (inByte != '\r' && inByte != '\n'){
        GPSBuffer[GPSIndex++] = inByte;
      }
   
      if (inByte == '\n'){
        GPSBuffer[GPSIndex++] = '\0';
        detect_new_line=true;
      }
    }
  } 
  Serial.print("---->");
  Serial.println(GPSBuffer);
  //Serial.flush();
}

bool isGGA(){
  return (GPSBuffer[1] == 'G' && (GPSBuffer[2] == 'P' || GPSBuffer[2] == 'N') && GPSBuffer[3] == 'G' && GPSBuffer[4] == 'G' && GPSBuffer[5] == 'A');
}

// GPGGA and GNGGA for the Ublox 8 series. Ex:
// no fix -> $GPGGA,,,,,,0,00,99.99,,,,,,*63
// fix    -> $GPGGA,171958.00,4903.61140,N,00203.95016,E,1,07,1.26,55.1,M,46.0,M,,*68

void decodeGGA(){
  int i, j;
  char latitude[30], longitude[30];
  byte latIndex=0, lgtIndex=0;
  
  for (i=7, j=0; (i<GPSIndex) && (j<9); i++) { // We start at 7 so we ignore the '$GNGGA,'
    
    if (GPSBuffer[i] == ','){
      j++;    // Segment index
    }
    else{
      if (j == 1){   //Latitude
        latitude[latIndex++]= GPSBuffer[i];
      }

      else if (j == 2){   //Latitude Direction: N=North, S=South
        lat_direction = GPSBuffer[i];
      }
      
      else if (j == 3){    //Longitude
        longitude[lgtIndex++]= GPSBuffer[i];
      }

      else if (j == 4){   //Longitude Direction: E=East, W=West
        lgt_direction = GPSBuffer[i];
      }
    }
  }
  //Serial.print("Lat Direction: "); Serial.println(lat_direction);
  //Serial.print("Lgt Direction: "); Serial.println(lgt_direction);
  gps_latitude = degree_location(atof(latitude), lat_direction);
  gps_longitude = degree_location(atof(longitude), lgt_direction);
}

bool isValidLocation(){
  return (gps_latitude != 0.0 && gps_longitude != 0.0 && (lat_direction == 'N' || lat_direction=='S') && (lgt_direction=='E'|| lgt_direction=='W'));
}

void display_location(){
  Serial.println("==============");
  Serial.print("Location: ");
  Serial.print(gps_latitude, 5);
  Serial.print(", ");
  Serial.println(gps_longitude, 5);
  Serial.println("=============="); 
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

double degree_location(double loc, char loc_direction){
  double degLoc = 0.0;
  double degWhole= loc/100; //gives the whole degree part of latitude
  double degDec = (degWhole - ((int)degWhole)) * 100 / 60; //gives fractional part of location
  degLoc = (int)degWhole + degDec; //gives complete correct decimal form of location degrees
  
  if (loc_direction =='S' || loc_direction =='W') {  
    degLoc = (-1)*degLoc;
  }
  return degLoc;  
}

void setup(){
  Serial.begin(9600);
  //if only 1 serial port then gps_serial is equivalent to Serial
#ifdef SOFTWARE_SERIAL_GPS  
  gps_serial.begin(9600);
#endif  
  Serial.println("GPS basic rogram");
}

void loop(){

  bool get_fix = false;
  
  while (!get_fix){
    parseNMEA();
    if (isGGA()){
      decodeGGA();
      if (isValidLocation()){
        display_location();
        get_fix = true;
      }
    }
  }
}
