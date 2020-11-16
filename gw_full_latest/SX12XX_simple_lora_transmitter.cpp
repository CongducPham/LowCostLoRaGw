/* 
 *  Very simple LoRa transmitter to test the new SX127XLT lib on the RPI
 *
 *	Author: Congduc Pham, 2020
 *
 *  Based on the simple transmitter example from SX127XLT lib
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h> 
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>

// please uncomment only 1 choice 
//#define SX126X
//#define SX127X
//#define SX128X

#ifdef SX126X
#include <SX126XLT.h>
SX126XLT LT;                                          
#endif

#ifdef SX127X
#include <SX127XLT.h>
SX127XLT LT;                                          
#endif

#ifdef SX128X
#include <SX128XLT.h>
SX128XLT LT;                                         
#endif

#include "SX12XX_simple_lora_transmitter.h"                 //include the setiings file, frequencies, LoRa settings etc   

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

uint8_t TXPacketL;
uint32_t TXPacketCount, startmS, endmS;

uint8_t buff[] = "Hello World 1234567890";

void packet_is_OK()
{
  uint16_t localCRC;

  PRINT_CSTSTR("  BytesSent,");
  PRINT_VALUE("%d", TXPacketL);                             //print transmitted packet length
  localCRC = LT.CRCCCITT(buff, TXPacketL, 0xFFFF);
  PRINT_CSTSTR("  CRC,");
  PRINT_HEX("%X",localCRC);                         //print CRC of transmitted packet
  PRINT_CSTSTR("  TransmitTime,");
  PRINT_VALUE("%ld", endmS - startmS);                       //print transmit time of packet
  PRINT_CSTSTR("mS");
  PRINT_CSTSTR("  PacketsSent,");
  PRINT_VALUE("%d", TXPacketCount);                         //print total of packets sent OK
  PRINTLN;
  FLUSHOUTPUT;
}


void packet_is_Error()
{
  uint16_t IRQStatus;
  IRQStatus = LT.readIrqStatus();                   //read the LoRa device IRQ status register

	PRINT_CSTSTR(" SendError");
	PRINT_CSTSTR("Length,");
	PRINT_VALUE("%d",TXPacketL);               //get the device packet length
	PRINT_CSTSTR(",IRQreg,");
	PRINT_HEX("%X",IRQStatus);
	LT.printIrqStatus();                            //print the names of the IRQ registers set
	PRINTLN;
  FLUSHOUTPUT;
}

void setup() {

  delay(1000); 
  
  SPI.begin(); 
  //Set Most significant bit first
  SPI.setBitOrder(MSBFIRST);
  //Divide the clock frequency
  SPI.setClockDivider(SPI_CLOCK_DIV64);
  //Set data mode
  SPI.setDataMode(SPI_MODE0); 

  //setup hardware pins used by device, then check if device is found
#ifdef SX126X
  if (LT.begin(NSS, NRESET, RFBUSY, DIO1, DIO2, DIO3, RX_EN, TX_EN, SW, LORA_DEVICE))
#endif

#ifdef SX127X
  if (LT.begin(NSS, NRESET, DIO0, DIO1, DIO2, LORA_DEVICE))
#endif

#ifdef SX128X
  if (LT.begin(NSS, NRESET, RFBUSY, DIO1, DIO2, DIO3, RX_EN, TX_EN, LORA_DEVICE))
#endif
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
  //LT.setupLoRa(Frequency, Offset, SpreadingFactor, Bandwidth, CodeRate, Optimisation);

  //***************************************************************************************************
  //Setup Lora device
  //***************************************************************************************************
  
  LT.setMode(MODE_STDBY_RC);
#ifdef SX126X
  LT.setRegulatorMode(USE_DCDC);
  LT.setPaConfig(0x04, PAAUTO, LORA_DEVICE);
  LT.setDIO3AsTCXOCtrl(TCXO_CTRL_3_3V);
  LT.calibrateDevice(ALLDevices);                //is required after setting TCXO
  LT.calibrateImage(Frequency);
  LT.setDIO2AsRfSwitchCtrl();
#endif
#ifdef SX128X
  LT.setRegulatorMode(USE_LDO);
#endif
  //set for LoRa transmissions                              
  LT.setPacketType(PACKET_TYPE_LORA);
  //set the operating frequency                 
  LT.setRfFrequency(Frequency, Offset);
//run calibration after setting frequency
#ifdef SX127X
  LT.calibrateImage(0);
#endif
  //set LoRa modem parameters
#if defined SX126X || defined SX127X
  LT.setModulationParams(SpreadingFactor, Bandwidth, CodeRate, Optimisation);
#endif
#ifdef SX128X
  LT.setModulationParams(SpreadingFactor, Bandwidth, CodeRate);
#endif                                     
  //where in the SX buffer packets start, TX and RX
  LT.setBufferBaseAddress(0x00, 0x00);
  //set packet parameters
#if defined SX126X || defined SX127X                     
  LT.setPacketParams(8, LORA_PACKET_VARIABLE_LENGTH, 255, LORA_CRC_ON, LORA_IQ_NORMAL);
#endif
#ifdef SX128X
  LT.setPacketParams(12, LORA_PACKET_VARIABLE_LENGTH, 255, LORA_CRC_ON, LORA_IQ_NORMAL, 0, 0);
#endif
  //syncword, LORA_MAC_PRIVATE_SYNCWORD = 0x12, or LORA_MAC_PUBLIC_SYNCWORD = 0x34
  //TODO check for sync word when SX128X 
#if defined SX126X || defined SX127X            
  LT.setSyncWord(LORA_MAC_PRIVATE_SYNCWORD);
  //set for highest sensitivity at expense of slightly higher LNA current
  LT.setHighSensitivity();  //set for maximum gain
#endif
#ifdef SX126X
  //set for IRQ on TX done and timeout on DIO1
  LT.setDioIrqParams(IRQ_RADIO_ALL, (IRQ_TX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);
#endif
#ifdef SX127X
  //set for IRQ on TX done
  LT.setDioIrqParams(IRQ_RADIO_ALL, IRQ_RX_DONE, 0, 0);
#ifdef PABOOST
  LT.setPA_BOOST(true);
  PRINTLN_CSTSTR("Use PA_BOOST amplifier line");
#endif
#endif
#ifdef SX128X
  LT.setDioIrqParams(IRQ_RADIO_ALL, (IRQ_TX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);
#endif    
  
  //***************************************************************************************************

	LT.setDevAddr(15);
	//TX power and ramp time
	PRINT_CSTSTR("Set LoRa txpower dBm to ");
	PRINTLN_VALUE("%d", MAX_DBM);  
	//to test setting txpower before only once	
	//LT.setTxParams(MAX_DBM, RADIO_RAMP_DEFAULT);            

  PRINTLN;
  LT.printModemSettings();                                     //reads and prints the configured LoRa settings, useful check
  PRINTLN;
  LT.printOperatingSettings();                                 //reads and prints the configured operting settings, useful check
  PRINTLN;
  PRINTLN;
#if defined SX126X || defined SX127X  
  //print contents of device registers, normally 0x00 to 0x4F
  LT.printRegisters(0x00, 0x4F);
#endif                       
#ifdef SX128X
  //print contents of device registers, normally 0x900 to 0x9FF 
  LT.printRegisters(0x900, 0x9FF);
#endif  
  PRINTLN;
  PRINTLN;

#ifdef SX126X
	PRINT_CSTSTR("SX126X - ");
#endif
#ifdef SX127X
	PRINT_CSTSTR("SX127X - ");
#endif
#ifdef SX128X
	PRINT_CSTSTR("SX128X - ");
#endif

  PRINTLN_CSTSTR("Transmitter ready");
}

void loop()
{

	PRINT_CSTSTR("Sending packet ");
	PRINTLN_VALUE("%d", TXPacketCount);
	
	TXPacketL = sizeof(buff);
  buff[TXPacketL - 1] = '*';                                   //replace null character at buffer end so its visible on reciver

  LT.printASCIIPacket(buff, TXPacketL);                        //print the buffer (the sent packet) as ASCII	
	
	startmS=millis();
	//txpower can be set to -1 if we want to keep the same power setting PROVIDED THAT setTxParams() has been called before
  if (LT.transmitAddressed(buff, TXPacketL, PKT_TYPE_DATA, DEFAULT_DEST_ADDR, LT.readDevAddr(), 10000, MAX_DBM, WAIT_RX)) 
  {
    endmS = millis();                                          //packet sent, note end time
    TXPacketCount++;
    packet_is_OK();
  }
  else
  {
    packet_is_Error();                                 //transmit packet returned 0, there was an error
  }
  
  delay(5000);
}

int main (int argc, char *argv[]){

  setup();
  
  while(1){
    loop();
  }
  
  return (0);
}

