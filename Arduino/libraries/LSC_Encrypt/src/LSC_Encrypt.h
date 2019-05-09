/* 
 *  Lightweight Stream Cipher example
 *
 *  Copyright (C) 2019 
 *  Raphaël Couturier, University of Franche Comté, France, raphael.couturier@univ-fcomte.fr
 *  Congduc Pham, University of Pau, France, Congduc.Pham@univ-pau.fr 
 *  
 *	The core works on LightWeight Steam Cipher are from:
 *		Hassan Noura, American University of Beirut, Lebanon
 *		Raphael Couturier, University of Franche Comte, France   
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
 *
 *
 */
 
#ifndef LSC_ENCRYPT_H
#define LSC_ENCRYPT_H

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

//if uncommented, do not change the keys nor tables, so deterministic behavior based on the initial LSC_Nonce key
#define LSC_DETERMINISTIC

//set the LSC_Nonce key size
#define LSC_SKEY 16

//indicate that we use a static, hard-coded key, otherwise, it will be a random key
#define LSC_STATIC_KEY

//then uncomment the appropriate define LSC_STATIC_KEY_XXX line
//
//#define LSC_STATIC_KEY_256
//#define LSC_STATIC_KEY_64
//#define LSC_STATIC_KEY_32
#define LSC_STATIC_KEY_16

#define LSC_MICv2
#define LSC_MIC	4

#define LSC_ENCRYPT 1
#define LSC_DECRYPT 0

//this variable keeps the last encoding/decoding time in micros
extern unsigned long LSC_encodeTimeMicros;
extern uint32_t xorshift32(const uint32_t t);

/*
*****************************************************************************************
* FUNCTION PROTOTYPES
*****************************************************************************************
*/

uint32_t xorshift32(const uint32_t t);
void LSC_encrypt(uint8_t* seq_in, uint8_t *seq_out, uint8_t size_mesg, uint8_t fcount, int enc);
void LSC_setMIC(uint8_t* seq_in, uint8_t *seq_out, uint8_t size_mesg, uint8_t fcount);
void LSC_session_init();
uint8_t LSC_get_block_size(uint8_t size_mesg);

#endif
