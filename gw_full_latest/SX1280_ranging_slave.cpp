/* 
 *  LoRa ranging slave to test ranging with SX1280 using SX127XLT lib on Arduino/RPI
 *
 *	Author: Congduc Pham, 2020
 *
 *  Based on the SX1280 ranging slave example from SX128XLT lib
 */

#ifdef ARDUINO
#include <SPI.h>
#define USE_SPI_TRANSACTION          //this is the standard behaviour of library, use SPI Transaction switching
#else
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef GETOPT_ISSUE
int getopt (int argc, char * const argv[], const char *optstring);
extern char *optarg;
extern int optind, opterr, optopt;
#endif
#include <getopt.h>
#include <termios.h> 
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#endif

#include <SX128XLT.h>
SX128XLT LT;                                         

#ifdef ARDUINO
#include "Arduino_LoRa_SX1280_ranging_slave.h"

///////////////////////////////////////////////////////////////////
//new small OLED screen, mostly based on SSD1306 
//#define OLED
//#define OLED_9GND876
#define OLED_6GND789

#ifdef OLED
#include <U8x8lib.h>
//you can also power the OLED screen with a digital pin, here pin 8
#define OLED_PWR_PIN 8
// connection may depend on the board. Use A5/A4 for most Arduino boards. On ESP8266-based board we use GPI05 and GPI04. Heltec ESP32 has embedded OLED.
#if defined ARDUINO_Heltec_WIFI_LoRa_32 || defined ARDUINO_WIFI_LoRa_32 || defined HELTEC_LORA
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);
#elif defined ESP8266 || defined ARDUINO_ESP8266_ESP01
//U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 5, /* data=*/ 4, /* reset=*/ U8X8_PIN_NONE);
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 12, /* data=*/ 14, /* reset=*/ U8X8_PIN_NONE);
#else
#ifdef OLED_GND234
  #ifdef OLED_PWR_PIN
    #undef OLED_PWR_PIN
    #define OLED_PWR_PIN 2
  #endif
  U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 3, /* data=*/ 4, /* reset=*/ U8X8_PIN_NONE);
#elif defined OLED_9GND876
  #ifdef OLED_PWR_PIN
    #undef OLED_PWR_PIN
    #define OLED_PWR_PIN 8
  #endif  
  U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 7, /* data=*/ 6, /* reset=*/ U8X8_PIN_NONE);
#elif defined OLED_6GND789
  #ifdef OLED_PWR_PIN
    #undef OLED_PWR_PIN
    #define OLED_PWR_PIN 7
  #endif  
  U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 8, /* data=*/ 9, /* reset=*/ U8X8_PIN_NONE);  
#elif defined OLED_7GND654
  #ifdef OLED_PWR_PIN
    #undef OLED_PWR_PIN
    #define OLED_PWR_PIN 6
  #endif  
  U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 5, /* data=*/ 4, /* reset=*/ U8X8_PIN_NONE);  
#elif defined OLED_GND13_12_11
  #ifdef OLED_PWR_PIN
    #undef OLED_PWR_PIN
    #define OLED_PWR_PIN 13
  #endif  
  U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 12, /* data=*/ 11, /* reset=*/ U8X8_PIN_NONE); 
#else
  U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ A5, /* data=*/ A4, /* reset=*/ U8X8_PIN_NONE);
#endif
#endif
char oled_msg[20];
#endif

#else
#include "SX1280_ranging_slave.h"
#endif

#ifdef ARDUINO
	#define PRINTLN                   Serial.println("")
	#define PRINT_CSTSTR(param)   		Serial.print(F(param))
	#define PRINTLN_CSTSTR(param)			Serial.println(F(param))
	#define PRINT_STR(fmt,param)      Serial.print(param)
	#define PRINTLN_STR(fmt,param)		Serial.println(param)
	#define PRINT_VALUE(fmt,param)    Serial.print(param)
	#define PRINTLN_VALUE(fmt,param)	Serial.println(param)
	#define PRINT_HEX(fmt,param)      Serial.print(param,HEX)
	#define PRINTLN_HEX(fmt,param)		Serial.println(param,HEX)
	#define FLUSHOUTPUT               Serial.flush()
#else
	#define PRINTLN                   printf("\n")
	#define PRINT_CSTSTR(param)       printf(param)
	#define PRINTLN_CSTSTR(param)			do {printf(param);printf("\n");} while(0)
	#define PRINT_STR(fmt,param)      printf(fmt,param)
	#define PRINTLN_STR(fmt,param)		{printf(fmt,param);printf("\n");}
	#define PRINT_VALUE(fmt,param)    printf(fmt,param)
	#define PRINTLN_VALUE(fmt,param)	do {printf(fmt,param);printf("\n");} while(0)
	#define PRINT_HEX(fmt,param)      printf(fmt,param)
	#define PRINTLN_HEX(fmt,param)		do {printf(fmt,param);printf("\n");} while(0)
	#define FLUSHOUTPUT               fflush(stdout)
#endif

uint16_t optBW=0; 
uint8_t optSF=0;
double optFQ=-1.0;

uint32_t endwaitmS;
uint16_t IrqStatus;
uint32_t response_sent;

void setup() {

  delay(1000); 
  
#ifdef ARDUINO  
  Serial.begin(38400);
#endif  

#ifdef OLED
#ifdef OLED_PWR_PIN
  pinMode(OLED_PWR_PIN, OUTPUT);
  digitalWrite(OLED_PWR_PIN, HIGH);
#ifdef OLED_9GND876
  //use pin 9 as ground
  pinMode(9, OUTPUT);
  digitalWrite(9, LOW);
#elif defined OLED_6GND789
  //use pin 6 as ground
  pinMode(6, OUTPUT);
  digitalWrite(6, LOW);  
#elif defined OLED_7GND654
  //use pin 7 as ground
  pinMode(7, OUTPUT);
  digitalWrite(7, LOW);
#endif
#endif
#endif

  SPI.begin();

#ifndef ARDUINO  
  //Set Most significant bit first
  SPI.setBitOrder(MSBFIRST);
  //Divide the clock frequency
  SPI.setClockDivider(SPI_CLOCK_DIV64);
  //Set data mode
  SPI.setDataMode(SPI_MODE0); 
#endif

  //setup hardware pins used by device, then check if device is found
  if (LT.begin(NSS, NRESET, RFBUSY, DIO1, LORA_DEVICE))
  {
    PRINTLN_CSTSTR("Lora Device found");
    delay(1000);
  }
  else
  {
    PRINTLN_CSTSTR("No device responding");
    while (1)
    {
    }
  }

  // has customized LoRa settings
    // has customized LoRa settings    
  if (optBW!=0 || optSF!=0 || optFQ>0) {
			
		if (optSF!=0) {
			PRINT_CSTSTR("LoRa SF ");
			PRINTLN_VALUE("%d", optSF);	
			if (optSF==5)
				SpreadingFactor = LORA_SF5;
			else		
			if (optSF==6)
				SpreadingFactor = LORA_SF6;
			else if (optSF==7)
				SpreadingFactor = LORA_SF7;
			else if (optSF==8)
				SpreadingFactor = LORA_SF8;
			else if (optSF==9)
				SpreadingFactor = LORA_SF9;
			else if (optSF==10)
				SpreadingFactor = LORA_SF10;			
			else {
				PRINT_CSTSTR("Unknown LoRa SF, taking SF10\n");
				SpreadingFactor = LORA_SF10;
			}																	
		}

		if (optBW!=0) {    
			PRINT_CSTSTR("LoRa BW ");
			PRINTLN_VALUE("%ld", optBW*1000);
			if (optBW==250 || optBW==406 || optBW==400)
				Bandwidth = LORA_BW_0400; //actually 406250hz
			else if (optBW==500 || optBW==812 || optBW==800)
				Bandwidth = LORA_BW_0800;	//actually 812500hz						    
			else if (optBW==1625 || optBW==1600)
				Bandwidth = LORA_BW_1600; //actually 1625000hz
			else {
				PRINT_CSTSTR("Unknown LoRa BW, taking BW406\n");
				Bandwidth = LORA_BW_0400;
			}										
		}
		
		// Select frequency
		if (optFQ>0.0) {
			Frequency=optFQ;
			
			PRINT_CSTSTR("Frequency ");
			PRINTLN_VALUE("%lu", Frequency);
		}			
  }
  
  //The function call list below shows the complete setup for the LoRa device using the information defined in the
  //Settings.h file.
  //The 'Setup Lora device' list below can be replaced with a single function call;
  //LT.setupRanging(Frequency, Offset, SpreadingFactor, Bandwidth, CodeRate, RangingAddress, RANGING_SLAVE);

  //***************************************************************************************************
  //Setup Lora device
  //***************************************************************************************************
  
  LT.setMode(MODE_STDBY_RC);
  LT.setPacketType(PACKET_TYPE_RANGING);
  LT.setModulationParams(SpreadingFactor, Bandwidth, CodeRate);
  LT.setPacketParams(12, LORA_PACKET_VARIABLE_LENGTH, 0, LORA_CRC_ON, LORA_IQ_NORMAL, 0, 0);
  LT.setRfFrequency(Frequency, Offset);
  LT.setTxParams(MAX_DBM, RADIO_RAMP_02_US);
  LT.setRangingMasterAddress(RangingAddress);
  LT.setRangingSlaveAddress(RangingAddress);
  LT.setRangingCalibration(LT.lookupCalibrationValue(SpreadingFactor, Bandwidth));
  LT.setRangingRole(RANGING_SLAVE);
  LT.writeRegister(REG_RANGING_FILTER_WINDOW_SIZE, 8); //set up window size for ranging averaging
  LT.setHighSensitivity();  
  //***************************************************************************************************

	//override automatic lookup of calibration value from library table
  //LT.setRangingCalibration(11300);               

  //reads and prints the configured LoRa settings, useful check
  LT.printModemSettings();                                     
  PRINTLN;
  //reads and prints the configured operting settings, useful check
  LT.printOperatingSettings();                                
  PRINTLN;

#ifdef OLED
  u8x8.begin();
  //u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.setFont(u8x8_font_pxplustandynewtv_r);
  u8x8.drawString(0, 0, "Ranging slave");
  sprintf(oled_msg,"SF%dBW%d", LT.getLoRaSF(), LT.returnBandwidth()/1000);
  u8x8.drawString(0, 1, oled_msg);
  sprintf(oled_msg,"Cal %d", LT.getSetCalibrationValue());
  u8x8.drawString(0, 2, oled_msg);   
#endif  
  
  PRINT_CSTSTR("Calibration,");
  PRINTLN_VALUE("%d", LT.getSetCalibrationValue());           //reads the calibratuion value currently set 
  
	PRINT_CSTSTR("SX128X - ");
  PRINTLN_CSTSTR("ranging slave ready");
  
  delay(2000);
}

void loop()
{
  LT.receiveRanging(RangingAddress, 0, MAX_DBM, NO_WAIT);

  endwaitmS = millis() + rangingRXTimeoutmS;
	
	do {
		IrqStatus = LT.readIrqStatus();
		delay(1);
	} while ( !(IrqStatus & IRQ_RANGING_SLAVE_RESPONSE_DONE) && !(IrqStatus & IRQ_RANGING_SLAVE_REQUEST_DISCARDED) && (millis() <= endwaitmS));	

  if (millis() >= endwaitmS)
  {
    PRINT_CSTSTR("Error - Ranging Receive Timeout!!\n");
  }
  else
  {
    IrqStatus = LT.readIrqStatus();

    if (IrqStatus & IRQ_RANGING_SLAVE_RESPONSE_DONE)
    {
      response_sent++;
      PRINT_VALUE("%d", response_sent);
      PRINT_CSTSTR(" Response sent");
    }
    else
    {
      PRINT_CSTSTR("Slave error,");
      PRINT_CSTSTR(",Irq,");
      PRINT_HEX("%K", IrqStatus);
      LT.printIrqStatus();
    }
    PRINTLN;
  }
}

#ifndef ARDUINO

int main (int argc, char *argv[]){

  int opt=0;
  
  //Specifying the expected options
  static struct option long_options[] = {
      {"bw", required_argument, 0,  'a' },
      {"sf", required_argument, 0,  'b' }, 
      {"freq", required_argument, 0,'c' },   
      {0, 0, 0,  0}
  };
  
  int long_index=0;
  
  while ((opt = getopt_long(argc, argv,"a:b:c", 
                 long_options, &long_index )) != -1) {
      switch (opt) {
           case 'a' : optBW = atoi(optarg); 
                      // 7, 10, 15, 20, 31, 41, 62, 125, 250 or 500 for SX126X and SX127X_lora_gateway
                      // 125/200/203, 250/400/406, 500/800/812 or 1600/1625 for SX128X
               break;
           case 'b' : optSF = atoi(optarg);
                      // 5, 6, 7, 8, 9, 10, 11 or 12
               break;             
           case 'c' : optFQ=atof(optarg)*1000000.0;
                      // optFQ in MHz e.g. 868.1
               break;     
      }
  }
  
  setup();
  
  while(1){
    loop();
  }
  
  return (0);
}

#endif
