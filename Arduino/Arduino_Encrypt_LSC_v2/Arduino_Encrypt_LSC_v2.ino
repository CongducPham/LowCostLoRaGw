/* 
 *  Lightweight Stream Cipher example
 *
 *  Copyright (C) 2019 
 *  Raphaël Couturier, University of Franche Comté, France, raphael.couturier@univ-fcomte.fr
 *  Congduc Pham, University of Pau, France, Congduc.Pham@univ-pau.fr 
 *  
 *  The core works on LightWeight Steam Cipher are from:
 *    Hassan Noura, American University of Beirut, Lebanon
 *    Raphael Couturier, University of Franche Comte, France   
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
 
#include "LSC_Encrypt.h"
#include <Base64.h>

uint8_t LSC_Nonce[16] = {
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C                                
};

//if commented, will use the same string: char plain_txt[] = "HELLO WORLD!!!!!!!!!";
//otherwise, will vary message size from 16 to 240 bytes, with random content
//
//#define LSC_ITERATE

//if uncommented, will computer a MIC by adding the 4-byte header, then encrypt HEADER+CIPHER. MIC will be 4 first bytes.
//then append MIC to get HEADER+CIPHER+MIC
#define WMIC

#ifdef WMIC
//#define MICv1
#define MICv2
//#define MICv3
#define HEADER 4
#define MIC 4
#else
#define HEADER 0
#define MIC 0
#endif

uint8_t fcount;

void setup ()
{
  Serial.begin(38400);
  delay(2000);

  //need to initialize the LSC encoder
  //each time you want to change the session key, you need to call this function
  LSC_session_init();

  fcount=0;
}

void loop ()
{
  int size_mesg = 0;
  uint8_t plain[256];
  uint8_t *cipher;
  uint8_t *check;
  uint32_t my_seed=678;
  
  while (1) {

#ifdef LSC_ITERATE

    if (size_mesg < 240)
      size_mesg += 16;
    else
      size_mesg = 16;

    //we keep the same message or not
    //randomSeed(334);
    for (int i = 0; i < size_mesg; i++) {
      my_seed = xorshift32(my_seed);
      plain[i] = my_seed & 0xFF;
    }
#else

    char plain_txt[] = "HELLO WORLD!!!!!!!!!";
    size_mesg = sizeof(plain_txt) - 1;
    memcpy((uint8_t*)plain, (uint8_t*)plain_txt, size_mesg);
#endif

    Serial.print("message size: ");
    Serial.println(size_mesg);

    //get the correct buffer size for encrypting
    //you can avoid to do that if you allocate much more than needed
    uint8_t lenH = LSC_get_block_size(size_mesg);

    Serial.print("buffer size: ");
    Serial.println(lenH);

    Serial.print("fcount: ");
    Serial.println(fcount);
    
    Serial.println("[PLAIN]:");
    for (int i = 0; i < size_mesg; i++) {
      Serial.print(plain[i]);
      Serial.print(" ");
    }
    Serial.println();
        
    cipher = (uint8_t*)calloc(lenH, sizeof(uint8_t));

    if (cipher == NULL) {
      Serial.println("cannot allocate memory for cipher buffer");
    }
    else {
      
      LSC_encrypt(plain, cipher+HEADER, size_mesg, fcount, LSC_ENCRYPT);

      Serial.print("Encryption took: ");
      Serial.println(LSC_encodeTimeMicros);

      Serial.println("[CIPHER]:");
      for (int i = 0; i < size_mesg; i++) {
        Serial.print(cipher[HEADER+i]);
        Serial.print(" ");
      }
      Serial.println();
        
#ifdef WMIC
      //compute MIC. We decide to use the packet header
      //
      cipher[0]=1;    //dst
      cipher[1]=0x10 | 0x04 | 0x02; //PTYPE=PKT_TYPE_DATA | PKT_FLAG_DATA_WAPPKEY | PKT_FLAG_DATA_ENCRYPTED
      cipher[2]=6;    //src
      cipher[3]=fcount;    //seq

      // compute the MIC on [HEADER+CIPHER]
      // re-use plain buffer which will contain encrypted HEADER+CIPHER
      // using a random number = fcount+1
      LSC_setMIC(cipher, plain, size_mesg+HEADER, fcount+1);

      Serial.println("[encrypted HEADER+CIPHER]:");
      for (int i = 0; i < size_mesg+HEADER; i++) {
        Serial.print(plain[i]);
        Serial.print(" ");      
      }
      Serial.println();

      Serial.println("[MIC]:");
      for (int i = 0; i < 4; i++) {
        Serial.print(cipher[size_mesg+HEADER+i]);
        Serial.print(" ");      
      }
      Serial.println();
      
      Serial.println("[ CIPHER | MIC[4] ]:");
      for (int i = 0; i < size_mesg+MIC; i++) {
        Serial.print(cipher[HEADER+i]);
        if (i==size_mesg-1)
          Serial.print("|");
        else  
          Serial.print(" ");
      }
      Serial.println();
      
      //copy back the plain content
      memcpy((uint8_t*)plain, (uint8_t*)plain_txt, size_mesg);
#endif

      check = (uint8_t*)calloc(lenH, sizeof(uint8_t));

      if (check == NULL) {
        Serial.println("cannot allocate memory for check buffer");
        free(cipher);
      }
      else {
        
        LSC_encrypt(cipher+HEADER, check, size_mesg, fcount, LSC_DECRYPT);

        Serial.print("Decryption took: ");
        Serial.println(LSC_encodeTimeMicros);

#ifndef LSC_ITERATE
        //need to take into account the ending \0 for string in C
        int b64_encodedLen = base64_enc_len(size_mesg+MIC)+1;
        char b64_encoded[b64_encodedLen];
               
        base64_encode(b64_encoded, (char*)(cipher+HEADER), size_mesg+MIC);
#ifdef WMIC
        Serial.println("[base64 CIPHER+MIC]:");
#else
        Serial.println("[base64 CIPHER]:");
#endif
        Serial.println(b64_encoded);
#endif

        Serial.println("[CHECK]:");
        for (int i = 0; i < size_mesg; i++) {
          Serial.print(check[i]);
          Serial.print(" ");
        }
        Serial.println();

        bool equal = true;
        
        for (int i = 0; i < size_mesg; i++) {
          if (check[i] != plain[i])
            equal = false;
        }
        
        Serial.print("CHECK ");
        Serial.println(equal);

        free(cipher);
        free(check);
      }
    }
#ifdef LSC_ITERATE
    delay(200);
#else
    delay(30000);
#endif
    fcount++;
  }
}
