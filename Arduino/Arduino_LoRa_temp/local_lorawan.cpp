#include "local_lorawan.h"
#include "band_config.h"
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

///////////////////////////////////////////////////////////////////
// DO NOT CHANGE HERE
uint16_t Frame_Counter_Up = 0x0000;
// we use the same convention than for LoRaWAN as we will use the same AES convention
// See LoRaWAN specifications
unsigned char Direction = 0x00;
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////

int local_lorawan_init(uint8_t SF) {

  int e;

  e = sx1272.setBW(BW_125);
  PRINT_CSTSTR("%s","Set BW to 125kHz: state"); 
  PRINT_VALUE("%d", e);
  PRINTLN; 
  
  //we can also change the SF value for LoRaWAN
  e = sx1272.setSF(SF);
  PRINT_CSTSTR("%s","Set SF to ");
  PRINT_VALUE("%d", SF);    
  PRINT_CSTSTR("%s",": state ");
  PRINT_VALUE("%d", e);
  PRINTLN;  
  
  // Select frequency channel
#ifdef BAND868
  //868.1MHz
  e = sx1272.setChannel(CH_18_868);
  PRINT_CSTSTR("%s","Set frequency to 868.1MHz: state ");
#elif defined BAND900
  //hardcoded with the first LoRaWAN frequency
  uint32_t loraChannel=923.2*1000000.0*RH_LORA_FCONVERT;
  e = sx1272.setChannel(loraChannel);
  PRINT_CSTSTR("%s","Set frequency to 923.2MHz: state ");
#elif defined BAND433
  //hardcoded with the first LoRaWAN frequency
  uint32_t loraChannel=433.175*1000000.0*RH_LORA_FCONVERT;
  e = sx1272.setChannel(loraChannel);
  PRINT_CSTSTR("%s","Set frequency to 433.175MHz: state ");
#endif 

  PRINT_VALUE("%d", e);
  PRINTLN;
  
  e = sx1272.setSyncWord(0x34);
  PRINT_CSTSTR("%s","Set sync word to 0x34: state ");
  PRINT_VALUE("%d", e);
  PRINTLN;
  
  return e;
}

uint8_t local_aes_lorawan_create_pkt(uint8_t* message, uint8_t pl, uint8_t app_key_offset, bool is_lorawan) {

      PRINT_STR("%s",(char*)(message+app_key_offset));
      PRINTLN;
      PRINT_CSTSTR("%s","plain payload hex\n");
      for (int i=0; i<pl;i++) {
        if (message[i]<16)
          PRINT_CSTSTR("%s","0");
        PRINT_HEX("%X", message[i]);
        PRINT_CSTSTR("%s"," ");
      }
      PRINTLN; 

      PRINT_CSTSTR("%s","Encrypting\n");     
      PRINT_CSTSTR("%s","encrypted payload\n");
      Encrypt_Payload((unsigned char*)message, pl, Frame_Counter_Up, Direction);
      //Print encrypted message
      for (int i = 0; i < pl; i++)      
      {
        if (message[i]<16)
          PRINT_CSTSTR("%s","0");
        PRINT_HEX("%X", message[i]);         
        PRINT_CSTSTR("%s"," ");
      }
      PRINTLN;   

      // with encryption, we use for the payload a LoRaWAN packet format to reuse available LoRaWAN encryption libraries
      //
      unsigned char LORAWAN_Data[80];
      unsigned char LORAWAN_Package_Length;
      unsigned char MIC[4];
      //Unconfirmed data up
      unsigned char Mac_Header = 0x40;
      // no ADR, not an ACK and no options
      unsigned char Frame_Control = 0x00;
      // with application data so Frame_Port = 1..223
      unsigned char Frame_Port = 0x01; 

      //Build the Radio Package, LoRaWAN format
      //See LoRaWAN specification
      LORAWAN_Data[0] = Mac_Header;
    
      LORAWAN_Data[1] = DevAddr[3];
      LORAWAN_Data[2] = DevAddr[2];
      LORAWAN_Data[3] = DevAddr[1];
      LORAWAN_Data[4] = DevAddr[0];
    
      LORAWAN_Data[5] = Frame_Control;
    
      LORAWAN_Data[6] = (Frame_Counter_Up & 0x00FF);
      LORAWAN_Data[7] = ((Frame_Counter_Up >> 8) & 0x00FF);
    
      LORAWAN_Data[8] = Frame_Port;
    
      //Set Current package length
      LORAWAN_Package_Length = 9;
      
      //Load Data
      for(int i = 0; i < pl; i++)
      {
        // see that we don't take the appkey, just the encrypted data that starts at message[app_key_offset]
        LORAWAN_Data[LORAWAN_Package_Length + i] = message[i];
      }
    
      //Add data Lenth to package length
      LORAWAN_Package_Length = LORAWAN_Package_Length + pl;
    
      PRINT_CSTSTR("%s","calculate MIC with NwkSKey\n");
      //Calculate MIC
      Calculate_MIC(LORAWAN_Data, MIC, LORAWAN_Package_Length, Frame_Counter_Up, Direction);
    
      //Load MIC in package
      for (int i=0; i < 4; i++)
      {
        LORAWAN_Data[i + LORAWAN_Package_Length] = MIC[i];
      }
    
      //Add MIC length to package length
      LORAWAN_Package_Length = LORAWAN_Package_Length + 4;
    
      PRINT_CSTSTR("%s","transmitted LoRaWAN-like packet:\n");
      PRINT_CSTSTR("%s","MHDR[1] | DevAddr[4] | FCtrl[1] | FCnt[2] | FPort[1] | EncryptedPayload | MIC[4]\n");
      //Print transmitted data
      for (int i = 0; i < LORAWAN_Package_Length; i++)
      {
        if (LORAWAN_Data[i]<16)
          PRINT_CSTSTR("%s","0");
        PRINT_HEX("%X", LORAWAN_Data[i]);         
        PRINT_CSTSTR("%s"," ");
      }
      PRINTLN;      

      // copy back to message
      memcpy(message,LORAWAN_Data,LORAWAN_Package_Length);
      pl = LORAWAN_Package_Length;

#ifdef AES_SHOWB64
      //need to take into account the ending \0 for string in C
      int b64_encodedLen = base64_enc_len(pl)+1;
      char b64_encoded[b64_encodedLen];
               
      base64_encode(b64_encoded, (char*)message, pl);
      PRINT_CSTSTR("%s","[base64 LoRaWAN HEADER+CIPHER+MIC]:");
      PRINT_STR("%s", b64_encoded);
      PRINTLN;
#endif      

      if (is_lorawan) {
          PRINT_CSTSTR("%s","end-device uses native LoRaWAN packet format\n");
          // indicate to SX1272 lib that raw mode at transmission is required to avoid our own packet header
          sx1272._rawFormat=true;
      }
      else {
          PRINT_CSTSTR("%s","end-device uses encapsulated LoRaWAN packet format only for encryption\n");
      }
      // in any case, we increment Frame_Counter_Up
      // even if the transmission will not succeed
      Frame_Counter_Up++;

      return pl;
}
