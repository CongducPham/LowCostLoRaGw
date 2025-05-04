//*******  Setup hardware pin definitions here ! ***************

//pinout is defined for our NiceRF SX1280 PCB breakout
//
// will be translated into GPIO8/SPI_CE0 by arduPI
#ifndef NSS
#define NSS 10                                  //select pin on LoRa device
#endif
// will be translated into GPIO4 by arduPI
#ifndef NRESET
#ifdef ARDUINO
#define NRESET 4
#else
#define NRESET 6                                //reset pin on LoRa device
#endif
#endif                               //reset pin on LoRa device
//will be translated into GPIO25/GEN06 by arduPI
#ifndef RFBUSY
#ifdef ARDUINO
#define RFBUSY 3
#else
#define RFBUSY 5                                //busy pin on LoRa device
#endif
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
uint32_t Frequency = 2403000000;           //frequency of transmissions in hertz
const uint32_t Offset = 0;                       //offset frequency for calibration purposes  
uint8_t Bandwidth = LORA_BW_0400;                //LoRa bandwidth
uint8_t SpreadingFactor = LORA_SF10;             //LoRa spreading factor
uint8_t CodeRate = LORA_CR_4_5;                  //LoRa coding rate        
const uint16_t Calibration = 11350;              //Manual Ranging calibrarion value
const uint32_t RangingAddress = 16;              //must match address in recever
const uint16_t  rangingRXTimeoutmS = 0xFFFF;     //ranging RX timeout in mS

#undef MAX_DBM
#define MAX_DBM									10                 

