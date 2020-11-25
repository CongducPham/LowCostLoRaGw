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

uint32_t endwaitmS;
uint16_t IrqStatus;
uint32_t response_sent;

void setup() {

  delay(1000); 
  
#ifdef ARDUINO  
  Serial.begin(38400);
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

  setup();
  
  while(1){
    loop();
  }
  
  return (0);
}

#endif
