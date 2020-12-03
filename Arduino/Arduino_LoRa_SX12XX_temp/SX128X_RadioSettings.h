///////////////////////////////////////////////////////////////////////////////////////////////////////////
// add here some specific board define statements if you want to implement user-defined specific settings
// A/ LoRa radio node from IoTMCU: https://www.tindie.com/products/IOTMCU/lora-radio-node-v10/
//#define IOTMCU_LORA_RADIO_NODE
// uncomment if you are sure you have an Heltec LoRa board and that it is not detected
//#define HELTEC_LORA
// uncomment if you are sure you have an ESP8266 board and that it is not detected
//#define ESP8266
///////////////////////////////////////////////////////////////////////////////////////////////////////////

/*******************************************************************************************************
  Based from SX12XX example - Stuart Robinson 
*******************************************************************************************************/

//*******  Setup hardware pin definitions here ! ***************

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
#define DIO2 -1                 //not used 
#define DIO3 -1                 //not used                      
#define RX_EN -1                //pin for RX enable, used on some SX1280 devices, set to -1 if not used
#define TX_EN -1                //pin for TX enable, used on some SX1280 devices, set to -1 if not used      

#define LORA_DEVICE DEVICE_SX1280               //we need to define the device we are using 

//*******  Setup LoRa Parameters Here ! ***************

//LoRa Modem Parameters
#define Offset 0                                 //offset frequency for calibration purposes  
#define Bandwidth LORA_BW_0200                   //LoRa bandwidth
#define SpreadingFactor LORA_SF12                 //LoRa spreading factor
#define CodeRate LORA_CR_4_5                     //LoRa coding rate

// can be set to LORA_IQ_NORMAL or LORA_IQ_INVERTED
const uint8_t IQ_Setting = LORA_IQ_NORMAL;

/*******************************************************************************************************
  End from SX12XX example - Stuart Robinson 
*******************************************************************************************************/

#define MAX_DBM 10

// taken from Semtech's LoRa 2.4GHz 3-channel reference design
const uint32_t CH_00_2400 = 2403000000;
const uint32_t CH_01_2400 = 2425000000;
const uint32_t CH_02_2400 = 2479000000;

const uint32_t DEFAULT_CHANNEL=CH_00_2400;

#define DEFAULT_DEST_ADDR       1

// END
///////////////////////////////////////////////////////////////////////////////////////////////////////////
