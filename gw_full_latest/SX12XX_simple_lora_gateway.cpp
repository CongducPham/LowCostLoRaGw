/* 
 *  Very simple LoRa gateway to test the new SX12XX lib
 *
 *  Based on the simple receiver example from SX12XX lib
 */

/* Output example 

-------------------------------------
Packet length: 23
Destination: 1
Packet Type: 16
Source: 15
SeqNo: 2
RXTimestamp: 92616
RXDoneTimestamp: 92616

Hello World 1234567890*
CRC,DAAB,RSSI,-61dBm,SNR,6dB,Length,23,Packets,3,Errors,0,IRQreg,8012
--> Packet is for gateway
--- rxlora. dst=1 type=0x10 src=15 seq=2 len=23 SNR=6 RSSIpkt=-61 BW=203 CR=4/5 SF=12
^p1,16,15,2,23,6,-61
^r203,5,12,2403000
^t#*92616000
⸮⸮Hello World 1234567890*

*/

#ifdef ARDUINO
#include <SPI.h>
#define USE_SPI_TRANSACTION          //this is the standard behaviour of library, use SPI Transaction switching
// please uncomment only 1 choice 
//#define SX126X
#define SX127X
//#define SX128X
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

#ifdef ARDUINO
#include "Arduino_LoRa_SX12XX_Simple_gateway.h"
#else
#include "SX12XX_simple_lora_gateway.h"                 //include the setiings file, frequencies, LoRa settings etc   
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

///////////////////////////////////////////////////////////////////
uint8_t GW_ADDR=1;
//////////////////////////////////////////////////////////////////

// will use 0xFF0xFE to prefix data received from LoRa, so that post-processing stage can differenciate
// data received from radio
#define WITH_DATA_PREFIX

#ifdef WITH_DATA_PREFIX
#define DATA_PREFIX_0 0xFF
#define DATA_PREFIX_1 0xFE
#endif

uint32_t RXpacketCount;
uint32_t errors;

uint8_t RXBUFFER[RXBUFFER_SIZE];                 //create the buffer that received packets are copied into

uint8_t RXPacketL;                               //stores length of packet received
int8_t  PacketRSSI;                              //stores RSSI of received packet
int8_t  PacketSNR;                               //stores signal to noise ratio (SNR) of received packet

void packet_is_OK()
{
  uint16_t IRQStatus, localCRC;

  IRQStatus = LT.readIrqStatus();                 //read the LoRa device IRQ status register

  RXpacketCount++;

  PRINTLN;
  LT.printASCIIPacket(RXBUFFER, RXPacketL);       //print the packet as ASCII characters
	PRINTLN;

  localCRC = LT.CRCCCITT(RXBUFFER, RXPacketL, 0xFFFF);  //calculate the CRC, this is the external CRC calculation of the RXBUFFER
  PRINT_CSTSTR("CRC,");                       //contents, not the LoRa device internal CRC
  PRINT_HEX("%X",localCRC);
  PRINT_CSTSTR(",RSSI,");
  PRINT_VALUE("%d",PacketRSSI);
  PRINT_CSTSTR("dBm,SNR,");
  PRINT_VALUE("%d",PacketSNR);
  PRINT_CSTSTR("dB,Length,");
  PRINT_VALUE("%d",RXPacketL);
  PRINT_CSTSTR(",Packets,");
  PRINT_VALUE("%d",RXpacketCount);
  PRINT_CSTSTR(",Errors,");
  PRINT_VALUE("%d",errors);
  PRINT_CSTSTR(",IRQreg,");
  PRINT_HEX("%X",IRQStatus);
  PRINTLN;
  FLUSHOUTPUT;
}


void packet_is_Error()
{
  uint16_t IRQStatus;
  IRQStatus = LT.readIrqStatus();                   //read the LoRa device IRQ status register

  if (IRQStatus & IRQ_RX_TIMEOUT)                   //check for an RX timeout
  {
  	//here if we don't receive anything, we normally print nothing
    //PRINT_CSTSTR("RXTimeout");
  }
  else
  {
    errors++;
    PRINT_CSTSTR("PacketError");
    PRINT_CSTSTR(",RSSI,");
    PRINT_VALUE("%d",PacketRSSI);
    PRINT_CSTSTR("dBm,SNR,");
    PRINT_VALUE("%d",PacketSNR);
    PRINT_CSTSTR("dB,Length,");
    PRINT_VALUE("%d",LT.readRXPacketL());               //get the device packet length
    PRINT_CSTSTR(",Packets,");
    PRINT_VALUE("%d",RXpacketCount);
    PRINT_CSTSTR(",Errors,");
    PRINT_VALUE("%d",errors);
    PRINT_CSTSTR(",IRQreg,");
    PRINT_HEX("%X",IRQStatus);
    LT.printIrqStatus();                            //print the names of the IRQ registers set
    PRINTLN;
  }

  FLUSHOUTPUT;
}

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
  //set for IRQ on RX done and timeout on DIO1
  LT.setDioIrqParams(IRQ_RADIO_ALL, (IRQ_RX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);
#endif
#ifdef SX127X
  //set for IRQ on RX done
  LT.setDioIrqParams(IRQ_RADIO_ALL, IRQ_RX_DONE, 0, 0);
#ifdef PABOOST
  LT.setPA_BOOST(true);
  PRINTLN_CSTSTR("Use PA_BOOST amplifier line");
#endif
#endif
#ifdef SX128X
  LT.setDioIrqParams(IRQ_RADIO_ALL, (IRQ_RX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);
#endif    
  
  //***************************************************************************************************

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

  PRINT_CSTSTR("Setting Power: ");
  PRINTLN_VALUE("%d", MAX_DBM);

  //the gateway needs to know its address to verify that a packet requesting an ack is for him
  //this is done in the SX12XX library
  //
  LT.setDevAddr(GW_ADDR);
  PRINT_CSTSTR("node addr: ");
  PRINT_VALUE("%d", GW_ADDR);
  PRINTLN;  
  //TX power and ramp time
  //this is required because the gateway can have to transmit ACK messages prior to send any downlink packet
  //so txpower needs to be set before hand.  
#ifdef SX127X   
  LT.setTxParams(MAX_DBM, RADIO_RAMP_DEFAULT);            
#endif
#if defined SX126X || defined SX128X
  LT.setTxParams(MAX_DBM, RAMP_TIME);
#endif
  
#ifdef SX126X
	PRINT_CSTSTR("SX126X - ");
#endif
#ifdef SX127X
	PRINT_CSTSTR("SX127X - ");
#endif
#ifdef SX128X
	PRINT_CSTSTR("SX128X - ");
#endif

  PRINT_CSTSTR("Receiver ready - RXBUFFER_SIZE ");
  PRINTLN_VALUE("%d",RXBUFFER_SIZE);
  PRINTLN;
}

void loop()
{
  char print_buf[100];
  
  RXPacketL = LT.receiveAddressed(RXBUFFER, RXBUFFER_SIZE, 10000, WAIT_RX); //wait for a packet to arrive with 60seconds (60000mS) timeout
  
  PacketRSSI = LT.readPacketRSSI();              //read the recived RSSI value
  PacketSNR = LT.readPacketSNR();                //read the received SNR value

  if (RXPacketL == 0)                            //if the LT.receive() function detects an error, RXpacketL is 0
  {
    packet_is_Error();
  }
  else
  {
  	PRINTLN;
  	PRINTLN_CSTSTR("-------------------------------------");
  	PRINT_CSTSTR("Packet length: ");
  	PRINTLN_VALUE("%d", RXPacketL);
    PRINT_CSTSTR("Destination: ");
    PRINTLN_VALUE("%d", LT.readRXDestination());
    PRINT_CSTSTR("Packet Type: ");
    PRINTLN_VALUE("%d", LT.readRXPacketType());
    PRINT_CSTSTR("Source: ");
    PRINTLN_VALUE("%d", LT.readRXSource());
    PRINT_CSTSTR("SeqNo: ");
    PRINTLN_VALUE("%d", LT.readRXSeqNo());
  	PRINT_CSTSTR("RXTimestamp: ");
  	PRINTLN_VALUE("%ld", LT.readRXTimestamp());
  	PRINT_CSTSTR("RXDoneTimestamp: ");
  	PRINTLN_VALUE("%ld", LT.readRXDoneTimestamp());    
    packet_is_OK();
    
    if (LT.readRXDestination()==1)
    	PRINTLN_CSTSTR("--> Packet is for gateway");

    // reproduce the behavior of the lora_gateway
    //
    sprintf(print_buf, "--- rxlora. dst=%d type=0x%02X src=%d seq=%d", 
      LT.readRXDestination(),
      LT.readRXPacketType(), 
      LT.readRXSource(),
      LT.readRXSeqNo());
 
    PRINT_STR("%s", print_buf);

    sprintf(print_buf, " len=%d SNR=%d RSSIpkt=%d BW=%d CR=4/%d SF=%d\n", 
      RXPacketL, 
      PacketSNR,
      PacketRSSI,
      (uint16_t)(LT.returnBandwidth()/1000),
#if defined SX126X || defined SX128X
      LT.getLoRaCodingRate()+4,
#else				
      LT.getLoRaCodingRate(),
#endif	
      LT.getLoRaSF());     

    PRINT_STR("%s", print_buf);              

    // provide a short output for external program to have information about the received packet
    // ^psrc_id,seq,len,SNR,RSSI
    sprintf(print_buf, "^p%d,%d,%d,%d,",
      LT.readRXDestination(),
      LT.readRXPacketType(), 
      LT.readRXSource(),
      LT.readRXSeqNo());
 
    PRINT_STR("%s", print_buf);       

    sprintf(print_buf, "%d,%d,%d\n",
      RXPacketL, 
      PacketSNR,
      PacketRSSI);

    PRINT_STR("%s", print_buf); 

    // ^rbw,cr,sf,fq
    sprintf(print_buf, "^r%d,%d,%d,%ld\n", 
      (uint16_t)(LT.returnBandwidth()/1000),
#if defined SX126X || defined SX128X
				LT.getLoRaCodingRate()+4,
#else				
				LT.getLoRaCodingRate(),
#endif	
      LT.getLoRaSF(),
      (uint32_t)Frequency/1000);

    PRINT_STR("%s", print_buf);

    //print the reception date and time
    sprintf(print_buf, "^t#*%ld\n", LT.readRXDoneTimestamp()*1000);
    PRINT_STR("%s", print_buf);

#if defined WITH_DATA_PREFIX
    PRINT_STR("%c",(char)DATA_PREFIX_0);        
    PRINT_STR("%c",(char)DATA_PREFIX_1);
#endif

    // print to stdout the content of the packet
    //
    FLUSHOUTPUT;
 
    for (int a=0; a<RXPacketL; a++) {
        PRINT_STR("%c",(char)RXBUFFER[a]);
      }
   
    PRINTLN;
    FLUSHOUTPUT;          
    FLUSHOUTPUT;	
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
