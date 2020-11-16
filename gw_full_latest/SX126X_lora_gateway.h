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
#endif
//will be translated into GPIO18/GEN01 by arduPI. WARNING: our current RFM95 PCB breakout does not expose DIO1
//we do not connect DIO1 as we use polling method
#ifndef DIO1 
#define DIO1 2                                  //DIO1 pin on LoRa device, used for RX and TX done 
#endif
#define DIO2 -1                                 //DIO2 pin on LoRa device, normally not used so set to -1 
#define DIO3 -1                                 //DIO3 pin on LoRa device, normally not used so set to -1
#define RX_EN -1                                //pin for RX enable, used on some SX126X devices, set to -1 if not used
#define TX_EN -1                                //pin for TX enable, used on some SX126X devices, set to -1 if not used 
#define SW -1                                   //SW pin on some Dorji LoRa devices, used to power antenna switch, set to -1 if not used

#define LORA_DEVICE DEVICE_SX1262               //we need to define the device we are using

//*******  Setup LoRa Parameters Here ! ***************

//LoRa Modem Parameters
const uint32_t Offset = 0;                      //offset frequency for calibration purposes
uint8_t Bandwidth = LORA_BW_125;          //LoRa bandwidth
uint8_t SpreadingFactor = LORA_SF12;       //LoRa spreading factor
uint8_t CodeRate = LORA_CR_4_5;           //LoRa coding rate
const uint8_t Optimisation = LDRO_AUTO;         //low data rate optimisation setting, normally set to auto

#define RXBUFFER_SIZE 250                  			//RX buffer size 

// IMPORTANT
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// it is better to use radio.makefile to indicate the default frequency band
#ifndef BAND868
#define BAND868
#endif
//#define BAND900
//#define BAND433
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//uncomment to force usage of a customized frequency. TTN plan includes 868.1/868.3/868.5/867.1/867.3/867.5/867.7/867.9 for LoRa
//#define MY_FREQUENCY 868100000
//#define MY_FREQUENCY 433170000

// IMPORTANT SETTINGS
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// please uncomment only 1 choice
//
#define ETSI_EUROPE_REGULATION
//#define FCC_US_REGULATION
//#define SENEGAL_REGULATION
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 

// For a Raspberry-based gateway the distribution uses a radio.makefile file that can define MAX_DBM
//
#ifndef MAX_DBM
#define MAX_DBM 14
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// DO NOT CHANGE

///////////////////////////////////////////////////////////////////
// GATEWAY HAS ADDRESS 1
#define GW_ADDR 1
///////////////////////////////////////////////////////////////////

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
uint32_t DEFAULT_CHANNEL=CH_04_868;
#else
uint32_t DEFAULT_CHANNEL=CH_10_868;
#endif
#elif defined BAND900
uint32_t DEFAULT_CHANNEL=CH_05_900;
//For HongKong, Japan, Malaysia, Singapore, Thailand, Vietnam: 920.36MHz     
//const uint32_t DEFAULT_CHANNEL=CH_08_900;
#elif defined BAND433
uint32_t DEFAULT_CHANNEL=CH_00_433;
#endif

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
