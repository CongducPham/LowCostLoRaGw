///////////////////////////////////////////////////////////////////////////////////////////////////////////
// add here some specific board define statements if you want to implement user-defined specific settings
// A/ LoRa radio node from IoTMCU: https://www.tindie.com/products/IOTMCU/lora-radio-node-v10/
//#define IOTMCU_LORA_RADIO_NODE
// uncomment if you are sure you have an Heltec LoRa board and that it is not detected
//#define HELTEC_LORA
// uncomment if you are sure you have an ESP8266 board and that it is not detected
//#define ESP8266
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//Use native LoRaWAN packet format to send to LoRaWAN gateway - beware it does not mean you device is a full LoRaWAN device
//#define LORAWAN

/*******************************************************************************************************
  Based from SX12XX example - Stuart Robinson 
*******************************************************************************************************/

//*******  Setup hardware pin definitions here ! ***************

#define NSS 10
#define RFBUSY 7
#define NRESET 9
#define LED1 8
#define DIO1 3
#define DIO2 -1                 //not used 
#define DIO3 -1                 //not used                      
#define RX_EN -1                //pin for RX enable, used on some SX1280 devices, set to -1 if not used
#define TX_EN -1                //pin for TX enable, used on some SX1280 devices, set to -1 if not used      

#define LORA_DEVICE DEVICE_SX1280               //we need to define the device we are using 


//*******  Setup LoRa Parameters Here ! ***************

//LoRa Modem Parameters
#define Frequency 2445000000                     //frequency of transmissions
#define Offset 0                                 //offset frequency for calibration purposes  
#define Bandwidth LORA_BW_0400                   //LoRa bandwidth
#define SpreadingFactor LORA_SF7                 //LoRa spreading factor
#define CodeRate LORA_CR_4_5                     //LoRa coding rate

/*******************************************************************************************************
  End from SX12XX example - Stuart Robinson 
*******************************************************************************************************/

#define MAX_DBM 10

const uint32_t DEFAULT_CHANNEL=Frequency;

#define PKT_TYPE_DATA   0x10
#define PKT_TYPE_ACK    0x20

#define PKT_TYPE_MASK   0xF0
#define PKT_FLAG_MASK   0x0F

#define PKT_FLAG_ACK_REQ            0x08
#define PKT_FLAG_DATA_ENCRYPTED     0x04
#define PKT_FLAG_DATA_WAPPKEY       0x02
#define PKT_FLAG_DATA_DOWNLINK      0x01

#define SX1272_ERROR_ACK        3
#define SX1272_ERROR_TOA        4

#define DEFAULT_DEST_ADDR       1

// END
///////////////////////////////////////////////////////////////////////////////////////////////////////////
