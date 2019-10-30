#include <stdint.h>
#include <stdio.h>

struct pack
{
	//! Structure Variable : Packet destination
	/*!
 	*/
	uint8_t dst;

    // added by C. Pham
    //! Structure Variable : Packet type
    /*!
    */
    uint8_t type;

	//! Structure Variable : Packet source
	/*!
 	*/
	uint8_t src;

	//! Structure Variable : Packet number
	/*!
 	*/
	uint8_t packnum;

    // modified by C. Pham
    // will not be used in the transmitted packet
	//! Structure Variable : Packet length
	/*!
 	*/
	uint8_t length;

    // modified by C. Pham
    // use a pointer instead of static variable to same memory footprint
	//! Structure Variable : Packet payload
	/*!
 	*/
    uint8_t* data;

    // modified by C. Pham
    // will not be used in the transmitted packet
	//! Structure Variable : Retry number
	/*!
 	*/
	uint8_t retry;
};

//! Variable : array with all the information about a sent packet.
//!
/*!
*/
pack packet_sent;

//! Variable : array with all the information about a received packet.
//!
/*!
*/
pack packet_received;

//! Variable : array with all the information about a sent/received ack.
//!
/*!
*/
pack ACK;

//! Structure Variable : Packet payload
/*!
*/
uint8_t packet_data[50];
uint8_t ack_data[2];

uint8_t getValue() {
	return 65;
}

#define LSB(w)           ((uint8_t)(w))
#define MSB(w)           ((uint8_t)(((uint16_t)(w) >> 8) & 0xFF))

int main() {

    // we use the same memory area to reduce memory footprint
    packet_sent.data=packet_data;
    packet_received.data=packet_data;
    // ACK packet has a very small separate memory area
    ACK.data=ack_data;
    
    for (int i=0; i<20; i++) {
    	packet_sent.data[i]=getValue();
    }
    
    for (int i=0; i<20; i++) {
    	printf("%d\n", packet_received.data[i]);
    }
    
    for (int i=0; i<20; i++) {
    	printf("%c\n", (char)packet_received.data[i]);
    }
    
    uint16_t value=0x1234;
    
    printf("MSB=%.2X\n", MSB(value));
    printf("LSB=%.2X\n", LSB(value));
}