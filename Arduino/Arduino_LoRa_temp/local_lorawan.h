#ifndef LOCAL_LORAWAN
#define LOCAL_LORAWAN

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include "AES-128_V10.h"
#include "Encrypt_V31.h"

#define AES_SHOWB64
#ifdef AES_SHOWB64
#include "Base64.h" 
#endif

extern uint8_t node_addr;

int local_lorawan_init(uint8_t SF);
uint8_t local_aes_lorawan_create_pkt(uint8_t* message, uint8_t pl, uint8_t app_key_offset, bool is_lorawan);

#endif
