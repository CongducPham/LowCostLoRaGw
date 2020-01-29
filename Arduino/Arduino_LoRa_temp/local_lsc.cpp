#include "local_lsc.h"
#include "SX1272.h"

// we wrapped Serial.println to support the Arduino Zero or M0
#if defined __SAMD21G18A__ && not defined ARDUINO_SAMD_FEATHER_M0
#define PRINTLN                   SerialUSB.println("")              
#define PRINT_CSTSTR(fmt,param)   SerialUSB.print(F(param))
#define PRINT_STR(fmt,param)      SerialUSB.print(param)
#define PRINT_VALUE(fmt,param)    SerialUSB.print(param)
#define PRINT_HEX(fmt,param)      SerialUSB.print(param,HEX)
#define FLUSHOUTPUT               SerialUSB.flush();
#else
#define PRINTLN                   Serial.println("")
#define PRINT_CSTSTR(fmt,param)   Serial.print(F(param))
#define PRINT_STR(fmt,param)      Serial.print(param)
#define PRINT_VALUE(fmt,param)    Serial.print(param)
#define PRINT_HEX(fmt,param)      Serial.print(param,HEX)
#define FLUSHOUTPUT               Serial.flush();
#endif

#define DEFAULT_DEST_ADDR 1

///////////////////////////////////////////////////////////////////
//Enter here your LSC encryption key
uint8_t LSC_Nonce[16] = { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C };
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////

uint8_t local_lsc_create_pkt(uint8_t* message, uint8_t pl, uint8_t app_key_offset, uint8_t p_type, uint8_t node_addr) {

      PRINT_STR("%s",(char*)(message+app_key_offset));
      PRINTLN;
      PRINT_CSTSTR("%s","[PLAIN HEX]:\n");
      for (int i=0; i<pl;i++) {
        if (message[i]<16)
          PRINT_CSTSTR("%s","0");
        PRINT_HEX("%X", message[i]);
        PRINT_CSTSTR("%s"," ");
      }
      PRINTLN; 

      //longer message may need a longer buffer
      unsigned char cipher[80];

      PRINT_CSTSTR("%s","Encrypting\n");
      LSC_encrypt(message, cipher+OFFSET_PAYLOADLENGTH, pl, sx1272._packetNumber, LSC_ENCRYPT);
      
      //Print encrypted message     
      PRINT_CSTSTR("%s","[CIPHER]:\n");
      for (int i = 0; i < pl; i++)      
      {
        if (cipher[OFFSET_PAYLOADLENGTH+i]<16)
          PRINT_CSTSTR("%s","0");
        PRINT_HEX("%X", cipher[OFFSET_PAYLOADLENGTH+i]);         
        PRINT_CSTSTR("%s"," ");
      }
      PRINTLN;   

      //compute MIC. We decide to use the packet header
      //
      cipher[0]=DEFAULT_DEST_ADDR;    //dst
      cipher[1]=p_type; //PTYPE
      cipher[2]=node_addr;    //src
      cipher[3]=sx1272._packetNumber;    // current seq

      PRINT_CSTSTR("%s","[HEADER+CIPHER]:\n");
      for (int i = 0; i < pl+OFFSET_PAYLOADLENGTH; i++) {
        if (cipher[i]<16)
          PRINT_CSTSTR("%s","0");
        PRINT_HEX("%X", cipher[i]);
        if (i==OFFSET_PAYLOADLENGTH-1)
          PRINT_CSTSTR("%s","|");
        else        
          PRINT_CSTSTR("%s"," ");      
      }
      PRINTLN;

      // compute the MIC on [HEADER+CIPHER]
      // using a random number = sx1272._packetNumber+1
      LSC_setMIC(cipher, message, pl+OFFSET_PAYLOADLENGTH, sx1272._packetNumber+1);

      PRINT_CSTSTR("%s","[encrypted HEADER+CIPHER]:\n");
      for (int i = 0; i < pl+OFFSET_PAYLOADLENGTH; i++) {
        if (message[i]<16)
          PRINT_CSTSTR("%s","0");
        PRINT_HEX("%X", message[i]);
        PRINT_CSTSTR("%s"," ");      
      }
      PRINTLN;
      
      PRINT_CSTSTR("%s","[MIC]:\n");
      for (int i = 0; i < 4; i++) {
        if (cipher[pl+OFFSET_PAYLOADLENGTH+i]<16)
          PRINT_CSTSTR("%s","0");        
        PRINT_HEX("%X", cipher[pl+OFFSET_PAYLOADLENGTH+i]);
        PRINT_CSTSTR("%s"," ");      
      }
      PRINTLN;
    
      PRINT_CSTSTR("%s","transmitted packet:\n");
      PRINT_CSTSTR("%s","CIPHER | MIC[4]\n");
      //Print transmitted data
      for(int i = 0; i < pl+LSC_MIC; i++)
      {
        if (cipher[OFFSET_PAYLOADLENGTH+i]<16)
          PRINT_CSTSTR("%s","0");
        PRINT_HEX("%X", cipher[OFFSET_PAYLOADLENGTH+i]);
        if (i==pl-1)
          PRINT_CSTSTR("%s","|");
        else                  
          PRINT_CSTSTR("%s"," ");
      }
      PRINTLN;      

      // copy back to message
      memcpy(message,cipher+OFFSET_PAYLOADLENGTH,pl+LSC_MIC);
      pl = pl+LSC_MIC;

#ifdef LSC_SHOWB64
        //need to take into account the ending \0 for string in C
        int b64_encodedLen = base64_enc_len(pl)+1;
        char b64_encoded[b64_encodedLen];
               
        base64_encode(b64_encoded, (char*)message, pl);
        PRINT_CSTSTR("%s","[base64 CIPHER+MIC]:");
        PRINT_STR("%s", b64_encoded);
        PRINTLN;
#endif   

      return pl;
}
