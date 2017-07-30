/*
    uCam.cpp
    
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

#include "uCam.h"
#ifdef SSSERIAL
#include <SoftwareSerial.h>
#endif

#include <string.h>

//uint8_t   _INITIAL[6] =      {0xAA,0x01,0x00,0x07,0x00,0x03}; //JPEG 160x128
uint8_t   _INITIAL[6] =      {0xAA,0x01,0x00,0x03,0x09,0x00}; //RAW 8-bit gray scale 128x128
uint8_t   _GET_PICTURE[6]=   {0xAA,0x04,0x01,0x00,0x00,0x00}; // SnapShot picture mode
uint8_t   _SNAPSHOT[6] =     {0xAA,0x05,0x01,0x00,0x00,0x00}; // RAW - current frame
uint8_t   _PACK_SIZE[6] =    {0xAA,0x06,0x08,0x00,0x02,0x00}; // only for JPEG - 512B
uint8_t	  _SET_BAUD_RATE[6] = {0xAA,0x07,0x1F,0x00,0x00,0x00}; // 115200 
uint8_t	  _RESET[6] 		= {0xAA,0x08,0x00,0x00,0x00,0x00};
uint8_t   _SYNC_COMMAND[6] = {0xAA,0x0D,0x00,0x00,0x00,0x00};
uint8_t   _ACK_COMMAND[6] =  {0xAA,0x0E,0x0D,0x00,0x00,0x00};
uint8_t   _ACK_PICTURE_COMMAND[6] = {0xAA,0x0E,0x0A,0x00,0x01,0x00};
uint8_t   _PACKET_ACK[6] =   {0xAA,0x0E,0x00,0x00,0x00,0x00};

// UCAM(&Serial1, UCAM_RAW, UCAM_RAW_128_128)
UCAM529::UCAM529(HardwareSerial *camPort, uint8_t mode, uint8_t quality) {
    this->camPort = camPort;
    this->camMode = mode;
    this->cam_sync=false;
#ifdef SSSERIAL    
    this->is_softwareserial=false;
#endif    
    if (mode==UCAM_RAW) {
    	_INITIAL[3] = UCAM_RAW;
    	_INITIAL[4] = quality;
    	
    }
    
    if (mode==UCAM_JPEG) {
    	_INITIAL[3] = UCAM_JPEG;
    	_INITIAL[5] = quality;
    	
    }
}

#ifdef SSSERIAL

UCAM529::UCAM529(SoftwareSerial *camPort, uint8_t mode, uint8_t quality) {
    this->ss_camPort = camPort;
    this->camMode = mode;
    this->cam_sync=false;
    this->is_softwareserial=true;
    
    if (mode==UCAM_RAW) {
    	_INITIAL[3] = UCAM_RAW;
    	_INITIAL[4] = quality;
    	
    }
    
    if (mode==UCAM_JPEG) {
    	_INITIAL[3] = UCAM_JPEG;
    	_INITIAL[5] = quality;
    	
    }
}

#endif

void UCAM529::init() {
	cam_sync = false;
    if (attempt_sync())
    {
        if (wait_for_sync())
        {
            cam_sync = true;
        }
    }
}

void UCAM529::set_camPort(HardwareSerial *camPort) {
	
	this->camPort = camPort;
#ifdef SSSERIAL	
	this->is_softwareserial=false;
#endif	
}

#ifdef SSSERIAL
void UCAM529::set_camPort(SoftwareSerial *camPort) {
	
	this->ss_camPort = camPort;
	this->is_softwareserial=true;
}
#endif

void UCAM529::takePicture(uint8_t pic_mode) {
    if (send_initial())
    {
        if (set_package_size())
        {
            if (do_snapshot())
            {
                if (get_picture(pic_mode))
                {
                    get_data();
                }
            }
        }
    }
}

int UCAM529::send_initial() {
    int ack_success = 0;
    //delay(500);
    uint8_t ack[6] = {0xAA, 0x0E, 0x01, 0x00, 0x00, 0x00};
    Serial.println("Initial is being sent");
    
    for (int i = 0; i< 6; i++) {
#ifdef SSSERIAL    	
    	if (this->is_softwareserial)
    		this->ss_camPort->write(_INITIAL[i]);
    	else
#endif    	
        	this->camPort->write(_INITIAL[i]);
    }

    if (wait_for_ack(ack)) {
        Serial.print("\nINITIAL has been acked...\n");
        ack_success = 1;
    }
    
    return ack_success;
}

/*
240031 (0x1F)47 (0x2F)
480031 (0x1F)23 (0x17)960031 (0x1F)11 (0x0B)1920031 (0x1F)53840031 (0x1F)25760031 (0x1F)111520031 (0x1F)015360072230400714608007092160011122880020184320010368640000
*/

int UCAM529::set_baud_rate(uint8_t firstDivider, uint8_t secondDivider) {
    int ack_success = 0;

    uint8_t ack[6] = {0xAA, 0x0E, 0x07, 0x00, 0x00, 0x00};

	_SET_BAUD_RATE[2]=firstDivider;
	_SET_BAUD_RATE[2]=secondDivider;	 
	
    for (int i = 0; i< 6; i++) {
#ifdef SSSERIAL    	
    	if (this->is_softwareserial)
    		this->ss_camPort->write(_SET_BAUD_RATE[i]);
    	else   
#endif    	 	
        	this->camPort->write(_SET_BAUD_RATE[i]);
    }

#ifdef SSSERIAL
	if (this->is_softwareserial)
		while (!this->ss_camPort->available());
	else
#endif	
		while (!this->camPort->available());
		

    if (wait_for_ack(ack)) {
        Serial.print("\nSET BAUD RATE has been acked...\n");
        ack_success = 1;
    }
    
    return ack_success;
}

int UCAM529::send_reset() {
    int ack_success = 0;
    //delay(500);
    uint8_t ack[6] = {0xAA, 0x0E, 0x08, 0x00, 0x00, 0x00};
    Serial.println("Reset is being sent");
    
    for (int i = 0; i< 6; i++) {
#ifdef SSSERIAL    	
    	if (this->is_softwareserial)
    		this->ss_camPort->write(_RESET[i]);
    	else
#endif    	
        	this->camPort->write(_RESET[i]);
    }

    if (wait_for_ack(ack)) {
        Serial.print("\nRESET has been acked...\n");
        ack_success = 1;
    }
    
    return ack_success;
}


int UCAM529::set_package_size() {
    int ack_success = 0;

    uint8_t ack[6] = {0xAA, 0x0E, 0x06, 0x00, 0x00, 0x00};

    for (int i = 0; i< 6; i++) {
#ifdef SSSERIAL    	
    	if (this->is_softwareserial)
    		this->ss_camPort->write(_PACK_SIZE[i]);
    	else    	
#endif
        	this->camPort->write(_PACK_SIZE[i]);
        //delay(300);
    }

#ifdef SSSERIAL
	if (this->is_softwareserial)
		while (!this->ss_camPort->available());
	else
#endif
		while (!this->camPort->available());

    if (wait_for_ack(ack)) {
        Serial.print("\nSET PACKAGE SIZE has been acked...\n");
        ack_success = 1;
    }
    
    return ack_success;    
}

int UCAM529::do_snapshot() {
    int ack_success = 0;

    uint8_t ack[6] = {0xAA, 0x0E, 0x05, 0x00, 0x00, 0x00};
    
	if (this->camMode == UCAM_RAW)
		_SNAPSHOT[2]=0x01;
	else
		_SNAPSHOT[2]=0x00;
	
	Serial.println("Snapshot is being sent");

	this->startCamSnapshotTime=millis();
			
    for (int i = 0; i< 6; i++) {
#ifdef SSSERIAL
    	if (this->is_softwareserial)
    		this->ss_camPort->write(_SNAPSHOT[i]);
    	else    	
#endif
        	this->camPort->write(_SNAPSHOT[i]);
    }

#ifdef SSSERIAL
	if (this->is_softwareserial)
		while (!this->ss_camPort->available());
	else
#endif
		while (!this->camPort->available());

    if (wait_for_ack(ack)) {
    	
    	this->stopCamSnapshotTime=millis();
        
        Serial.print("\nSNAPSHOT has been acked...\n");
        ack_success = 1;
    }
	return ack_success;
}

int UCAM529::get_picture(uint8_t pic_mode) {
    int ack_success = 0;

    uint8_t ack[6] = {0xAA, 0x0E, 0x04, 0x00, 0x00, 0x00};

	_GET_PICTURE[2]=pic_mode;
	
	Serial.println("Get picture is being sent");
	
	this->startCamGetPictureTime=millis();
	    
    for (int i = 0; i< 6; i++) {
#ifdef SSSERIAL
    	if (this->is_softwareserial)
    		this->ss_camPort->write(_GET_PICTURE[i]);
    	else     	
#endif
        	this->camPort->write(_GET_PICTURE[i]);
    }

#ifdef SSSERIAL
	if (this->is_softwareserial)
		while (!this->ss_camPort->available());
	else
#endif
		while (!this->camPort->available());

    if (wait_for_ack(ack)) {
    	
    	this->stopCamGetPictureTime=millis();
    		
        Serial.print("\nGET PICTURE has been acked...\n");
        ack_success = 1;
    }
	return ack_success;
}

int UCAM529::get_data() {
    uint8_t my_ack[6] = {0xAA, 0x0E, 0x00, 0x00, 0x00, 0x00};
    uint8_t data[6];
	int get_data_success = 0;
	
    int i = 0;

    uint8_t b;

	Serial.println("Get picture DATA");

#ifdef SSSERIAL
	if (this->is_softwareserial) {
		while (!this->ss_camPort->available());
	
	    while(this->ss_camPort->available())
	    {
	        b = this->ss_camPort->read();
	        if (b == 0xAA)
	        {
	            data[i++] = b;
	            break;
	        }
	    }
	
	    while (this->ss_camPort->available() && i<6) {
	        b = this->ss_camPort->read();
	        Serial.println(b, HEX);
	        data[i++] = b;
	    }		
	}
	else {	
#endif			
		while (!this->camPort->available());
	
	    while(this->camPort->available())
	    {
	        b = this->camPort->read();
	        if (b == 0xAA)
	        {
	            data[i++] = b;
	            break;
	        }
	    }
	
	    while (this->camPort->available() && i<6) {
	        b = this->camPort->read();
	        Serial.println(b, HEX);
	        data[i++] = b;
	    }
#ifdef SSSERIAL
	}
#endif
    
    // check the second byte
    if (data[1]!=0x0A)
    	return get_data_success;

	// check the image type just for sure
	if (data[2]==PICTURE_SNAPSHOT_TYPE || data[2]==PICTURE_RAW_TYPE || data[2]==PICTURE_JPEG_TYPE)
		get_data_success=1;
	else
		return get_data_success;
	
    long image_size = 0;
	
	// get the image size given by the ucam, in the last 3 bytes that should have been received
    for(int i = 5; i >= 3; i--)
    {
        image_size = (image_size << 8) | data[i];
    }

    Serial.print("Size of the image = ");
    Serial.println(image_size);

	return get_data_success;
	
/*
	if (this->camMode == UCAM_RAW) {
		
		
	}
	
	if (this->camMode == UCAM_JPEG) {
	
		// for JPEG 512B per packet
	    int blocks = ceil((float(image_size)/506.0));
	
	    Serial.println("Number of packets = ");
	    Serial.println(blocks);
	
	    uint8_t k = 0x00;
	    int z = 0;
	
	    while(this->camPort->available())
	    {
	        b = this->camPort->read();
	        if (b > 0)
	            Serial.print("b");
	    }
	
	    for (int j = 0; j < blocks; j++)
	    {
	        z = 0;
	
	        my_ack[4] = k++;
	
	        for (int i = 0; i< 6; i++) {
	            this->camPort->write(my_ack[i]);
	        }
	
	
	        while (!this->camPort->available());
	
	        while (z < 4)
	        {
	            b = this->camPort->read();
	            if(b != -1)
	            {
	                z++;
	            }
	        }
	
	        int size_to_read = 506;
	
	        if (j == blocks - 1)
	        {
	            size_to_read = image_size - j * 506;
	        }
	
	
	        z = 0;
	
	        Serial.print("Packet number = ");
	        Serial.println(j + 1);
	
	        while (z < size_to_read)
	        {
	            if (this->camPort->available())
	            {
	                b = this->camPort->read();
	
	
	                if (b == 0x00)
	                {
	                    //Serial3.print("ONI");
	                    // modem.send("ONI");
	                    //dataDevice->send("ONI");
	                }
	                else
	                {
	                    //Serial3.print(b);
	                    // modem.sendByte(b);
	                    //dataDevice->sendByte(b);
	                }
	
	                // delay(1);
	                //dataDevice->flowControl();
	
	                z++;
	
	              //Serial.print(z);
	            }
	        }
	
	        Serial.print("Bytes read = ");
	        Serial.println(z);
	
	        z = 0;
	
	        while (z < 2)
	        {
	            b = this->camPort->read();
	            if(b != -1)
	            {
	                z++;
	            }
	        }
	
	    }
    
	}

*/
    //dataDevice->send("BYE");
}


int UCAM529::send_ack() {

    Serial.println("Sending ACK for sync");
    for (int i = 0; i< 6; i++) {
#ifdef SSSERIAL
     	if (this->is_softwareserial)
    		this->ss_camPort->write(_ACK_COMMAND[i]);
    	else     	
#endif
        	this->camPort->write(_ACK_COMMAND[i]);
    }

    Serial.print("Now we can take images!\n");
	
    //delay(50);
    return 1;
}

int UCAM529::send_ack_picture() {

    Serial.println("Sending ACK for end of data picture");
    for (int i = 0; i< 6; i++) {
#ifdef SSSERIAL
     	if (this->is_softwareserial)
    		this->ss_camPort->write(_ACK_PICTURE_COMMAND[i]);
       	else     	
#endif
        	this->camPort->write(_ACK_PICTURE_COMMAND[i]);
    }

    Serial.print("Finish getting picture data\n");

    //delay(3000);
    return 1;
}

int UCAM529::wait_for_sync() {
    uint8_t cam_reply;

    int sync_success = 0;

    int last_reply = 0;

    Serial.print("\nWaiting for SYNC...\n");

#ifdef SSSERIAL
	if (this->is_softwareserial)
		while (!this->ss_camPort->available());
	else
#endif
		while (!this->camPort->available());

    Serial.print("\nReceiving data. Testing to see if it is SYNC...\n");

#ifdef SSSERIAL

	if (this->is_softwareserial) {
	    while (this->ss_camPort->available()) {
	
	        cam_reply = this->ss_camPort->read();
	
	        Serial.println(cam_reply, HEX);
	        Serial.flush();
	
	        if (cam_reply == 0xAA && last_reply == 0) {
	            last_reply++;
	            Serial.println("matched 0");
	        }
			else
	        if (cam_reply == 0x0D && last_reply == 1) {
	            last_reply++;
	            Serial.println("matched 1");
	        }
			else
	        if (cam_reply == 0x00 && last_reply == 2) {
	            last_reply++;
	            Serial.println("matched 2");
	        }
			else
	        if (cam_reply == 0x00 && last_reply == 3) {
	            last_reply++;
	            Serial.println("matched 3");
	        }
			else
	        if (cam_reply == 0x00 && last_reply == 4) {
	            last_reply++;
	            Serial.println("matched 4");
	        }
			else
	        if (cam_reply == 0x00 && last_reply == 5) {
	            last_reply++;
	            Serial.println("matched 5 -success");
	            Serial.print("\nCamera has SYNCED...\n");
	            sync_success = 1;
	
	            break;
	        }
	    }		
	}
	else {
#endif		
	    while (this->camPort->available()) {
	
	        cam_reply = this->camPort->read();
	
	        Serial.println(cam_reply, HEX);
	        Serial.flush();
	
	        if (cam_reply == 0xAA && last_reply == 0) {
	            last_reply++;
	            Serial.println("matched 0");
	        }
			else
	        if (cam_reply == 0x0D && last_reply == 1) {
	            last_reply++;
	            Serial.println("matched 1");
	        }
			else
	        if (cam_reply == 0x00 && last_reply == 2) {
	            last_reply++;
	            Serial.println("matched 2");
	        }
			else
	        if (cam_reply == 0x00 && last_reply == 3) {
	            last_reply++;
	            Serial.println("matched 3");
	        }
			else
	        if (cam_reply == 0x00 && last_reply == 4) {
	            last_reply++;
	            Serial.println("matched 4");
	        }
			else
	        if (cam_reply == 0x00 && last_reply == 5) {
	            last_reply++;
	            Serial.println("matched 5 -success");
	            Serial.print("\nCamera has SYNCED...\n");
	            sync_success = 1;
	
	            break;
	        }
	    }
#ifdef SSSERIAL	    
	}
#endif	

    return sync_success;
}

int UCAM529::attempt_sync()
{
    int attempts = 0;

    uint8_t cam_reply;

    int ack_success = 0;

    int last_reply = 0;

    uint8_t ack[6] = {0xAA, 0x0E, 0x0D, 0x00, 0x00, 0x00};

    while(attempts < 5 && ack_success == 0) {
    	
    	Serial.print("\nAttempt sync ");
    	Serial.println(attempts);
    	
        for (int i = 0; i< 6; i++) {
#ifdef SSSERIAL
	     	if (this->is_softwareserial)
	    		this->ss_camPort->write(_SYNC_COMMAND[i]);
	    	else        	
#endif
            	this->camPort->write(_SYNC_COMMAND[i]);
        }

        attempts++;
        
        delay(50);

#ifdef SSSERIAL
	    if (this->is_softwareserial) {    
	        if (this->ss_camPort->available())
	        {
	            if (wait_for_ack(ack)) {
	                Serial.print("\nCamera has Acked...\n");
	                ack_success = 1;
	                return ack_success;
	            }
	        }	    	
	    }
	    else {	    
#endif	    	
	        if (this->camPort->available())
	        {
	            if (wait_for_ack(ack)) {
	                Serial.print("\nCamera has Acked...\n");
	                ack_success = 1;
	                return ack_success;
	            }
	        }
#ifdef SSSERIAL	        
	    }
#endif	    
		// wait a bit between 2 retries
		delay(100);
    }

	// reset the uCam for next sync try
	//if (!ack_success) {
	//	send_reset();
	//}
	
    return ack_success;
}


int UCAM529::wait_for_ack(uint8_t* command) {
    uint8_t cam_reply;

    int wait_success = 0;
    int last_reply = 0;
    long t0=millis();

#ifdef SSSERIAL	
	if (this->is_softwareserial)
		while (!this->ss_camPort->available() && millis()-t0 < 3000);
	else
#endif
    	while (!this->camPort->available() && millis()-t0 < 3000);
    
    if (millis()-t0 > 3000)
    	return wait_success;

    Serial.println("Wait Ack");

#ifdef SSSERIAL
	if (this->is_softwareserial) {
	    while (this->ss_camPort->available()) {
	    	
	        cam_reply = this->ss_camPort->read();
	        Serial.println(cam_reply, HEX);
	        Serial.flush();
	        
	        //delay(100);
	        if (cam_reply == command[last_reply] && last_reply == 0) {
	            last_reply++;
	            //Serial.println("matched 0");
	        }
			else
	        if (cam_reply == command[last_reply] && last_reply == 1) {
	            last_reply++;
	            //Serial.println("matched 1");
	        }
			else
	        if (cam_reply == command[last_reply] && last_reply == 2) {
	            last_reply++;
	            //Serial.println("matched 2");
	        }
			else
	        if (last_reply == 3) {
	            last_reply++;
	            //Serial.println("skipped 3");
	        }
			else
	        if (cam_reply == command[last_reply] && last_reply == 4) {
	            last_reply++;
	            //Serial.println("matched 4");
	        }
			else
	        if (cam_reply == command[last_reply] && last_reply == 5) {
	            last_reply++;
	            //Serial.println("matched 5 - success");
	            wait_success = 1;
	            break;
	        }
	    }		
	}
	else {
#endif		
	    while (this->camPort->available()) {
	    	
	        cam_reply = this->camPort->read();
	        Serial.println(cam_reply, HEX);
	        //Serial.flush();
	        
	        if (cam_reply == command[last_reply] && last_reply == 0) {
	            last_reply++;
	            //Serial.println("matched 0");
	        }
			else
	        if (cam_reply == command[last_reply] && last_reply == 1) {
	            last_reply++;
	            //Serial.println("matched 1");
	        }
			else
	        if (cam_reply == command[last_reply] && last_reply == 2) {
	            last_reply++;
	            //Serial.println("matched 2");
	        }
			else
	        if (last_reply == 3) {
	            last_reply++;
	            //Serial.println("skipped 3");
	        }
			else
	        if (cam_reply == command[last_reply] && last_reply == 4) {
	            last_reply++;
	            //Serial.println("matched 4");
	        }
			else
	        if (cam_reply == command[last_reply] && last_reply == 5) {
	            last_reply++;
	            //Serial.println("matched 5 - success");
	            wait_success = 1;
	            break;
	        }
	      	delay(1);
	    }
#ifdef SSSERIAL	    
	}
#endif
    return wait_success;
}




