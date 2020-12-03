////////////////////////////////////////////////////////////////
#ifdef SX126X
//*******  Setup hardware pin definitions here ! ***************

//pinout is defined for our RFM95 PCB breakout used to host a NiceRF SX1262 module which almost the same pinout
//
#define NSS 10                                  //select pin on LoRa device
#define NRESET 4                                //reset pin on LoRa device
//we do not connect RFBUSY
//if you have a NiceRF SX1262 which has almost the same pinout than the RFM95 then
//you can use our ProMini LoRa PCB for RFM95 where RFBUSY (marked DIO2 on the RFM95 PCB) can be connected to D5
#define RFBUSY 5                                //busy pin on LoRa device 
//we do not connect DIO1 as we use polling method
//if you have a NiceRF SX1262 which has almost the same pinout than the RFM95 then
//you can use our ProMini LoRa PCB for RFM95 where DIO1 can be connected to D3
//in that case, comment #define USE_POLLING in SX126XLT.cpp to use DIO1 interrrupt pin
//#define DIO1 3                                  //DIO1 pin on LoRa device, used for RX and TX done
#define DIO1 -1                                  //DIO1 pin on LoRa device, used for RX and TX done
//*******
#define DIO2 -1                                 //DIO2 pin on LoRa device, normally not used so set to -1 
#define DIO3 -1                                 //DIO3 pin on LoRa device, normally not used so set to -1
#define RX_EN -1                                //pin for RX enable, used on some SX126X devices, set to -1 if not used
#define TX_EN -1                                //pin for TX enable, used on some SX126X devices, set to -1 if not used 
#define SW -1                                   //SW pin on some Dorji LoRa devices, used to power antenna switch, set to -1 if not used

#define LORA_DEVICE DEVICE_SX1262               //we need to define the device we are using

//*******  Setup LoRa Parameters Here ! ***************
//LoRa Modem Parameters
const uint32_t Frequency = 865200000;           //frequency of transmissions in hertz
const uint32_t Offset = 0;                      //offset frequency for calibration purposes
uint8_t Bandwidth = LORA_BW_125;          			//LoRa bandwidth
uint8_t SpreadingFactor = LORA_SF12;       			//LoRa spreading factor
uint8_t CodeRate = LORA_CR_4_5;           			//LoRa coding rate
const uint8_t Optimisation = LDRO_AUTO;         //low data rate optimisation setting, normally set to auto                               
#endif

////////////////////////////////////////////////////////////////
#ifdef SX127X
//*******  Setup hardware pin definitions here ! ***************

//pinout is defined for our RFM95 PCB breakout
//
#if defined ARDUINO_Heltec_WIFI_LoRa_32 || defined ARDUINO_WIFI_LoRa_32 || defined HELTEC_LORA
  // for the Heltec ESP32 WiFi LoRa module
#define NSS 18
#else
#define NSS 10                                  //select pin on LoRa device
#endif
#define NRESET 4                                //reset pin on LoRa device
//we do not connect DIO0 as we use polling method
//if you have an RFM95 then
//you can use our ProMini LoRa PCB for RFM95 where DIO0 can be connected to D2
//in that case, comment #define USE_POLLING in SX127XLT.cpp to use DIO0 interrrupt pin
//#define DIO0 2                                  //DIO0 pin on LoRa device, used for RX and TX done 
#define DIO0 -1                                  //DIO0 pin on LoRa device, used for RX and TX done 
//*******
#define DIO1 -1                                 //DIO1 pin on LoRa device, normally not used so set to -1 
#define DIO2 -1                                 //DIO2 pin on LoRa device, normally not used so set to -1

#define LORA_DEVICE DEVICE_SX1276               //we need to define the device we are using

//*******  Setup LoRa Parameters Here ! ***************
//LoRa Modem Parameters
const uint32_t Frequency = 865200000;           //frequency of transmissions in hertz
const uint32_t Offset = 0;                      //offset frequency for calibration purposes
const uint8_t Bandwidth = LORA_BW_125;          //LoRa bandwidth
const uint8_t SpreadingFactor = LORA_SF12;      //LoRa spreading factor
const uint8_t CodeRate = LORA_CR_4_5;           //LoRa coding rate
const uint8_t Optimisation = LDRO_AUTO;         //low data rate optimisation setting, normally set to auto

#define PABOOST                          
#endif

////////////////////////////////////////////////////////////////
#ifdef SX128X
//*******  Setup hardware pin definitions here ! ***************

//pinout is defined for our NiceRF SX1280 PCB breakout
//
#define NSS 10
#define NRESET 4
//we do not connect RFBUSY
//if you have a NiceRF SX1280
//you can use our ProMini LoRa PCB for SX1280 where RFBUSY is by default connected to D3
#define RFBUSY 3                                //busy pin on LoRa device 
//we do not connect DIO1 as we use polling method
//if you have a NiceRF SX1280
//you can use our ProMini LoRa PCB for SX1280 where DIO1 can be connected to D2
//in that case, comment #define USE_POLLING in SX128XLT.cpp to use DIO1 interrrupt pin
//#define DIO1 2                                  //DIO1 pin on LoRa device, used for RX and TX done
#define DIO1 -1                                  //DIO1 pin on LoRa device, used for RX and TX done
//*******
#define DIO2 -1                 								//not used 
#define DIO3 -1                 								//not used                      
#define RX_EN -1                								//pin for RX enable, used on some SX1280 devices, set to -1 if not used
#define TX_EN -1                								//pin for TX enable, used on some SX1280 devices, set to -1 if not used      

#define LORA_DEVICE DEVICE_SX1280               //we need to define the device we are using 

//*******  Setup LoRa Parameters Here ! ***************
//LoRa Modem Parameters
const uint32_t Frequency = 2403000000;           //frequency of transmissions in hertz
const uint32_t Offset = 0;                       //offset frequency for calibration purposes  
uint8_t Bandwidth = LORA_BW_0200;                //LoRa bandwidth
uint8_t SpreadingFactor = LORA_SF12;             //LoRa spreading factor
uint8_t CodeRate = LORA_CR_4_5;                  //LoRa coding rate                         

#define MAX_DBM                  10                 
#endif

#ifndef MAX_DBM
#define MAX_DBM                 14
#endif

#define DEFAULT_DEST_ADDR       1
