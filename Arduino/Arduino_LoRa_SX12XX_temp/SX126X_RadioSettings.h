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
const uint32_t Offset = 0;                      //offset frequency for calibration purposes
const uint8_t Bandwidth = LORA_BW_125;          //LoRa bandwidth
const uint8_t SpreadingFactor = LORA_SF12;       //LoRa spreading factor
const uint8_t CodeRate = LORA_CR_4_5;           //LoRa coding rate
const uint8_t Optimisation = LDRO_AUTO;         //low data rate optimisation setting, normally set to auto

// can be set to LORA_IQ_NORMAL or LORA_IQ_INVERTED
const uint8_t IQ_Setting = LORA_IQ_NORMAL; 

//for SX1262, SX1268 power range is +22dBm to -9dBm
//for SX1261, power range is +15dBm t0 -9dBm

/*******************************************************************************************************
  End from SX12XX example - Stuart Robinson 
*******************************************************************************************************/

// IMPORTANT SETTINGS
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// please uncomment only 1 choice
//
#define ETSI_EUROPE_REGULATION
//#define FCC_US_REGULATION
//#define SENEGAL_REGULATION
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// please uncomment only 1 choice
#define BAND868
//#define BAND900
//#define BAND433
///////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// DO NOT CHANGE

#ifdef ETSI_EUROPE_REGULATION
#define MAX_DBM 14
#elif defined SENEGAL_REGULATION
#define MAX_DBM 10
#elif defined FCC_US_REGULATION
#define MAX_DBM 14
#endif

//FREQUENCY CHANNELS:
// added by C. Pham for Senegal
const uint32_t CH_04_868 = 863200000;
const uint32_t CH_05_868 = 863500000;
const uint32_t CH_06_868 = 863800000;
const uint32_t CH_07_868 = 864100000;
const uint32_t CH_08_868 = 864400000;
const uint32_t CH_09_868 = 864700000;
//
const uint32_t CH_10_868 = 865200000;
const uint32_t CH_11_868 = 865500000;
const uint32_t CH_12_868 = 865800000;
const uint32_t CH_13_868 = 866100000;
const uint32_t CH_14_868 = 866400000;
const uint32_t CH_15_868 = 866700000;
const uint32_t CH_16_868 = 867000000;
const uint32_t CH_17_868 = 868000000;

// added by C. Pham
const uint32_t CH_18_868 = 868100000; 
// end
const uint32_t CH_00_900 = 903080000;
const uint32_t CH_01_900 = 905240000;
const uint32_t CH_02_900 = 907400000;
const uint32_t CH_03_900 = 909560000;
const uint32_t CH_04_900 = 911720000;
const uint32_t CH_05_900 = 913880000;
const uint32_t CH_06_900 = 916040000;
const uint32_t CH_07_900 = 918200000;
const uint32_t CH_08_900 = 920360000;
const uint32_t CH_09_900 = 922520000;
const uint32_t CH_10_900 = 924680000;
const uint32_t CH_11_900 = 926840000;
const uint32_t CH_12_900 = 915000000;

// added by C. Pham
const uint32_t CH_00_433 = 433300000;
const uint32_t CH_01_433 = 433600000;
const uint32_t CH_02_433 = 433900000;
const uint32_t CH_03_433 = 434300000;
// end

#ifdef LORAWAN

// Select frequency channel
#ifdef BAND868
//868.1MHz
const uint32_t DEFAULT_CHANNEL=CH_18_868;
#elif defined BAND900
//hardcoded with the first LoRaWAN frequency
const uint32_t DEFAULT_CHANNEL=923200000;
#elif defined BAND433
//hardcoded with the first LoRaWAN frequency
const uint32_t DEFAULT_CHANNEL=433175000;
#endif 

#else //NO LORAWAN

#ifdef BAND868
#ifdef SENEGAL_REGULATION
const uint32_t DEFAULT_CHANNEL=CH_04_868;
#else
const uint32_t DEFAULT_CHANNEL=CH_10_868;
#endif
#elif defined BAND900
const uint32_t DEFAULT_CHANNEL=CH_05_900;
//For HongKong, Japan, Malaysia, Singapore, Thailand, Vietnam: 920.36MHz     
//const uint32_t DEFAULT_CHANNEL=CH_08_900;
#elif defined BAND433
const uint32_t DEFAULT_CHANNEL=CH_00_433;
#endif

#endif

#define DEFAULT_DEST_ADDR       1

// END
///////////////////////////////////////////////////////////////////////////////////////////////////////////
