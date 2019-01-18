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

/*
*****************************************************************************************
* INCLUDE FILES
*****************************************************************************************
*/

#include "LSC_Encrypt.h"

//size of the block 
//don't change it
const int LSC_h=4;
const int LSC_h2=LSC_h*LSC_h;
uint32_t LSC_seed=123;
//uint32_t LSC_myrand;
int LSC_rp=1;
unsigned long LSC_encodeTimeMicros;

#ifdef LSC_STATIC_KEY
#ifdef LSC_STATIC_KEY_256
extern uint8_t LSC_Nonce[256];
/*
uint8_t LSC_Nonce[256] = {
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C,
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C,
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C,
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C,
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C,
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C,
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C,
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C,
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C,
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C,
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C,
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C,
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C,
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C,
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C,
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C                                
};
*/
#elif defined LSC_STATIC_KEY_64
extern uint8_t LSC_Nonce[64];
/*
uint8_t LSC_Nonce[64] = {
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C,
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C,
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C,
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C                                
};
*/
#elif defined LSC_STATIC_KEY_32
extern uint8_t LSC_Nonce[32];
/*
uint8_t LSC_Nonce[32] = {
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C,
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C                                
};
*/
#elif defined LSC_STATIC_KEY_16
extern uint8_t LSC_Nonce[16];
/*
uint8_t LSC_Nonce[16] = {
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C                                
};
*/
#endif
#else
uint8_t LSC_Nonce[LSC_SKEY];
#endif

//dynamic key
uint8_t DK[LSC_SKEY];

//dynamic tables
//uint8_t sc[256];  
uint8_t PboxRM[LSC_h2];
uint8_t Sbox1[256];
uint8_t Sbox2[256];
uint8_t RM1[LSC_h2];  
uint8_t RM2[LSC_h2];
uint8_t RMorig[LSC_h2];

uint32_t xorshift32(const uint32_t t)
{
   /* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
        uint32_t x = t;
        x ^= (x << 13);
        x ^= (x >> 17);
        x ^= (x << 5);
        return x;
}

void rc4key(uint8_t *key, uint8_t *sc, int size_DK) {

  for(int i=0;i<256;i+=4) {
    sc[i]=i;
    sc[i+1]=i+1;
    sc[i+2]=i+2;
    sc[i+3]=i+3;
  }

  uint8_t j0 = 0;
  for(int i0=0; i0<256; i0++) {
    j0 = (j0 + sc[i0] + key[i0&(size_DK-1)] );
    uint8_t tmp = sc[i0];
    sc[i0] = sc[j0 ];
    sc[j0] = tmp;
  }
}

void rc4keyperm(uint8_t *key, int len, int rp, uint8_t *sc, int size_DK) {

  //sc=1:len;                                                                                                                 

  for (int i=0;i<len;i++) {
    sc[i]=i;
  }

  for (int it = 0; it < rp; it++) {
    int j0 = 1;
    for(int i0 = 0; i0<len; i0++) {
      j0 = (j0 + sc[i0] + sc[j0] + key[i0%size_DK] )% len;
      int tmp = sc[i0];
      sc[i0] = sc[j0];
      sc[j0] = tmp;
    }
  }
}

void prga(uint8_t *sc, int ldata, uint8_t *r) {
  uint8_t i0=0;
  uint8_t j0=0;

  for (int it=0; it<ldata; it++) {
    i0 = ((i0+1)%255);                                                                                              
    j0 = (j0 + sc[i0])&0xFF;
    uint8_t tmp = sc[i0];
    sc[i0] = sc[j0];
    sc[j0] = tmp;
    r[it]=sc[(sc[i0]+sc[j0])&0xFF];
  }
}

void LSC_encrypt_ctr(uint8_t* seq_in, uint8_t *seq_out, int len, uint8_t* RM1, uint8_t *PboxRM, uint8_t *Sbox1, uint8_t *Sbox2, uint32_t myrand) {

  uint8_t X[LSC_h2];
  uint8_t fX[LSC_h2];
  int ind=0;

  unsigned long ms1 = micros ();
  
  for(int a=0;a<LSC_h2;a+=4) {

    uint32_t mm=myrand;
    
    X[a]=Sbox1[RM1[a]^(mm&255)];           //Warning according to the size of h2, we can be outsize of Sbox1[a]
    mm>>=8;
    X[a+1]=Sbox2[RM1[a+1]^(mm&255)];
    mm>>=8;
    X[a+2]=Sbox1[RM1[a+2]^(mm&255)];
    mm>>=8;
    X[a+3]=Sbox2[RM1[a+3]^(mm&255)];
  }
  
  for(int it=0;it<len;it++) {

    for(int a=0;a<LSC_h2;a+=4) {
//#ifndef LSC_DETERMINISTIC      
      myrand=xorshift32(myrand);
//#endif      
      uint32_t mm=myrand;
      X[a]=Sbox2[X[a]^RM1[a]^(mm&255)];
      mm>>=8;
      X[a+1]=Sbox1[X[a+1]^RM1[a+1]^(mm&255)];
      mm>>=8;
      X[a+2]=Sbox2[X[a+2]^RM1[a+2]^(mm&255)];
      mm>>=8;
      X[a+3]=Sbox1[X[a+3]^RM1[a+3]^(mm&255)];
    }

    for(int a=0;a<LSC_h2;a+=4) {
      fX[a]=X[a]^seq_in[ind+a];
      fX[a+1]=X[a+1]^seq_in[ind+a+1];
      fX[a+2]=X[a+2]^seq_in[ind+a+2];
      fX[a+3]=X[a+3]^seq_in[ind+a+3];
    }

    for(int a=0;a<LSC_h2;a+=4) {
      seq_out[ind+a]=fX[a];
      seq_out[ind+a+1]=fX[a+1];
      seq_out[ind+a+2]=fX[a+2];
      seq_out[ind+a+3]=fX[a+3];
    }
      
    for(int a=0;a<LSC_h2;a+=4) {
      RM1[a]=Sbox2[RM1[PboxRM[a]]];  
      RM1[a+1]=Sbox1[RM1[PboxRM[a+1]]];
      RM1[a+2]=Sbox2[RM1[PboxRM[a+2]]];
      RM1[a+3]=Sbox1[RM1[PboxRM[a+3]]];
    }

    ind+=LSC_h2;
  }

  unsigned long ms2 = micros ();  

  LSC_encodeTimeMicros=ms2-ms1;  
}

void LSC_restore_RM() {
  //to use the same RM1 if this is a desirable behavior 
  for(int i=0;i<LSC_h2;i++) {
    RM1[i]=RM2[i]=RMorig[i];
  }  
}

void LSC_encrypt(uint8_t* seq_in, uint8_t *seq_out, uint8_t size_mesg, uint8_t fcount, int enc) {

  uint8_t lenH=(size_mesg+LSC_h2-1)/LSC_h2;

  if (enc==LSC_DECRYPT)
    LSC_encrypt_ctr(seq_in, seq_out, lenH, RM2, PboxRM, Sbox1, Sbox2, fcount);    
  else {
    //RM1 is changed in the encryption so we make a copy in RM2 for the decryption  
    for(int i=0;i<LSC_h2;i++) {
      RM2[i]=RM1[i];
    }
    LSC_encrypt_ctr(seq_in, seq_out, lenH, RM1, PboxRM, Sbox1, Sbox2, fcount);
  }  

#ifdef LSC_DETERMINISTIC
  LSC_restore_RM();
#endif  
}

// Compute the LSC MIC from a seq_in buffer of size size_mesg.
// Buffer should have 4 available extra at the end to be append the 4-byte MIC 
//
// IN:
// seq_in is the source buffer
// seq_out is a buffer of size at least equal to seq_in
// size_mesg is the size of the data on which the MIC is computed
// fcount is a random number, usually the packet sequence number to allow decryption at the other end
//
// OUT:
// seq_in will be appended by the 4-byte MIC 
// seq_out will contain the encrypted version of the original seq_in, i.e. without the MIC
//
void LSC_setMIC(uint8_t* seq_in, uint8_t *seq_out, uint8_t size_mesg, uint8_t fcount) {

      //re-use plain buffer, encrypt HEADER+CIPHER
      //but use seq+1 as new random number
      LSC_encrypt(seq_in, seq_out, size_mesg, fcount, LSC_ENCRYPT);

#ifdef LSC_MICv1
      //skip the first 4 bytes and take the next 4 bytes of encrypted HEADER+CIPHER
      seq_in[size_mesg]=seq_out[4];
      seq_in[size_mesg+1]=seq_out[5];
      seq_in[size_mesg+2]=seq_out[6];
      seq_in[size_mesg+3]=seq_out[7];  
            
#elif defined LSC_MICv2
      uint32_t myMIC=0;

      //first, compute byte-sum of encrypted HEADER+CIPHER
      for (int i = 0; i < size_mesg; i++) {
        myMIC+=seq_out[i];  
      }

      //append MIC in cipher
      seq_in[size_mesg]=(uint8_t)xorshift32(myMIC % 7); 
      seq_in[size_mesg+1]=(uint8_t)xorshift32(myMIC % 13);
      seq_in[size_mesg+2]=(uint8_t)xorshift32(myMIC % 29);
      seq_in[size_mesg+3]=(uint8_t)xorshift32(myMIC % 57);      
      //         
      //end compute MIC
#elif defined LSC_MICv3
      //should implement a better algorithm?
      //XTEA?: http://code.activestate.com/recipes/496737-python-xtea-encryption/      
#endif
}

//void printArray(uint8_t *mes, int n) {
//  for (uint8_t i = 0; i < n; i++) {
//    Serial.print(mes[i]);
//    Serial.print(" ");
//  }
//  Serial.println();
//}

void LSC_session_init() {
  
  //initialization of the key
  for(int i=0;i<LSC_SKEY;i+=4) {
     LSC_seed=xorshift32(LSC_seed);
     uint32_t val=LSC_seed;
     DK[i]=val&0xFF;
     val>>=8;
     DK[i+1]=val&0xFF;
     val>>=8;
     DK[i+2]=val&0xFF;
     val>>=8;
     DK[i+3]=val&0xFF;
  }

#ifndef LSC_STATIC_KEY

  for(int i=0;i<LSC_SKEY;i+=4) {
     LSC_seed=xorshift32(LSC_seed);
     uint32_t val=LSC_seed;
     LSC_Nonce[i]=val&0xFF;
     val>>=8;
     LSC_Nonce[i+1]=val&0xFF;
     val>>=8;
     LSC_Nonce[i+2]=val&0xFF;
     val>>=8;
     LSC_Nonce[i+3]=val&0xFF;
  }

#endif

  //Serial.println("\nDK:");
  for(int i=0;i<LSC_SKEY;i++) {
    DK[i]^=LSC_Nonce[i];
    //Serial.print(DK[i]);
    //Serial.print(" ");
  }
  //Serial.println();

  uint8_t* sc = (uint8_t*)calloc(256, sizeof(uint8_t));
  
  //computation of sbox and other dynamic tables
  rc4key(&DK[0], sc, LSC_SKEY/4);
  prga(sc, LSC_h2, RM1);
  rc4keyperm(&DK[LSC_SKEY/4], LSC_h2, LSC_rp, PboxRM, LSC_SKEY/4);
  rc4key(&DK[2*LSC_SKEY/4], Sbox1, LSC_SKEY/4);
  rc4key(&DK[3*LSC_SKEY/4], Sbox2, LSC_SKEY/4);

  free(sc);
  
  //RM1 is changed in the encryption so we make a copy in RM2 for the decryption  
  //and keep the original in RMorig
  for(int i=0;i<LSC_h2;i++) {
    RMorig[i]=RM1[i];
    RM2[i]=RM1[i];
  }
       
  //LSC_myrand=0;
  //
  //for(int i=0; i<((LSC_SKEY<32)?LSC_SKEY:32); i++) {
  //  LSC_myrand|=DK[i]&1;
  //  LSC_myrand<<=1;
  //}  
}

uint8_t LSC_get_block_size(uint8_t size_mesg) {

  //number of blocks
  uint8_t lenH=(size_mesg+LSC_h2-1);
    
  return lenH;
}
