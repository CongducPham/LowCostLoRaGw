/*
  Copyright 2019 - Stuart Robinson
  Licensed under a MIT license displayed at the bottom of this document.
  17/12/19
*/

/*
Parts of code Copyright (c) 2013, SEMTECH S.A.
See LICENSE.TXT file included in the library
*/

/**************************************************************************
  Change logs
  
	Jan. 15th, 2021
		- move to wiringPI for low-level SPI communication. arduinoPi.cpp is a wrapper providing the SPI class for Raspberry
		
	Dec. 21st, 2020
		- add CollisionAvoidance mechanism: LT.CollisionAvoidance(uint8_t pl, uint8_t ca=1)
			- returns a backoff timer expressed in ms
			- if 0 then node can transmit data packet
		- add a pointer to a user-defined low power function
			- if no low power function is provided, then delay function is used in CollisionAvoidance
			- setLowPowerFctPtr(void (*pf)(unsigned long)) is used to set the low power function 
			- e.g. in Arduino sketch: LT.setLowPowerFctPtr(&lowPower) where lowPower() is the user-defined function
**************************************************************************/

/**************************************************************************
  CHANGES by C. Pham, October 2020

  - Ensure that lib compiles on Raspberry - done
  	- change pow(2,X) to (1 << X)
  	- ...
  - Add ack transaction with transmitAddressed-receiveAddressed	
  	- in the return ack, the SNR of the received packet at the gateway is included
  	- the sending of the ack and the reception of the ack use inverted IQ
  	- you can disable IQ inversion in ack transaction by commenting #define INVERTIQ_ON_ACK    	
  - Add polling mechanism to avoid additional DIO0 connection - done
  	- uncomment #define USE_POLLING  
  	- this is the default behavior    	
  - All Serial.print replaced by macros - done
  	- lib can be used on Arduino and on UNIX-based computer such as RaspberryPI
  - separate management of IQ inversion - done. 
  	- invertIQ(true) or invertIQ(false)  	
  - add returnBandwidth() function to get the operating bandwidth value: 7800, ..., 125000, 250000, ...	- done
  - add getToA(uint8_t pl) function to get the time-on-air of a pl-byte packet according to current LoRa settings - done
  - change the order of header and add a seq number in the header - done
  	- for transmitAddressed, receiveAddressed
  	- new 4-byte header is : destination, packet type, source, sequence number
  - add readRXSeqNo() function to get the RX sequence number in the received packet - done
  - add TX sequence number management - done
  	- setTXSeqNo(uint8_t seqno)
  	- readTXSeqNo()
  - add packet receive timestamp information based on millis() counter
  	- readRXTimestamp() when valid header has been detected - done
  	- readRXDoneTimestamp() when entire packet reception has been detected - done
  - add CAD - done
  	- doCad(uint8_t counter) realizes a number of CAD  
  - add carrier sense mechanism - done.
  	- CarrierSense(uint8_t cs) performs 3 variants of carrier sense methods 	
  - _TXPacketL type changed to uint8_t - done
  - RSSI & SNR type (should be signed value?) changed to int8_t - done
  - calcSymbolTime() has been modified to avoid usage of pow() function - done

**************************************************************************/

/**************************************************************************
Added by C. Pham - Oct. 2020
**************************************************************************/

#if defined __SAMD21G18A__ && not defined ARDUINO_SAMD_FEATHER_M0
	#define PRINTLN                   SerialUSB.println("")
	#define PRINT_CSTSTR(param)   		SerialUSB.print(F(param))
	#define PRINTLN_CSTSTR(param)			SerialUSB.println(F(param))
	#define PRINT_STR(fmt,param)      SerialUSB.print(param)
	#define PRINTLN_STR(fmt,param)		SerialUSB.println(param)
	#define PRINT_VALUE(fmt,param)    SerialUSB.print(param)
	#define PRINTLN_VALUE(fmt,param)	SerialUSB.println(param)
	#define PRINT_HEX(fmt,param)      SerialUSB.print(param,HEX)
	#define PRINTLN_HEX(fmt,param)		SerialUSB.println(param,HEX)
	#define FLUSHOUTPUT               SerialUSB.flush()
#elif defined ARDUINO
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

//! MACROS //
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)  // read a bit
#define bitSet(value, bit) ((value) |= (1UL << (bit)))    // set bit to '1'
#define bitClear(value, bit) ((value) &= ~(1UL << (bit))) // set bit to '0'
#define lowByte(value) ((value) & 0x00FF)
#define highByte(value) (((value) >> 8) & 0x00FF)

/**************************************************************************
End by C. Pham - Oct. 2020
**************************************************************************/

/**************************************************************************
Modified  by C. Pham - Oct. 2020
**************************************************************************/

#ifdef ARDUINO
#include <SPI.h>
#define USE_SPI_TRANSACTION          //this is the standard behaviour of library, use SPI Transaction switching
#else
#include <math.h>
#endif

#include <SX126XLT.h>

//use polling with RADIO_GET_IRQSTATUS instead of DIO1
#define USE_POLLING
//invert IQ in ack transaction
#define INVERTIQ_ON_ACK
/**************************************************************************
End by C. Pham - Oct. 2020
**************************************************************************/

#define LTUNUSED(v) (void) (v)       //add LTUNUSED(variable); to avoid compiler warnings 

//#define DEBUGBUSY                   //comment out if you do not want a busy timeout message
//#define SX126XDEBUG1               //enable debug messages
//#define SX126XDEBUG3              //enable debug messages
#define SX126XDEBUGACK               //enable ack transaction debug messages
#define SX126XDEBUGRTS
//#define SX126XDEBUGPINS           //enable pin allocation debug messages
//#define DEBUGFSKRTTY                 //enable for FSKRTTY debugging 

/*
****************************************************************************
  To Do:


****************************************************************************
*/

SX126XLT::SX126XLT()
{
  //Anything you need when instantiating your object goes here
  /**************************************************************************
	Added by C. Pham - Oct. 2020
  **************************************************************************/  
  
  _TXSeqNo = 0;
  _RXSeqNo = 0;

  _lowPowerFctPtr=NULL;
  
//#define RADIO_FAKE_TEST
#ifdef RADIO_FAKE_TEST 
  savedModParam1 = LORA_SF12;
  savedModParam2 = LORA_BW_125;
  savedModParam3 = LORA_CR_4_5;
  savedModParam4 = LDRO_ON;	  
  
  savedPacketParam1 = 8;
  savedPacketParam2 = LORA_PACKET_VARIABLE_LENGTH;
  savedPacketParam3 = 255;
  savedPacketParam4 = LORA_CRC_ON;
  savedPacketParam5 = LORA_IQ_NORMAL;  
#endif
    
  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/  
}

/* Formats for :begin
  original > begin(int8_t pinNSS, int8_t pinNRESET, int8_t pinRFBUSY, int8_t pinDIO1, int8_t pinDIO2, int8_t pinDIO3, int8_t pinSW, uint8_t device);
1 All pins > begin(int8_t pinNSS, int8_t pinNRESET, int8_t pinRFBUSY, int8_t pinDIO1, int8_t pinDIO2, int8_t pinDIO3, int8_t pinRXEN, int8_t pinTXEN, int8_t pinSW, uint8_t device)
2 NiceRF   > begin(int8_t pinNSS, int8_t pinNRESET, int8_t pinRFBUSY, int8_t pinDIO1, uint8_t device)
3 Dorji    > begin(int8_t pinNSS, int8_t pinNRESET, int8_t pinRFBUSY, int8_t pinDIO1, int8_t pinSW, uint8_t device)
4 Ebyte    > begin(int8_t pinNSS, int8_t pinNRESET, int8_t pinRFBUSY, int8_t pinDIO1, int8_t pinRXEN, int8_t pinTXEN, uint8_t device)
*/


bool SX126XLT::begin(int8_t pinNSS, int8_t pinNRESET, int8_t pinRFBUSY, int8_t pinDIO1, int8_t pinDIO2, int8_t pinDIO3, int8_t pinRXEN, int8_t pinTXEN, int8_t pinSW, uint8_t device)
{

#if not defined ARDUINO && not defined USE_ARDUPI
  wiringPiSetup() ;
  PRINTLN_CSTSTR("<<< using wiringPi >>>"); 
#endif

  //format 1 pins, assign all available pins  
  _NSS = pinNSS;
  _NRESET = pinNRESET;
  _RFBUSY = pinRFBUSY;
  _DIO1 = pinDIO1;
  _DIO2 = pinDIO2;
  _DIO3 = pinDIO3;
  _RXEN = pinRXEN;
  _TXEN = pinTXEN;
  _SW = pinSW;
  _Device = device;
  
  _TXDonePin = pinDIO1;        //this is defalt pin for sensing TX done
  _RXDonePin = pinDIO1;        //this is defalt pin for sensing RX done

  pinMode(_NSS, OUTPUT);
  digitalWrite(_NSS, HIGH);
  pinMode(_NRESET, OUTPUT);
  digitalWrite(_NRESET, LOW);
  pinMode(_RFBUSY, INPUT);


#ifdef SX126XDEBUGPINS
  PRINTLN_CSTSTR("format 1 begin()");
  PRINTLN_CSTSTR("SX126XLT constructor instantiated successfully");
  PRINT_CSTSTR("NSS ");
  PRINTLN_VALUE("%d",_NSS);
  PRINT_CSTSTR("NRESET ");
  PRINTLN_VALUE("%d",_NRESET);
  PRINT_CSTSTR("RFBUSY ");
  PRINTLN_VALUE("%d",_RFBUSY);
  PRINT_CSTSTR("DIO1 ");
  PRINTLN_VALUE("%d",_DIO1);
  PRINT_CSTSTR("DIO2 ");
  PRINTLN_VALUE("%d",_DIO2);
  PRINT_CSTSTR("DIO3 ");
  PRINTLN_VALUE("%d",_DIO3);
  PRINT_CSTSTR("RX_EN ");
  PRINTLN_VALUE("%d",_RXEN);
  PRINT_CSTSTR("TXEN ");
  PRINTLN_VALUE("%d",_TXEN);
  PRINT_CSTSTR("SW ");
  PRINTLN_VALUE("%d",_SW);
#endif


  if (_DIO1 >= 0)
  {
    pinMode( _DIO1, INPUT);
  }
  
  if (_DIO2 >= 0)
  {
    pinMode( _DIO2, INPUT);
  }
 
  if (_DIO3 >= 0)
  {
    pinMode( _DIO3, INPUT);
  }
  
  
  if ((_RXEN >= 0) && (_TXEN >= 0))
  {
#ifdef SX126XDEBUGPINS
    PRINTLN_CSTSTR("RX_EN & TX_EN switching enabled");
#endif
    pinMode(_RXEN, OUTPUT);
    pinMode(_TXEN, OUTPUT);
    _rxtxpinmode = true;
  }
  else
  {
#ifdef SX126XDEBUGPINS
    PRINTLN_CSTSTR("RX_EN & TX_EN not used");
#endif
    _rxtxpinmode = false;
  }

   
  if (_SW >= 0)
  {
    pinMode( _SW, OUTPUT);                     //Dorji devices have an RW pin that needs to be set high to power antenna switch
    digitalWrite(_SW, HIGH);
  }
 
  resetDevice();
  if (checkDevice())
  {
    return true;
  }

  return false;
}


bool SX126XLT::begin(int8_t pinNSS, int8_t pinNRESET, int8_t pinRFBUSY, int8_t pinDIO1, uint8_t device)
{

#if not defined ARDUINO && not defined USE_ARDUPI
  wiringPiSetup() ;
  PRINTLN_CSTSTR("<<< using wiringPi >>>"); 
#endif

  //format 2 pins for NiceRF, NSS, NRESET, RFBUSY, DIO1  
  _NSS = pinNSS;
  _NRESET = pinNRESET;
  _RFBUSY = pinRFBUSY;
  _DIO1 = pinDIO1;
  _DIO2 = -1;
  _DIO3 = -1;
  _RXEN = -1;
  _TXEN = -1;
  _SW = -1;
  _Device = device;
  
  _TXDonePin = pinDIO1;        //this is defalt pin for sensing TX done
  _RXDonePin = pinDIO1;        //this is defalt pin for sensing RX done

  pinMode(_NSS, OUTPUT);
  digitalWrite(_NSS, HIGH);
  pinMode(_NRESET, OUTPUT);
  digitalWrite(_NRESET, LOW);
  pinMode(_RFBUSY, INPUT);


#ifdef SX126XDEBUGPINS
  PRINTLN_CSTSTR("format 2 NiceRF begin()");
  PRINTLN_CSTSTR("SX126XLT constructor instantiated successfully");
  PRINT_CSTSTR("NSS ");
  PRINTLN_VALUE("%d",_NSS);
  PRINT_CSTSTR("NRESET ");
  PRINTLN_VALUE("%d",_NRESET);
  PRINT_CSTSTR("RFBUSY ");
  PRINTLN_VALUE("%d",_RFBUSY);
  PRINT_CSTSTR("DIO1 ");
  PRINTLN_VALUE("%d",_DIO1);
  PRINT_CSTSTR("DIO2 ");
  PRINTLN_VALUE("%d",_DIO2);
  PRINT_CSTSTR("DIO3 ");
  PRINTLN_VALUE("%d",_DIO3);
  PRINT_CSTSTR("RX_EN ");
  PRINTLN_VALUE("%d",_RXEN);
  PRINT_CSTSTR("TX_EN ");
  PRINTLN_VALUE("%d",_TXEN);
  PRINT_CSTSTR("SW ");
  PRINTLN_VALUE("%d",_SW);
#endif


  if (_DIO1 >= 0)
  {
    pinMode( _DIO1, INPUT);
  }
  

#ifdef SX126XDEBUGPINS
  PRINTLN_CSTSTR("RX_EN & TX_EN switching disabled");
#endif
  
  _rxtxpinmode = false;
   
  resetDevice();
  
  if (checkDevice())
  {
    return true;
  }

  return false;
}



bool SX126XLT::begin(int8_t pinNSS, int8_t pinNRESET, int8_t pinRFBUSY, int8_t pinDIO1, int8_t pinSW, uint8_t device)
{

#if not defined ARDUINO && not defined USE_ARDUPI
  wiringPiSetup() ;
  PRINTLN_CSTSTR("<<< using wiringPi >>>"); 
#endif

  //format 3 pins for Dorji, NSS, NRESET, RFBUSY, DIO1, SW  
  _NSS = pinNSS;
  _NRESET = pinNRESET;
  _RFBUSY = pinRFBUSY;
  _DIO1 = pinDIO1;
  _DIO2 = -1;
  _DIO3 = -1;
  _RXEN = -1;
  _TXEN = -1;
  _SW = pinSW;
  _Device = device;
  
  _TXDonePin = pinDIO1;        //this is defalt pin for sensing TX done
  _RXDonePin = pinDIO1;        //this is defalt pin for sensing RX done

  pinMode(_NSS, OUTPUT);
  digitalWrite(_NSS, HIGH);
  pinMode(_NRESET, OUTPUT);
  digitalWrite(_NRESET, LOW);
  pinMode(_RFBUSY, INPUT);


#ifdef SX126XDEBUGPINS
  PRINTLN_CSTSTR("format 3 Dorji begin()");
  PRINTLN_CSTSTR("SX126XLT constructor instantiated successfully");
  PRINT_CSTSTR("NSS ");
  PRINTLN_VALUE("%d",_NSS);
  PRINT_CSTSTR("NRESET ");
  PRINTLN_VALUE("%d",_NRESET);
  PRINT_CSTSTR("RFBUSY ");
  PRINTLN_VALUE("%d",_RFBUSY);
  PRINT_CSTSTR("DIO1 ");
  PRINTLN_VALUE("%d",_DIO1);
  PRINT_CSTSTR("DIO2 ");
  PRINTLN_VALUE("%d",_DIO2);
  PRINT_CSTSTR("DIO3 ");
  PRINTLN_VALUE("%d",_DIO3);
  PRINT_CSTSTR("RX_EN ");
  PRINTLN_VALUE("%d",_RXEN);
  PRINT_CSTSTR("TX_EN ");
  PRINTLN_VALUE("%d",_TXEN);
  PRINT_CSTSTR("SW ");
  PRINTLN_VALUE("%d",_SW);
#endif


  if (_DIO1 >= 0)
  {
    pinMode( _DIO1, INPUT);
  }
 
#ifdef SX126XDEBUGPINS
  PRINTLN_CSTSTR("RX_EN & TX_EN switching disabled");
#endif
  
  _rxtxpinmode = false;

   
  if (_SW >= 0)
  {
    pinMode( _SW, OUTPUT);                     //Dorji devices have an RW pin that needs to be set high to power antenna switch
    digitalWrite(_SW, HIGH);
  }
    
  resetDevice();
  
  if (checkDevice())
  {
    return true;
  }

  return false;
}


bool SX126XLT::begin(int8_t pinNSS, int8_t pinNRESET, int8_t pinRFBUSY, int8_t pinDIO1, int8_t pinRXEN, int8_t pinTXEN, uint8_t device)
{

#if not defined ARDUINO && not defined USE_ARDUPI
  wiringPiSetup() ;
  PRINTLN_CSTSTR("<<< using wiringPi >>>"); 
#endif

  //format 4 pins for Ebyte (not tested) , NSS, NRESET, RFBUSY, DIO1, RXEN, TXEN
  _NSS = pinNSS;
  _NRESET = pinNRESET;
  _RFBUSY = pinRFBUSY;
  _DIO1 = pinDIO1;
  _DIO2 = -1;
  _DIO3 = -1;
  _RXEN = pinRXEN;
  _TXEN = pinTXEN;
  _SW = -1;
  _Device = device;
  
  _TXDonePin = pinDIO1;        //this is defalt pin for sensing TX done
  _RXDonePin = pinDIO1;        //this is defalt pin for sensing RX done

  pinMode(_NSS, OUTPUT);
  digitalWrite(_NSS, HIGH);
  pinMode(_NRESET, OUTPUT);
  digitalWrite(_NRESET, LOW);
  pinMode(_RFBUSY, INPUT);


#ifdef SX126XDEBUGPINS
  PRINTLN_CSTSTR("format 4 Ebyte begin()");
  PRINTLN_CSTSTR("SX126XLT constructor instantiated successfully");
  PRINT_CSTSTR("NSS ");
  PRINTLN_VALUE("%d",_NSS);
  PRINT_CSTSTR("NRESET ");
  PRINTLN_VALUE("%d",_NRESET);
  PRINT_CSTSTR("RFBUSY ");
  PRINTLN_VALUE("%d",_RFBUSY);
  PRINT_CSTSTR("DIO1 ");
  PRINTLN_VALUE("%d",_DIO1);
  PRINT_CSTSTR("DIO2 ");
  PRINTLN_VALUE("%d",_DIO2);
  PRINT_CSTSTR("DIO3 ");
  PRINTLN_VALUE("%d",_DIO3);
  PRINT_CSTSTR("RX_EN ");
  PRINTLN_VALUE("%d",_RXEN);
  PRINT_CSTSTR("TX_EN ");
  PRINTLN_VALUE("%d",_TXEN);
  PRINT_CSTSTR("SW ");
  PRINTLN_VALUE("%d",_SW);
#endif


  if (_DIO1 >= 0)
  {
    pinMode( _DIO1, INPUT);
  }
  
  if ((_RXEN >= 0) && (_TXEN >= 0))
  {
#ifdef SX126XDEBUGPINS
    PRINTLN_CSTSTR("RX_EN & TX_EN switching enabled");
#endif
    pinMode(_RXEN, OUTPUT);
    pinMode(_TXEN, OUTPUT);
    _rxtxpinmode = true;
  }
  else
  {
#ifdef SX126XDEBUGPINS
    PRINTLN_CSTSTR("RX_EN & TX_EN switching disabled");
#endif
    _rxtxpinmode = false;
  }

   
  if (_SW >= 0)
  {
    pinMode( _SW, OUTPUT);                     //Dorji devices have an RW pin that needs to be set high to power antenna switch
    digitalWrite(_SW, HIGH);
  }
  
  resetDevice();
  if (checkDevice())
  {
    return true;
  }

  return false;
}


void SX126XLT::checkBusy()
{
#ifdef SX126XDEBUG1
  //PRINTLN_CSTSTR("checkBusy()");
#endif

  uint8_t busy_timeout_cnt;
  busy_timeout_cnt = 0;

  while (digitalRead(_RFBUSY))
  {
    delay(1);
    busy_timeout_cnt++;
    

    //this function checks for a timeout on the busy pin
    //if there is a timeout the device is set back to the saved settings
    //the fuction is of limited benefit, since you cannot know at which stage of the 
    //operation the timeout occurs, so operation could resume 
    if (busy_timeout_cnt > 10) //wait 10mS for busy to complete
    {
      busy_timeout_cnt = 0;
#ifdef DEBUGBUSY
      PRINTLN_CSTSTR("ERROR - Busy Timeout!");
#endif
      resetDevice();          //reset device
      setMode(MODE_STDBY_RC);
      config();               //re-run saved config
      break;
    }
  }
}


void SX126XLT::writeCommand(uint8_t Opcode, uint8_t *buffer, uint16_t size)
{
#ifdef SX126XDEBUG1
  //PRINTLN_CSTSTR("writeCommand()");
#endif

  uint8_t index;
  checkBusy();

#ifdef USE_SPI_TRANSACTION     //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);
#if defined ARDUINO || defined USE_ARDUPI   
  SPI.transfer(Opcode);

  for (index = 0; index < size; index++)
  {
    SPI.transfer(buffer[index]);
  }
#else
	uint8_t spibuf[size+1];
	spibuf[0] = Opcode;
	for (index = 0; index < size; index++)
	{
		spibuf[1+index]=buffer[index];
	}		
	wiringPiSPIDataRW(SPI_CHANNEL, spibuf, size+1);
#endif 
  digitalWrite(_NSS, HIGH);

#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif

  if (Opcode != RADIO_SET_SLEEP)
  {
    checkBusy();
  }
}


void SX126XLT::readCommand(uint8_t Opcode, uint8_t *buffer, uint16_t size)
{
#ifdef SX126XDEBUG1
  //PRINTLN_CSTSTR("readCommand()");
#endif

  uint8_t i;
  checkBusy();

#ifdef USE_SPI_TRANSACTION     //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);
#if defined ARDUINO || defined USE_ARDUPI  
  SPI.transfer(Opcode);
  SPI.transfer(0xFF);

  for ( i = 0; i < size; i++ )
  {
    *(buffer + i) = SPI.transfer(0xFF);
  }
#else
	uint8_t spibuf[size+2];
	spibuf[0] = Opcode;
	spibuf[1] = 0xFF;
	for (i = 0; i < size; i++)
	{
		spibuf[2+i]=0xFF;
	}		
	wiringPiSPIDataRW(SPI_CHANNEL, spibuf, size+2);
	for (i = 0; i < size; i++)
	{
		*(buffer + i) = spibuf[2+i];
	} 
#endif 
  digitalWrite(_NSS, HIGH);

#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif
}


void SX126XLT::writeRegisters(uint16_t address, uint8_t *buffer, uint16_t size)
{
#ifdef SX126XDEBUG1
  //PRINTLN_CSTSTR("writeRegisters()");
#endif
  uint8_t addr_l, addr_h;
  uint8_t i;

  addr_l = address & 0xff;
  addr_h = address >> 8;
  checkBusy();

#ifdef USE_SPI_TRANSACTION     //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);
#if defined ARDUINO || defined USE_ARDUPI  
  SPI.transfer(RADIO_WRITE_REGISTER);
  SPI.transfer(addr_h);   //MSB
  SPI.transfer(addr_l);   //LSB

  for (i = 0; i < size; i++)
  {
    SPI.transfer(buffer[i]);
  }
#else
	uint8_t spibuf[size+3];
	spibuf[0] = RADIO_WRITE_REGISTER;
	spibuf[1] = addr_h;
	spibuf[2] = addr_l;
	for (i = 0; i < size; i++)
	{
		spibuf[3+i]=buffer[i];
	}		
	wiringPiSPIDataRW(SPI_CHANNEL, spibuf, size+3);
#endif
  digitalWrite(_NSS, HIGH);

#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif
}


void SX126XLT::writeRegister(uint16_t address, uint8_t value)
{
#ifdef SX126XDEBUG1
  //PRINTLN_CSTSTR("writeRegisters()");
#endif

  writeRegisters( address, &value, 1 );
}

void SX126XLT::readRegisters(uint16_t address, uint8_t *buffer, uint16_t size)
{
#ifdef SX126XDEBUG1
  //PRINTLN_CSTSTR("readRegisters()");
#endif

  uint16_t index;
  uint8_t addr_l, addr_h;

  addr_h = address >> 8;
  addr_l = address & 0x00FF;
  checkBusy();

#ifdef USE_SPI_TRANSACTION     //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);
#if defined ARDUINO || defined USE_ARDUPI  
  SPI.transfer(RADIO_READ_REGISTER);
  SPI.transfer(addr_h);               //MSB
  SPI.transfer(addr_l);               //LSB
  SPI.transfer(0xFF);
  for (index = 0; index < size; index++)
  {
    *(buffer + index) = SPI.transfer(0xFF);
  }
#else
	uint8_t spibuf[size+4];
	spibuf[0] = RADIO_READ_REGISTER;
	spibuf[1] = addr_h;
	spibuf[2] = addr_l;
	spibuf[3] = 0xFF;
	for (index = 0; index < size; index++)
	{
		spibuf[4+index]= 0xFF;
	}		
	wiringPiSPIDataRW(SPI_CHANNEL, spibuf, size+4);
	for (index = 0; index < size; index++)
	{
		*(buffer + index) = spibuf[4+index];
	}  	
#endif
  digitalWrite(_NSS, HIGH);

#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif
}


uint8_t SX126XLT::readRegister(uint16_t address)
{
#ifdef SX126XDEBUG1
  //PRINTLN_CSTSTR("readRegister()");
#endif

  uint8_t data;

  readRegisters(address, &data, 1);
  return data;
}

void SX126XLT::resetDevice()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("resetDevice()");
#endif

  delay(10);
  digitalWrite(_NRESET, LOW);
  delay(2);
  digitalWrite(_NRESET, HIGH);
  delay(25);
  checkBusy();
}



bool SX126XLT::checkDevice()
{
  //check there is a device out there, writes a register and reads back
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("checkDevice()");
#endif

  uint8_t Regdata1, Regdata2;
  Regdata1 = readRegister(0x88e);               //low byte of frequency setting
  writeRegister(0x88e, (Regdata1 + 1));
  Regdata2 = readRegister(0x88e);               //read changed value back
  writeRegister(0x88e, Regdata1);               //restore register to original value

  if (Regdata2 == (Regdata1 + 1))
  {
    return true;
  }
  else
  {
    return false;
  }
}

void SX126XLT::setupLoRa(uint32_t frequency, int32_t offset, uint8_t modParam1, uint8_t modParam2, uint8_t  modParam3, uint8_t modParam4)
{
  //order of passed parameters is, frequency, offset, spreadingfactor, bandwidth, coderate, optimisation

#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("setupLoRa()");
#endif
  setMode(MODE_STDBY_RC);
  setRegulatorMode(USE_DCDC);
  setPaConfig(0x04, PAAUTO, _Device);         //use _Device, saved by begin.
  setDIO3AsTCXOCtrl(TCXO_CTRL_3_3V);
  calibrateDevice(ALLDevices);                //is required after setting TCXO
  calibrateImage(frequency);
  setDIO2AsRfSwitchCtrl();
  setPacketType(PACKET_TYPE_LORA);
  setRfFrequency(frequency, offset);
  setModulationParams(modParam1, modParam2, modParam3, modParam4);
  setBufferBaseAddress(0, 0);
  setPacketParams(8, LORA_PACKET_VARIABLE_LENGTH, 255, LORA_CRC_ON, LORA_IQ_NORMAL);
  setDioIrqParams(IRQ_RADIO_ALL, (IRQ_TX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);   //set for IRQ on TX done and timeout on DIO1
  setHighSensitivity();  //set for maximum gain
  setSyncWord(LORA_MAC_PRIVATE_SYNCWORD);
}


void SX126XLT::setMode(uint8_t modeconfig)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("setMode()");
#endif

  checkBusy();

#ifdef USE_SPI_TRANSACTION     //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);
#if defined ARDUINO || defined USE_ARDUPI  
  SPI.transfer(RADIO_SET_STANDBY);
  SPI.transfer(modeconfig);
#else
	uint8_t spibuf[2];
	spibuf[0] = RADIO_SET_STANDBY;
	spibuf[1] = modeconfig;
	wiringPiSPIDataRW(SPI_CHANNEL, spibuf, 2);
#endif    
  digitalWrite(_NSS, HIGH);

#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif

  _OperatingMode = modeconfig;
}


void SX126XLT::setRegulatorMode(uint8_t mode)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("setRegulatorMode()");
#endif

  savedRegulatorMode = mode;

  writeCommand(RADIO_SET_REGULATORMODE, &mode, 1);
}


void SX126XLT::setPaConfig(uint8_t dutycycle, uint8_t hpMax, uint8_t device)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("setPaConfig()");
#endif


  uint8_t buffer[4];

  if (hpMax == PAAUTO)
  {
    if (device == DEVICE_SX1261)
    {
      hpMax = 0x00;
    }
    if (device == DEVICE_SX1262)
    {
      hpMax = 0x07;
    }
    if (device == DEVICE_SX1268)
    {
      hpMax = 0x07;
    }
  }

  if (_Device == DEVICE_SX1261)
  {
    device = 1;
  }
  else
  {
    device = 0;
  }

  buffer[0] = dutycycle;   //paDutyCycle
  buffer[1] = hpMax;       //hpMax:0x00~0x07; 7 for =22dbm
  buffer[2] = device;      //deviceSel: 0 = SX1262; 1 = SX1261; 0 = SX1268;
  buffer[3] = 0x01;        //reserved, always 0x01

  writeCommand(RADIO_SET_PACONFIG, buffer, 4);
}


void SX126XLT::setDIO3AsTCXOCtrl(uint8_t tcxoVoltage)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("setDIO3AsTCXOCtrl()");
#endif

  uint8_t buffer[4];

  buffer[0] = tcxoVoltage;
  buffer[1] = 0x00;
  buffer[2] = 0x00;
  buffer[3] = 0x64;

  writeCommand(RADIO_SET_TCXOMODE, buffer, 4);
}


void SX126XLT::calibrateDevice(uint8_t devices)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("calibrateDevice()");
#endif

  writeCommand(RADIO_CALIBRATE, &devices, 1);
  delay(5);                              //calibration time for all devices is 3.5mS, SX126x
}


void SX126XLT::calibrateImage(uint32_t freq)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("calibrateImage()");
#endif

  uint8_t calFreq[2];

  if ( freq > 900000000 )
  {
    calFreq[0] = 0xE1;
    calFreq[1] = 0xE9;
  }
  else if ( freq > 850000000 )
  {
    calFreq[0] = 0xD7;
    calFreq[1] = 0xD8;
  }
  else if ( freq > 770000000 )
  {
    calFreq[0] = 0xC1;
    calFreq[1] = 0xC5;
  }
  else if ( freq > 460000000 )
  {
    calFreq[0] = 0x75;
    calFreq[1] = 0x81;
  }
  else if ( freq > 425000000 )
  {
    calFreq[0] = 0x6B;
    calFreq[1] = 0x6F;
  }
  writeCommand( RADIO_CALIBRATEIMAGE, calFreq, 2 );
}


void SX126XLT::setDIO2AsRfSwitchCtrl()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("setDIO2AsRfSwitchCtrl()");
#endif

  uint8_t mode = 0x01;

  writeCommand(RADIO_SET_RFSWITCHMODE, &mode, 1);
}


void SX126XLT::setPacketType(uint8_t packettype )
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("setPacketType()");
#endif
  savedPacketType = packettype;

  writeCommand(RADIO_SET_PACKETTYPE, &packettype, 1);
}


void SX126XLT::setModulationParams(uint8_t modParam1, uint8_t modParam2, uint8_t  modParam3, uint8_t  modParam4)
{
  //order for LoRa is spreading factor, bandwidth, code rate, optimisation

#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("setModulationParams()");
#endif
  uint8_t regvalue;
  uint8_t buffer[4];

  regvalue = readRegister(REG_TX_MODULATION);

  savedModParam1 = modParam1;
  savedModParam2 = modParam2;
  savedModParam3 = modParam3;

  if (modParam2 == LORA_BW_500)
  {
    writeRegister(REG_TX_MODULATION, (regvalue & 0xFB));         //if bandwidth is 500k set bit 2 to 0, see datasheet 15.1.1
  }
  else
  {
    writeRegister(REG_TX_MODULATION, (regvalue | 0x04));         //if bandwidth is < 500k set bit 2 to 0 see datasheet 15.1.1
  }

  if (modParam4 == LDRO_AUTO)
  {
    modParam4 = returnOptimisation(modParam1, modParam2);        //pass Spreading factor then bandwidth to optimisation calc
  }

  savedModParam4 = modParam4;

  buffer[0] = modParam1;
  buffer[1] = modParam2;
  buffer[2] = modParam3;
  buffer[3] = modParam4;

  writeCommand(RADIO_SET_MODULATIONPARAMS, buffer, 4);

	/**************************************************************************
  	Added by C. Pham - Oct. 2020
	**************************************************************************/  

  //we need set CAD params each time the modulation params change
	setCadParams();
	
	/**************************************************************************
  	End by C. Pham - Oct. 2020
	**************************************************************************/	
}


uint8_t SX126XLT::returnOptimisation(uint8_t SpreadingFactor, uint8_t Bandwidth)
{
  //from the passed bandwidth (bandwidth) and spreading factor this routine
  //calculates whether low data rate optimisation should be on or off
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("returnOptimisation()");
#endif

  uint32_t tempBandwidth;
  float symbolTime;
  
  tempBandwidth = returnBandwidth(Bandwidth);
  
  symbolTime = calcSymbolTime(tempBandwidth, SpreadingFactor);

  if (symbolTime > 16)
  {
    return LDRO_ON;
  }
  else
  {
    return LDRO_OFF;
  }
}


uint32_t SX126XLT::returnBandwidth(uint8_t BWregvalue)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("returnBandwidth()");
#endif

  switch (BWregvalue)
  {
    case 0:
      return 7800;

    case 8:
      return 10400;

    case 1:
      return 15600;

    case 9:
      return 20800;

    case 2:
      return 31200;

    case 10:
      return 41700;

    case 3:
      return 62500;

    case 4:
      return 125000;

    case 5:
      return 250000;

    case 6:
      return 500000;

    default:
      break;
  }
  return 0xFFFF;                      //so that a bandwidth not set can be identified
}



float SX126XLT::calcSymbolTime(float Bandwidth, uint8_t SpreadingFactor)
{
  //calculates symbol time from passed bandwidth (lbandwidth) and Spreading factor (lSF)and returns in mS
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("calcSymbolTime()");
#endif

  float symbolTimemS;
  /**************************************************************************
	Modified by C. Pham - Oct. 2020
  **************************************************************************/  
  
  symbolTimemS = 1000 / (Bandwidth / ( 1 << SpreadingFactor));

  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/  
  
  /* original code
   ***************

  symbolTimemS = (Bandwidth / pow(2, SpreadingFactor));
  symbolTimemS = (1000 / symbolTimemS);
  
  */
  
  return symbolTimemS;
}


void SX126XLT::setBufferBaseAddress(uint8_t txBaseAddress, uint8_t rxBaseAddress)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("setBufferBaseAddress()");
#endif

  uint8_t buffer[2];

  buffer[0] = txBaseAddress;
  buffer[1] = rxBaseAddress;
  writeCommand(RADIO_SET_BUFFERBASEADDRESS, buffer, 2);
}


void SX126XLT::setPacketParams(uint16_t packetParam1, uint8_t  packetParam2, uint8_t packetParam3, uint8_t packetParam4, uint8_t packetParam5)
{
  //order is preamble, header type, packet length, CRC, IQ

#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("SetPacketParams()");
#endif

  uint8_t preambleMSB, preambleLSB;

  preambleMSB = packetParam1 >> 8;
  preambleLSB = packetParam1 & 0xFF;

  savedPacketParam1 = packetParam1;
  savedPacketParam2 = packetParam2;
  savedPacketParam3 = packetParam3;
  savedPacketParam4 = packetParam4;
  savedPacketParam5 = packetParam5;

  uint8_t buffer[9];
  buffer[0] = preambleMSB;
  buffer[1] = preambleLSB;
  buffer[2] = packetParam2;
  buffer[3] = packetParam3;
  buffer[4] = packetParam4;
  buffer[5] = packetParam5;
  buffer[6] = 0xFF;
  buffer[7] = 0xFF;
  buffer[8] = 0xFF;
  writeCommand(RADIO_SET_PACKETPARAMS, buffer, 9);
  //TODO C.Pham
  // Bit 2 at address 0x0736 must be set to “0” when using inverted IQ polarity; “1” when using standard IQ polarity
  // See 15.4.2 Workaround
}


void SX126XLT::setDioIrqParams(uint16_t irqMask, uint16_t dio1Mask, uint16_t dio2Mask, uint16_t dio3Mask )
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("setDioIrqParams()");
#endif

  savedIrqMask = irqMask;
  savedDio1Mask = dio1Mask;
  savedDio2Mask = dio2Mask;
  savedDio3Mask = dio3Mask;

  uint8_t buffer[8];

  buffer[0] = (uint8_t) (irqMask >> 8);
  buffer[1] = (uint8_t) (irqMask & 0xFF);
  buffer[2] = (uint8_t) (dio1Mask >> 8);
  buffer[3] = (uint8_t) (dio1Mask & 0xFF);
  buffer[4] = (uint8_t) (dio2Mask >> 8);
  buffer[5] = (uint8_t) (dio2Mask & 0xFF);
  buffer[6] = (uint8_t) (dio3Mask >> 8);
  buffer[7] = (uint8_t) (dio3Mask & 0xFF);
  writeCommand(RADIO_CFG_DIOIRQ, buffer, 8);
}


void SX126XLT::setHighSensitivity()
{
  //set RX Boosted gain mode
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("setHighSensitivity()");
#endif
  writeRegister( REG_RX_GAIN, BOOSTED_GAIN );   //max LNA gain, increase current by ~2mA for around ~3dB in sensivity
}

void SX126XLT::setLowPowerRX()
{
  //set RX power saving mode
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("setLowPowerRX()");
#endif

  writeRegister( REG_RX_GAIN, POWER_SAVE_GAIN ); // min LNA gain, reduce current by 2mA for around 3dB loss in sensivity
}


void SX126XLT::setSyncWord(uint16_t syncword)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("setSyncWord()");
#endif
  writeRegister( REG_LR_SYNCWORD, ( syncword >> 8 ) & 0xFF );
  writeRegister( REG_LR_SYNCWORD + 1, syncword & 0xFF );
}


void SX126XLT::printModemSettings()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("printModemSettings()");
#endif

  printDevice();
  PRINT_CSTSTR(",");
  PRINT_VALUE("%lu",getFreqInt());
  PRINT_CSTSTR("hz,SF");
  PRINT_VALUE("%d",getLoRaSF());
  PRINT_CSTSTR(",BW");
  PRINT_VALUE("%ld",returnBandwidth(savedModParam2));
  PRINT_CSTSTR(",CR4:");
  PRINT_VALUE("%d",(getLoRaCodingRate() + 4));
  PRINT_CSTSTR(",LDRO_");

  if (getOptimisation())
  {
    PRINT_CSTSTR("On");
  }
  else
  {
    PRINT_CSTSTR("Off");
  }

  PRINT_CSTSTR(",SyncWord_0x");
  PRINT_HEX("%X",getSyncWord());
  if (getInvertIQ() == LORA_IQ_INVERTED)
  {
    PRINT_CSTSTR(",IQInverted");
  }
  else
  {
    PRINT_CSTSTR(",IQNormal");
  }
  PRINT_CSTSTR(",Preamble_");
  PRINT_VALUE("%d",getPreamble());
}


uint32_t SX126XLT::getFreqInt()
{
  //get the current set device frequency from registers, return as long integer
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("getFreqInt()");
#endif

  uint8_t MsbH, MsbL, Mid, Lsb;
  uint32_t uinttemp;
  float floattemp;
  MsbH = readRegister(REG_RFFrequency31_24);
  MsbL = readRegister(REG_RFFrequency23_16);
  Mid = readRegister(REG_RFFrequency15_8);
  Lsb = readRegister(REG_RFFrequency7_0);
  floattemp = ( (MsbH * 0x1000000ul) + (MsbL * 0x10000ul) + (Mid * 0x100ul) + Lsb);
  floattemp = ((floattemp * FREQ_STEP) / 1000000ul);
  uinttemp = (uint32_t)(floattemp * 1000000);
  return uinttemp;
}


uint8_t SX126XLT::getLoRaCodingRate()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("getLoRaCodingRate");
#endif

  return savedModParam3;
}


uint8_t SX126XLT::getOptimisation()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("getOptimisation");
#endif

  return savedModParam4;
}


uint16_t SX126XLT::getSyncWord()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("getSyncWord");
#endif

  uint8_t msb, lsb;
  uint16_t syncword;
  msb = readRegister(REG_LR_SYNCWORD);
  lsb = readRegister(REG_LR_SYNCWORD + 1);
  syncword = (msb << 8) + lsb;

  return syncword;
}


uint16_t SX126XLT::getPreamble()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("getPreamble");
#endif

  return savedPacketParam1;
}


void SX126XLT::printOperatingSettings()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("printOperatingSettings()");
#endif

  printDevice();

  PRINT_CSTSTR(",PacketMode_");

  if (savedPacketType == PACKET_TYPE_LORA)
  {
    PRINT_CSTSTR("LoRa");
  }
  
  if (savedPacketType == PACKET_TYPE_GFSK)
  {
    PRINT_CSTSTR("GFSK");
  }
  
  if (getHeaderMode())
  {
    PRINT_CSTSTR(",Implicit");
  }
  else
  {
    PRINT_CSTSTR(",Explicit");
  }

   PRINT_CSTSTR(",LNAgain_");

  if (getLNAgain() == BOOSTED_GAIN)
  {
    PRINT_CSTSTR("Boosted");
  }
  else
  {
    PRINT_CSTSTR("Powersave");
  }

}


uint8_t SX126XLT::getHeaderMode()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("getHeaderMode");
#endif

  return savedPacketParam2;
}


uint8_t SX126XLT::getLNAgain()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("getLNAgain");
#endif

  return readRegister(REG_RX_GAIN);
}


void SX126XLT::setTxParams(int8_t TXpower, uint8_t RampTime)
{
  //note this routine does not check if power levels are valid for the module in use
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("setTxParams()");
#endif

  uint8_t buffer[2];

  /**************************************************************************
	Modified by C. Pham - Oct. 2020
  **************************************************************************/
  
	if (TXpower < 0)
		return;

  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/
  
  savedTXPower = TXpower;

  buffer[0] = TXpower;
  buffer[1] = (uint8_t)RampTime;
  writeCommand(RADIO_SET_TXPARAMS, buffer, 2);
}


void SX126XLT::setTx(uint32_t timeout)
{
  //SX126x base timeout in units of 15.625 µs
  //Note: timeout passed to function is in mS

#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("setTx()");
#endif
  uint8_t buffer[3];

  clearIrqStatus(IRQ_RADIO_ALL);

  if (_rxtxpinmode)
  {
   txEnable();
  }

  timeout = timeout << 6;         //timeout passed in mS, convert to units of 15.625us

  buffer[0] = (timeout >> 16) & 0xFF;
  buffer[1] = (timeout >> 8) & 0xFF;
  buffer[2] = timeout & 0xFF;
  
  writeCommand(RADIO_SET_TX, buffer, 3 );
  _OperatingMode = MODE_TX;
}


void SX126XLT::clearIrqStatus(uint16_t irqMask)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("clearIrqStatus()");
#endif

  uint8_t buffer[2];

  buffer[0] = (uint8_t) (irqMask >> 8);
  buffer[1] = (uint8_t) (irqMask & 0xFF);
  writeCommand(RADIO_CLR_IRQSTATUS, buffer, 2);
}


uint16_t SX126XLT::readIrqStatus()
{
#ifdef SX126XDEBUG1
  PRINT_CSTSTR("readIrqStatus()");
#endif

  uint16_t temp;
  uint8_t buffer[2];

  readCommand(RADIO_GET_IRQSTATUS, buffer, 2);
  temp = ((buffer[0] << 8) + buffer[1]);
  return temp;
}


uint16_t SX126XLT::CRCCCITT(uint8_t *buffer, uint8_t size, uint16_t start)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("CRCCCITT()");
#endif

  uint16_t index, libraryCRC;
  uint8_t j;

  libraryCRC = start;                                  //start value for CRC16

  for (index = 0; index < size; index++)
  {
    libraryCRC ^= (((uint16_t)buffer[index]) << 8);
    for (j = 0; j < 8; j++)
    {
      if (libraryCRC & 0x8000)
        libraryCRC = (libraryCRC << 1) ^ 0x1021;
      else
        libraryCRC <<= 1;
    }
  }

  return libraryCRC;
}


uint8_t SX126XLT::transmit(uint8_t *txbuffer, uint8_t size, uint32_t txtimeout, int8_t txpower, uint8_t wait)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("transmit()");
#endif
  uint8_t index;
  uint8_t bufferdata;

  if (size == 0)
  {
    return false;
  }

  setMode(MODE_STDBY_RC);
  setBufferBaseAddress(0, 0);
  
  checkBusy();

#ifdef USE_SPI_TRANSACTION     //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);
#if defined ARDUINO || defined USE_ARDUPI  
  SPI.transfer(RADIO_WRITE_BUFFER);
  SPI.transfer(0);

  for (index = 0; index < size; index++)
  {
    bufferdata = txbuffer[index];
    SPI.transfer(bufferdata);
  }
#else
	uint8_t spibuf[size+2];
	spibuf[0] = RADIO_WRITE_BUFFER;
	spibuf[1] = 0;			
	
	for (index = 0; index < size; index++)
	{
		bufferdata = txbuffer[index];	
		spibuf[2+index]=bufferdata;
	}	
	
	wiringPiSPIDataRW(SPI_CHANNEL, spibuf, size+2); 
#endif
  digitalWrite(_NSS, HIGH);

#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif

  _TXPacketL = size;
  writeRegister(REG_LR_PAYLOADLENGTH, _TXPacketL);
  setTxParams(txpower, RADIO_RAMP_200_US);
  
  setDioIrqParams(IRQ_RADIO_ALL, (IRQ_TX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);   //set for IRQ on TX done and timeout on DIO1
  setTx(txtimeout);                                                          //this starts the TX

  /**************************************************************************
	Added by C. Pham - Oct. 2020
  **************************************************************************/  
  
  // increment packet sequence number
  _TXSeqNo++;

  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/
  
  if (!wait)
  {
    return _TXPacketL;
  }

  /**************************************************************************
	Modified by C. Pham - Oct. 2020
  **************************************************************************/

#ifdef USE_POLLING

	uint16_t regdata;
	
	do {
		regdata = readIrqStatus();
			
	} while ( !(regdata & IRQ_TX_DONE) && !(regdata & IRQ_RX_TX_TIMEOUT) );
	
	if (regdata & IRQ_RX_TX_TIMEOUT )                        //check for timeout
		
#else		 
  
  while (!digitalRead(_TXDonePin));       //Wait for DIO1 to go high
  
  if (readIrqStatus() & IRQ_RX_TX_TIMEOUT )                        //check for timeout
#endif  

  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/
  {
    return 0;
  }
  else
  {
    return _TXPacketL;
  }
}


void SX126XLT::printIrqStatus()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("printIrqStatus()");
#endif

  uint16_t _IrqStatus;
  _IrqStatus = readIrqStatus();

  //0x0001
  if (_IrqStatus & IRQ_TX_DONE)
  {
    PRINT_CSTSTR(",IRQ_TX_DONE");
  }

  //0x0002
  if (_IrqStatus & IRQ_RX_DONE)
  {
    PRINT_CSTSTR(",IRQ_RX_DONE");
  }

  //0x0004
  if (_IrqStatus & IRQ_PREAMBLE_DETECTED)
  {
    PRINT_CSTSTR(",IRQ_PREAMBLE_DETECTED");
  }

  //0x0008
  if (_IrqStatus & IRQ_SYNCWORD_VALID)
  {
    PRINT_CSTSTR(",IRQ_SYNCWORD_VALID");
  }

  //0x0010
  if (_IrqStatus & IRQ_HEADER_VALID)
  {
    PRINT_CSTSTR(",IRQ_HEADER_VALID");
  }

  //0x0020
  if (_IrqStatus & IRQ_HEADER_ERROR)
  {
    PRINT_CSTSTR(",IRQ_HEADER_ERROR");
  }

  //0x0040
  if (_IrqStatus & IRQ_CRC_ERROR)
  {
    PRINT_CSTSTR(",IRQ_CRC_ERROR");
  }

  //0x0080
  if (_IrqStatus & IRQ_CAD_DONE)
  {
    PRINT_CSTSTR(",IRQ_CAD_DONE");
  }

  //0x0100
  if (_IrqStatus & IRQ_CAD_ACTIVITY_DETECTED)
  {
    PRINT_CSTSTR(",IRQ_CAD_ACTIVITY_DETECTED");
  }

  //0x0200
  if (_IrqStatus & IRQ_RX_TX_TIMEOUT)
  {
    PRINT_CSTSTR(",IRQ_RX_TX_TIMEOUT");
  }

}


void SX126XLT::printRegisters(uint16_t Start, uint16_t End)
{
  //prints the contents of SX1262 registers to serial monitor
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("printRegisters()");
#endif

  uint16_t Loopv1, Loopv2, RegData;

  PRINT_CSTSTR("Reg    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F");
  PRINTLN;

  for (Loopv1 = Start; Loopv1 <= End;)           //32 lines
  {
    PRINT_CSTSTR("0x");
    PRINT_HEX("%X",(Loopv1));                 //print the register number
    PRINT_CSTSTR("  ");
    for (Loopv2 = 0; Loopv2 <= 15; Loopv2++)
    {
      RegData = readRegister(Loopv1);
      if (RegData < 0x10)
      {
        PRINT_CSTSTR("0");
      }
      PRINT_HEX("%X",RegData);                //print the register number
      PRINT_CSTSTR(" ");
      Loopv1++;
    }
    PRINTLN;
  }
}


void SX126XLT::printDevice()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("printDevice()");
#endif

  switch (_Device)
  {
    case DEVICE_SX1261:
      PRINT_CSTSTR("SX1261");
      break;

    case DEVICE_SX1262:
      PRINT_CSTSTR("SX1262");
      break;

    case DEVICE_SX1268:
      PRINT_CSTSTR("SX1268");
      break;

    default:
      PRINT_CSTSTR("Unknown Device");

  }
}


bool SX126XLT::config()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("config()");
#endif

  resetDevice();
  setMode(MODE_STDBY_RC);
  setRegulatorMode(savedRegulatorMode);
  setPacketType(savedPacketType);
  setRfFrequency(savedFrequency, savedOffset);
  setModulationParams(savedModParam1, savedModParam2, savedModParam3, LDRO_ON);
  setPacketParams(savedPacketParam1, savedPacketParam2, savedPacketParam3, savedPacketParam4, savedPacketParam5);
  setDioIrqParams(savedIrqMask, savedDio1Mask, savedDio2Mask, savedDio3Mask);       //set for IRQ on RX done on DIO1
  _TXPacketL = 0;
  _RXPacketL = 0;
  return true;
}


void SX126XLT::setRfFrequency( uint32_t frequency, int32_t offset )
{
  //Note RF_Freq = freq_reg*32M/(2^25)-----> freq_reg = (RF_Freq * (2^25))/32

#ifdef SX126XDEBUG1
  PRINT_CSTSTR("setRfFrequency()  ");
  PRINTLN_VALUE("%lu",frequency + offset);
#endif

  uint8_t buffer[4];
  uint32_t localfrequencyRegs;

  savedFrequency = frequency;
  savedOffset = offset;

  localfrequencyRegs = frequency + offset;

  localfrequencyRegs = ( uint32_t )( ( double )localfrequencyRegs / ( double )FREQ_STEP );
  
  savedFrequencyReg = localfrequencyRegs;
  
  buffer[0] = (localfrequencyRegs >> 24) & 0xFF; //MSB
  buffer[1] = (localfrequencyRegs >> 16) & 0xFF;
  buffer[2] = (localfrequencyRegs >> 8) & 0xFF;
  buffer[3] = localfrequencyRegs & 0xFF;//LSB
  
  _freqregH = buffer[0];
  _freqregMH = buffer[1];
  _freqregML = buffer[2];
  _freqregL = buffer[3];

  writeCommand(RADIO_SET_RFFREQUENCY, buffer, 4);
}


uint8_t SX126XLT::getLoRaSF()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("getLoRaSF()");
#endif

  return savedModParam1;
}


uint8_t SX126XLT::getInvertIQ()
{
  //IQ mode reg 0x33
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("getInvertIQ");
#endif

  return readRegister(REG_IQ_POLARITY_SETUP);
}


void SX126XLT::rxEnable()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("rxEnable()");
#endif

  digitalWrite(_RXEN, HIGH);
  digitalWrite(_TXEN, LOW);
}


void SX126XLT::txEnable()
{
#ifdef SX126XDEBUGPINS
  PRINTLN_CSTSTR("txEnable()");
#endif

  digitalWrite(_RXEN, LOW);
  digitalWrite(_TXEN, HIGH);
}

void SX126XLT::printASCIIPacket(uint8_t *buffer, uint8_t size)
{
#ifdef SX126XDEBUGPINS
  PRINTLN_CSTSTR("printASCIIPacket()");
#endif

  uint8_t index;

  for (index = 0; index < size; index++)
  {
    Serial.write(buffer[index]);
  }

}


uint8_t SX126XLT::receive(uint8_t *rxbuffer, uint8_t size, uint32_t rxtimeout, uint8_t wait)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("receive()");
#endif

  uint8_t index, RXstart, RXend;
  uint16_t regdata;
  uint8_t buffer[2];

  setDioIrqParams(IRQ_RADIO_ALL, (IRQ_RX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);  //set for IRQ on RX done or timeout
  setRx(rxtimeout);

  if (!wait)
  {
    return 0;                             //not wait requested so no packet length to pass
  }

  /**************************************************************************
	Modified by C. Pham - Oct. 2020
  **************************************************************************/

#ifdef USE_POLLING

	do {
		regdata = readIrqStatus();
		
		if (regdata & IRQ_HEADER_VALID)
			_RXTimestamp=millis();
		
		delay(1);
			
	} while ( !(regdata & IRQ_RX_DONE) && !(regdata & IRQ_RX_TX_TIMEOUT) );

  setMode(MODE_STDBY_RC);                 //ensure to stop further packet reception
  
	if (regdata & IRQ_RX_DONE) 
		_RXDoneTimestamp=millis();
		
#else		 
  
  while (!digitalRead(_RXDonePin));       //Wait for DIO1 to go high

	_RXDoneTimestamp=millis();
	
  setMode(MODE_STDBY_RC);                 //ensure to stop further packet reception

  regdata = readIrqStatus();
  
#endif  

  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/
  
  if ( (regdata & IRQ_HEADER_ERROR) | (regdata & IRQ_CRC_ERROR) | (regdata & IRQ_RX_TX_TIMEOUT ) ) //check if any of the preceding IRQs is set
  {
    //packet is errored somewhere so return 0
    return 0;
  }

  readCommand(RADIO_GET_RXBUFFERSTATUS, buffer, 2);
  _RXPacketL = buffer[0];

  if (_RXPacketL > size)                 //check passed buffer is big enough for packet
  {
    _RXPacketL = size;                   //truncate packet if not enough space
  }

  RXstart = buffer[1];

  RXend = RXstart + _RXPacketL;

  checkBusy();
  
#ifdef USE_SPI_TRANSACTION           //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);             //start the burst read
#if defined ARDUINO || defined USE_ARDUPI  
  SPI.transfer(RADIO_READ_BUFFER);
  SPI.transfer(RXstart);
  SPI.transfer(0xFF);

  for (index = RXstart; index < RXend; index++)
  {
    regdata = SPI.transfer(0);
    rxbuffer[index] = regdata;
  }
#else
	uint8_t spibuf[_RXPacketL+3];
	spibuf[0] = RADIO_READ_BUFFER;
	spibuf[1] = RXstart;
	spibuf[2] = 0xFF;

  for (index = RXstart; index < RXend; index++)
	{
		spibuf[3+index]=0x00;
	}	
	
	wiringPiSPIDataRW(SPI_CHANNEL, spibuf, _RXPacketL+3);

  for (index = RXstart; index < RXend; index++)
  {
    rxbuffer[index] = spibuf[3+index];
  }  
#endif
  digitalWrite(_NSS, HIGH);

#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif

  return _RXPacketL;                     //so we can check for packet having enough buffer space
}

/**************************************************************************
Modified by C. Pham - Oct. 2020
to return int8_t instead of uint8_t
**************************************************************************/

int8_t SX126XLT::readPacketRSSI()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("readPacketRSSI()");
#endif

  uint8_t status[5];

  readCommand(RADIO_GET_PACKETSTATUS, status, 5) ;
  _PacketRSSI = -status[0] / 2;

  return _PacketRSSI;
}


int8_t SX126XLT::readPacketSNR()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("readPacketSNR()");
#endif

  uint8_t status[5];

  readCommand(RADIO_GET_PACKETSTATUS, status, 5) ;

  if ( status[1] < 128 )
  {
    _PacketSNR = status[1] / 4 ;
  }
  else
  {
    _PacketSNR = (( status[1] - 256 ) / 4);
  }
  
  return _PacketSNR;
}

/**************************************************************************
End by C. Pham - Oct. 2020
**************************************************************************/

uint8_t SX126XLT::readRXPacketL()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("readRXPacketL()");
#endif

  uint8_t buffer[2];

  readCommand(RADIO_GET_RXBUFFERSTATUS, buffer, 2);
  _RXPacketL = buffer[0];
  return _RXPacketL;
}


void SX126XLT::setRx(uint32_t timeout)
{
  //SX126x base timeout in units of 15.625 µs
  //timeout passed to function in mS
  //range is 1mS to 262 seconds

#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("setRx()");
#endif
  uint8_t buffer[3];
  clearIrqStatus(IRQ_RADIO_ALL);

  if (_rxtxpinmode)
  {
    rxEnable();
  }

  timeout = timeout << 6;           //timeout passed in mS, multiply by 64 to convert units of 15.625us to 1mS

  buffer[0] = (timeout >> 16) & 0xFF;
  buffer[1] = (timeout >> 8) & 0xFF;
  buffer[2] = timeout & 0xFF;
  writeCommand(RADIO_SET_RX, buffer, 3 );
}

/***************************************************************************
//Start direct access SX buffer routines
***************************************************************************/

//WARNING/TODO C. Pham. Now with wiringPi, successive SPI.transfer() calls is not working anymore!
//Therefore writing/reading directly from buffer is not supported 

void SX126XLT::startWriteSXBuffer(uint8_t ptr)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("startWriteSXBuffer()");
#endif

  _TXPacketL = 0;                   //this variable used to keep track of bytes written
  setMode(MODE_STDBY_RC);
  setBufferBaseAddress(ptr, 0);     //TX,RX
  
  checkBusy();
  
#ifdef USE_SPI_TRANSACTION     //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif
  
  digitalWrite(_NSS, LOW);
  SPI.transfer(RADIO_WRITE_BUFFER);
  SPI.transfer(ptr);
  //SPI interface ready for byte to write to buffer
}


uint8_t  SX126XLT::endWriteSXBuffer()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("endWriteSXBuffer()");
#endif

  digitalWrite(_NSS, HIGH);
  
#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif

  return _TXPacketL; 
}


void SX126XLT::startReadSXBuffer(uint8_t ptr)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("startReadSXBuffer");
#endif

  _RXPacketL = 0;
  
  checkBusy();
  
#ifdef USE_SPI_TRANSACTION             //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);               //start the burst read
  SPI.transfer(RADIO_READ_BUFFER);
  SPI.transfer(ptr);
  SPI.transfer(0xFF);

  //next line would be data = SPI.transfer(0);
  //SPI interface ready for byte to read from
}


uint8_t SX126XLT::endReadSXBuffer()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("endReadSXBuffer()");
#endif

  digitalWrite(_NSS, HIGH);
  
#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif
  
  return _RXPacketL;
}


void SX126XLT::writeUint8(uint8_t x)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("writeUint8()");
#endif

  SPI.transfer(x);

  _TXPacketL++;                        //increment count of bytes written
}

uint8_t SX126XLT::readUint8()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("readUint8()");
#endif
  byte x;

  x = SPI.transfer(0);

  _RXPacketL++;                        //increment count of bytes read
  return (x);
}


void SX126XLT::writeInt8(int8_t x)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("writeInt8()");
#endif

  SPI.transfer(x);

  _TXPacketL++;                        //increment count of bytes written
}


int8_t SX126XLT::readInt8()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("readInt8()");
#endif
  int8_t x;

  x = SPI.transfer(0);

  _RXPacketL++;                        //increment count of bytes read
  return (x);
}


void SX126XLT::writeInt16(int16_t x)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("writeInt16()");
#endif

  SPI.transfer(lowByte(x));
  SPI.transfer(highByte(x));

  _TXPacketL = _TXPacketL + 2;         //increment count of bytes written
}


int16_t SX126XLT::readInt16()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("readInt16()");
#endif
  byte lowbyte, highbyte;

  lowbyte = SPI.transfer(0);
  highbyte = SPI.transfer(0);

  _RXPacketL = _RXPacketL + 2;         //increment count of bytes read
  return ((highbyte << 8) + lowbyte);
}


void SX126XLT::writeUint16(uint16_t x)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("writeUint16()");
#endif

  SPI.transfer(lowByte(x));
  SPI.transfer(highByte(x));

  _TXPacketL = _TXPacketL + 2;         //increment count of bytes written
}


uint16_t SX126XLT::readUint16()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("writeUint16()");
#endif
  byte lowbyte, highbyte;

  lowbyte = SPI.transfer(0);
  highbyte = SPI.transfer(0);

  _RXPacketL = _RXPacketL + 2;         //increment count of bytes read
  return ((highbyte << 8) + lowbyte);
}


void SX126XLT::writeInt32(int32_t x)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("writeInt32()");
#endif

  byte i, j;

  union
  {
    byte b[4];
    int32_t f;
  } data;
  data.f = x;

  for (i = 0; i < 4; i++)
  {
    j = data.b[i];
    SPI.transfer(j);
  }

  _TXPacketL = _TXPacketL + 4;         //increment count of bytes written
}


int32_t SX126XLT::readInt32()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("readInt32()");
#endif

  byte i, j;

  union
  {
    byte b[4];
    int32_t f;
  } readdata;

  for (i = 0; i < 4; i++)
  {
    j = SPI.transfer(0);
    readdata.b[i] = j;
  }
  _RXPacketL = _RXPacketL + 4;         //increment count of bytes read
  return readdata.f;
}


void SX126XLT::writeUint32(uint32_t x)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("writeUint32()");
#endif

  byte i, j;

  union
  {
    byte b[4];
    uint32_t f;
  } data;
  data.f = x;

  for (i = 0; i < 4; i++)
  {
    j = data.b[i];
    SPI.transfer(j);
  }

  _TXPacketL = _TXPacketL + 4;         //increment count of bytes written
}


uint32_t SX126XLT::readUint32()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("readUint32()");
#endif

  byte i, j;

  union
  {
    byte b[4];
    uint32_t f;
  } readdata;

  for (i = 0; i < 4; i++)
  {
    j = SPI.transfer(0);
    readdata.b[i] = j;
  }
  _RXPacketL = _RXPacketL + 4;         //increment count of bytes read
  return readdata.f;
}


void SX126XLT::writeFloat(float x)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("writeFloat()");
#endif

  byte i, j;

  union
  {
    byte b[4];
    float f;
  } data;
  data.f = x;

  for (i = 0; i < 4; i++)
  {
    j = data.b[i];
    SPI.transfer(j);
  }

  _TXPacketL = _TXPacketL + 4;         //increment count of bytes written
}


float SX126XLT::readFloat()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("readFloat()");
#endif

  byte i, j;

  union
  {
    byte b[4];
    float f;
  } readdata;

  for (i = 0; i < 4; i++)
  {
    j = SPI.transfer(0);
    readdata.b[i] = j;
  }
  _RXPacketL = _RXPacketL + 4;         //increment count of bytes read
  return readdata.f;
}


uint8_t SX126XLT::transmitSXBuffer(uint8_t startaddr, uint8_t length, uint32_t txtimeout, int8_t txpower, uint8_t wait)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("transmitSXBuffer()");
#endif

  setBufferBaseAddress(startaddr, 0);          //TX, RX

  setPacketParams(savedPacketParam1, savedPacketParam2, length, savedPacketParam4, savedPacketParam5);
  setTxParams(txpower, RAMP_TIME);
  
  setDioIrqParams(IRQ_RADIO_ALL, (IRQ_TX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);   //set for IRQ on TX done and timeout on DIO1
  setTx(txtimeout);                            //this starts the TX

  /**************************************************************************
	Added by C. Pham - Oct. 2020
  **************************************************************************/  
  
  // increment packet sequence number
  _TXSeqNo++;

  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/
  
  if (!wait)
  {
    return _TXPacketL;
  }

  /**************************************************************************
	Modified by C. Pham - Oct. 2020
  **************************************************************************/

#ifdef USE_POLLING

	uint16_t regdata;
	
	do {
		regdata = readIrqStatus();
			
	} while ( !(regdata & IRQ_TX_DONE) && !(regdata & IRQ_RX_TX_TIMEOUT) );
	
	if (regdata & IRQ_RX_TX_TIMEOUT )                        //check for timeout
		
#else		 
  
  while (!digitalRead(_TXDonePin));       //Wait for DIO1 to go high
  
  if (readIrqStatus() & IRQ_RX_TX_TIMEOUT )                        //check for timeout
#endif  

  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/
  {
    return 0;
  }
  else
  {
    return _TXPacketL;
  }
}


void SX126XLT::writeBuffer(uint8_t *txbuffer, uint8_t size)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("writeBuffer()");
#endif

  uint8_t index, regdata;

  _TXPacketL = _TXPacketL + size;      //these are the number of bytes that will be added

  size--;                              //loose one byte from size, the last byte written MUST be a 0

  for (index = 0; index < size; index++)
  {
    regdata = txbuffer[index];
    SPI.transfer(regdata);
  }

  SPI.transfer(0);                     //this ensures last byte of buffer written really is a null (0)

}


uint8_t SX126XLT::receiveSXBuffer(uint8_t startaddr, uint32_t rxtimeout, uint8_t wait )
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("receiveSXBuffer()");
#endif

  uint16_t regdata;
  uint8_t buffer[2];

  setMode(MODE_STDBY_RC);
  
  setBufferBaseAddress(0, startaddr);               //order is TX RX
  
  setDioIrqParams(IRQ_RADIO_ALL, (IRQ_RX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);  //set for IRQ on RX done or timeout
  setRx(rxtimeout);                                 //no actual RX timeout in this function

  if (!wait)
  {
    return 0;
  }
  
  /**************************************************************************
	Modified by C. Pham - Oct. 2020
  **************************************************************************/

#ifdef USE_POLLING

	do {
		regdata = readIrqStatus();
		
		if (regdata & IRQ_HEADER_VALID)
			_RXTimestamp=millis();
		
		delay(1);
			
	} while ( !(regdata & IRQ_RX_DONE) && !(regdata & IRQ_RX_TX_TIMEOUT) );

  setMode(MODE_STDBY_RC);                 //ensure to stop further packet reception
  
	if (regdata & IRQ_RX_DONE) 
		_RXDoneTimestamp=millis();
		
#else		 
  
  while (!digitalRead(_RXDonePin));       //Wait for DIO1 to go high

	_RXDoneTimestamp=millis();
	
  setMode(MODE_STDBY_RC);                 //ensure to stop further packet reception

  regdata = readIrqStatus();
  
#endif  

  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/
  
  if ( (regdata & IRQ_HEADER_ERROR) | (regdata & IRQ_CRC_ERROR) | (regdata & IRQ_RX_TX_TIMEOUT ) )
  {
    return 0;                                        //no RX done and header valid only, could be CRC error
  }

   readCommand(RADIO_GET_RXBUFFERSTATUS, buffer, 2);
  _RXPacketL = buffer[0];

  return _RXPacketL;                           
}


uint8_t SX126XLT::readBuffer(uint8_t *rxbuffer)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("readBuffer()");
#endif

  uint8_t index = 0, regdata;

  do                                     //need to find the size of the buffer first
  {
    regdata = SPI.transfer(0);
    rxbuffer[index] = regdata;           //fill the buffer.
    index++;
  } while (regdata != 0);                //keep reading until we have reached the null (0) at the buffer end
                                         //or exceeded size of buffer allowed
  _RXPacketL = _RXPacketL + index;       //increment count of bytes read
  return index;                          //return the actual size of the buffer, till the null (0) detected

}

/***************************************************************************
//End direct access SX buffer routines
***************************************************************************/

uint16_t SX126XLT::CRCCCITTSX(uint8_t startadd, uint8_t endadd, uint16_t startvalue)
{
  //genrates a CRC of an area of the internal SX buffer

#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("CRCCCITTSX()");
#endif


  uint16_t index, libraryCRC;
  uint8_t j;

  libraryCRC = startvalue;               //start value for CRC16

  startReadSXBuffer(startadd);           //begin the buffer read

  for (index = startadd; index <= endadd; index++)
  {
    libraryCRC ^= (((uint16_t) readUint8() ) << 8);
    for (j = 0; j < 8; j++)
    {
      if (libraryCRC & 0x8000)
        libraryCRC = (libraryCRC << 1) ^ 0x1021;
      else
        libraryCRC <<= 1;
    }
  }
  
  endReadSXBuffer();                    //end the buffer read

  return libraryCRC;
}


void SX126XLT::setSleep(uint8_t sleepconfig)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("setSleep()");
#endif
  setMode(MODE_STDBY_RC);
  checkBusy();
  
#ifdef USE_SPI_TRANSACTION          //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif
  
  digitalWrite(_NSS, LOW);
#if defined ARDUINO || defined USE_ARDUPI  
  SPI.transfer(RADIO_SET_SLEEP);
  SPI.transfer(sleepconfig); 
#else
	uint8_t spibuf[2];
	spibuf[0] = RADIO_SET_SLEEP;
	spibuf[1] = sleepconfig;
	
	wiringPiSPIDataRW(SPI_CHANNEL, spibuf, 2);
#endif  
  digitalWrite(_NSS, HIGH);
  
  #ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
  #endif
  
  if (_SW >= 0)
  { 
   digitalWrite(_SW, LOW);            //turn off antenna switch if SW pin in use, saves 9uA.  
  }
  
  delay(1);                           //allow time for shutdown
}


void SX126XLT::wake()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("wake()");
#endif

if (_SW >= 0)
  {
   digitalWrite(_SW, HIGH);          //turn on antenna switch if SW pin in use  
  }

digitalWrite(_NSS, LOW);
delay(1);
digitalWrite(_NSS, HIGH);
delay(1);
}


void SX126XLT::setupDirect(uint32_t frequency, int32_t offset)
{
  //setup LoRa device for direct modulation mode
#ifdef SX126XDEBUG1
  PRINT_CSTSTR("setupDirect()");
#endif
  setMode(MODE_STDBY_RC);
  setRegulatorMode(USE_DCDC);
  setPaConfig(0x04, PAAUTO, _Device);         //use _Device, saved by begin.
  setDIO3AsTCXOCtrl(TCXO_CTRL_3_3V);
  calibrateDevice(ALLDevices);                //is required after setting TCXO
  calibrateImage(frequency);
  setDIO2AsRfSwitchCtrl();
  setRfFrequency(frequency, offset);
}


void SX126XLT::setTXDirect()
{
 //turns on transmitter,in direct mode for FSK and audio  power level is from 2 to 17
#ifdef SX126XDEBUG1
  PRINT_CSTSTR("setTxFSK()");
#endif
  writeCommand(RADIO_SET_TXCONTINUOUSWAVE, 0, 0);
}


void SX126XLT::toneFM(uint16_t frequency, uint32_t length, uint32_t deviation, float adjust, uint8_t txpower)
{
#ifdef SX126XDEBUG1
  PRINT_CSTSTR("toneFM()");
#endif
  uint16_t index;
  uint32_t ToneDelayus;
  uint32_t registershift;
  uint32_t shiftedfreqregH, shiftedfreqregL;
  uint32_t loopcount;
     
  registershift = deviation/FREQ_STEP;
  shiftedfreqregH = savedFrequencyReg + registershift;
  shiftedfreqregL = savedFrequencyReg - registershift;
  
  uint8_t HighShiftH = shiftedfreqregH >> 24;
  uint8_t HighShiftMH = shiftedfreqregH >> 16;
  uint8_t HighShiftML = shiftedfreqregH >> 8;
  uint8_t HighShiftL = shiftedfreqregH;
  uint8_t LowShiftH = shiftedfreqregL >> 24;
  uint8_t LowShiftMH = shiftedfreqregL >> 16;
  uint8_t LowShiftML = shiftedfreqregL >> 8;
  uint8_t LowShiftL = shiftedfreqregL;
  uint8_t freqregH = savedFrequencyReg >> 24;
  uint8_t freqregMH = savedFrequencyReg >> 16;
  uint8_t freqregML = savedFrequencyReg >> 8;
  uint8_t freqregL = savedFrequencyReg;
  
    
  ToneDelayus = ((500000/frequency));
  loopcount = (length * 500) / (ToneDelayus);
  ToneDelayus = ToneDelayus * adjust;

  
#ifdef SX126XDEBUG3
  PRINT_CSTSTR("frequency ");
  PRINTLN_VALUE("%d",frequency);
  PRINT_CSTSTR("length ");
  PRINTLN_VALUE("%ld",length);
  
  PRINT_CSTSTR("savedFrequencyReg ");
  PRINTLN_HEX("%lX",savedFrequencyReg);
  PRINT_CSTSTR("registershift ");
  PRINTLN_VALUE("%ld",registershift);
  shiftedfreqregH = savedFrequencyReg + (registershift/2);
  shiftedfreqregL = savedFrequencyReg - (registershift/2);
  PRINT_CSTSTR("shiftedfreqregH ");
  PRINTLN_HEX("%X",shiftedfreqregH);
  PRINT_CSTSTR("shiftedfreqregL ");
  PRINTLN_HEX("%X",shiftedfreqregL);
  
  PRINT_CSTSTR("ShiftedHigh,");
  PRINT_HEX("%X",HighShiftH);
  PRINT_CSTSTR(",");
  PRINT_HEX("%X",HighShiftMH);
  PRINT_CSTSTR(",");
  PRINT_HEX("%X",HighShiftML);
  PRINT_CSTSTR(",");
  PRINTLN_HEX("%X",HighShiftL);
  
  PRINT_CSTSTR("ShiftedLow,");
  PRINT_HEX("%X",LowShiftH);
  PRINT_CSTSTR(",");
  PRINT_HEX("%X",LowShiftMH);
  PRINT_CSTSTR(",");
  PRINT_HEX("%X",LowShiftML);
  PRINT_CSTSTR(",");
  PRINTLN_HEX("%X",LowShiftL);
  PRINT_CSTSTR("ToneDelayus,");
  PRINTLN_VALUE("%ld",ToneDelayus);
  PRINT_CSTSTR("loopcount,");
  PRINTLN_VALUE("%ld",loopcount);
  PRINTLN;
  PRINTLN;
 #endif

  setTxParams(txpower, RADIO_RAMP_200_US);
  setTXDirect();
  
#ifdef USE_SPI_TRANSACTION     //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif
    
  for (index = 1; index <= loopcount; index++)
  {
    digitalWrite(_NSS, LOW);
#if defined ARDUINO || defined USE_ARDUPI    
    SPI.transfer(RADIO_SET_RFFREQUENCY); 
    SPI.transfer(HighShiftH);
    SPI.transfer(HighShiftMH);
    SPI.transfer(HighShiftML);
    SPI.transfer(HighShiftL);
#else    
		uint8_t spibuf[5];
		spibuf[0] = RADIO_SET_RFFREQUENCY;
		spibuf[1] = HighShiftH;
		spibuf[2] = HighShiftMH;
		spibuf[3] = HighShiftML;		
		spibuf[4] = HighShiftL;
		
		wiringPiSPIDataRW(SPI_CHANNEL, spibuf, 5);
#endif    
    digitalWrite(_NSS, HIGH);
  
    delayMicroseconds(ToneDelayus);
  
    digitalWrite(_NSS, LOW);
#if defined ARDUINO || defined USE_ARDUPI    
    SPI.transfer(RADIO_SET_RFFREQUENCY); 
    SPI.transfer(LowShiftH);
    SPI.transfer(LowShiftMH);
    SPI.transfer(LowShiftML);
    SPI.transfer(LowShiftL);
#else    
		spibuf[0] = RADIO_SET_RFFREQUENCY;
		spibuf[1] = LowShiftH;
		spibuf[2] = LowShiftMH;
		spibuf[3] = LowShiftML;		
		spibuf[4] = LowShiftL;
		
		wiringPiSPIDataRW(SPI_CHANNEL, spibuf, 5);
#endif    
    digitalWrite(_NSS, HIGH);

    delayMicroseconds(ToneDelayus);
  }  
  
  //now set the frequency registers back to centre
  digitalWrite(_NSS, LOW);                  //set NSS low
#if defined ARDUINO || defined USE_ARDUPI  
  SPI.transfer(0x86);                       //address for write to REG_FRMSB
  SPI.transfer(freqregH);
  SPI.transfer(freqregMH);
  SPI.transfer(freqregML);
  SPI.transfer(freqregL);
#else    
		uint8_t spibuf[5];
		spibuf[0] = 0x86;
		spibuf[1] = freqregH;
		spibuf[2] = freqregMH;
		spibuf[3] = freqregML;		
		spibuf[4] = freqregL;
		
		wiringPiSPIDataRW(SPI_CHANNEL, spibuf, 5);
#endif  
  digitalWrite(_NSS, HIGH);                 //set NSS high
  
#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif
  
  setMode(MODE_STDBY_RC);                   //turns off carrier
}


uint8_t SX126XLT::getByteSXBuffer(uint8_t addr)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("getByteSXBuffer()");
#endif

  uint8_t regdata;
  setMode(MODE_STDBY_RC);                     //this is needed to ensure we can read from buffer OK.

#ifdef USE_SPI_TRANSACTION                    //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);             //start the burst read
#if defined ARDUINO || defined USE_ARDUPI  
  SPI.transfer(RADIO_READ_BUFFER);
  SPI.transfer(addr);
  SPI.transfer(0xFF);
  regdata = SPI.transfer(0);
#else
	uint8_t spibuf[4];
	spibuf[0] = RADIO_READ_BUFFER;
	spibuf[1] = addr;
	spibuf[2] = 0xFF;
	spibuf[3] = 0x00;
	
	wiringPiSPIDataRW(SPI_CHANNEL, spibuf, 4);
	
	regdata = spibuf[3];
#endif  
  digitalWrite(_NSS, HIGH);

#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif

  return regdata;
}


void SX126XLT::printSXBufferHEX(uint8_t start, uint8_t end)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("printSXBufferHEX()");
#endif

  uint8_t index, regdata;

  setMode(MODE_STDBY_RC);

#ifdef USE_SPI_TRANSACTION     //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);                       //start the burst read
#if defined ARDUINO || defined USE_ARDUPI  
  SPI.transfer(RADIO_READ_BUFFER);
  SPI.transfer(start);
  SPI.transfer(0xFF);

  for (index = start; index <= end; index++)
  {
    regdata = SPI.transfer(0);
    printHEXByte(regdata);
    PRINT_CSTSTR(" ");
  }
#else
	uint8_t spibuf[end-start+1+3];
	spibuf[0] = RADIO_READ_BUFFER;
	spibuf[1] = start;
	spibuf[2] = 0xFF;

  for (index = start; index <= end; index++)
	{
		spibuf[3+index-start]=0x00;
	}	
	
	wiringPiSPIDataRW(SPI_CHANNEL, spibuf, end-start+1+3);

  for (index = start; index <= end; index++)
	{
		regdata=spibuf[3+index-start];
    printHEXByte(regdata);
    PRINT_CSTSTR(" ");		
	}	
#endif 
  digitalWrite(_NSS, HIGH);
  
#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif

}


int32_t SX126XLT::getFrequencyErrorHz()
{
  //Note: Semtech appear to have stated that the frequency error function that this code uses,
  //is not supported for SX126X, for reasons that have not been given, so use at your own risk.
  //The fuctions here are a replication of the routines for the very similar SX128X  
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("getFrequencyErrorHz()");
#endif
   
  int32_t error, regvalue;
  uint32_t bandwidth;
  float divider;
  
  bandwidth = returnBandwidth(savedModParam2);      //gets the last configured bandwidth in hz
  divider = (float) 1625000 / bandwidth;            //why the values from the SX1280 datasheet work I have no idea
    
  regvalue = getFrequencyErrorRegValue();
  
  error = (FREQ_ERROR_CORRECTION * regvalue) / divider;
  
  return error;
}


int32_t SX126XLT::getFrequencyErrorRegValue()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("getFrequencyErrorRegValue()");
#endif

  int32_t FrequencyError;
  uint32_t regmsb, regmid, reglsb, allreg;
  
  setMode(MODE_STDBY_XOSC);
  
  regmsb = readRegister( REG_FREQUENCY_ERRORBASEADDR );
  regmsb = regmsb & 0x0F;       //clear bit 20 which is always set
  
  regmid = readRegister( REG_FREQUENCY_ERRORBASEADDR + 1 );
  
  reglsb = readRegister( REG_FREQUENCY_ERRORBASEADDR + 2 );
    
  setMode(MODE_STDBY_RC);

#ifdef SX126XDEBUG1
  PRINTLN;
  PRINT_CSTSTR("Registers ");
  PRINT_HEX("%lX",regmsb);
  PRINT_CSTSTR(" ");
  PRINT_HEX("%lX",regmid);
  PRINT_CSTSTR(" ");
  PRINTLN_HEX("%lX",reglsb);
#endif
    
  allreg = (uint32_t) ( regmsb << 16 ) | ( regmid << 8 ) | reglsb;

  if (allreg & 0x80000)
  {
    FrequencyError = (0xFFFFF - allreg) * -1;
  }
  else
  {
    FrequencyError = allreg; 
  }

  return FrequencyError;
}



void SX126XLT::printHEXByte(uint8_t temp)
{
  if (temp < 0x10)
  {
    PRINT_CSTSTR("0");
  }
  PRINT_HEX("%X",temp);
}



uint8_t SX126XLT::transmitAddressed(uint8_t *txbuffer, uint8_t size, char txpackettype, char txdestination, char txsource, uint32_t txtimeout, int8_t txpower, uint8_t wait)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("transmitAddressed()");
#endif
  
  uint8_t index;
  uint8_t bufferdata;

  if (size == 0)
  {
   return false;
  }
  
  setMode(MODE_STDBY_RC);

  checkBusy();
  
#ifdef USE_SPI_TRANSACTION     //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif
    
  digitalWrite(_NSS, LOW);
  
#if defined ARDUINO || defined USE_ARDUPI   
  SPI.transfer(RADIO_WRITE_BUFFER);
  SPI.transfer(0);

  /**************************************************************************
	Added by C. Pham - Oct. 2020
  **************************************************************************/ 
    
  // we insert our header
  SPI.transfer(txdestination);                    //Destination node
  SPI.transfer(txpackettype);                     //Write the packet type
  SPI.transfer(txsource);                         //Source node
  SPI.transfer(_TXSeqNo);                           //Sequence number  
  _TXPacketL = HEADER_SIZE + size;                      //we have added 4 header bytes to size

  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/  
  
  /* original code
   ***************
    
  SPI.transfer(txpackettype);                     //Write the packet type
  SPI.transfer(txdestination);                    //Destination node
  SPI.transfer(txsource);                         //Source node
  _TXPacketL = 3 + size;                          //we have added 3 header bytes to size
  
  */

  for (index = 0; index < size; index++)
  {
    bufferdata = txbuffer[index];
    SPI.transfer(bufferdata);
  }
#else
	uint8_t spibuf[size+HEADER_SIZE+2];
	spibuf[0] = RADIO_WRITE_BUFFER;
	spibuf[1] = 0;
	spibuf[2] = txdestination;	
	spibuf[3] = txpackettype;	
	spibuf[4] = txsource;	
	spibuf[5] = _TXSeqNo;				

	_TXPacketL = HEADER_SIZE + size; 
	
	for (index = 0; index < size; index++)
	{
		bufferdata = txbuffer[index];	
		spibuf[2+HEADER_SIZE+index]=bufferdata;
	}	
	
	wiringPiSPIDataRW(SPI_CHANNEL, spibuf, _TXPacketL+2); 
#endif
  digitalWrite(_NSS, HIGH);
  
#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif
  
  //checkBusy();

  writeRegister(REG_LR_PAYLOADLENGTH, _TXPacketL);
  setTxParams(txpower, RAMP_TIME);
  setTx(txtimeout);                                //this starts the TX

  /**************************************************************************
	Added by C. Pham - Oct. 2020
  **************************************************************************/  
  
  // increment packet sequence number
  if ((uint8_t)txpackettype!=PKT_TYPE_RTS)
  	_TXSeqNo++;

  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/
    
  if (!wait)
  {
    return _TXPacketL;
  }
  
  /**************************************************************************
	Modified by C. Pham - Oct. 2020
  **************************************************************************/

#ifdef USE_POLLING

	uint16_t regdata;
	
	do {
		regdata = readIrqStatus();
			
	} while ( !(regdata & IRQ_TX_DONE) && !(regdata & IRQ_RX_TX_TIMEOUT) );
	
	if (regdata & IRQ_RX_TX_TIMEOUT )                        //check for timeout
		
#else		 
  
  while (!digitalRead(_TXDonePin));       //Wait for DIO1 to go high
  
  if (readIrqStatus() & IRQ_RX_TX_TIMEOUT )                        //check for timeout
#endif  
  {
    return 0;
  }
  
	_AckStatus=0;

	if (txpackettype & PKT_FLAG_ACK_REQ) {
		uint8_t RXAckPacketL;
		const uint8_t RXBUFFER_SIZE=3;
		uint8_t RXBUFFER[RXBUFFER_SIZE];

#ifdef SX126XDEBUGACK
		PRINTLN_CSTSTR("transmitAddressed is waiting for an ACK");
#endif
#ifdef INVERTIQ_ON_ACK
#ifdef SX126XDEBUGACK
		PRINTLN_CSTSTR("invert IQ for ack reception");
#endif
		invertIQ(true);
#endif
		delay(10);			
		//try to receive the ack
		RXAckPacketL=receiveAddressed(RXBUFFER, RXBUFFER_SIZE, 2000, WAIT_RX);
		
		if (RXAckPacketL) {
#ifdef SX126XDEBUGACK
			PRINT_CSTSTR("received ");
			PRINT_VALUE("%d", RXAckPacketL);
			PRINTLN_CSTSTR(" bytes");
			PRINT_CSTSTR("dest: ");
			PRINTLN_VALUE("%d", readRXDestination());
			PRINT_CSTSTR("ptype: ");
			PRINTLN_VALUE("%d", readRXPacketType());		
			PRINT_CSTSTR("src: ");
			PRINTLN_VALUE("%d", readRXSource());
			PRINT_CSTSTR("seq: ");
			PRINTLN_VALUE("%d", readRXSeqNo());				 
#endif		
			if ( (readRXSource()==(uint8_t)txdestination) && (readRXDestination()==(uint8_t)txsource) && (readRXPacketType()==PKT_TYPE_ACK) && (readRXSeqNo()==(_TXSeqNo-1)) ) {
				//seems that we got the correct ack
#ifdef SX126XDEBUGACK
				PRINTLN_CSTSTR("ACK is for me!");
#endif		
				_AckStatus=1;
				uint8_t value=RXBUFFER[2];
				
				if (value & 0x80 ) // The SNR sign bit is 1
				{
					// Invert and divide by 4
					value = ( ( ~value + 1 ) & 0xFF ) >> 2;
					_PacketSNRinACK = -value;
				}
				else
				{
					// Divide by 4
					_PacketSNRinACK = ( value & 0xFF ) >> 2;
				}				
			}
			else {
#ifdef SX126XDEBUGACK
				PRINTLN_CSTSTR("not for me!");
#endif			
			}
		}
		else {
#ifdef SX126XDEBUGACK
		PRINTLN_CSTSTR("received nothing");		 
#endif			
		}
#ifdef INVERTIQ_ON_ACK
#ifdef SX126XDEBUGACK
		PRINTLN_CSTSTR("set back IQ to normal");
#endif
		invertIQ(false);
#endif		
	} 

  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/
    
  return _TXPacketL;
}


uint8_t SX126XLT::readRXPacketType()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("readRXPacketType()");
#endif
  return _RXPacketType;
}


uint8_t SX126XLT::readRXDestination()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("readRXDestination()");
#endif
  return _RXDestination;
}


uint8_t SX126XLT::readRXSource()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("readRXSource()");
#endif
  return _RXSource;
}

uint8_t SX126XLT::receiveAddressed(uint8_t *rxbuffer, uint8_t size, uint32_t rxtimeout, uint8_t wait)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("receiveAddressed()");
#endif

  uint8_t index, RXstart, RXend;
  uint16_t regdata;
  uint8_t buffer[2];
  
  setDioIrqParams(IRQ_RADIO_ALL, (IRQ_RX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);   //set for IRQ on RX done or timeout
  setRx(rxtimeout);                                                                
  
  if (!wait)
  {
    return 0;                                                                 //not wait requested so no packet length to pass
  }

  /**************************************************************************
	Modified by C. Pham - Oct. 2020
  **************************************************************************/

#ifdef USE_POLLING

	do {
		regdata = readIrqStatus();
		
		if (regdata & IRQ_HEADER_VALID) {		
			_RXTimestamp=millis();
#ifdef SX126XDEBUG1	
  		PRINTLN_CSTSTR("Valid header detected"); 		
#endif			
		}
		delay(1);
					
	} while ( !(regdata & IRQ_RX_DONE) && !(regdata & IRQ_RX_TX_TIMEOUT) );

  setMode(MODE_STDBY_RC);                 //ensure to stop further packet reception
  
	if (regdata & IRQ_RX_DONE) 
		_RXDoneTimestamp=millis();
		
#else		 
  
  while (!digitalRead(_RXDonePin));       //Wait for DIO1 to go high

	_RXDoneTimestamp=millis();
	
  setMode(MODE_STDBY_RC);                 //ensure to stop further packet reception

  regdata = readIrqStatus();
  
#endif  

  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/
  
	if ( (regdata & IRQ_HEADER_ERROR) | (regdata & IRQ_CRC_ERROR) | (regdata & IRQ_RX_TX_TIMEOUT ) ) //check if any of the preceding IRQs is set
  {
  	//packet is errored somewhere so return 0
  	return 0;
  }
    
   readCommand(RADIO_GET_RXBUFFERSTATUS, buffer, 2);
  _RXPacketL = buffer[0];
  
  RXstart = buffer[1];
    
#ifdef USE_SPI_TRANSACTION     //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);               //start the burst read
  
  /**************************************************************************
	Added by C. Pham - Oct. 2020
  **************************************************************************/  

#ifdef SX126XDEBUG1 
  PRINT_CSTSTR("Receive ");
  PRINT_VALUE("%d", _RXPacketL);
  PRINTLN_CSTSTR(" bytes");   
#endif  

#if defined ARDUINO || defined USE_ARDUPI 
  SPI.transfer(RADIO_READ_BUFFER);
  SPI.transfer(RXstart);
  SPI.transfer(0xFF); 
  
  // we read our header
  _RXDestination = SPI.transfer(0);
  _RXPacketType = SPI.transfer(0);
  _RXSource = SPI.transfer(0);
  _RXSeqNo = SPI.transfer(0);
  
  //the header is not passed to the user
  _RXPacketL=_RXPacketL-HEADER_SIZE;
  
  if (_RXPacketL > size)               //check passed buffer is big enough for packet
  {
  	_RXPacketL = size;                   //truncate packet if not enough space
  }
  
  RXend = RXstart + _RXPacketL;  

  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/  
  
  /* original code
   ***************
    
  _RXPacketType = SPI.transfer(0);
  _RXDestination = SPI.transfer(0);
  _RXSource = SPI.transfer(0);
  
  */
  
  for (index = RXstart; index < RXend; index++)
  {
    regdata = SPI.transfer(0);
    rxbuffer[index] = regdata;
  }
#else
	uint8_t spibuf[_RXPacketL+3];
	spibuf[0] = RADIO_READ_BUFFER;
	spibuf[1] = RXstart;
	spibuf[2] = 0xFF;

	for (index = 0; index < _RXPacketL; index++)
	{
		spibuf[3+index]=0x00;
	}	
	
	wiringPiSPIDataRW(SPI_CHANNEL, spibuf, _RXPacketL+3);
	
  // we read our header
  _RXDestination = spibuf[3];
  _RXPacketType = spibuf[4];
  _RXSource = spibuf[5];
  _RXSeqNo = spibuf[6];

#ifdef SX128XDEBUG1
	PRINT_CSTSTR("dest: ");
	PRINTLN_VALUE("%d", _RXDestination);
	PRINT_CSTSTR("ptype: ");
	PRINTLN_VALUE("%d", _RXPacketType);		
	PRINT_CSTSTR("src: ");
	PRINTLN_VALUE("%d", _RXSource);
	PRINT_CSTSTR("seq: ");
	PRINTLN_VALUE("%d", _RXSeqNo);				 
#endif
	
  //the header is not passed to the user
  _RXPacketL=_RXPacketL-HEADER_SIZE;
  
  if (_RXPacketL > size)               //check passed buffer is big enough for packet
  {
    _RXPacketL = size;                 //truncate packet if not enough space
  }

  RXend = RXstart + _RXPacketL; 	

  for (index = RXstart; index < RXend; index++)
  {
    rxbuffer[index] = spibuf[3+HEADER_SIZE+index];
  }  
#endif
  digitalWrite(_NSS, HIGH);
  
#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif

  /**************************************************************************
	Added by C. Pham - Oct. 2020
  **************************************************************************/ 
  
  //sender request an ACK
	if ( (_RXPacketType & PKT_FLAG_ACK_REQ) && (_RXDestination==_DevAddr) ) {
		uint8_t TXAckPacketL;
		const uint8_t TXBUFFER_SIZE=3;
		uint8_t TXBUFFER[TXBUFFER_SIZE];
		
#ifdef SX126XDEBUGACK
  	PRINT_CSTSTR("ACK requested by ");
  	PRINTLN_VALUE("%d", _RXSource);
  	PRINTLN_CSTSTR("ACK transmission turnaround safe wait...");
#endif
#ifdef INVERTIQ_ON_ACK
#ifdef SX126XDEBUGACK
		PRINTLN_CSTSTR("invert IQ for ack transmission");
#endif
		invertIQ(true);
#endif			
		delay(100);
		TXBUFFER[0]=2; //length
		TXBUFFER[1]=0; //RFU
		uint8_t status[5];
		readCommand(RADIO_GET_PACKETSTATUS, status, 5) ;
		TXBUFFER[2]=status[1]; //SNR of received packet
		
		uint8_t saveTXSeqNo=_TXSeqNo;
		_TXSeqNo=_RXSeqNo;
		// use -1 as txpower to use default setting
		TXAckPacketL=transmitAddressed(TXBUFFER, 3, PKT_TYPE_ACK, _RXSource, _DevAddr, 10000, -1, WAIT_TX);	

#ifdef SX126XDEBUGACK
		if (TXAckPacketL)
  		PRINTLN_CSTSTR("ACK sent");
  	else	
  		PRINTLN_CSTSTR("error when sending ACK"); 
#endif
#ifdef INVERTIQ_ON_ACK
#ifdef SX126XDEBUGACK
		PRINTLN_CSTSTR("set back IQ to normal");
#endif
		invertIQ(false);
#endif				
		_TXSeqNo=saveTXSeqNo;
	}

  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/ 	
  
  return _RXPacketL;                     //so we can check for packet having enough buffer space
}


/**************************************************************************
Added by C. Pham - Dec. 2020
**************************************************************************/

uint8_t SX126XLT::receiveRTSAddressed(uint8_t *rxbuffer, uint8_t size, uint32_t rxtimeout, uint8_t wait)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("receiveRTSAddressed()");
#endif

  uint8_t index, RXstart, RXend;
  uint16_t regdata;
  uint8_t buffer[2];
  uint32_t endtimeoutmS;
  bool validHeader=false;  
  
  setDioIrqParams(IRQ_RADIO_ALL, (IRQ_RX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);   //set for IRQ on RX done or timeout
  setRx(rxtimeout);                                                                
  
  if (!wait)
  {
    return 0;                                                                 //not wait requested so no packet length to pass
  }

#ifdef USE_POLLING

	do {
		regdata = readIrqStatus();
		
		if (regdata & IRQ_HEADER_VALID) {

			validHeader=true;
				
			_RXTimestamp=millis();
#ifdef SX126XDEBUG1	
  		PRINTLN_CSTSTR("Valid header detected"); 		
#endif

			//update endtimeoutmS to try to get the RXDone in a short time interval
			endtimeoutmS = (_RXTimestamp + 250);			
		}
		delay(1);
					
	} while ( !(regdata & IRQ_RX_DONE) && !(regdata & IRQ_RX_TX_TIMEOUT) && (millis() < endtimeoutmS));

  setMode(MODE_STDBY_RC);                 //ensure to stop further packet reception
  
	if (regdata & IRQ_RX_DONE) 
		_RXDoneTimestamp=millis();
		
#else		 
  
  while (!digitalRead(_RXDonePin));       //Wait for DIO1 to go high

	_RXDoneTimestamp=millis();
	
  setMode(MODE_STDBY_RC);                 //ensure to stop further packet reception

  regdata = readIrqStatus();
  
#endif  
  
	if ( (regdata & IRQ_HEADER_ERROR) | (regdata & IRQ_CRC_ERROR) | (regdata & IRQ_RX_TX_TIMEOUT ) ) //check if any of the preceding IRQs is set
  {
		// if we got a valid header but no RXDone, it is maybe a long data packet
		// so we return a value greater than 1
		if (validHeader)
			return(2);
		else	    
    	return 0;
  }
    
   readCommand(RADIO_GET_RXBUFFERSTATUS, buffer, 2);
  _RXPacketL = buffer[0];

  RXstart = buffer[1];  
    
#ifdef USE_SPI_TRANSACTION     //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);               //start the burst read
#if defined ARDUINO || defined USE_ARDUPI  
  SPI.transfer(RADIO_READ_BUFFER);
  SPI.transfer(RXstart);
  SPI.transfer(0xFF);
  
  // we read our header
  _RXDestination = SPI.transfer(0);
  _RXPacketType = SPI.transfer(0);
  _RXSource = SPI.transfer(0);
  _RXSeqNo = SPI.transfer(0);
  
  //the header is not passed to the user
  _RXPacketL=_RXPacketL-HEADER_SIZE;
  
  if (_RXPacketL > size)               //check passed buffer is big enough for packet
  {
  	// if _RXPacketL!=1 it is a very short data packet because we have RXDone
  	// so we ignore it
    return(0);
  }

  RXend = RXstart + _RXPacketL;  

  for (index = RXstart; index < RXend; index++)
  {
    regdata = SPI.transfer(0);
    rxbuffer[index] = regdata;
  }
#else
	uint8_t spibuf[_RXPacketL+3];
	spibuf[0] = RADIO_READ_BUFFER;
	spibuf[1] = RXstart;
	spibuf[2] = 0xFF;

	for (index = 0; index < _RXPacketL; index++)
	{
		spibuf[3+index]=0x00;
	}	
	
	wiringPiSPIDataRW(SPI_CHANNEL, spibuf, _RXPacketL+3);
	
  // we read our header
  _RXDestination = spibuf[3];
  _RXPacketType = spibuf[4];
  _RXSource = spibuf[5];
  _RXSeqNo = spibuf[6];

#ifdef SX128XDEBUG1
	PRINT_CSTSTR("dest: ");
	PRINTLN_VALUE("%d", _RXDestination);
	PRINT_CSTSTR("ptype: ");
	PRINTLN_VALUE("%d", _RXPacketType);		
	PRINT_CSTSTR("src: ");
	PRINTLN_VALUE("%d", _RXSource);
	PRINT_CSTSTR("seq: ");
	PRINTLN_VALUE("%d", _RXSeqNo);				 
#endif
	
  //the header is not passed to the user
  _RXPacketL=_RXPacketL-HEADER_SIZE;
  
  if (_RXPacketL > size)               //check passed buffer is big enough for packet
  {
  	// if _RXPacketL!=1 it is a very short data packet because we have RXDone
  	// so we ignore it
    return(0);
  }

  RXend = RXstart + _RXPacketL; 	

  for (index = RXstart; index < RXend; index++)
  {
    rxbuffer[index] = spibuf[3+HEADER_SIZE+index];
  }  
#endif
  digitalWrite(_NSS, HIGH);
  
#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif

	// check if it is really an RTS packet
	if (_RXPacketType!=PKT_TYPE_RTS)
		return(0);
		  
  return _RXPacketL;                     //so we can check for packet having enough buffer space
}

/**************************************************************************
End by C. Pham - Dec. 2020
**************************************************************************/

void SX126XLT::clearDeviceErrors()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("clearDeviceErrors()");
#endif
  
  uint8_t buffer[2];

  buffer[0] = 0x00;                     //can only clear all errors
  buffer[1] = 0x00;
  
  writeCommand(RADIO_CLEAR_ERRORS, buffer, 2);
}


void SX126XLT::printDeviceErrors()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("printDeviceErrors()");
#endif

 uint16_t errors;
 uint8_t buffer[2];
 
 readCommand(RADIO_GET_ERROR, buffer, 2);

 errors = (buffer[0] << 8) + buffer[1];
 
  //0x0001
  if (errors & RC64K_CALIB_ERR)
  {
    PRINT_CSTSTR(",RC64K_CALIB_ERR");
  }

  //0x0002
  if (errors & RC13M_CALIB_ERR)
  {
    PRINT_CSTSTR(",RC13M_CALIB_ERR");
  }

  //0x0004
  if (errors & PLL_CALIB_ERR)
  {
    PRINT_CSTSTR(",PLL_CALIB_ERR");
  }

  //0x0008
  if (errors & ADC_CALIB_ERR)
  {
    PRINT_CSTSTR(",ADC_CALIB_ERR");
  }

  //0x0010
  if (errors & IMG_CALIB_ERR)
  {
    PRINT_CSTSTR(",IMG_CALIB_ERR");
  }

  //0x0020
  if (errors & XOSC_START_ERR)
  {
    PRINT_CSTSTR(",XOSC_START_ERR");
  }

  //0x0040
  if (errors & PLL_LOCK_ERR)
  {
    PRINT_CSTSTR(",PLL_LOCK_ERR");
  }

  //0x0080
  if (errors & RFU)
  {
    PRINT_CSTSTR(",RFU");
  }

  //0x0100
  if (errors & PA_RAMP_ERR)
  {
    PRINT_CSTSTR(",PA_RAMP_ERR");
  }
}


void SX126XLT::printHEXPacket(uint8_t *buffer, uint8_t size)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("printHEXPacket()");
#endif

  uint8_t index;
  
  for (index = 0; index < size; index++)
  {
    //PRINT_CSTSTR("[");
    //PRINT_VALUE("%d",index);
    //PRINT_CSTSTR("],");
    printHEXByte(buffer[index]);
    PRINT_CSTSTR(" ");
  }
}


void SX126XLT::printHEXByte0x(uint8_t temp)
{
  //print a byte, adding 0x
  PRINT_CSTSTR("0x");
  if (temp < 0x10)
  {
    PRINT_CSTSTR("0");
  }
  PRINT_HEX("%X",temp);
}


uint8_t SX126XLT::readsavedModParam1()
{
//return previously set spreading factor
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("readsavedModParam1()");
#endif
  return savedModParam1;
}


uint8_t SX126XLT::readsavedModParam2()
{
  //return previously set bandwidth
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("readsavedModParam2()");
#endif
  return savedModParam2;
}


uint8_t SX126XLT::readsavedModParam3()
{
//return previously set code rate
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("readsavedModParam3()");
#endif
  return savedModParam3;
}


uint8_t SX126XLT::readsavedModParam4()
{
  //return previously set optimisation
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("readsavedModParam4()");
#endif
  return savedModParam4;
}

uint8_t SX126XLT::readsavedPower()
{
 #ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("readsavedPower()");
#endif
  return savedTXPower;
}

uint8_t SX126XLT::getPacketMode()
{
  //its either LoRa or FSK
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("getPacketMode()");
#endif
  return savedPacketType;
}


uint8_t SX126XLT::readsavedPacketParam1()
{
//return previously set preamble
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("readsavedPacketParam1()");
#endif
  return savedPacketParam1;
}


uint8_t SX126XLT::readsavedPacketParam2()
{
//return previously set header type
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("readsavedPacketParam2()");
#endif
  return savedPacketParam2;
}


uint8_t SX126XLT::readsavedPacketParam3()
{
//return previously set packet length
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("readsavedPacketParam3()");
#endif
  return savedPacketParam3;
}



uint8_t SX126XLT::readsavedPacketParam4()
{
//return previously set CRC
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("readsavedPacketParam4()");
#endif
  return savedPacketParam4;
}


uint8_t SX126XLT::readsavedPacketParam5()
{
//return previously set IQ
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("readsavedPacketParam5()");
#endif
  return savedPacketParam5;
}

uint8_t SX126XLT::getOpmode()
{
//return last saved opmode
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("getOpmode()");
#endif
  return _OperatingMode;
}

uint8_t SX126XLT::getCRCMode()
{
//return last saved opmode
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("getCRCMode()");
#endif
  return savedPacketParam4;
}



void SX126XLT::fillSXBuffer(uint8_t startaddress, uint8_t size, uint8_t character)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("fillSXBuffer()");
#endif
  uint8_t index;

  setMode(MODE_STDBY_RC);
  //writeRegister(REG_FIFOADDRPTR, startaddress);     //and save in FIFO access ptr

#ifdef USE_SPI_TRANSACTION                          //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);
#if defined ARDUINO || defined USE_ARDUPI  
  SPI.transfer(RADIO_WRITE_BUFFER);
  SPI.transfer(startaddress);
  //SPI interface ready for byte to write to buffer
  
  for (index = 0; index < size; index++)
  {
    SPI.transfer(character);
  }
#else
	uint8_t spibuf[size+2];
	spibuf[0] = RADIO_WRITE_BUFFER;
	spibuf[1] = startaddress;

  for (index = 0; index < size; index++)
	{
		spibuf[2+index]=character;
	}	
	
	wiringPiSPIDataRW(SPI_CHANNEL, spibuf, size+2);
#endif
  digitalWrite(_NSS, HIGH);

#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif

}


uint8_t SX126XLT::readPacket(uint8_t *rxbuffer, uint8_t size)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("readPacket()");
#endif

  uint8_t index, regdata, RXstart, RXend;
  uint8_t buffer[2];
  
  readCommand(RADIO_GET_RXBUFFERSTATUS, buffer, 2);
  _RXPacketL = buffer[0];
  
  if (_RXPacketL > size)               //check passed buffer is big enough for packet
  {
  _RXPacketL = size;                   //truncate packet if not enough space
  }
  
  RXstart = buffer[1];
  
  RXend = RXstart + _RXPacketL;

#ifdef USE_SPI_TRANSACTION     //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif
  
  digitalWrite(_NSS, LOW);               //start the burst read
#if defined ARDUINO || defined USE_ARDUPI  
  SPI.transfer(RADIO_READ_BUFFER);
  SPI.transfer(RXstart);
  SPI.transfer(0xFF);

  for (index = RXstart; index < RXend; index++)
  {
    regdata = SPI.transfer(0);
    rxbuffer[index] = regdata;
  }
#else
	uint8_t spibuf[_RXPacketL+3];
	spibuf[0] = RADIO_READ_BUFFER;
	spibuf[1] = RXstart;
	spibuf[2] = 0xFF;

  for (index = RXstart; index < RXend; index++)
	{
		spibuf[3+index]=0x00;
	}	
	
	wiringPiSPIDataRW(SPI_CHANNEL, spibuf, _RXPacketL+3);

  for (index = RXstart; index < RXend; index++)
  {
    rxbuffer[index] = spibuf[3+index];
  }  
#endif
  digitalWrite(_NSS, HIGH);
  
#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif

  return _RXPacketL;                     //so we can check for packet having enough buffer space
}


void SX126XLT::writeByteSXBuffer(uint8_t addr, uint8_t regdata)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("writeByteSXBuffer");
#endif

  setMode(MODE_STDBY_RC);                 //this is needed to ensure we can write to buffer OK.

#ifdef USE_SPI_TRANSACTION                //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);
#if defined ARDUINO || defined USE_ARDUPI  
  SPI.transfer(RADIO_WRITE_BUFFER);
  SPI.transfer(addr);
  SPI.transfer(regdata);
#else
	uint8_t spibuf[3];
	spibuf[0] = RADIO_WRITE_BUFFER;
	spibuf[1] = addr;
	spibuf[2] = regdata;
	
	wiringPiSPIDataRW(SPI_CHANNEL, spibuf, 3);
#endif  
  digitalWrite(_NSS, HIGH);

#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif
}


void SX126XLT::printSXBufferASCII(uint8_t start, uint8_t end)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("printSXBufferASCII)");
#endif

  uint8_t index, regdata;
  setMode(MODE_STDBY_RC);

  for (index = start; index <= end; index++)
  {
    regdata = getByteSXBuffer(index);
    Serial.write(regdata);
  }

}


void SX126XLT::startFSKRTTY(uint32_t freqshift, uint8_t pips, uint16_t pipPeriodmS, uint16_t pipDelaymS, uint16_t leadinmS)
{
  
#ifdef SX126XDEBUG1
  PRINT_CSTSTR("startFSKRTTY()");
#endif
  
  uint32_t shiftedFrequencyRegisters;
  uint8_t index;
  uint32_t endmS;
  uint32_t calculatedRegShift;
    
  calculatedRegShift = (uint32_t) (freqshift/FREQ_STEP);
   
  shiftedFrequencyRegisters =  savedFrequencyReg + calculatedRegShift; 
 
  _ShiftfreqregH = (shiftedFrequencyRegisters >> 24) & 0xFF;  //MSB
  _ShiftfreqregMH = (shiftedFrequencyRegisters >> 16) & 0xFF;
  _ShiftfreqregML = (shiftedFrequencyRegisters >> 8) & 0xFF;
  _ShiftfreqregL = shiftedFrequencyRegisters & 0xFF;          //LSB
 
#ifdef DEBUGFSKRTTY
  PRINT_CSTSTR("NotShiftedFrequencyRegisters ");
  PRINTLN_HEX("%lX",savedFrequencyReg);
  PRINT_CSTSTR("calculatedRegShift ");
  PRINTLN_HEX("%lX",calculatedRegShift);
  PRINT_CSTSTR("ShiftedFrequencyRegisters ");
  PRINT_HEX("%lX",(uint32_t) shiftedFrequencyRegisters);
  PRINT_CSTSTR(" (");
  PRINT_HEX("%X",_ShiftfreqregH);
  PRINT_CSTSTR(" ");
  PRINT_HEX("%X",_ShiftfreqregMH);
  PRINT_CSTSTR(" "); 
  PRINT_HEX("%X",_ShiftfreqregML);
  PRINT_CSTSTR(" "); 
  PRINT_HEX("%X",_ShiftfreqregL);
  PRINT_CSTSTR(" )");
  PRINTLN;
#endif
 
  setTxParams(10, RADIO_RAMP_200_US);
  
  for (index = 1; index <= pips; index++)
  {
    setRfFrequencyDirect(_ShiftfreqregH, _ShiftfreqregMH, _ShiftfreqregML, _ShiftfreqregL); //set carrier frequency
    setTXDirect();                                                           //turn on carrier 
    delay(pipPeriodmS);
    setMode(MODE_STDBY_RC);                                                  //turns off carrier
    delay(pipDelaymS);
  }
  
  setRfFrequencyDirect(_ShiftfreqregH, _ShiftfreqregMH, _ShiftfreqregML,_ShiftfreqregL); //set carrier frequency 
  endmS = millis() + leadinmS; 
  setTXDirect();                                                           //turn on carrier 
  while (millis() < endmS);                                                //leave leadin on
  
}

 
 void SX126XLT::transmitFSKRTTY(uint8_t chartosend, uint8_t databits, uint8_t stopbits, uint8_t parity, uint16_t baudPerioduS, int8_t pin)
{
  //micros() will rollover at 4294967295 or 71mins 35secs
  //assume slowest baud rate is 45 (baud period of 22222us) then with 11 bits max to send if routine starts 
  //when micros() > (4294967295 - (22222 * 11) = 4294722855 = 0xFFFC4525 then it could overflow during send
  //Rather than deal with rolloever in the middle of a character lets wait till it overflows and then
  //start the character  
  
#ifdef SX126XDEBUG1
  PRINT_CSTSTR("transmitFSKRTTY()");
#endif
   
  uint8_t numbits;
  uint32_t enduS;
  uint8_t bitcount = 0;                       //set when a bit is 1
  
  if (micros() > 0xFFFB6000)                  //check if micros would overflow within circa 300mS, approx 1 char at 45baud
  {
#ifdef DEBUGFSKRTTY
    PRINT_CSTSTR("Overflow pending - micros() = ");
    PRINTLN_HEX("%lX",micros());
#endif
  
    while (micros() > 0xFFFB6000);              //wait a short while until micros overflows to 0
  
#ifdef DEBUGFSKRTTY
    PRINT_CSTSTR("Paused - micros() = ");
    PRINTLN_HEX("%lX",micros());
#endif
  }
  
  enduS = micros() + baudPerioduS;
  setRfFrequencyDirect(_freqregH, _freqregMH, _freqregML, _freqregL); //set carrier frequency  (low)
  
  if (pin >= 0)
  {
   digitalWrite(pin, LOW); 
  }
  
  while (micros() < enduS);                   //start bit
  
  for (numbits = 1;  numbits <= databits; numbits++) //send bits, LSB first
  {
    enduS = micros() + baudPerioduS;          //start the timer 
    if ((chartosend & 0x01) != 0)             //test for bit set, a 1
    {
       bitcount++;
       if (pin >= 0)
       {
       digitalWrite(pin, HIGH); 
       }
    setRfFrequencyDirect(_ShiftfreqregH, _ShiftfreqregMH, _ShiftfreqregML, _ShiftfreqregL);       //set carrier frequency for a 1 
    }
    else
    {
       if (pin >= 0)
       {
       digitalWrite(pin, LOW); 
       }     
      setRfFrequencyDirect(_freqregH, _freqregMH, _freqregML, _freqregL);                         //set carrier frequency for a 0
    }
    chartosend = (chartosend >> 1);           //get the next bit
    while (micros() < enduS);
  }
   
   enduS = micros() + baudPerioduS;          //start the timer for possible parity bit
   
   switch (parity) 
   {
    case ParityNone:
         break;
    
    case ParityZero:
         setRfFrequencyDirect(_freqregH, _freqregMH, _freqregML, _freqregL);                      //set carrier frequency for a 0
         while (micros() < enduS);
		 break;
    
    case ParityOne:
         
         setRfFrequencyDirect(_ShiftfreqregH, _ShiftfreqregMH, _ShiftfreqregML, _ShiftfreqregL);   //set carrier frequency for a 1
         while (micros() < enduS);
		 break;

	case ParityOdd:
         if (bitRead(bitcount, 0))
         {
         setRfFrequencyDirect(_ShiftfreqregH, _ShiftfreqregMH, _ShiftfreqregML, _ShiftfreqregL);   //set carrier frequency for a 1 
         }
		 else
         {
         setRfFrequencyDirect(_freqregH, _freqregMH, _freqregML, _freqregL);                       //set carrier frequency for a 0 
         }
         while (micros() < enduS);
		 break;

    case ParityEven:
         if (bitRead(bitcount, 0))
         {
         setRfFrequencyDirect(_freqregH, _freqregMH, _freqregML, _freqregL);                       //set carrier frequency for a 0 
         }
		 else
         {
         setRfFrequencyDirect(_ShiftfreqregH, _ShiftfreqregMH, _ShiftfreqregML, _ShiftfreqregL);   //set carrier frequency for a 1 
         }
		 while (micros() < enduS);
         break; 
     
    default:
	     break;
    } 

  //stop bits, normally 1 or 2
  enduS = micros() + (baudPerioduS * stopbits);
  
  if (pin >= 0)
  {
    digitalWrite(pin, HIGH); 
  }
  
  setRfFrequencyDirect(_ShiftfreqregH, _ShiftfreqregMH, _ShiftfreqregML, _ShiftfreqregL);          //set carrier frequency  for a 1
  
  while (micros() < enduS);
}



void SX126XLT::transmitFSKRTTY(uint8_t chartosend, uint16_t baudPerioduS, int8_t pin)
{
  //micros() will rollover at 4294967295 or 71mins 35secs
  //assume slowest baud rate is 45 (baud period of 22222us) then with 11 bits max to send if routine starts 
  //when micros() > (4294967295 - (22222 * 11) = 4294722855 = 0xFFFC4525 then it could overflow during send
  //Rather than deal with rolloever in the middle of a character lets wait till it overflows and then
  //start the character
  //This overloaded version of transmitFSKRTTY() uses 1 start bit, 7 data bits, no parity and 2 stop bits. 
  
#ifdef SX126XDEBUG1
  PRINT_CSTSTR("transmitFSKRTTY()");
#endif
   
  uint8_t numbits;
  uint32_t enduS;
  
  if (micros() > 0xFFFB6000)                  //check if micros would overflow within circa 300mS, approx 1 char at 45baud
  {
#ifdef DEBUGFSKRTTY
    PRINT_CSTSTR("Overflow pending - micros() = ");
    PRINTLN_HEX("%lX",micros());
#endif
  while (micros() > 0xFFFB6000);              //wait a short while until micros overflows to 0
#ifdef DEBUGFSKRTTY
    PRINT_CSTSTR("Paused - micros() = ");
    PRINTLN_HEX("%lX",micros());
#endif
  }
  
  enduS = micros() + baudPerioduS;
  setRfFrequencyDirect(_freqregH, _freqregMH, _freqregML, _freqregL); //set carrier frequency  (low)
  
  if (pin >= 0)
  {
   digitalWrite(pin, LOW); 
  }
  
  while (micros() < enduS);                   //start bit
  
  for (numbits = 1;  numbits <= 7; numbits++) //send bits, LSB first
  {
    enduS = micros() + baudPerioduS;          //start the timer 
    if ((chartosend & 0x01) != 0)             //test for bit set, a 1
    {
       if (pin >= 0)
       {
       digitalWrite(pin, HIGH); 
       }
    setRfFrequencyDirect(_ShiftfreqregH, _ShiftfreqregMH, _ShiftfreqregML, _ShiftfreqregL); //set carrier frequency for a 1 
    }
    else
    {
       if (pin >= 0)
       {
       digitalWrite(pin, LOW); 
       }     
      setRfFrequencyDirect(_freqregH, _freqregMH, _freqregML, _freqregL);           //set carrier frequency for a 0
    }
    chartosend = (chartosend >> 1);           //get the next bit
    while (micros() < enduS);
  }
   
  //stop bits, normally 1 or 2
  enduS = micros() + (baudPerioduS * 2);
  
  if (pin >= 0)
  {
    digitalWrite(pin, HIGH); 
  }
  
  setRfFrequencyDirect(_ShiftfreqregH, _ShiftfreqregMH, _ShiftfreqregML, _ShiftfreqregL); //set carrier frequency
  
  while (micros() < enduS);
  
}


void SX126XLT::printRTTYregisters()
{
#ifdef SX126XDEBUG1
  PRINT_CSTSTR("printRTTYregisters()");
#endif
   
  PRINT_CSTSTR("NoShift Registers ");
  PRINT_HEX("%X",_freqregH);
  PRINT_CSTSTR(" ");
  PRINT_HEX("%X",_freqregMH);
  PRINT_CSTSTR(" ");
  PRINT_HEX("%X",_freqregML);
  PRINT_CSTSTR(" ");
  PRINTLN_HEX("%X",_freqregL);

  PRINT_CSTSTR("Shifted Registers ");
  PRINT_HEX("%X",_ShiftfreqregH);
  PRINT_CSTSTR(" ");
  PRINT_HEX("%X",_ShiftfreqregMH);
  PRINT_CSTSTR(" ");
  PRINT_HEX("%X",_ShiftfreqregML);
  PRINT_CSTSTR(" ");
  PRINTLN_HEX("%X",_ShiftfreqregL);
}


void SX126XLT::endFSKRTTY()
{
#ifdef SX126XDEBUG1
  PRINT_CSTSTR("endFSKRTTY()");
#endif
  
  setMode(MODE_STDBY_RC);
}


void SX126XLT::getRfFrequencyRegisters(uint8_t *buff)
{
  //returns the register values for the current set frequency
#ifdef SX126XDEBUG1
  PRINT_CSTSTR("getRfFrequencyRegisters()");
#endif

  buff[0] = _freqregH;
  buff[1] = _freqregMH;
  buff[2] = _freqregML;
  buff[3] = _freqregL;
}


void SX126XLT::setRfFrequencyDirect(uint8_t high, uint8_t midhigh, uint8_t midlow, uint8_t low)
{   
#ifdef SX126XDEBUG1
  PRINT_CSTSTR("setRfFrequencyDirect()");
#endif
  
  uint8_t buffer[4];
  
  buffer[0] = high; //MSB
  buffer[1] = midhigh;
  buffer[2] = midlow;
  buffer[3] = low;//LSB

  writeCommand(RADIO_SET_RFFREQUENCY, buffer, 4); 
}

/**************************************************************************
  Added by C. Pham - Oct. 2020
**************************************************************************/

uint32_t SX126XLT::returnBandwidth()
{
  return (returnBandwidth(savedModParam2));
}

void SX126XLT::setCadParams()
{
	uint8_t cadSymbolNum, cadDetMin, cadDetPeak;
	
	if (savedModParam2==LORA_BW_500) {
  	switch (savedModParam1)
  	{
  		// from AN1200.48 Nov 2019
    	case 5:
    	case 6:
    	case 7: cadDetMin=10;
    					cadDetPeak=21;
    					cadSymbolNum=LORA_CAD_04_SYMBOL;
    					break;
    					
    	case 8: cadDetMin=10;
    					cadDetPeak=22;
    					cadSymbolNum=LORA_CAD_04_SYMBOL;
    					break;
    	
    	case 9: cadDetMin=10;
    					cadDetPeak=22;
    					cadSymbolNum=LORA_CAD_04_SYMBOL;
    					break;
    	
    	case 10: cadDetMin=10;
    					cadDetPeak=23;
    					cadSymbolNum=LORA_CAD_04_SYMBOL;
    					break;
    	
    	case 11: cadDetMin=10;
    					cadDetPeak=25;
    					cadSymbolNum=LORA_CAD_04_SYMBOL;
    					break;
    	
    	case 12: cadDetMin=10;
    					cadDetPeak=22;
    					cadSymbolNum=LORA_CAD_08_SYMBOL;
    					break;
		}	
	}	
	else {
		// all other bandwidth values
		// from AN1200.48 Nov 2019 for LORA_BW_125
		// we decided to take same parameters for all other bandwidth values
		switch (savedModParam1)
			{
				case 5:
				case 6:
				case 7:
				case 8: cadDetMin=10;
								cadDetPeak=22;
								cadSymbolNum=LORA_CAD_02_SYMBOL;
								break;
			
				case 9: cadDetMin=10;
								cadDetPeak=23;
								cadSymbolNum=LORA_CAD_04_SYMBOL;
								break;
			
				case 10: cadDetMin=10;
								cadDetPeak=24;
								cadSymbolNum=LORA_CAD_04_SYMBOL;
								break;
			
				case 11: cadDetMin=10;
								cadDetPeak=25;
								cadSymbolNum=LORA_CAD_04_SYMBOL;
								break;
			
				case 12: cadDetMin=10;
								cadDetPeak=28;
								cadSymbolNum=LORA_CAD_04_SYMBOL;
								break;
			}
	}

  uint8_t buffer[7];
  buffer[0] = cadSymbolNum;
  buffer[1] = cadDetPeak;
  buffer[2] = cadDetMin;
  buffer[3] = LORA_CAD_ONLY;
  //cadTimeout is not used with CAD_ONLY
  buffer[4] = 0;
	buffer[5] = 0;
	buffer[6] = 0;
  writeCommand(RADIO_SET_CADPARAMS, buffer, 7);
  
  _cadSymbolNum=cadSymbolNum;
}

/*
 Function: Configures the module to perform CAD.
 Returns: Integer that determines if the number of requested CAD have been successfull
   state = 1  --> There has been an error while executing the command
   state = 0  --> No activity detected
   state < 0  --> Activity detected, return mean RSSI computed duuring CAD
*/
int8_t SX126XLT::doCAD(uint8_t counter)
{
#ifdef SX126XDEBUG1
	PRINTLN_CSTSTR("doCAD()");
#endif
	
	clearIrqStatus(IRQ_RADIO_ALL);
	
	// trigger CAD
  writeCommand(RADIO_SET_CAD, NULL, 0);	

	//wait for CAD done
	while ( !(readIrqStatus() & IRQ_CAD_DONE) )
		;

  //TODO C. Pham. can we use GetRssiInst command to get the RSSI during CAD?
  if (readIrqStatus() & IRQ_CAD_ACTIVITY_DETECTED)
  	return(-1);
  else
  	return(0);
}

#ifdef SX126XDEBUG3

void printDouble(double val, byte precision){
    // prints val with number of decimal places determine by precision
    // precision is a number from 0 to 6 indicating the desired decimial places
    // example: lcdPrintDouble( 3.1415, 2); // prints 3.14 (two decimal places)

    if(val < 0.0){
        PRINT_CSTSTR("-");
        val = -val;
    }

    PRINT_VALUE("%d",int(val));  //prints the int part
    if( precision > 0) {
        PRINT_CSTSTR("."); // print the decimal point
        unsigned long frac;
        unsigned long mult = 1;
        byte padding = precision -1;
        while(precision--)
            mult *=10;

        if(val >= 0)
            frac = (val - int(val)) * mult;
        else
            frac = (int(val)- val ) * mult;
        unsigned long frac1 = frac;
        while( frac1 /= 10 )
            padding--;
        while(  padding--)
            PRINT_CSTSTR("0");
        PRINT_VALUE("%d",frac) ;
    }
}

#endif

uint16_t SX126XLT::getToA(uint8_t pl) {
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("getToA()");
#endif
	
  uint8_t LDRO = 0;
  uint8_t sf;
  uint8_t cr;
  uint32_t airTime = 0;
  uint32_t bw=0;

  bw=returnBandwidth();
  sf=getLoRaSF();
  cr=getLoRaCodingRate();

#ifdef SX126XDEBUG3
  PRINT_CSTSTR("BW is ");
  PRINTLN_VALUE("%ld",bw);

  PRINT_CSTSTR("SF is ");
  PRINTLN_VALUE("%d",sf);
  
  PRINT_CSTSTR("CR is ");
  PRINTLN_VALUE("%d",cr);  
#endif

  // Symbol rate : time for one symbol (secs)
  //double ts = 1.0 / ((double)bw / ( 1 << sf));
	double ts = (double)( 1 << sf) / (double)bw;

  // must add 4.25 to the programmed preamble length to get the effective preamble length
  // for sf==5 and sf==6, must add 6.25
  double tPreamble=(getPreamble()+((sf<7)?6.25:4.25))*ts;

#ifdef SX126XDEBUG3
  PRINT_CSTSTR("ts is ");
  printDouble(ts,6);
  PRINTLN;
  PRINT_CSTSTR("tPreamble is ");
  printDouble(tPreamble,6);
  PRINTLN;
#endif

  // for low data rate optimization
  if ((ts*1000.0)>16)
      LDRO=1;

#ifdef SX126XDEBUG3
  PRINT_CSTSTR("LDRO is ");
  PRINTLN_VALUE("%d",LDRO);
  PRINT_CSTSTR("CRC is ");
  PRINTLN_VALUE("%d",(savedPacketParam4==LORA_CRC_ON)?1:0);  
  PRINT_CSTSTR("header is ");
  PRINTLN_VALUE("%d",(savedPacketParam2==LORA_PACKET_VARIABLE_LENGTH)?1:0);     
#endif
  
  // Symbol length of payload and time
  double tmp = (8*pl + ((savedPacketParam4==LORA_CRC_ON)?16:0) - 4*sf + ((sf<7)?0:8) + ((savedPacketParam2==LORA_PACKET_VARIABLE_LENGTH)?20:0)) / (double)(4*(sf-2*LDRO));

	tmp = ( tmp > 0 ) ? tmp : 0;
	
  tmp = ceil(tmp)*(cr + 4);

  double nPayload = 8 + tmp;

#ifdef SX126XDEBUG3
  PRINT_CSTSTR("nSymbol_payload is ");
  PRINTLN_VALUE("%d",(uint16_t)nPayload);
   PRINT_CSTSTR("nSymbol_total is ");
  PRINTLN_VALUE("%d",(uint16_t)nPayload+(getPreamble()+((sf<7)?6.25:4.25))); 
#endif

  double tPayload = nPayload * ts;
  // Time on air in us
  double tOnAir = (tPreamble + tPayload)*1e6;
  
  //in ms
  airTime = tOnAir / 1000;
  
  // in us secs
  //airTime = floor( tOnAir * 1e6 + 0.999 );

  //////

#ifdef SX126XDEBUG3
  PRINT_CSTSTR("airTime is ");
  PRINTLN_VALUE("%ld",airTime);
#endif
	return(airTime);
  // return in ms
  //return (ceil(airTime/1000)+1);
}

// need to set cad_number to a value > 0
// we advise using cad_number=3 for a SIFS and DIFS=3*SIFS
#define DEFAULT_CAD_NUMBER    3

void SX126XLT::CarrierSense(uint8_t cs, bool extendedIFS, bool onlyOnce) {
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("CarrierSense()");
#endif
  
  if (cs==1)
    CarrierSense1(DEFAULT_CAD_NUMBER, extendedIFS, onlyOnce);

  if (cs==2)
    CarrierSense2(DEFAULT_CAD_NUMBER, extendedIFS);

  if (cs==3)
    CarrierSense3(DEFAULT_CAD_NUMBER);
}

uint16_t SX126XLT::CollisionAvoidance(uint8_t pl, uint8_t ca) {
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("CollisionAvoidance()");
#endif
  
  if (ca==1)
  	// we force cad_number to 0 to skip the CAD procedure to test the collision avoidance mechanism
  	// in a real deployment, set back to DEFAULT_CAD_NUMBER
    return(CollisionAvoidance0(pl, 0 /* DEFAULT_CAD_NUMBER */));
    
  return(0);  
}

void SX126XLT::CarrierSense1(uint8_t cad_number, bool extendedIFS, bool onlyOnce) {

  int e;
  uint8_t retries=3;
  uint8_t DIFSretries=8;
  uint8_t cad_value;
  unsigned long _startDoCad, _endDoCad;

  // symbol time in ms
  double ts = 1000.0 / (returnBandwidth() / ( 1 << getLoRaSF()));

  //approximate duration of a CAD
  cad_value=_cadSymbolNum*ts;

  PRINT_CSTSTR("--> CS1\n");

  if (cad_number) {

    do {
      DIFSretries=8;
      do {
        // check for free channel (SIFS/DIFS)
        _startDoCad=millis();
        e = doCAD(cad_number);
        _endDoCad=millis();

        PRINT_CSTSTR("--> CAD ");
        PRINT_VALUE("%d",_endDoCad-_startDoCad);
        PRINTLN;

        if (e==0) {
            PRINT_CSTSTR("OK1\n");

            if (extendedIFS)  {
                // wait for random number of CAD
#ifdef ARDUINO                
                uint8_t w = random(1,8);
#else
                uint8_t w = rand() % 8 + 1;
#endif
                PRINT_CSTSTR("--> wait for ");
                PRINT_VALUE("%d",w);
                PRINT_CSTSTR(" CAD = ");
                PRINT_VALUE("%d",cad_value*w);
                PRINTLN;

                delay(cad_value*w);

                // check for free channel (SIFS/DIFS) once again
                _startDoCad=millis();
                e = doCAD(cad_number);
                _endDoCad=millis();

                PRINT_CSTSTR("--> CAD ");
                PRINT_VALUE("%d",_endDoCad-_startDoCad);
                PRINTLN;

                if (e==0) {
                    PRINT_CSTSTR("OK2");
                    return;	
                }
                else
                    PRINT_CSTSTR("#2");

                PRINTLN;
            }
            return;
        }
        else {
            PRINT_CSTSTR("#1\n");
            
            // if we have "only once" behavior then exit here to not have retries
          	if (onlyOnce)
          		return; 
          		
            // wait for random number of DIFS
#ifdef ARDUINO                
            uint8_t w = random(1,8);
#else
            uint8_t w = rand() % 8 + 1;
#endif

            PRINT_CSTSTR("--> wait for ");
            PRINT_VALUE("%d",w);
            PRINT_CSTSTR(" DIFS=3SIFS= ");
            PRINT_VALUE("%d",cad_value*3*w);
            PRINTLN;

            delay(cad_value*3*w);

            PRINT_CSTSTR("--> retry\n");
        }

      } while (e!=0 && --DIFSretries);

    } while (--retries);
    
    if (!retries)
    	PRINT_CSTSTR("--> abort\n");
  }
}

void SX126XLT::CarrierSense2(uint8_t cad_number, bool extendedIFS) {

  int e;
  uint8_t foundBusyDuringDIFSafterBusyState=0;
  uint8_t retries=3;
  uint8_t DIFSretries=8;
  uint8_t n_collision=0;
  // upper bound of the random backoff timer
  uint8_t W=2;
  uint32_t max_toa = getToA(255);
  uint8_t cad_value;
  unsigned long _startDoCad, _endDoCad;

  // symbol time in ms
  double ts = 1000.0 / (returnBandwidth() / ( 1 << getLoRaSF()));

  //approximate duration of a CAD
  cad_value=_cadSymbolNum*ts;

  // do CAD for DIFS=9CAD
  PRINT_CSTSTR("--> CS2\n");

  if (cad_number) {

    do {
      DIFSretries=8;
      do {
        //D f W
        //2 2 4
        //3 3 8
        //4 4 16
        //5 5 16
        //6 6 16
        //...

        if (foundBusyDuringDIFSafterBusyState>1 && foundBusyDuringDIFSafterBusyState<5)
          W=W*2;

        // check for free channel (SIFS/DIFS)
        _startDoCad=millis();
        e = doCAD(cad_number);
        _endDoCad=millis();

        PRINT_CSTSTR("--> DIFS ");
        PRINT_VALUE("%d",_endDoCad-_startDoCad);
        PRINTLN;

        // successull SIFS/DIFS
        if (e==0) {
          // previous collision detected
          if (n_collision) {
            PRINT_CSTSTR("--> count for ");
            // count for random number of CAD/SIFS/DIFS?
            // SIFS=3CAD
            // DIFS=9CAD
#ifdef ARDUINO            
            uint8_t w = random(0,W*cad_number);
#else
            uint8_t w = rand() % (W*cad_number) + 1;
#endif
            PRINTLN_VALUE("%d",w);

            int busyCount=0;
            bool nowBusy=false;

            do {
              if (nowBusy)
                  e = doCAD(cad_number);
              else
                  e = doCAD(1);

              if (nowBusy && e!=0) {
                  PRINT_CSTSTR("#");
                  busyCount++;
              }
              else if (nowBusy && e==0) {
                  PRINT_CSTSTR("|");
                  nowBusy=false;
              }
              else if (e==0) {
                  w--;
                  PRINT_CSTSTR("-");
              }
              else {
                  PRINT_CSTSTR("*");
                  nowBusy=true;
                  busyCount++;
              }

            } while (w);

              // if w==0 then we exit and
              // the packet will be sent
              PRINTLN;
              PRINT_CSTSTR("--> busy during ");
              PRINTLN_VALUE("%d",busyCount);
          }
          else {
            PRINTLN_CSTSTR("OK1");

            if (extendedIFS)  {
              // wait for random number of CAD
#ifdef ARDUINO                
              uint8_t w = random(1,8);
#else
              uint8_t w = rand() % 8 + 1;
#endif

              PRINT_CSTSTR("--> extended wait for ");
              PRINTLN_VALUE("%d",w);
              PRINT_CSTSTR(" CAD = ");
              PRINTLN_VALUE("%d",cad_value*w);

              delay(cad_value*w);

              // check for free channel (SIFS/DIFS) once again
              _startDoCad=millis();
              e = doCAD(cad_number);
              _endDoCad=millis();

              PRINT_CSTSTR("--> CAD ");
              PRINTLN_VALUE("%d",_endDoCad-_startDoCad);

              if (e==0) {
                  PRINT_CSTSTR("OK2");
                  return;	
              }
              else
                  PRINT_CSTSTR("#2");

              PRINTLN;
            }
            return;
          }
        }
        else {
          n_collision++;
          foundBusyDuringDIFSafterBusyState++;
          PRINT_CSTSTR("###");
          PRINTLN_VALUE("%d",n_collision);

          PRINTLN_CSTSTR("--> CAD until clear");

          int busyCount=0;

          _startDoCad=millis();
          do {
            e = doCAD(1);

            if (e!=0) {
                PRINT_CSTSTR("R");
                busyCount++;
            }
          } while (e!=0 && (millis()-_startDoCad < 2*max_toa)); 

          _endDoCad=millis();

          PRINTLN;
          PRINT_CSTSTR("--> busy during ");
          PRINTLN_VALUE("%d",busyCount);

          PRINT_CSTSTR("--> wait ");
          PRINTLN_VALUE("%d",_endDoCad-_startDoCad);

          // to perform a new DIFS
          PRINTLN_CSTSTR("--> retry");
          e=1;
        }
      } while (e!=0 && --DIFSretries);
    } while (--retries);

    if (!retries)
    	PRINT_CSTSTR("--> abort\n");    
  }
}

void SX126XLT::CarrierSense3(uint8_t cad_number) {

  int e;
  bool carrierSenseRetry=false;
  uint8_t n_collision=0;
  uint8_t retries=3;
  uint8_t n_cad=9;
  uint32_t max_toa = getToA(255);
  unsigned long _startDoCad, _endDoCad;

  PRINTLN_CSTSTR("--> CS3");

  //unsigned long end_carrier_sense=0;

  if (cad_number) {
    do {
      PRINT_CSTSTR("--> CAD for MaxToa=");
      PRINTLN_VALUE("%d",max_toa);

      //end_carrier_sense=millis()+(max_toa/n_cad)*(n_cad-1);

      for (int i=0; i<n_cad; i++) {
        _startDoCad=millis();
        e = doCAD(1);
        _endDoCad=millis();

        if (e==0) {
          PRINT_VALUE("%d",_endDoCad);
          PRINT_CSTSTR(" 0 ");
          PRINT_VALUE("%d",_RSSI);
          PRINT_CSTSTR(" ");
          PRINTLN_VALUE("%d",_endDoCad-_startDoCad);
        }
        else
            continue;

        // wait in order to have n_cad CAD operations during max_toa
        delay(max_toa/(n_cad-1)-(millis()-_startDoCad));
      }

      if (e!=0) {
        n_collision++;
        PRINT_CSTSTR("#");
        PRINTLN_VALUE("%d",n_collision);

        PRINT_CSTSTR("Busy. Wait MaxToA=");
        PRINTLN_VALUE("%d",max_toa);
        delay(max_toa);
        // to perform a new max_toa waiting
        PRINTLN_CSTSTR("--> retry");
        carrierSenseRetry=true;
      }
      else
        carrierSenseRetry=false;
    } while (carrierSenseRetry && --retries);
  }
}

uint8_t SX126XLT::transmitRTSAddressed(uint8_t pl) {

	uint8_t TXRTSPacketL;
	const uint8_t TXBUFFER_SIZE=1;
	uint8_t TXBUFFER[TXBUFFER_SIZE];
	
#ifdef SX126XDEBUG1
	PRINTLN_CSTSTR("transmitRTSAddressed()");
#endif
			
	TXBUFFER[0]=pl; //length of next data packet

#ifdef SX126XDEBUGRTS
	PRINT_CSTSTR("--> RTS: FOR ");
	PRINT_VALUE("%d", pl);
  PRINTLN_CSTSTR(" Bytes");		
#endif
	
	// use -1 as txpower to use default setting
	TXRTSPacketL=transmitAddressed(TXBUFFER, 1, PKT_TYPE_RTS, 0, _DevAddr, 10000, -1, WAIT_TX);	

#ifdef SX126XDEBUGRTS
	if (TXRTSPacketL)
		PRINTLN_CSTSTR("--> RTS: SENT");
	else	
		PRINTLN_CSTSTR("--> RTS: ERROR"); 
#endif

	return(TXRTSPacketL);
}

uint16_t SX126XLT::CollisionAvoidance0(uint8_t pl, uint8_t cad_number) {

  int e;
	// P=0.1, i.e. 10%
	// we mainly use P=0 in real deployment (see scientific papers for simulation studies)
	uint8_t P=0;
	uint8_t WL=7;
	uint8_t W2=7;
	//W2afterP1 will be 10 if W2=7
	uint8_t W2afterP1=W2+3;
	uint8_t W3=W2;
  double difs;
  uint16_t listenRTSduration;
	bool forceListen=false;
  unsigned long _startDoCad, _endDoCad;
	uint8_t sf=getLoRaSF();
	
  // symbol time in ms
  double ts = 1000.0 / (returnBandwidth() / ( 1 << sf));

  //set difs to packet's preamble length, in ms
  difs=(getPreamble()+((sf<7)?6.25:4.25))*ts;
  
  // WL*DIFS+TOA(sizeof(RTS)), in ms
  listenRTSduration=WL*(uint16_t)difs+getToA(HEADER_SIZE+1);

  PRINT_CSTSTR("--> CA\n");

#ifdef SX126XDEBUGRTS
  PRINT_CSTSTR("--> CA: ts ");
	PRINT_VALUE("%d", (uint16_t)ts);
  PRINT_CSTSTR(" DIFS ");
	PRINT_VALUE("%d", (uint16_t)difs);
	PRINT_CSTSTR(" listenRTSduration ");
	PRINTLN_VALUE("%d", listenRTSduration);			 
#endif

	// first we try to see whether can detect activity
  if (cad_number) {
  	// check for free channel
    _startDoCad=millis();
    e = doCAD(cad_number);
    _endDoCad=millis();
    
    PRINT_CSTSTR("--> --> CA: CAD ");
  	PRINT_VALUE("%d",_endDoCad-_startDoCad);
    PRINTLN;    
  }
  else
  	e==0;
  
  // we detected activity!
  // since there is low probability of false positive
  // it means that there is an ongoing transmission
  // so we return the toa of the maximum packet length, i.e. 255 bytes
  if (e!=0) {
#ifdef SX126XDEBUGRTS
  	PRINT_CSTSTR("--> CA: BUSY DEFER BY MAX_TOA ");
		PRINTLN_VALUE("%d", getToA(255));			 
#endif  
  	return(getToA(255));
	}
	// here e==0 so we detected no activity
	// run the proposed channel access mechanism
	// see scientific article

	//[0,100]	
#ifdef ARDUINO                
  uint8_t myP = random(100+1);
#else
  uint8_t myP = rand() % 100;
#endif		

#ifdef SX126XDEBUGRTS
  	PRINT_CSTSTR("--> CA: FREE, INITIATE CA ");
  	PRINT_CSTSTR("P=");
  	PRINT_VALUE("%d", P);
  	PRINT_CSTSTR(" myP=");
		PRINTLN_VALUE("%d", myP);			 
#endif 
	
	// we will break from this infinite loop with a return
	while (1) {
	
		// 0<= myP <=P then go to phase 2
		// if myP>P we start at phase 1 to listen for RTS
		if (myP>P || forceListen) {
	
			uint8_t RXRTSPacketL;
			const uint8_t RXBUFFER_SIZE=1;
			uint8_t RXBUFFER[RXBUFFER_SIZE];

#ifdef SX126XDEBUGRTS
			PRINT_CSTSTR("--> CA: LISTEN ");
			if (forceListen)
				PRINTLN_CSTSTR("P2");
			else
				PRINTLN_CSTSTR("P1");			
							 
			PRINTLN_CSTSTR("--> CA: WAIT FOR RTS");
#endif
		
			//try to receive the RTS
			RXRTSPacketL=receiveRTSAddressed(RXBUFFER, RXBUFFER_SIZE, listenRTSduration, WAIT_RX);
		
			// length is 1 indicates an RTS
			if (RXRTSPacketL==1) {
#ifdef SX126XDEBUGRTS
				PRINT_CSTSTR("--> CA: RECEIVE RTS(");
				PRINT_VALUE("%d", RXBUFFER[0]);
				PRINT_CSTSTR(") FROM ");
				PRINTLN_VALUE("%d", readRXSource());
				PRINT_CSTSTR("--> CA: NAV(RTS) FOR listenRTSduration+W3*DIFS+ToA(");
				PRINT_VALUE("%d", RXBUFFER[0]);
				PRINT_CSTSTR(") ");						
				PRINT_VALUE("%d", listenRTSduration+W3*(uint16_t)difs+getToA(RXBUFFER[0]));
				PRINTLN_CSTSTR(" ms"); 
#endif		
				return(listenRTSduration+W3*(uint16_t)difs+getToA(RXBUFFER[0]));
			}
			else 
			//indicate a longer data packet
			if (RXRTSPacketL>1) {
#ifdef SX126XDEBUGRTS
				PRINT_CSTSTR("--> CA: ValidHeader FOR DATA DEFER BY MAX_TOA ");
				PRINTLN_VALUE("%d", getToA(255));			 
#endif  
				return(getToA(255));	
			}
			else {
#ifdef SX126XDEBUGRTS
				PRINTLN_CSTSTR("--> CA: NO RTS");		 
#endif		
			}
		}		

    // wait for random number of DIFS 
    // [0, W2] if direct to phase 2
    // [0, W2afterP1] if phase 2 after phase 1
    // [0, W3] in phase 3
    uint8_t w;
        
		// we are actually finishing the second listening period
		// we backoff before transmitting W3*DIFS
		if (forceListen) {
#ifdef SX126XDEBUGRTS
			PRINT_CSTSTR("--> CA: P3 W3=");
			PRINTLN_VALUE("%d", W3);	 
#endif
#ifdef ARDUINO		
			w = random(0,W3+1);
#else
			w = rand() % (W3+1);
#endif			
		}
		else {	
			// we were in phase 1
			if (myP>P) {
#ifdef SX126XDEBUGRTS
				PRINT_CSTSTR("--> CA: P1->P2 W2afterP1=");
				PRINTLN_VALUE("%d", W2afterP1);	 
#endif			  
#ifdef ARDUINO			          
				w = random(0,W2afterP1+1);
#else
				w = rand() % (W2afterP1+1);
#endif								
			}	
			else {
#ifdef SX126XDEBUGRTS
				PRINT_CSTSTR("--> CA: direct P2 W2=");
				PRINTLN_VALUE("%d", W2);	 
#endif			
#ifdef ARDUINO			
				w = random(0,W2+1);	
#else		
				w = rand() % (W2+1);
#endif		
			}		
		}
		
#ifdef SX126XDEBUGRTS
		PRINT_CSTSTR("--> CA: WAIT FOR ");
		PRINT_VALUE("%d", w);
		PRINTLN_CSTSTR(" DIFS");		 
#endif

		if (_lowPowerFctPtr==NULL)
			delay(w*(uint16_t)difs);
		else
			(*_lowPowerFctPtr)(w*(uint16_t)difs);
			
		// if forceListen==true then this is the second time
		// so we just exit after having waited for a random number of DIFS to send the data packet
		// 
		if (forceListen) {
#ifdef SX126XDEBUGRTS
			PRINTLN_CSTSTR("--> CA: EXIT, ALLOW DATA TRANSMIT");
#endif		
			return(0);
		}	
		// if forceListen==false then this is the first time for nodes starting directly at phase 2
		else {	
#ifdef SX126XDEBUGRTS
			PRINTLN_CSTSTR("--> CA: SEND RTS");
#endif			
		
			// phase 2 where we send an RTS
			transmitRTSAddressed(pl);
		
			// go for another listening period
			forceListen=true;
		}
	}
}

uint8_t	SX126XLT::invertIQ(bool invert)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("invertIQ()");
#endif
	if (invert)
		setPacketParams(savedPacketParam1, savedPacketParam2, savedPacketParam3, savedPacketParam4, LORA_IQ_INVERTED);
	else
		setPacketParams(savedPacketParam1, savedPacketParam2, savedPacketParam3, savedPacketParam4, LORA_IQ_NORMAL);	
  return(0);
}

void SX126XLT::setTXSeqNo(uint8_t seqno)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("SetTXSeqNo()");
#endif
  
  _TXSeqNo=seqno;
}

uint8_t SX126XLT::readTXSeqNo()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("readTXSeqNo()");
#endif
  
  return(_TXSeqNo);
}

uint8_t SX126XLT::readRXSeqNo()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("readRXSeqNo()");
#endif
  
  return(_RXSeqNo);
}

uint32_t SX126XLT::readRXTimestamp()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("readRXTimestamp()");
#endif
  
  return(_RXTimestamp);
}

uint32_t SX126XLT::readRXDoneTimestamp()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("readRXDoneTimestamp()");
#endif
  
  return(_RXDoneTimestamp);
}

void SX126XLT::setDevAddr(uint8_t addr)
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("setDevAddr()");
#endif
	
	_DevAddr=addr;
}

uint8_t SX126XLT::readDevAddr()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("readDevAddr()");
#endif

  return(_DevAddr);
}

uint8_t SX126XLT::readAckStatus()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("readAckStatus()");
#endif

  return(_AckStatus);
}

int8_t SX126XLT::readPacketSNRinACK()
{
#ifdef SX126XDEBUG1
  PRINTLN_CSTSTR("readPacketSNRinACK()");
#endif

  return(_PacketSNRinACK);
}

void SX126XLT::setLowPowerFctPtr(void (*pf)(unsigned long))
{
#ifdef SX126XDEBUG
  PRINTLN_CSTSTR("setLowPowerFctPtr()");
#endif

	_lowPowerFctPtr=pf;
}
/**************************************************************************
  End by C. Pham - Oct. 2020
**************************************************************************/
 
 
/*
  MIT license

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
  documentation files (the "Software"), to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
  and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions
  of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
  CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.
*/
