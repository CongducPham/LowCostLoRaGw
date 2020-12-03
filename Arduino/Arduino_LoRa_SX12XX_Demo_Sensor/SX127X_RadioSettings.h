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
const uint32_t Offset = 0;                      //offset frequency for calibration purposes
const uint8_t Bandwidth = LORA_BW_125;          //LoRa bandwidth
const uint8_t SpreadingFactor = LORA_SF12;      //LoRa spreading factor
const uint8_t CodeRate = LORA_CR_4_5;           //LoRa coding rate
const uint8_t Optimisation = LDRO_AUTO;         //low data rate optimisation setting, normally set to auto
// set to 1 if your radio is an HopeRF RFM92W, HopeRF RFM95W, Modtronix inAir9B, NiceRF1276
// or you known from the circuit diagram that output use the PABOOST line instead of the RFO line
const uint8_t PA_BOOST = 1;
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
//#define USE_20DBM
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

#ifdef USE_20DBM
#undef MAX_DBM
#define MAX_DBM 20
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

#define DEFAULT_DEST_ADDR       1

// END
///////////////////////////////////////////////////////////////////////////////////////////////////////////
