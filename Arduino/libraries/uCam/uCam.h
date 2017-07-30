/*
    uCam.h
    
    Some code for controlling uCam 529 by 
    4D Systems (http://www.4dsystems.com.au/)
    
    (c) Copyright 2012 9 Circuits. All Rights Reserved.
    
    This file is part of uCam Control Code.

    uCam Control Code is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    uCam Control Code is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with uCam Control Code.  If not, see <http://www.gnu.org/licenses/>.
*/

/* Debugged and improved by Congduc Pham, University of Pau */
/* December, 2014. http://www.univ-pau.fr/~cpham            */
/* Support only RAW mode as we only need this mode to       */
/* encode ourself the image                                 */

#ifndef UCAM529_h
#define UCAM529_h

// Added by C. Pham
#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <inttypes.h>

#define UCAM_RAW 0x03
#define UCAM_JPEG 0x07

#define UCAM_RAW_80_60 0x01
#define UCAM_RAW_160_120 0x03
#define UCAM_RAW_128_128 0x09
#define UCAM_RAW_128_96 0x0B

#define UCAM_JPEG_160_128 0x03
#define UCAM_JPEG_320_200 0x05
#define UCAM_JPEG_640_480 0x07

#define PICTURE_SNAPSHOT_TYPE 0x01
#define PICTURE_RAW_TYPE 0x02
#define PICTURE_JPEG_TYPE 0x05

//#define SSSERIAL

class UCAM529 {

public:
    UCAM529(HardwareSerial *camPort, uint8_t mode, uint8_t quality);
#ifdef SSSERIAL
	UCAM529(SoftwareSerial *camPort, uint8_t mode, uint8_t quality);
#endif	
    void init();
	void set_camPort(HardwareSerial *camPort);
#ifdef SSSERIAL	
	void set_camPort(SoftwareSerial *camPort);
#endif	
    void takePicture(uint8_t pic_mode);

    HardwareSerial *camPort;   
#ifdef SSSERIAL    
    SoftwareSerial *ss_camPort;   
#endif    
    //GM862 *dataDevice;
    
    long startCamSnapshotTime;
    long stopCamSnapshotTime;
    long startCamGetPictureTime;
    long stopCamGetPictureTime;
    
//private:
    int send_initial();
    int send_reset();
    int set_package_size();
    int set_baud_rate(uint8_t firstDivider, uint8_t secondDivider);
    int do_snapshot();
    int get_picture(uint8_t pic_mode);
    int get_data();
    int send_ack();
    int send_ack_picture();
    int wait_for_sync();
    int attempt_sync();
    int wait_for_ack(uint8_t* command);

    boolean cam_sync; 
#ifdef SSSERIAL     
    boolean is_softwareserial;
#endif  
    char pin[5];
    uint8_t camMode;
};


#endif
