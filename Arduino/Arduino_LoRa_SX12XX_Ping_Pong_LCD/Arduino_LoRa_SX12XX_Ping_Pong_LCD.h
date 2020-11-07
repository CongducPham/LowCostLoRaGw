////////////////////////////////////////////////////////////////
#ifdef SX126X
//*******  Setup hardware pin definitions here ! ***************
#define NSS 10                                  //select pin on LoRa device
#ifdef ARDUINO
#define NRESET 4                                //reset pin on LoRa device
#else
// will be translated into GPIO4 by arduPI
#define NRESET 6                                //reset pin on LoRa device
#endif
#define RFBUSY 5                                //busy pin on LoRa device 
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
#define NSS 10                                  //select pin on LoRa device
#ifdef ARDUINO
#define NRESET 4                                //reset pin on LoRa device
#else
// will be translated into GPIO4 by arduPI
#define NRESET 6                                //reset pin on LoRa device
#endif
#define DIO0 2                                  //DIO0 pin on LoRa device, used for RX and TX done 
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
#define NSS 10
#ifdef ARDUINO
#define NRESET 4                                //reset pin on LoRa device
#else
// will be translated into GPIO4 by arduPI
#define NRESET 6                                //reset pin on LoRa device
#endif
#define RFBUSY 5
#define DIO1 2
#define DIO2 -1                 								//not used 
#define DIO3 -1                 								//not used                      
#define RX_EN -1                								//pin for RX enable, used on some SX1280 devices, set to -1 if not used
#define TX_EN -1                								//pin for TX enable, used on some SX1280 devices, set to -1 if not used      

#define LORA_DEVICE DEVICE_SX1280               //we need to define the device we are using 

//*******  Setup LoRa Parameters Here ! ***************
//LoRa Modem Parameters
const uint32_t Frequency = 2403000000;           //frequency of transmissions in hertz
const uint32_t Offset = 0;                       //offset frequency for calibration purposes  
uint8_t Bandwidth = LORA_BW_0400;                //LoRa bandwidth
uint8_t SpreadingFactor = LORA_SF12;             //LoRa spreading factor
uint8_t CodeRate = LORA_CR_4_5;                  //LoRa coding rate                         
#endif

#define RXBUFFER_SIZE 250                        //RX buffer size  
#define MAX_DBM                  14
#define PKT_TYPE_DATA           0x10
#define DEFAULT_DEST_ADDR       1
