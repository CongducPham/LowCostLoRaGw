/* 
 *  Library for LoRa 868 / 915MHz SX1272 LoRa module
 *  
 *  Copyright (C) Libelium Comunicaciones Distribuidas S.L. 
 *  http://www.libelium.com 
 *  
 *  This program is free software: you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License as published by 
 *  the Free Software Foundation, either version 3 of the License, or 
 *  (at your option) any later version. 
 *  
 *  This program is distributed in the hope that it will be useful, 
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License 
 *  along with this program.  If not, see http://www.gnu.org/licenses/. 
 *  
 *  Version:           1.1
 *  Design:            David Gascón 
 *  Implementation:    Covadonga Albiñana & Victor Boria
 */


#ifndef SX1272_h
#define SX1272_h

/******************************************************************************
 * Includes
 ******************************************************************************/

#include <stdint.h>

#ifdef RASPBERRY2
#include "arduPi_pi2.h"
#else
#include "arduPi.h"
#endif

#ifndef inttypes_h
	#include <inttypes.h>
#endif

/******************************************************************************
 * Definitions & Declarations
 *****************************************************************************/

// added by C. Pham
#define W_REQUESTED_ACK
//#define W_NET_KEY
//#define W_INITIALIZATION
#define SX1272_RST  7

#define SX1272Chip  0
#define SX1276Chip  1
// end

#define SX1272_SS 10

#define SX1272_debug_mode 0

//! MACROS //
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)  // read a bit
#define bitSet(value, bit) ((value) |= (1UL << (bit)))    // set bit to '1'
#define bitClear(value, bit) ((value) &= ~(1UL << (bit))) // set bit to '0'

//! REGISTERS //

#define        REG_FIFO        					0x00
#define        REG_OP_MODE        				0x01
#define        REG_BITRATE_MSB    				0x02
#define        REG_BITRATE_LSB    				0x03
#define        REG_FDEV_MSB   					0x04
#define        REG_FDEV_LSB    					0x05
#define        REG_FRF_MSB    					0x06
#define        REG_FRF_MID    					0x07
#define        REG_FRF_LSB    					0x08
#define        REG_PA_CONFIG    				0x09
#define        REG_PA_RAMP    					0x0A
#define        REG_OCP    						0x0B
#define        REG_LNA    						0x0C
#define        REG_RX_CONFIG    				0x0D
#define        REG_FIFO_ADDR_PTR  				0x0D
#define        REG_RSSI_CONFIG   				0x0E
#define        REG_FIFO_TX_BASE_ADDR 		    0x0E
#define        REG_RSSI_COLLISION    			0x0F
#define        REG_FIFO_RX_BASE_ADDR   			0x0F
#define        REG_RSSI_THRESH    				0x10
#define        REG_FIFO_RX_CURRENT_ADDR   		0x10
#define        REG_RSSI_VALUE_FSK	    		0x11
#define        REG_IRQ_FLAGS_MASK    			0x11
#define        REG_RX_BW		    			0x12
#define        REG_IRQ_FLAGS	    			0x12
#define        REG_AFC_BW		    			0x13
#define        REG_RX_NB_BYTES	    			0x13
#define        REG_OOK_PEAK	    				0x14
#define        REG_RX_HEADER_CNT_VALUE_MSB  	0x14
#define        REG_OOK_FIX	    				0x15
#define        REG_RX_HEADER_CNT_VALUE_LSB  	0x15
#define        REG_OOK_AVG	 					0x16
#define        REG_RX_PACKET_CNT_VALUE_MSB  	0x16
#define        REG_RX_PACKET_CNT_VALUE_LSB  	0x17
#define        REG_MODEM_STAT	  				0x18
#define        REG_PKT_SNR_VALUE	  			0x19
#define        REG_AFC_FEI	  					0x1A
#define        REG_PKT_RSSI_VALUE	  			0x1A
#define        REG_AFC_MSB	  					0x1B
#define        REG_RSSI_VALUE_LORA	  			0x1B
#define        REG_AFC_LSB	  					0x1C
#define        REG_HOP_CHANNEL	  				0x1C
#define        REG_FEI_MSB	  					0x1D
#define        REG_MODEM_CONFIG1	 		 	0x1D
#define        REG_FEI_LSB	  					0x1E
#define        REG_MODEM_CONFIG2	  			0x1E
#define        REG_PREAMBLE_DETECT  			0x1F
#define        REG_SYMB_TIMEOUT_LSB  			0x1F
#define        REG_RX_TIMEOUT1	  				0x20
#define        REG_PREAMBLE_MSB_LORA  			0x20
#define        REG_RX_TIMEOUT2	  				0x21
#define        REG_PREAMBLE_LSB_LORA  			0x21
#define        REG_RX_TIMEOUT3	 				0x22
#define        REG_PAYLOAD_LENGTH_LORA		 	0x22
#define        REG_RX_DELAY	 					0x23
#define        REG_MAX_PAYLOAD_LENGTH 			0x23
#define        REG_OSC		 					0x24
#define        REG_HOP_PERIOD	  				0x24
#define        REG_PREAMBLE_MSB_FSK 			0x25
#define        REG_FIFO_RX_BYTE_ADDR 			0x25
#define        REG_PREAMBLE_LSB_FSK 			0x26
// added by C. Pham
#define        REG_MODEM_CONFIG3	  			0x26
// end
#define        REG_SYNC_CONFIG	  				0x27
#define        REG_SYNC_VALUE1	 				0x28
#define        REG_SYNC_VALUE2	  				0x29
#define        REG_SYNC_VALUE3	  				0x2A
#define        REG_SYNC_VALUE4	  				0x2B
#define        REG_SYNC_VALUE5	  				0x2C
#define        REG_SYNC_VALUE6	  				0x2D
#define        REG_SYNC_VALUE7	  				0x2E
#define        REG_SYNC_VALUE8	  				0x2F
#define        REG_PACKET_CONFIG1	  			0x30
#define        REG_PACKET_CONFIG2	  			0x31
#define        REG_DETECT_OPTIMIZE              0x31
#define        REG_PAYLOAD_LENGTH_FSK			0x32
#define        REG_NODE_ADRS	  				0x33
#define        REG_BROADCAST_ADRS	 		 	0x34
#define        REG_FIFO_THRESH	  				0x35
#define        REG_SEQ_CONFIG1	  				0x36
#define        REG_SEQ_CONFIG2	  				0x37
#define        REG_DETECTION_THRESHOLD          0x37
#define        REG_TIMER_RESOL	  				0x38
#define        REG_TIMER1_COEF	  				0x39
// added by C. Pham
#define        REG_SYNC_WORD                    0x39
//end
#define        REG_TIMER2_COEF	  				0x3A
#define        REG_IMAGE_CAL	  				0x3B
#define        REG_TEMP		  					0x3C
#define        REG_LOW_BAT	  					0x3D
#define        REG_IRQ_FLAGS1	  				0x3E
#define        REG_IRQ_FLAGS2	  				0x3F
#define        REG_DIO_MAPPING1	  				0x40
#define        REG_DIO_MAPPING2	  				0x41
#define        REG_VERSION	  					0x42
#define        REG_AGC_REF	  					0x43
#define        REG_AGC_THRESH1	  				0x44
#define        REG_AGC_THRESH2	  				0x45
#define        REG_AGC_THRESH3	  				0x46
#define        REG_PLL_HOP	  					0x4B
#define        REG_TCXO		  					0x58
#define        REG_PA_DAC		  				0x5A
#define        REG_PLL		  					0x5C
#define        REG_PLL_LOW_PN	  				0x5E
#define        REG_FORMER_TEMP	  				0x6C
#define        REG_BIT_RATE_FRAC	  			0x70

// added by C. Pham
// copied from LoRaMAC-Node
/*!
 * RegImageCal
 */
#define RF_IMAGECAL_AUTOIMAGECAL_MASK               0x7F
#define RF_IMAGECAL_AUTOIMAGECAL_ON                 0x80
#define RF_IMAGECAL_AUTOIMAGECAL_OFF                0x00  // Default

#define RF_IMAGECAL_IMAGECAL_MASK                   0xBF
#define RF_IMAGECAL_IMAGECAL_START                  0x40

#define RF_IMAGECAL_IMAGECAL_RUNNING                0x20
#define RF_IMAGECAL_IMAGECAL_DONE                   0x00  // Default

#define RF_IMAGECAL_TEMPCHANGE_HIGHER               0x08
#define RF_IMAGECAL_TEMPCHANGE_LOWER                0x00

#define RF_IMAGECAL_TEMPTHRESHOLD_MASK              0xF9
#define RF_IMAGECAL_TEMPTHRESHOLD_05                0x00
#define RF_IMAGECAL_TEMPTHRESHOLD_10                0x02  // Default
#define RF_IMAGECAL_TEMPTHRESHOLD_15                0x04
#define RF_IMAGECAL_TEMPTHRESHOLD_20                0x06

#define RF_IMAGECAL_TEMPMONITOR_MASK                0xFE
#define RF_IMAGECAL_TEMPMONITOR_ON                  0x00 // Default
#define RF_IMAGECAL_TEMPMONITOR_OFF                 0x01

// added by C. Pham
// The crystal oscillator frequency of the module
#define RH_LORA_FXOSC 32000000.0

// The Frequency Synthesizer step = RH_LORA_FXOSC / 2^^19
#define RH_LORA_FCONVERT  (524288 / RH_LORA_FXOSC)

// Frf = frf(Hz)*2^19/RH_LORA_FXOSC

/////

//FREQUENCY CHANNELS:
// added by C. Pham for Senegal
const uint32_t CH_04_868 = 0xD7CCCC; // channel 04, central freq = 863.20MHz
const uint32_t CH_05_868 = 0xD7E000; // channel 05, central freq = 863.50MHz
const uint32_t CH_06_868 = 0xD7F333; // channel 06, central freq = 863.80MHz
const uint32_t CH_07_868 = 0xD80666; // channel 07, central freq = 864.10MHz
const uint32_t CH_08_868 = 0xD81999; // channel 08, central freq = 864.40MHz
const uint32_t CH_09_868 = 0xD82CCC; // channel 09, central freq = 864.70MHz
//
const uint32_t CH_10_868 = 0xD84CCC; // channel 10, central freq = 865.20MHz
const uint32_t CH_11_868 = 0xD86000; // channel 11, central freq = 865.50MHz
const uint32_t CH_12_868 = 0xD87333; // channel 12, central freq = 865.80MHz
const uint32_t CH_13_868 = 0xD88666; // channel 13, central freq = 866.10MHz
const uint32_t CH_14_868 = 0xD89999; // channel 14, central freq = 866.40MHz
const uint32_t CH_15_868 = 0xD8ACCC; // channel 15, central freq = 866.70MHz
const uint32_t CH_16_868 = 0xD8C000; // channel 16, central freq = 867.00MHz
const uint32_t CH_17_868 = 0xD90000; // channel 17, central freq = 868.00MHz

// added by C. Pham
const uint32_t CH_18_868 = 0xD90666; // 868.1MHz for LoRaWAN test

const uint32_t CH_00_900 = 0xE1C51E; // channel 00, central freq = 903.08MHz
const uint32_t CH_01_900 = 0xE24F5C; // channel 01, central freq = 905.24MHz
const uint32_t CH_02_900 = 0xE2D999; // channel 02, central freq = 907.40MHz
const uint32_t CH_03_900 = 0xE363D7; // channel 03, central freq = 909.56MHz
const uint32_t CH_04_900 = 0xE3EE14; // channel 04, central freq = 911.72MHz
const uint32_t CH_05_900 = 0xE47851; // channel 05, central freq = 913.88MHz
const uint32_t CH_06_900 = 0xE5028F; // channel 06, central freq = 916.04MHz
const uint32_t CH_07_900 = 0xE58CCC; // channel 07, central freq = 918.20MHz
const uint32_t CH_08_900 = 0xE6170A; // channel 08, central freq = 920.36MHz
const uint32_t CH_09_900 = 0xE6A147; // channel 09, central freq = 922.52MHz
const uint32_t CH_10_900 = 0xE72B85; // channel 10, central freq = 924.68MHz
const uint32_t CH_11_900 = 0xE7B5C2; // channel 11, central freq = 926.84MHz
const uint32_t CH_12_900 = 0xE4C000; // default channel 915MHz, the module is configured with it

// added by C. Pham
const uint32_t CH_00_433 = 0x6C5333; // 433.3MHz
const uint32_t CH_01_433 = 0x6C6666; // 433.6MHz
const uint32_t CH_02_433 = 0x6C7999; // 433.9MHz
const uint32_t CH_03_433 = 0x6C9333; // 434.3MHz
// end

//LORA BANDWIDTH:
// modified by C. Pham
const uint8_t SX1272_BW_125 = 0x00;
const uint8_t SX1272_BW_250 = 0x01;
const uint8_t SX1272_BW_500 = 0x02;

// use the following constants with setBW()
const uint8_t BW_7_8 = 0x00;
const uint8_t BW_10_4 = 0x01;
const uint8_t BW_15_6 = 0x02;
const uint8_t BW_20_8 = 0x03;
const uint8_t BW_31_25 = 0x04;
const uint8_t BW_41_7 = 0x05;
const uint8_t BW_62_5 = 0x06;
const uint8_t BW_125 = 0x07;
const uint8_t BW_250 = 0x08;
const uint8_t BW_500 = 0x09;
// end

const double SignalBwLog[] =
{
    5.0969100130080564143587833158265,
    5.397940008672037609572522210551,
    5.6989700043360188047862611052755
};

//LORA CODING RATE:
const uint8_t CR_5 = 0x01;
const uint8_t CR_6 = 0x02;
const uint8_t CR_7 = 0x03;
const uint8_t CR_8 = 0x04;

//LORA SPREADING FACTOR:
const uint8_t SF_6 = 0x06;
const uint8_t SF_7 = 0x07;
const uint8_t SF_8 = 0x08;
const uint8_t SF_9 = 0x09;
const uint8_t SF_10 = 0x0A;
const uint8_t SF_11 = 0x0B;
const uint8_t SF_12 = 0x0C;

//LORA MODES:
const uint8_t LORA_SLEEP_MODE = 0x80;
const uint8_t LORA_STANDBY_MODE = 0x81;
const uint8_t LORA_TX_MODE = 0x83;
const uint8_t LORA_RX_MODE = 0x85;

// added by C. Pham
const uint8_t LORA_CAD_MODE = 0x87;
#define LNA_MAX_GAIN                0x23
#define LNA_OFF_GAIN                0x00
#define LNA_LOW_GAIN		    	0x20
//end

const uint8_t LORA_STANDBY_FSK_REGS_MODE = 0xC1;

//FSK MODES:
const uint8_t FSK_SLEEP_MODE = 0x00;
const uint8_t FSK_STANDBY_MODE = 0x01;
const uint8_t FSK_TX_MODE = 0x03;
const uint8_t FSK_RX_MODE = 0x05;

//OTHER CONSTANTS:

const uint8_t HEADER_ON = 0;
const uint8_t HEADER_OFF = 1;
const uint8_t CRC_ON = 1;
const uint8_t CRC_OFF = 0;
const uint8_t LORA = 1;
const uint8_t FSK = 0;
const uint8_t BROADCAST_0 = 0x00;
const uint8_t MAX_LENGTH = 255;
const uint8_t MAX_PAYLOAD = 251;
const uint8_t MAX_LENGTH_FSK = 64;
const uint8_t MAX_PAYLOAD_FSK = 60;
//modified by C. Pham, 7 instead of 5 because we added a type field which should be PKT_TYPE_ACK and the SNR
const uint8_t ACK_LENGTH = 7;
// added by C. Pham
#ifdef W_NET_KEY
const uint8_t NET_KEY_LENGTH=2;
const uint8_t OFFSET_PAYLOADLENGTH = 4+NET_KEY_LENGTH;
const uint8_t net_key_0 = 0x12;
const uint8_t net_key_1 = 0x34;
#else
// modified by C. Pham to remove the retry field
const uint8_t OFFSET_PAYLOADLENGTH = 4;
#endif
const uint8_t OFFSET_RSSI = 139;
const uint8_t NOISE_FIGURE = 6.0;
const uint8_t NOISE_ABSOLUTE_ZERO = 174.0;
const uint16_t MAX_TIMEOUT = 8000;		//8000 msec = 8.0 sec
const uint16_t MAX_WAIT = 12000;		//12000 msec = 12.0 sec
const uint8_t MAX_RETRIES = 5;
const uint8_t CORRECT_PACKET = 0;
const uint8_t INCORRECT_PACKET = 1;

// added by C. Pham
// Packet type definition

#define PKT_TYPE_MASK   0xF0
#define PKT_FLAG_MASK   0x0F

#define PKT_TYPE_DATA   0x10
#define PKT_TYPE_ACK    0x20

#define PKT_FLAG_ACK_REQ            0x08
#define PKT_FLAG_DATA_ENCRYPTED     0x04
#define PKT_FLAG_DATA_WAPPKEY       0x02
#define PKT_FLAG_DATA_ISBINARY      0x01

#define SX1272_ERROR_ACK        3
#define SX1272_ERROR_TOA        4

//! Structure :
/*!
 */
struct pack
{
        // added by C. Pham
#ifdef W_NET_KEY
        uint8_t netkey[NET_KEY_LENGTH];
#endif
	//! Structure Variable : Packet destination
	/*!
 	*/
	uint8_t dst;

    // added by C. Pham
    //! Structure Variable : Packet type
    /*!
    */
    uint8_t type;

	//! Structure Variable : Packet source
	/*!
 	*/
	uint8_t src;

	//! Structure Variable : Packet number
	/*!
 	*/
	uint8_t packnum;

    // modified by C. Pham
    // will not be used in the transmitted packet
	//! Structure Variable : Packet length
	/*!
 	*/
	uint8_t length;

	//! Structure Variable : Packet payload
	/*!
 	*/
	uint8_t data[MAX_PAYLOAD];

    // modified by C. Pham
    // will not be used in the transmitted packet
	//! Structure Variable : Retry number
	/*!
 	*/
	uint8_t retry;
};

/******************************************************************************
 * Class
 ******************************************************************************/

//! SX1272 Class
/*!
	SX1272 Class defines all the variables and functions used to manage
	SX1272 modules.
 */
class SX1272
{

public:

	SX1272();

	//! It puts the module ON
  	/*!
	\param void
	\return uint8_t setLORA state
	 */
	uint8_t ON();

	//! It puts the module OFF
  	/*!
	\param void
	\return void
	 */
	void OFF();

	//! It reads an internal module register.
  	/*!
  	\param byte address : address register to read from.
	\return the content of the register.
	 */
	byte readRegister(byte address);

	//! It writes in an internal module register.
  	/*!
  	\param byte address : address register to write in.
  	\param byte data : value to write in the register.
	 */
	void writeRegister(byte address, byte data);

	//! It clears the interruption flags.
  	/*!
	\param void
	\return void
	 */
	void clearFlags();

	//! It sets the LoRa mode on.
  	/*!
  	It stores in global '_LORA' variable '1' when success
    \return '0' on success, '1' otherwise
	 */
	uint8_t setLORA();

	//! It sets the FSK mode on.
  	/*!
  	It stores in global '_FSK' variable '1' when success
	\return '0' on success, '1' otherwise
	 */
	uint8_t setFSK();

	//! It gets the BW, SF and CR of the module.
  	/*!
	It stores in global '_bandwidth' variable the BW
	It stores in global '_codingRate' variable the CR
	It stores in global '_spreadingFactor' variable the SF
	\return '0' on success, '1' otherwise
	 */
	uint8_t getMode();

	//! It sets the BW, SF and CR of the module.
  	/*!
	It stores in global '_bandwidth' variable the BW
	It stores in global '_codingRate' variable the CR
	It stores in global '_spreadingFactor' variable the SF
	\param uint8_t mode : there is a mode number to different values of
	the	configured parameters with this function.
	\return '0' on success, '1' otherwise
	 */
	int8_t setMode(uint8_t mode);

	//! It gets the header mode configured.
  	/*!
  	It stores in global '_header' variable '0' when header is sent
  	(explicit header mode) or '1' when is not sent (implicit header
  	mode).
	\return '0' on success, '1' otherwise
	 */
	uint8_t	getHeader();

	//! It sets explicit header mode.
  	/*!
  	It stores in global '_header' variable '1' when success
	\return '0' on success, '1' otherwise
	 */
	int8_t	setHeaderON();

	//! It sets implicit header mode.
  	/*!
  	It stores in global '_header' variable '0' when success
	\return '0' on success, '1' otherwise
	 */
	int8_t	setHeaderOFF();

	//! It gets the CRC configured.
  	/*!
  	It stores in global '_CRC' variable '1' enabling CRC generation on
  	payload, or '0' disabling the CRC.
	\return '0' on success, '1' otherwise
	 */
	uint8_t	getCRC();

	//! It sets CRC on.
  	/*!
  	It stores in global '_CRC' variable '1' when success
	\return '0' on success, '1' otherwise
	 */
	uint8_t	setCRC_ON();

	//! It sets CRC off.
  	/*!
  	It stores in global '_CRC' variable '0' when success
	\return '0' on success, '1' otherwise
	 */
	uint8_t	setCRC_OFF();

	//! It is true if the SF selected exists.
  	/*!
	\param uint8_t spr : spreading factor value to check.
	\return 'true' on success, 'false' otherwise
	 */
	boolean	isSF(uint8_t spr);

	//! It gets the SF configured.
  	/*!
	It stores in global '_spreadingFactor' variable the current value of SF
	\return '0' on success, '1' otherwise
	 */
	int8_t	getSF();

	//! It sets the SF.
  	/*!
	It stores in global '_spreadingFactor' variable the current value of SF
	\param uint8_t spr : spreading factor value to set in the configuration.
	\return '0' on success, '1' otherwise
	 */
	uint8_t	setSF(uint8_t spr);

	//! It is true if the BW selected exists.
  	/*!
	\param uint16_t band : bandwidth value to check.
	\return 'true' on success, 'false' otherwise
	 */
	boolean	isBW(uint16_t band);

	//! It gets the BW configured.
  	/*!
	It stores in global '_bandwidth' variable the BW selected
	in the configuration
	\return '0' on success, '1' otherwise
	 */
	int8_t	getBW();

	//! It sets the BW.
  	/*!
	It stores in global '_bandwidth' variable the BW selected
	in the configuration
	\param uint16_t band : bandwidth value to set in the configuration.
	\return '0' on success, '1' otherwise
	 */
	int8_t setBW(uint16_t band);

	//! It is true if the CR selected exists.
  	/*!
	\param uint8_t cod : the coding rate value to check.
	\return 'true' on success, 'false' otherwise
	 */
	boolean	isCR(uint8_t cod);

	//! It gets the CR configured.
  	/*!
	It stores in global '_codingRate' variable the CR selected
	in the configuration
	\return '0' on success, '1' otherwise
	 */
	int8_t	getCR();

	//! It sets the CR.
  	/*!
	It stores in global '_codingRate' variable the CR selected
	in the configuration
	\param uint8_t cod : coding rate value to set in the configuration.
	\return '0' on success, '1' otherwise
	 */
	int8_t	setCR(uint8_t cod);


	//! It is true if the channel selected exists.
  	/*!
	\param uint32_t ch : frequency channel value to check.
	\return 'true' on success, 'false' otherwise
	 */
	boolean isChannel(uint32_t ch);

	//! It gets frequency channel the module is using.
  	/*!
	It stores in global '_channel' variable the frequency channel
	\return '0' on success, '1' otherwise
	 */
	uint8_t getChannel();

	//! It sets frequency channel the module is using.
  	/*!
	It stores in global '_channel' variable the frequency channel
	\param uint32_t ch : frequency channel value to set in the configuration.
	\return '0' on success, '1' otherwise
	 */
	int8_t setChannel(uint32_t ch);

	//! It gets the output power of the signal.
  	/*!
	It stores in global '_power' variable the output power of the signal
	\return '0' on success, '1' otherwise
	 */
	uint8_t getPower();

	//! It sets the output power of the signal.
  	/*!
	It stores in global '_power' variable the output power of the signal
	\param char p : 'M', 'H' or 'L' if you want Maximum, High or Low
	output power signal.
	\return '0' on success, '1' otherwise
	 */
	int8_t setPower(char p);

	//! It sets the output power of the signal.
  	/*!
	It stores in global '_power' variable the output power of the signal
	\param uint8_t pow : value to set as output power.
	\return '0' on success, '1' otherwise
	 */
	int8_t setPowerNum(uint8_t pow);

	//! It gets the preamble length configured.
  	/*!
	It stores in global '_preamblelength' variable the preamble length
	\return '0' on success, '1' otherwise
	 */
	uint8_t getPreambleLength();

	//! It sets the preamble length.
  	/*!
    It stores in global '_preamblelength' variable the preamble length
  	\param uint16_t l : preamble length to set in the configuration.
	\return '0' on success, '1' otherwise
	 */
	uint8_t setPreambleLength(uint16_t l);

	//! It gets the payload length of the last packet to send/receive.
  	/*!
    It stores in global '_payloadlength' variable the payload length of
    the last packet to send/receive.
	\return '0' on success, '1' otherwise
	 */
	uint8_t getPayloadLength();

	//! It sets the packet length to send/receive.
  	/*!
  	It stores in global '_payloadlength' variable the payload length of
    the last packet to send/receive.
	\return '0' on success, '1' otherwise
	 */
	int8_t setPacketLength();

	//! It sets the packet length to send/receive.
  	/*!
  	It stores in global '_payloadlength' variable the payload length of
    the last packet to send/receive.
  	\param uint8_t l : payload length to set in the configuration.
	\return '0' on success, '1' otherwise
	 */
	int8_t setPacketLength(uint8_t l);

	//! It gets the node address of the mote.
  	/*!
  	It stores in global '_nodeAddress' variable the node address
	\return '0' on success, '1' otherwise
	 */
	uint8_t getNodeAddress();

	//! It sets the node address of the mote.
  	/*!
  	It stores in global '_nodeAddress' variable the node address
  	\param uint8_t addr : address value to set as node address.
	\return '0' on success, '1' otherwise
	 */
	int8_t setNodeAddress(uint8_t addr);

	//! It gets the SNR of the latest received packet.
  	/*!
	It stores in global '_SNR' variable the SNR
	\return '0' on success, '1' otherwise
	 */
	int8_t getSNR();

	//! It gets the current value of RSSI.
  	/*!
	It stores in global '_RSSI' variable the current value of RSSI
	\return '0' on success, '1' otherwise
	 */
	uint8_t getRSSI();

	//! It gets the RSSI of the latest received packet.
  	/*!
	It stores in global '_RSSIpacket' variable the RSSI of the latest
	packet received.
	\return '0' on success, '1' otherwise
	 */
	int16_t getRSSIpacket();

	//! It sets the total of retries when a packet is not correctly received.
	/*!
	It stores in global '_maxRetries' variable the number of retries.
  	\param uint8_t ret : number of retries.
	\return '0' on success, '1' otherwise
	 */
	uint8_t setRetries(uint8_t ret);

	//! It gets the maximum current supply by the module.
	/*!
  	 *
	\return '0' on success, '1' otherwise
	 */
	uint8_t getMaxCurrent();

	//! It sets the maximum current supply by the module.
	/*!
	It stores in global '_maxCurrent' variable the maximum current supply.
  	\param uint8_t rate : maximum current supply.
	\return '0' on success, '1' otherwise
	 */
	int8_t setMaxCurrent(uint8_t rate);

	//! It gets the content of the main configuration registers.
  	/*!
	It stores in global '_bandwidth' variable the BW.
	It stores in global '_codingRate' variable the CR.
	It stores in global '_spreadingFactor' variable the SF.
	It stores in global '_power' variable the output power of the signal.
  	It stores in global '_channel' variable the frequency channel.
  	It stores in global '_CRC' variable '1' enabling CRC generation on
  	payload, or '0' disabling the CRC.
  	It stores in global '_header' variable '0' when header is sent
  	(explicit header mode) or '1' when is not sent (implicit header
  	mode).
	It stores in global '_preamblelength' variable the preamble length.
    It stores in global '_payloadlength' variable the payload length of
    the last packet to send/receive.
  	It stores in global '_nodeAddress' variable the node address.
	It stores in global '_temp' variable the module temperature.
	\return '0' on success, '1' otherwise
	 */
	uint8_t getRegs();

	//! It sets the maximum number of bytes from a frame that fit in a packet structure.
	/*!
	It stores in global '_payloadlength' variable the maximum number of bytes.
  	\param uint16_t length16 : total frame length.
	\return '0' on success, '1' otherwise
	 */
	uint8_t truncPayload(uint16_t length16);

	//! It writes an ACK in FIFO to send it.
	/*!
	 *
	\return '0' on success, '1' otherwise
	*/
	uint8_t setACK();

	//! It puts the module in reception mode.
  	/*!
  	 *
	\return '0' on success, '1' otherwise
	 */
	uint8_t receive();

	//! It receives a packet before MAX_TIMEOUT.
  	/*!
  	 *
	\return '0' on success, '1' otherwise
	 */
	uint8_t receivePacketMAXTimeout();

	//! It receives a packet before a timeout.
  	/*!
  	 *
	\return '0' on success, '1' otherwise
	 */
	uint8_t receivePacketTimeout();

	//! It receives a packet before a timeout.
  	/*!
  	\param uint16_t wait : time to wait to receive something.
	\return '0' on success, '1' otherwise
	 */
	uint8_t receivePacketTimeout(uint16_t wait);

	//! It receives a packet before MAX_TIMEOUT and reply with an ACK.
  	/*!
  	 *
	\return '0' on success, '1' otherwise
	 */
	uint8_t receivePacketMAXTimeoutACK();

	//! It receives a packet before a timeout and reply with an ACK.
  	/*!
  	 *
	\return '0' on success, '1' otherwise
	 */
	uint8_t receivePacketTimeoutACK();

	//! It receives a packet before a timeout and reply with an ACK.
  	/*!
  	\param uint16_t wait : time to wait to receive something.
	\return '0' on success, '1' otherwise
	 */
	uint8_t receivePacketTimeoutACK(uint16_t wait);

	//! It puts the module in 'promiscuous' reception mode.
  	/*!
  	 *
	\return '0' on success, '1' otherwise
	 */
	uint8_t receiveAll();

	//! It puts the module in 'promiscuous' reception mode with a timeout.
  	/*!
  	\param uint16_t wait : time to wait to receive something.
	\return '0' on success, '1' otherwise
	 */
	uint8_t receiveAll(uint16_t wait);

	//! It checks if there is an available packet and its destination.
  	/*!
  	 *
	\return 'true' on success, 'false' otherwise
	 */
	boolean	availableData();

	//! It checks if there is an available packet and its destination before a timeout.
  	/*!
  	 *
  	\param uint16_t wait : time to wait while there is no a valid header received.
	\return 'true' on success, 'false' otherwise
	 */
	boolean	availableData(uint16_t wait);

	//! It writes a packet in FIFO in order to send it.
	/*!
	\param uint8_t dest : packet destination.
	\param char *payload : packet payload.
	\return '0' on success, '1' otherwise
	*/
	uint8_t setPacket(uint8_t dest, char *payload);

	//! It writes a packet in FIFO in order to send it.
	/*!
	\param uint8_t dest : packet destination.
	\param uint8_t *payload: packet payload.
	\return '0' on success, '1' otherwise
	*/
	uint8_t setPacket(uint8_t dest, uint8_t *payload);

	//! It reads a received packet from the FIFO, if it arrives before ending MAX_TIMEOUT time.
	/*!
	 *
	\return '0' on success, '1' otherwise
	*/
	uint8_t getPacketMAXTimeout();

	//! It reads a received packet from the FIFO, if it arrives before ending '_sendTime' time.
	/*!
	 *
	\return '0' on success, '1' otherwise
	*/
	int8_t getPacket();

	//! It receives and gets a packet from FIFO, if it arrives before ending 'wait' time.
	/*!
	 *
	\param uint16_t wait : time to wait while there is no a complete packet received.
	\return '0' on success, '1' otherwise
	*/
	int8_t getPacket(uint16_t wait);

	//! It sends the packet stored in FIFO before ending MAX_TIMEOUT.
	/*!
	 *
	\return '0' on success, '1' otherwise
	*/
	uint8_t sendWithMAXTimeout();

	//! It sends the packet stored in FIFO before ending _sendTime time.
	/*!
	 *
	\return '0' on success, '1' otherwise
	*/
	uint8_t sendWithTimeout();

	//! It tries to send the packet stored in FIFO before ending 'wait' time.
	/*!
	\param uint16_t wait : time to wait to send the packet.
	\return '0' on success, '1' otherwise
	*/
	uint8_t sendWithTimeout(uint16_t wait);

	//! It tries to send the packet wich payload is a parameter before ending MAX_TIMEOUT.
	/*!
	\param uint8_t dest : packet destination.
	\param char *payload : packet payload.
	\return '0' on success, '1' otherwise
	*/
	uint8_t sendPacketMAXTimeout(uint8_t dest, char *payload);

	//! It tries to send the packet wich payload is a parameter before ending MAX_TIMEOUT.
	/*!
	\param uint8_t dest : packet destination.
	\param uint8_t *payload : packet payload.
	\param uint16_t length : payload buffer length.
	\return '0' on success, '1' otherwise
	*/
	uint8_t sendPacketMAXTimeout(uint8_t dest, uint8_t *payload, uint16_t length);


	//! It sends the packet wich payload is a parameter before ending MAX_TIMEOUT.
	/*!
	\param uint8_t dest : packet destination.
	\param char *payload : packet payload.
	\return '0' on success, '1' otherwise
	*/
	uint8_t sendPacketTimeout(uint8_t dest, char *payload);

	//! It sends the packet wich payload is a parameter before ending MAX_TIMEOUT.
	/*!
	\param uint8_t dest : packet destination.
	\param uint8_t *payload: packet payload.
	\param uint16_t length : payload buffer length.
	\return '0' on success, '1' otherwise
	*/
	uint8_t sendPacketTimeout(uint8_t dest, uint8_t *payload, uint16_t length);

	//! It sends the packet wich payload is a parameter before ending 'wait' time.
	/*!
	\param uint8_t dest : packet destination.
	\param char *payload : packet payload.
	\param uint16_t wait : time to wait.
	\return '0' on success, '1' otherwise
	*/
	uint8_t sendPacketTimeout(uint8_t dest, char *payload, uint16_t wait);

	//! It sends the packet wich payload is a parameter before ending 'wait' time.
	/*!
	\param uint8_t dest : packet destination.
	\param uint8_t *payload : packet payload.
	\param uint16_t length : payload buffer length.
	\param uint16_t wait : time to wait.
	\return '0' on success, '1' otherwise
	*/
	uint8_t sendPacketTimeout(uint8_t dest, uint8_t *payload, uint16_t length, uint16_t wait);

	//! It sends the packet wich payload is a parameter before MAX_TIMEOUT, and replies with ACK.
	/*!
	\param uint8_t dest : packet destination.
	\param char *payload : packet payload.
	\return '0' on success, '1' otherwise
	*/
	uint8_t sendPacketMAXTimeoutACK(uint8_t dest, char *payload);

	//! It sends the packet wich payload is a parameter before MAX_TIMEOUT, and replies with ACK.
	/*!
	\param uint8_t dest : packet destination.
	\param uint8_t payload: packet payload.
	\param uint16_t length : payload buffer length.
	\return '0' on success, '1' otherwise
	*/
	uint8_t sendPacketMAXTimeoutACK(uint8_t dest, uint8_t *payload, uint16_t length);

	//! It sends the packet wich payload is a parameter before a timeout, and replies with ACK.
	/*!
	\param uint8_t dest : packet destination.
	\param char *payload : packet payload.
	\return '0' on success, '1' otherwise
	*/
	uint8_t sendPacketTimeoutACK(uint8_t dest, char *payload);

	//! It sends the packet wich payload is a parameter before a timeout, and replies with ACK.
	/*!
	\param uint8_t dest : packet destination.
	\param uint8_t payload: packet payload.
	\param uint16_t length : payload buffer length.
	\return '0' on success, '1' otherwise
	*/
	uint8_t sendPacketTimeoutACK(uint8_t dest, uint8_t *payload, uint16_t length);

	//! It sends the packet wich payload is a parameter before 'wait' time, and replies with ACK.
	/*!
	\param uint8_t dest : packet destination.
	\param char *payload : packet payload.
	\param uint16_t wait : time to wait to send the packet.
	\return '0' on success, '1' otherwise
	*/
	uint8_t sendPacketTimeoutACK(uint8_t dest, char *payload, uint16_t wait);

	//! It sends the packet wich payload is a parameter before 'wait' time, and replies with ACK.
	/*!
	\param uint8_t dest : packet destination.
	\param uint8_t payload: packet payload.
	\param uint16_t length : payload buffer length.
	\param uint16_t wait : time to wait to send the packet.
	\return '0' on success, '1' otherwise
	*/
	uint8_t sendPacketTimeoutACK(uint8_t dest, uint8_t *payload, uint16_t length, uint16_t wait);

	//! It sets the destination of a packet.
  	/*!
  	\param uint8_t dest : value to set as destination address.
	\return '0' on success, '1' otherwise
	 */
	int8_t setDestination(uint8_t dest);

	//! It sets the waiting time to send a packet.
  	/*!
   	It stores in global '_sendTime' variable the time for each mode.
	\return '0' on success, '1' otherwise
	 */
	uint8_t setTimeout();

	//! It sets the payload of the packet that is going to be sent.
  	/*!
  	\param char *payload : packet payload.
	\return '0' on success, '1' otherwise
	 */
	uint8_t setPayload(char *payload);

	//! It sets the payload of the packet that is going to be sent.
  	/*!
  	\param uint8_t payload: packet payload.
	\return '0' on success, '1' otherwise
	 */
	uint8_t setPayload(uint8_t *payload);

	//! If an ACK is received, it gets it and checks its content.
	/*!
	 *
	\return '0' on success, '1' otherwise
	*/
	uint8_t getACK();

 	//! It receives and gets an ACK from FIFO, if it arrives before ending 'wait' time.
	/*!
	 *
	\param uint16_t wait : time to wait while there is no an ACK received.
	\return '0' on success, '1' otherwise
	*/
	uint8_t getACK(uint16_t wait);

	//! It sends a packet, waits to receive an ACK and updates the _retries value, before ending MAX_TIMEOUT time.
	/*!
	\param uint8_t dest : packet destination.
	\param char *payload : packet payload.
	\return '0' on success, '1' otherwise
	*/
	uint8_t sendPacketMAXTimeoutACKRetries(uint8_t dest, char *payload);

	//! It sends a packet, waits to receive an ACK and updates the _retries value, before ending MAX_TIMEOUT time.
	/*!
	\param uint8_t dest : packet destination.
	\param uint8_t *payload : packet payload.
	\param uint16_t length : payload buffer length.
	\return '0' on success, '1' otherwise
	*/
	uint8_t sendPacketMAXTimeoutACKRetries(uint8_t dest, uint8_t *payload, uint16_t length);

	//! It sends a packet, waits to receive an ACK and updates the _retries value.
	/*!
	\param uint8_t dest : packet destination.
	\param char *payload : packet payload.
	\return '0' on success, '1' otherwise
	*/
	uint8_t sendPacketTimeoutACKRetries(uint8_t dest, char *payload);

	//! It sends a packet, waits to receive an ACK and updates the _retries value.
	/*!
	\param uint8_t dest : packet destination.
	\param uint8_t *payload : packet payload.
	\param uint16_t length : payload buffer length.
	\return '0' on success, '1' otherwise
	*/
	uint8_t sendPacketTimeoutACKRetries(uint8_t dest, uint8_t *payload, uint16_t length);

	//! It sends a packet, waits to receive an ACK and updates the _retries value, before ending 'wait' time.
	/*!
	\param uint8_t dest : packet destination.
	\param char *payload : packet payload.
	\param uint16_t wait : time to wait while trying to send the packet.
	\return '0' on success, '1' otherwise
	*/
	uint8_t sendPacketTimeoutACKRetries(uint8_t dest, char *payload, uint16_t wait);

	//! It sends a packet, waits to receive an ACK and updates the _retries value, before ending 'wait' time.
	/*!
	\param uint8_t dest : packet destination.
	\param uint8_t *payload : packet payload.
	\param uint16_t length : payload buffer length.
	\param uint16_t wait : time to wait while trying to send the packet.
	\return '0' on success, '1' otherwise
	*/
	uint8_t sendPacketTimeoutACKRetries(uint8_t dest, uint8_t *payload, uint16_t length, uint16_t wait);

	//! It gets the internal temperature of the module.
	/*!
	It stores in global '_temp' variable the module temperature.
	\return '0' on success, '1' otherwise
	*/
	uint8_t getTemp();

        // added by C. Pham
    void setPacketType(uint8_t type);
    void RxChainCalibration();
    uint8_t doCAD(uint8_t counter);
    uint16_t getToA(uint8_t pl);
    void CarrierSense();
    int8_t setSyncWord(uint8_t sw);
    int8_t getSyncWord();
    int8_t setSleepMode();
    int8_t setPowerDBM(uint8_t dbm);
    long limitToA();
    long getRemainingToA();
    long removeToA(uint16_t toa);

    // SX1272 or SX1276?
    uint8_t _board;
    uint8_t _syncWord;
    uint8_t _defaultSyncWord;
    unsigned long _starttime;
    unsigned long _stoptime;
    unsigned long _startDoCad;
    unsigned long _endDoCad;
    uint8_t _loraMode;
    uint8_t _send_cad_number;
    bool _extendedIFS;
    bool _RSSIonSend;
    bool _enableCarrierSense;
    bool _rawFormat;
    int8_t _rcv_snr_in_ack;
    bool _needPABOOST;
    uint8_t _rawSNR;

#ifdef W_REQUESTED_ACK
    uint8_t _requestACK;
    uint8_t _requestACK_indicator;
#endif

#ifdef W_NET_KEY
    uint8_t _my_netkey[NET_KEY_LENGTH];
    uint8_t _the_net_key_0;
    uint8_t _the_net_key_1;
#endif
    //end

	/// Variables /////////////////////////////////////////////////////////////

	//! Variable : bandwidth configured in LoRa mode.
	//!    bandwidth = 00  --> BW = 125KHz
	//!    bandwidth = 01  --> BW = 250KHz
	//!    bandwidth = 10  --> BW = 500KHz
  	/*!
   	*/
	uint8_t _bandwidth;

	//! Variable : coding rate configured in LoRa mode.
	//!    codingRate = 001  --> CR = 4/5
	//!    codingRate = 010  --> CR = 4/6
	//!    codingRate = 011  --> CR = 4/7
	//!    codingRate = 100  --> CR = 4/8
  	/*!
   	*/
	uint8_t _codingRate;

	//! Variable : spreading factor configured in LoRa mode.
	//!    spreadingFactor = 6   --> SF = 6, 64 chips/symbol
	//!    spreadingFactor = 7   --> SF = 7, 128 chips/symbol
	//!    spreadingFactor = 8   --> SF = 8, 256 chips/symbol
	//!    spreadingFactor = 9   --> SF = 9, 512 chips/symbol
	//!    spreadingFactor = 10  --> SF = 10, 1024 chips/symbol
	//!    spreadingFactor = 11  --> SF = 11, 2048 chips/symbol
	//!    spreadingFactor = 12  --> SF = 12, 4096 chips/symbol
  	/*!
   	*/
	uint8_t _spreadingFactor;

	//! Variable : frequency channel.
	//!    channel = 0xD84CCC  --> CH = 10_868, 865.20MHz
	//!    channel = 0xD86000  --> CH = 11_868, 865.50MHz
	//!    channel = 0xD87333  --> CH = 12_868, 865.80MHz
	//!    channel = 0xD88666  --> CH = 13_868, 866.10MHz
	//!    channel = 0xD89999  --> CH = 14_868, 866.40MHz
	//!    channel = 0xD8ACCC  --> CH = 15_868, 866.70MHz
	//!    channel = 0xD8C000  --> CH = 16_868, 867.00MHz
	//!    channel = 0xE1C51E  --> CH = 00_900, 903.08MHz
	//!    channel = 0xE24F5C  --> CH = 01_900, 905.24MHz
	//!    channel = 0xE2D999  --> CH = 02_900, 907.40MHz
	//!    channel = 0xE363D7  --> CH = 03_900, 909.56MHz
	//!    channel = 0xE3EE14  --> CH = 04_900, 911.72MHz
	//!    channel = 0xE47851  --> CH = 05_900, 913.88MHz
	//!    channel = 0xE5028F  --> CH = 06_900, 916.04MHz
	//!    channel = 0xE58CCC  --> CH = 07_900, 918.20MHz
	//!    channel = 0xE6170A  --> CH = 08_900, 920.36MHz
	//!    channel = 0xE6A147  --> CH = 09_900, 922.52MHz
	//!    channel = 0xE72B85  --> CH = 10_900, 924.68MHz
	//!    channel = 0xE7B5C2  --> CH = 11_900, 926.84MHz
  	/*!
   	*/
	uint32_t _channel;

	//! Variable : output power.
	//!
  	/*!
   	*/
	uint8_t _power;

	//! Variable : SNR from the last packet received in LoRa mode.
	//!
  	/*!
   	*/
	int8_t _SNR;

	//! Variable : RSSI current value.
	//!
  	/*!
   	*/
	int8_t _RSSI;

	//! Variable : RSSI from the last packet received in LoRa mode.
	//!
  	/*!
   	*/
	int16_t _RSSIpacket;

	//! Variable : preamble length sent/received.
	//!
  	/*!
   	*/
	uint16_t _preamblelength;

	//! Variable : payload length sent/received.
	//!
  	/*!
   	*/
	uint16_t _payloadlength;

	//! Variable : node address.
	//!
  	/*!
   	*/
	uint8_t _nodeAddress;

	//! Variable : implicit or explicit header in LoRa mode.
	//!
  	/*!
   	*/
	uint8_t _header;

	//! Variable : header received while waiting a packet to arrive.
	//!
  	/*!
   	*/
	uint8_t _hreceived;

	//! Variable : presence or absence of CRC calculation.
	//!
  	/*!
   	*/
	uint8_t _CRC;

	//! Variable : packet destination.
	//!
  	/*!
   	*/
	uint8_t _destination;

	//! Variable : packet number.
	//!
  	/*!
   	*/
	uint8_t _packetNumber;

	//! Variable : indicates if received packet is correct or incorrect.
	//!
  	/*!
   	*/
   	uint8_t _reception;

	//! Variable : number of current retry.
	//!
  	/*!
   	*/
   	uint8_t _retries;

   	//! Variable : maximum number of retries.
	//!
  	/*!
   	*/
   	uint8_t _maxRetries;

   	//! Variable : maximum current supply.
	//!
  	/*!
   	*/
   	uint8_t _maxCurrent;

	//! Variable : indicates FSK or LoRa 'modem'.
	//!
  	/*!
   	*/
	uint8_t _modem;

	//! Variable : array with all the information about a sent packet.
	//!
  	/*!
   	*/
	pack packet_sent;

	//! Variable : array with all the information about a received packet.
	//!
  	/*!
   	*/
	pack packet_received;

	//! Variable : array with all the information about a sent/received ack.
	//!
  	/*!
   	*/
	pack ACK;

	//! Variable : temperature module.
	//!
  	/*!
   	*/
	int _temp;

	//! Variable : current timeout to send a packet.
	//!
  	/*!
   	*/
	uint16_t _sendTime;

private:

	void maxWrite16();

	char txbuf[2];
	char rxbuf[2];

    // added by C. Pham for ToA management
    //
    bool _limitToA;
    long _remainingToA;
    unsigned long _startToAcycle;
    unsigned long _endToAcycle;
    uint16_t _currentToA;
};

extern SX1272	sx1272;

#endif
