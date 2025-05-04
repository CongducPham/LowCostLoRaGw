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
#define DIO2 -1                 //not used 
#define DIO3 -1                 //not used                      
#define RX_EN -1                //pin for RX enable, used on some SX1280 devices, set to -1 if not used
#define TX_EN -1                //pin for TX enable, used on some SX1280 devices, set to -1 if not used      

#define LORA_DEVICE DEVICE_SX1280               //we need to define the device we are using 


//*******  Setup LoRa Parameters Here ! ***************

//LoRa Modem Parameters
const uint32_t Offset = 0;                                 //offset frequency for calibration purposes  
uint8_t Bandwidth = LORA_BW_0200;                   //LoRa bandwidth
uint8_t SpreadingFactor = LORA_SF12;                 //LoRa spreading factor
uint8_t CodeRate = LORA_CR_4_5;                     //LoRa coding rate

#define RXBUFFER_SIZE 250                  			//RX buffer size 

// IMPORTANT
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// For a Raspberry-based gateway the distribution uses a radio.makefile file that can define MAX_DBM
// 10dBm EIRP is for Europe in 2.4GHz band. It may me needed to set MAX_DBM to a higher value to get an EIRP of 10dBm.
#ifndef MAX_DBM
#define MAX_DBM 10
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// DO NOT CHANGE

// here we force BAND2400 as SX128X only works in this band
#ifndef BAND2400
#define BAND2400
#endif

#ifdef BAND868
#undef BAND868
#endif

#ifdef BAND900
#undef BAND900
#endif

#ifdef BAND433
#undef BAND433
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// GATEWAY HAS ADDRESS 1
#define GW_ADDR 1
///////////////////////////////////////////////////////////////////

// added by C. Pham
// taken from Semtech's LoRa 2.4GHz 3-channel reference design
const uint32_t CH_00_2400 = 2403000000;
const uint32_t CH_01_2400 = 2425000000;
const uint32_t CH_02_2400 = 2479000000;

uint32_t DEFAULT_CHANNEL=CH_00_2400;

#define PKT_TYPE_DATA   0x10
#define PKT_TYPE_ACK    0x20

#define PKT_TYPE_MASK   0xF0
#define PKT_FLAG_MASK   0x0F

#define PKT_FLAG_ACK_REQ            0x08
#define PKT_FLAG_DATA_ENCRYPTED     0x04
#define PKT_FLAG_DATA_WAPPKEY       0x02
#define PKT_FLAG_DATA_DOWNLINK      0x01

// END
///////////////////////////////////////////////////////////////////////////////////////////////////////////
