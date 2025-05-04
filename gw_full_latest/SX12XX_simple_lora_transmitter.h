////////////////////////////////////////////////////////////////
#ifdef SX126X
//*******  Setup hardware pin definitions here ! ***************

//pinout is defined for our RFM95 PCB breakout used to host a NiceRF SX1262 module which almost the same pinout
//
//will be translated into GPIO8/SPI_CE0 by arduPI
#ifndef NSS
#define NSS 10                                  //select pin on LoRa device
#endif
//will be translated into GPIO4 by arduPI
#ifndef NRESET
#define NRESET 6                                //reset pin on LoRa device
#endif                               //reset pin on LoRa device
//will be translated into GPIO25/GEN06 by arduPI. WARNING: old RFM95 PCB breakout does not expose DIO2(RFM95)/BUSY(NiceRFSX1280)
//so if you are using the old RFM95 PCB you need to solder a wire
#ifndef RFBUSY
#define RFBUSY 5                                //busy pin on LoRa device
#endif                              //busy pin on LoRa device
//will be translated into GPIO18/GEN01 by arduPI. WARNING: our current RFM95 PCB breakout does not expose DIO1
//we do not connect DIO1 as we use polling method 
#define DIO1 2                                  //DIO1 pin on LoRa device, used for RX and TX done 
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
//will be translated into GPIO8/SPI_CE0 by arduPI
#ifndef NSS
#define NSS 10                                  //select pin on LoRa device
#endif
//will be translated into GPIO4 by arduPI
#ifndef NRESET
#define NRESET 6                                //reset pin on LoRa device
#endif
//will be translated into GPIO18/GEN01 by arduPI. WARNING: our current RFM95 PCB breakout does not connect DIO0 on the RPI header pin
//we do not connect DIO0 as we use polling method
#ifndef DIO0
#define DIO0 2                                  //DIO0 pin on LoRa device, used for RX and TX done 
#endif
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

#ifndef PABOOST
#define PABOOST
#endif                          
#endif

////////////////////////////////////////////////////////////////
#ifdef SX128X
//*******  Setup hardware pin definitions here ! ***************

//pinout is defined for our NiceRF SX1280 PCB breakout
//
// will be translated into GPIO8/SPI_CE0 by arduPI
#ifndef NSS
#define NSS 10                                  //select pin on LoRa device
#endif
// will be translated into GPIO4 by arduPI
#ifndef NRESET
#define NRESET 6                                //reset pin on LoRa device
#endif                               //reset pin on LoRa device
//will be translated into GPIO25/GEN06 by arduPI
#ifndef RFBUSY
#define RFBUSY 5                                //busy pin on LoRa device
#endif
//will be translated into GPIO18/GEN01 by arduPI
//we do not connect DIO1 as we use polling method
#ifndef DIO1 
#define DIO1 2                                  //DIO1 pin on LoRa device, used for RX and TX done 
#endif
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

#undef MAX_DBM
#define MAX_DBM									10                 
#endif

#ifndef MAX_DBM
#define MAX_DBM									14
#endif
#define PKT_TYPE_DATA   				0x10
#define DEFAULT_DEST_ADDR       1

