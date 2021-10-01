/*
  Copyright 2019 - Stuart Robinson
  Licensed under a MIT license displayed at the bottom of this document.
  06/02/20
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
  - Add ack transaction with transmitAddressed-receiveAddressed	- done
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
  - add getToA(uint8_t pl) function to get the time-on-air of a pl-byte packet according to current LoRa settings - done - to test
  - change the order of header and add a seq number in the header - done
  	- for transmitAddressed, receiveAddressed
  	- new 4-byte header is : destination, packet type, source, sequence number
  - add readRXDestination() and readRXSource() that were missing	- done
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

#include <SX128XLT.h>

//use polling with RADIO_GET_IRQSTATUS instead of DIO1
#define USE_POLLING
//invert IQ in ack transaction
#define INVERTIQ_ON_ACK
/**************************************************************************
End by C. Pham - Oct. 2020
**************************************************************************/

#define LTUNUSED(v) (void) (v)    //add LTUNUSED(variable); to avoid compiler warnings 

//#define SX128XDEBUG             //enable debug messages
//#define SX128XDEBUG3             //enable debug messages
#define SX128XDEBUG_TEST           //enable test debug messages
#define SX128XDEBUGACK               //enable ack transaction debug messages
#define SX128XDEBUGRTS
//#define RANGINGDEBUG            //enable debug messages for ranging
//#define SX128XDEBUGRXTX        //enable debug messages for RX TX switching
//#define SX128XDEBUGPINS           //enable pin allocation debug messages

SX128XLT::SX128XLT()
{
  /**************************************************************************
	Added by C. Pham - Oct. 2020
  **************************************************************************/  
  
  _TXSeqNo = 0;
  _RXSeqNo = 0;
  
  _lowPowerFctPtr=NULL;

//#define RADIO_FAKE_TEST
#ifdef RADIO_FAKE_TEST 
  savedModParam1 = LORA_SF12;
  savedModParam2 = LORA_BW_0200;
  savedModParam3 = LORA_CR_4_5;
  
  savedPacketParam1 = 12;
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
1 All pins > begin(int8_t pinNSS, int8_t pinNRESET, int8_t pinRFBUSY, int8_t pinDIO1, int8_t pinDIO2, int8_t pinDIO3, int8_t pinRXEN, int8_t pinTXEN, uint8_t device)
2 NiceRF   > begin(int8_t pinNSS, int8_t pinNRESET, int8_t pinRFBUSY, int8_t pinDIO1, uint8_t device)
3 Ebyte    > begin(int8_t pinNSS, int8_t pinNRESET, int8_t pinRFBUSY, int8_t pinDIO1, int8_t pinRXEN, int8_t pinTXEN, uint8_t device); 
*/


bool SX128XLT::begin(int8_t pinNSS, int8_t pinNRESET, int8_t pinRFBUSY, int8_t pinDIO1, int8_t pinDIO2, int8_t pinDIO3, int8_t pinRXEN, int8_t pinTXEN, uint8_t device)
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
  _Device = device;
  _TXDonePin = pinDIO1;        //this is defalt pin for sensing TX done
  _RXDonePin = pinDIO1;        //this is defalt pin for sensing RX done

  pinMode(_NSS, OUTPUT);
  digitalWrite(_NSS, HIGH);
  pinMode(_NRESET, OUTPUT);
  digitalWrite(_NRESET, LOW);
  pinMode(_RFBUSY, INPUT);


#ifdef SX128XDEBUGPINS
  PRINTLN_CSTSTR("begin()");
  PRINTLN_CSTSTR("SX128XLT constructor instantiated successfully");
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
#ifdef SX128XDEBUGPINS
   	PRINTLN_CSTSTR("RX_EN & TX_EN switching enabled");
#endif
   	pinMode(_RXEN, OUTPUT);
   	pinMode(_TXEN, OUTPUT);
   	_rxtxpinmode = true;
  }
  else
	{
#ifdef SX128XDEBUGPINS
  	PRINTLN_CSTSTR("RX_EN & TX_EN switching disabled");
#endif
  	_rxtxpinmode = false;
  }

  resetDevice();

  if (checkDevice())
  {
    return true;
  }

  return false;
}


bool SX128XLT::begin(int8_t pinNSS, int8_t pinNRESET, int8_t pinRFBUSY, int8_t pinDIO1, uint8_t device)
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
  _RXEN = -1;                  //not defined, so mark as unused
  _TXEN = -1;                  //not defined, so mark as unused
  _Device = device;
  _TXDonePin = pinDIO1;        //this is defalt pin for sensing TX done
  _RXDonePin = pinDIO1;        //this is defalt pin for sensing RX done

  pinMode(_NSS, OUTPUT);
  digitalWrite(_NSS, HIGH);
  pinMode(_NRESET, OUTPUT);
  digitalWrite(_NRESET, LOW);
  pinMode(_RFBUSY, INPUT);

#ifdef SX128XDEBUGPINS
  PRINTLN_CSTSTR("format 2 NiceRF begin()");
  PRINTLN_CSTSTR("SX128XLT constructor instantiated successfully");
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
#endif

  if (_DIO1 >= 0)
  {
    pinMode( _DIO1, INPUT);
  }
  
#ifdef SX128XDEBUGPINS
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


bool SX128XLT::begin(int8_t pinNSS, int8_t pinNRESET, int8_t pinRFBUSY, int8_t pinDIO1, int8_t pinRXEN, int8_t pinTXEN, uint8_t device)
{

#if not defined ARDUINO && not defined USE_ARDUPI
  wiringPiSetup() ;
  PRINTLN_CSTSTR("<<< using wiringPi >>>"); 
#endif

  //format 3 pins for Ebyte, NSS, NRESET, RFBUSY, DIO1, RX_EN, TX_EN 
  _NSS = pinNSS;
  _NRESET = pinNRESET;
  _RFBUSY = pinRFBUSY;
  _DIO1 = pinDIO1;
  _DIO2 = -1;
  _DIO3 = -1;
  _RXEN = pinRXEN;
  _TXEN = pinTXEN;
  _Device = device;
  _TXDonePin = pinDIO1;        //this is defalt pin for sensing TX done
  _RXDonePin = pinDIO1;        //this is defalt pin for sensing RX done

  pinMode(_NSS, OUTPUT);
  digitalWrite(_NSS, HIGH);
  pinMode(_NRESET, OUTPUT);
  digitalWrite(_NRESET, LOW);
  pinMode(_RFBUSY, INPUT);

#ifdef SX128XDEBUGPINS
  PRINTLN_CSTSTR("format 3 Ebyte begin()");
  PRINTLN_CSTSTR("SX128XLT constructor instantiated successfully");
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
#endif

  if (_DIO1 >= 0)
  {
    pinMode( _DIO1, INPUT);
  }
 
  if ((_RXEN >= 0) && (_TXEN >= 0))
  {
#ifdef SX128XDEBUGPINS
   	PRINTLN_CSTSTR("RX_EN & TX_EN switching enabled");
#endif
   	pinMode(_RXEN, OUTPUT);
   	pinMode(_TXEN, OUTPUT);
   	_rxtxpinmode = true;
  }
  else
  {
#ifdef SX128XDEBUGPINS
  	PRINTLN_CSTSTR("RX_EN & TX_EN switching disabled");
#endif
  	_rxtxpinmode = false;
  }

  resetDevice();

  if (checkDevice())
  {
    return true;
  }

  return false;
}


void SX128XLT::rxEnable()
{
  //Enable RX mode on device such as Ebyte E28-2G4M20S which have RX and TX enable pins
#ifdef SX128XDEBUGRXTX
  PRINTLN_CSTSTR("rxEnable()");
#endif

  digitalWrite(_RXEN, HIGH);
  digitalWrite(_TXEN, LOW);
}


void SX128XLT::txEnable()
{
  //Enable RX mode on device such as Ebyte E28-2G4M20S which have RX and TX enable pins
#ifdef SX128XDEBUGRXTX
  PRINTLN_CSTSTR("txEnable()");
#endif

  digitalWrite(_RXEN, LOW);
  digitalWrite(_TXEN, HIGH);
}



void SX128XLT::checkBusy()
{
#ifdef SX128XDEBUG
  //PRINTLN_CSTSTR("checkBusy()");
#endif

  uint8_t busy_timeout_cnt;
  busy_timeout_cnt = 0;

  while (digitalRead(_RFBUSY))
  {
    delay(1);
    busy_timeout_cnt++;

    if (busy_timeout_cnt > 10)                     //wait 5mS for busy to complete
    {
      PRINTLN_CSTSTR("ERROR - Busy Timeout!");
      resetDevice();
      setMode(MODE_STDBY_RC);
      config();                                   //re-run saved config
      break;
    }
  }
}


bool SX128XLT::config()
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("config()");
#endif

  resetDevice();
  setMode(MODE_STDBY_RC);
  setRegulatorMode(savedRegulatorMode);
  setPacketType(savedPacketType);
  setRfFrequency(savedFrequency, savedOffset);
  setModulationParams(savedModParam1, savedModParam2, savedModParam3);
  setPacketParams(savedPacketParam1, savedPacketParam2, savedPacketParam3, savedPacketParam4, savedPacketParam5, savedPacketParam6, savedPacketParam7);
  setDioIrqParams(savedIrqMask, savedDio1Mask, savedDio2Mask, savedDio3Mask);       //set for IRQ on RX done on DIO1
  return true;
}

void SX128XLT::readRegisters(uint16_t address, uint8_t *buffer, uint16_t size)
{
#ifdef SX128XDEBUG
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

uint8_t SX128XLT::readRegister(uint16_t address)
{
#ifdef SX128XDEBUG
  //PRINTLN_CSTSTR("readRegister()");
#endif

  uint8_t data;

  readRegisters(address, &data, 1);
  return data;
}

void SX128XLT::writeRegisters(uint16_t address, uint8_t *buffer, uint16_t size)
{
#ifdef SX128XDEBUG
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

  //checkBusy();
}

void SX128XLT::writeRegister(uint16_t address, uint8_t value)
{
#ifdef SX128XDEBUG
  //PRINTLN_CSTSTR("writeRegister()");
#endif

  writeRegisters( address, &value, 1 );
}


void SX128XLT::writeCommand(uint8_t Opcode, uint8_t *buffer, uint16_t size)
{
#ifdef SX128XDEBUG
  //PRINT_CSTSTR("writeCommand() ");
  //PRINTLN_HEX("%X",Opcode);
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


void SX128XLT::readCommand(uint8_t Opcode, uint8_t *buffer, uint16_t size)
{
#ifdef SX128XDEBUG
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
  //checkBusy();
}

void SX128XLT::resetDevice()
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("resetDevice()");
#endif

  //timings taken from Semtech library
  delay(20);
  digitalWrite(_NRESET, LOW);
  delay(50);
  digitalWrite(_NRESET, HIGH);
  delay(20);

  /**************************************************************************
	Added by C. Pham - Oct. 2020
  **************************************************************************/   
  setCadParams();
  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/   
}


bool SX128XLT::checkDevice()
{
  //check there is a device out there, writes a register and reads back
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("checkDevice()");
#endif

  uint8_t Regdata1, Regdata2;
  Regdata1 = readRegister(0x0908);               //low byte of frequency setting
  writeRegister(0x0908, (Regdata1 + 1));
  Regdata2 = readRegister(0x0908);               //read changed value back
  writeRegister(0x0908, Regdata1);               //restore register to original value

  if (Regdata2 == (Regdata1 + 1))
  {
    return true;
  }
  else
  {
    return false;
  }
}


void SX128XLT::setupLoRa(uint32_t frequency, int32_t offset, uint8_t modParam1, uint8_t modParam2, uint8_t  modParam3)
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("setupLoRa()");
#endif

  setMode(MODE_STDBY_RC);
  setRegulatorMode(USE_LDO);
  setPacketType(PACKET_TYPE_LORA);
  setRfFrequency(frequency, offset);
  setBufferBaseAddress(0, 0);
  setModulationParams(modParam1, modParam2, modParam3);
  setPacketParams(12, LORA_PACKET_VARIABLE_LENGTH, 255, LORA_CRC_ON, LORA_IQ_NORMAL, 0, 0);
  setDioIrqParams(IRQ_RADIO_ALL, (IRQ_TX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);
}


void SX128XLT::setMode(uint8_t modeconfig)
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("setMode()");
#endif

  uint8_t Opcode = 0x80;

  checkBusy();

#ifdef USE_SPI_TRANSACTION     //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);
#if defined ARDUINO || defined USE_ARDUPI  
  SPI.transfer(Opcode);
  SPI.transfer(modeconfig);
#else
	uint8_t spibuf[2];
	spibuf[0] = Opcode;
	spibuf[1] = modeconfig;
	wiringPiSPIDataRW(SPI_CHANNEL, spibuf, 2);
#endif  
  digitalWrite(_NSS, HIGH);

#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif

  _OperatingMode = modeconfig;

}

void SX128XLT::setRegulatorMode(uint8_t mode)
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("setRegulatorMode()");
#endif

  savedRegulatorMode = mode;

  writeCommand(RADIO_SET_REGULATORMODE, &mode, 1);
}

void SX128XLT::setPacketType(uint8_t packettype )
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("setPacketType()");
#endif
  savedPacketType = packettype;

  writeCommand(RADIO_SET_PACKETTYPE, &packettype, 1);
}


void SX128XLT::setRfFrequency(uint32_t frequency, int32_t offset)
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("setRfFrequency()");
#endif

  savedFrequency = frequency;
  savedOffset = offset;

  frequency = frequency + offset;
  uint8_t buffer[3];
  uint32_t freqtemp = 0;
  freqtemp = ( uint32_t )( (double) frequency / (double)FREQ_STEP);
  buffer[0] = ( uint8_t )( ( freqtemp >> 16 ) & 0xFF );
  buffer[1] = ( uint8_t )( ( freqtemp >> 8 ) & 0xFF );
  buffer[2] = ( uint8_t )( freqtemp & 0xFF );
  writeCommand(RADIO_SET_RFFREQUENCY, buffer, 3);
}

void SX128XLT::setBufferBaseAddress(uint8_t txBaseAddress, uint8_t rxBaseAddress)
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("setBufferBaseAddress()");
#endif

  uint8_t buffer[2];

  buffer[0] = txBaseAddress;
  buffer[1] = rxBaseAddress;
  writeCommand(RADIO_SET_BUFFERBASEADDRESS, buffer, 2);
}


void SX128XLT::setModulationParams(uint8_t modParam1, uint8_t modParam2, uint8_t  modParam3)
{
	//sequence is spreading factor, bandwidth, coding rate.
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("setModulationParams()");
#endif

  uint8_t buffer[3];

  savedModParam1 = modParam1;
  savedModParam2 = modParam2;
  savedModParam3 = modParam3;

  buffer[0] = modParam1;
  buffer[1] = modParam2;
  buffer[2] = modParam3;

  writeCommand(RADIO_SET_MODULATIONPARAMS, buffer, 3);
}


void SX128XLT::setPacketParams(uint8_t packetParam1, uint8_t  packetParam2, uint8_t packetParam3, uint8_t packetParam4, uint8_t packetParam5, uint8_t packetParam6, uint8_t packetParam7)
{
	//for LoRa order is PreambleLength, HeaderType, PayloadLength, CRC, InvertIQ/chirp invert, not used, not used
	//for FLRC order is PreambleLength, SyncWordLength, SyncWordMatch, HeaderType, PayloadLength, CrcLength, Whitening
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("SetPacketParams()");
#endif

  savedPacketParam1 = packetParam1;
  savedPacketParam2 = packetParam2;
  savedPacketParam3 = packetParam3;
  savedPacketParam4 = packetParam4;
  savedPacketParam5 = packetParam5;
  savedPacketParam6 = packetParam6;
  savedPacketParam7 = packetParam7;

  uint8_t buffer[7];
  buffer[0] = packetParam1;
  buffer[1] = packetParam2;
  buffer[2] = packetParam3;
  buffer[3] = packetParam4;
  buffer[4] = packetParam5;
  buffer[5] = packetParam6;
  buffer[6] = packetParam7;
  writeCommand(RADIO_SET_PACKETPARAMS, buffer, 7);
}


void SX128XLT::setDioIrqParams(uint16_t irqMask, uint16_t dio1Mask, uint16_t dio2Mask, uint16_t dio3Mask )
{
#ifdef SX128XDEBUG
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
  writeCommand(RADIO_SET_DIOIRQPARAMS, buffer, 8);
}


void SX128XLT::setHighSensitivity()
{
  //set bits 7,6 of REG_LNA_REGIME
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("setHighSensitivity()");
#endif

  writeRegister(REG_LNA_REGIME, (readRegister(REG_LNA_REGIME) | 0xC0));
}

void SX128XLT::setLowPowerRX()
{
  //clear bits 7,6 of REG_LNA_REGIME
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("setLowPowerRX()");
#endif

  writeRegister(REG_LNA_REGIME, (readRegister(REG_LNA_REGIME) & 0x3F));
}


void SX128XLT::printModemSettings()
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("printModemSettings()");
#endif

  printDevice();
  PRINT_CSTSTR(",");
  PRINT_VALUE("%lu",getFreqInt());
  PRINT_CSTSTR("hz");
  
  /**************************************************************************
	Added by C. Pham - Oct. 2020
  **************************************************************************/  
  
  if (savedPacketType == PACKET_TYPE_LORA || savedPacketType == PACKET_TYPE_RANGING)

  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/  
  
  /* original code
   ***************  
  
  if (savedPacketType == PACKET_TYPE_LORA)
  */
  {
		PRINT_CSTSTR(",SF");
		PRINT_VALUE("%d",getLoRaSF());
		PRINT_CSTSTR(",BW");
		PRINT_VALUE("%ld",returnBandwidth(savedModParam2));
		PRINT_CSTSTR(",CR4:");
		PRINT_VALUE("%d",(getLoRaCodingRate() + 4));

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
	
	if (savedPacketType == PACKET_TYPE_FLRC) 
	{
		if (savedPacketParam1 == 0)
		{
	 		PRINT_CSTSTR(",No_Syncword");
		}

		if (savedPacketParam1 == 4)
		{
		 	PRINT_CSTSTR(",32bit_Syncword");
		}  


		switch (savedPacketParam3)
		{
			case RADIO_RX_MATCH_SYNCWORD_OFF:
				PRINT_CSTSTR(",SYNCWORD_OFF");
				break;

			case RADIO_RX_MATCH_SYNCWORD_1:
				PRINT_CSTSTR(",SYNCWORD_1");
				break;

			case RADIO_RX_MATCH_SYNCWORD_2:
				PRINT_CSTSTR(",SYNCWORD_2");
				break;

			case RADIO_RX_MATCH_SYNCWORD_1_2:
				PRINT_CSTSTR(",SYNCWORD_1_2");
				break;

			case RADIO_RX_MATCH_SYNCWORD_3:
				PRINT_CSTSTR(",SYNCWORD_3");
				break;

			case RADIO_RX_MATCH_SYNCWORD_1_3:
				PRINT_CSTSTR(",SYNCWORD_1_3");
				break;

			case RADIO_RX_MATCH_SYNCWORD_2_3:
				PRINT_CSTSTR(",SYNCWORD_2_3");
				break;

			case RADIO_RX_MATCH_SYNCWORD_1_2_3:
				PRINT_CSTSTR(",SYNCWORD_1_2_3");
				break;

			default:
				PRINT_CSTSTR("Unknown_SYNCWORD");
		}
 
		if (savedPacketParam4 == RADIO_PACKET_FIXED_LENGTH)
		{
		 	PRINT_CSTSTR(",PACKET_FIXED_LENGTH");
		}  

		if (savedPacketParam4 == RADIO_PACKET_VARIABLE_LENGTH)
		{
		 	PRINT_CSTSTR(",PACKET_VARIABLE_LENGTH");
		}  

		switch (savedPacketParam6)
		{
			case RADIO_CRC_OFF:
				PRINT_CSTSTR(",CRC_OFF");
				break;

			case RADIO_CRC_1_BYTES:
				PRINT_CSTSTR(",CRC_1_BYTES");
				break;

			case RADIO_CRC_2_BYTES:
				PRINT_CSTSTR(",CRC_2_BYTES");
				break;

			case RADIO_CRC_3_BYTES:
				PRINT_CSTSTR(",CRC_3_BYTES");
				break;

			default:
				PRINT_CSTSTR(",Unknown_CRC");
		}

		if (savedPacketParam7 == RADIO_WHITENING_ON)
		{
		 	PRINT_CSTSTR(",WHITENING_ON");
		}  

		if (savedPacketParam7 == RADIO_WHITENING_OFF)
		{
		 	PRINT_CSTSTR(",WHITENING_OFF");
		}  
  }
}


void SX128XLT::printDevice()
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("printDevice()");
#endif


  switch (_Device)
  {
    case DEVICE_SX1280:
      PRINT_CSTSTR("SX1280");
      break;

    case DEVICE_SX1281:
      PRINT_CSTSTR("SX1281");
      break;

    default:
      PRINT_CSTSTR("Unknown Device");

  }
}


uint32_t SX128XLT::getFreqInt()
{

#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("getFreqInt");
#endif

  //get the current set device frequency, return as long integer
  uint8_t Msb = 0;
  uint8_t Mid = 0;
  uint8_t Lsb = 0;

  uint32_t uinttemp;
  float floattemp;
  
  LTUNUSED(Msb);           //to prevent a compiler warning
  LTUNUSED(Mid);           //to prevent a compiler warning
  LTUNUSED(Lsb);           //to prevent a compiler warning

  /**************************************************************************
	Added by C. Pham - Oct. 2020
  **************************************************************************/  
  
  if (savedPacketType == PACKET_TYPE_LORA || savedPacketType == PACKET_TYPE_RANGING)

  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/  
  
  /* original code
   ***************  
  
  if (savedPacketType == PACKET_TYPE_LORA)
  */
  { 
  Msb = readRegister(REG_RFFrequency23_16);
  Mid = readRegister(REG_RFFrequency15_8);
  Lsb = readRegister(REG_RFFrequency7_0);
  }
  
  if (savedPacketType == PACKET_TYPE_FLRC)
  { 
  Msb = readRegister(REG_FLRC_RFFrequency23_16);
  Mid = readRegister(REG_FLRC_RFFrequency15_8);
  Lsb = readRegister(REG_FLRC_RFFrequency7_0);
  }
  
  floattemp = ((Msb * 0x10000ul) + (Mid * 0x100ul) + Lsb);
  floattemp = ((floattemp * FREQ_STEP) / 1000000ul);
  uinttemp = (uint32_t)(floattemp * 1000000);
  return uinttemp;
}


uint8_t SX128XLT::getLoRaSF()
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("getLoRaSF()");
#endif
  return (savedModParam1 >> 4);
}


uint32_t SX128XLT::returnBandwidth(uint8_t data)
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("returnBandwidth()");
#endif

  switch (data)
  {
    case LORA_BW_0200:
      return 203125;

    case LORA_BW_0400:
      return 406250;

    case LORA_BW_0800:
      return 812500;

    case LORA_BW_1600:
      return 1625000;

    default:
      break;
  }

  return 0x0;                      //so that a bandwidth not set can be identified
}


uint8_t SX128XLT::getLoRaCodingRate()
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("getLoRaCodingRate");
#endif

  return savedModParam3;
}


uint8_t SX128XLT::getInvertIQ()
{
//IQ mode reg 0x33
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("getInvertIQ");
#endif

  return savedPacketParam5;
}


uint16_t SX128XLT::getPreamble()
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("getPreamble");
#endif

  return savedPacketParam1;
}

void SX128XLT::printOperatingSettings()
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("printOperatingSettings()");
#endif

  printDevice();

  PRINT_CSTSTR(",PacketMode_");

  switch (savedPacketType)
  {
    case PACKET_TYPE_GFSK:
      PRINT_CSTSTR("GFSK");
      break;

    case PACKET_TYPE_LORA:
      PRINT_CSTSTR("LORA");
      break;

    case PACKET_TYPE_RANGING:
      PRINT_CSTSTR("RANGING");
      break;

    case PACKET_TYPE_FLRC:
      PRINT_CSTSTR("FLRC");
      break;


    case PACKET_TYPE_BLE:
      PRINT_CSTSTR("BLE");
      break;

    default:
      PRINT_CSTSTR("Unknown");

  }

  switch (savedPacketParam2)
  {
    case LORA_PACKET_VARIABLE_LENGTH:
      PRINT_CSTSTR(",Explicit");
      break;

    case LORA_PACKET_FIXED_LENGTH:
      PRINT_CSTSTR(",Implicit");
      break;

    default:
      PRINT_CSTSTR(",Unknown");
  }

  PRINT_CSTSTR(",LNAgain_");


  if (getLNAgain() == 0xC0)
  {
    PRINT_CSTSTR("HighSensitivity");
  }
  else
  {
    PRINT_CSTSTR("LowPowerRX");
  }

}


uint8_t SX128XLT::getLNAgain()
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("getLNAgain");
#endif

  return (readRegister(REG_LNA_REGIME) & 0xC0);
}



void SX128XLT::printRegisters(uint16_t Start, uint16_t End)
{
  //prints the contents of SX1280 registers to serial monitor

#ifdef SX128XDEBUG
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


void SX128XLT::printASCIIPacket(uint8_t *buffer, uint8_t size)
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("printASCIIPacket()");
#endif

  uint8_t index;

  for (index = 0; index < size; index++)
  {
    Serial.write(buffer[index]);
  }

}


uint8_t SX128XLT::transmit(uint8_t *txbuffer, uint8_t size, uint16_t timeout, int8_t txpower, uint8_t wait)
{
#ifdef SX128XDEBUG
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

  if (savedPacketType == PACKET_TYPE_LORA)
  {
   writeRegister(REG_LR_PAYLOADLENGTH, _TXPacketL);                           //only seems to work for lora  
  }  
  else if (savedPacketType == PACKET_TYPE_FLRC)
  {
  setPacketParams(savedPacketParam1, savedPacketParam2, savedPacketParam3, savedPacketParam4, _TXPacketL, savedPacketParam6, savedPacketParam7);
  }

  setTxParams(txpower, RAMP_TIME);
  setDioIrqParams(IRQ_RADIO_ALL, (IRQ_TX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);   //set for IRQ on TX done and timeout on DIO1
  setTx(timeout);                                                          //this starts the TX

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


void SX128XLT::setTxParams(int8_t TXpower, uint8_t RampTime)
{
#ifdef SX128XDEBUG
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

  //power register is set to 0 to 31 which is -18dBm to +12dBm
  buffer[0] = (TXpower + 18);
  buffer[1] = (uint8_t)RampTime;
  writeCommand(RADIO_SET_TXPARAMS, buffer, 2);
}


void SX128XLT::setTx(uint16_t timeout)
{

#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("setTx()");
#endif

  if (_rxtxpinmode)
  {
    txEnable();
  }

  //PRINT_CSTSTR("timeout "); 
  //PRINTLN_VALUE("%d",timeout);
  //PRINT_CSTSTR("_PERIODBASE "); 
  //PRINTLN_VALUE("%d",_PERIODBASE);
  
  uint8_t buffer[3];

  clearIrqStatus(IRQ_RADIO_ALL);                             //clear all interrupt flags
  buffer[0] = _PERIODBASE;
  buffer[1] = ( uint8_t )( ( timeout >> 8 ) & 0x00FF );
  buffer[2] = ( uint8_t )( timeout & 0x00FF );
  writeCommand(RADIO_SET_TX, buffer, 3 );
}


void SX128XLT::clearIrqStatus(uint16_t irqMask)
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("clearIrqStatus()");
#endif

  uint8_t buffer[2];

  buffer[0] = (uint8_t) (irqMask >> 8);
  buffer[1] = (uint8_t) (irqMask & 0xFF);
  writeCommand(RADIO_CLR_IRQSTATUS, buffer, 2);
}


uint16_t SX128XLT::readIrqStatus()
{
#ifdef SX128XDEBUG
  PRINT_CSTSTR("readIrqStatus()");
#endif

  uint16_t temp;
  uint8_t buffer[2];

  readCommand(RADIO_GET_IRQSTATUS, buffer, 2);
  temp = ((buffer[0] << 8) + buffer[1]);
  return temp;
}


void SX128XLT::printIrqStatus()
{
#ifdef SX128XDEBUG
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
  if (_IrqStatus & IRQ_SYNCWORD_VALID)
  {
    PRINT_CSTSTR(",IRQ_SYNCWORD_VALID");
  }

  //0x0008
  if (_IrqStatus & IRQ_SYNCWORD_ERROR)
  {
    PRINT_CSTSTR(",IRQ_SYNCWORD_ERROR");
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
  if (_IrqStatus & IRQ_RANGING_SLAVE_RESPONSE_DONE)
  {
    PRINT_CSTSTR(",IRQ_RANGING_SLAVE_RESPONSE_DONE");
  }

  //0x0100
  if (_IrqStatus & IRQ_RANGING_SLAVE_REQUEST_DISCARDED)
  {
    PRINT_CSTSTR(",IRQ_RANGING_SLAVE_REQUEST_DISCARDED");
  }

  //0x0200
  if (_IrqStatus & IRQ_RANGING_MASTER_RESULT_VALID)
  {
    PRINT_CSTSTR(",IRQ_RANGING_MASTER_RESULT_VALID");
  }

  //0x0400
  if (_IrqStatus & IRQ_RANGING_MASTER_RESULT_TIMEOUT)
  {
    PRINT_CSTSTR(",IRQ_RANGING_MASTER_RESULT_TIMEOUT");
  }

  //0x0800
  if (_IrqStatus & IRQ_RANGING_SLAVE_REQUEST_VALID)
  {
    PRINT_CSTSTR(",IRQ_RANGING_SLAVE_REQUEST_VALID");
  }

  //0x1000
  if (_IrqStatus & IRQ_CAD_DONE)
  {
    PRINT_CSTSTR(",IRQ_CAD_DONE");
  }

  //0x2000
  if (_IrqStatus & IRQ_CAD_ACTIVITY_DETECTED)
  {
    PRINT_CSTSTR(",IRQ_CAD_ACTIVITY_DETECTED");
  }

  //0x4000
  if (_IrqStatus & IRQ_RX_TX_TIMEOUT)
  {
    PRINT_CSTSTR(",IRQ_RX_TX_TIMEOUT");
  }

  //0x8000
  if (_IrqStatus & IRQ_PREAMBLE_DETECTED)
  {
    PRINT_CSTSTR(",IRQ_PREAMBLE_DETECTED");
  }
}


uint16_t SX128XLT::CRCCCITT(uint8_t *buffer, uint8_t size, uint16_t start)
{
#ifdef SX128XDEBUG
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


uint8_t SX128XLT::receive(uint8_t *rxbuffer, uint8_t size, uint16_t timeout, uint8_t wait)
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("receive()");
#endif

  uint8_t index, RXstart, RXend;
  uint16_t regdata;
  uint8_t buffer[2];

  setDioIrqParams(IRQ_RADIO_ALL, (IRQ_RX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);  //set for IRQ on RX done or timeout
  setRx(timeout);

  if (!wait)
  {
    return 0;                                                               //not wait requested so no packet length to pass
  }

  /**************************************************************************
	Modified by C. Pham - Oct. 2020
  **************************************************************************/

#ifdef USE_POLLING

	do {
		regdata = readIrqStatus();
		
		if (regdata & IRQ_HEADER_VALID)
			_RXTimestamp=millis();
			
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
    return 0;                          //packet is errored somewhere so return 0
  }

  readCommand(RADIO_GET_RXBUFFERSTATUS, buffer, 2);
  _RXPacketL = buffer[0];
  
  if (_RXPacketL > size)               //check passed buffer is big enough for packet
  {
    _RXPacketL = size;                 //truncate packet if not enough space
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

int8_t SX128XLT::readPacketRSSI()
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("readPacketRSSI()");
#endif

  uint8_t status[5];

  readCommand(RADIO_GET_PACKETSTATUS, status, 5) ;
  _PacketRSSI = -status[0] / 2;

	//TODO
	//if the SNR ≤ 0, RSSI_{packet, real} = RSSI_{packet,measured} – SNR_{measured}
  return _PacketRSSI;
}


int8_t SX128XLT::readPacketSNR()
{
#ifdef SX128XDEBUG
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

uint8_t SX128XLT::readRXPacketL()
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("readRXPacketL()");
#endif

  uint8_t buffer[2];

  readCommand(RADIO_GET_RXBUFFERSTATUS, buffer, 2);
  _RXPacketL = buffer[0];
  return _RXPacketL;
}


void SX128XLT::setRx(uint16_t timeout)
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("setRx()");
#endif

  if (_rxtxpinmode)
  {
    rxEnable();
  }

  uint8_t buffer[3];

  clearIrqStatus(IRQ_RADIO_ALL);                             //clear all interrupt flags
  buffer[0] = _PERIODBASE;                                   //use pre determined period base setting
  buffer[1] = ( uint8_t ) ((timeout >> 8 ) & 0x00FF);
  buffer[2] = ( uint8_t ) (timeout & 0x00FF);
  writeCommand(RADIO_SET_RX, buffer, 3);
}

/***************************************************************************
//Start direct access SX buffer routines
***************************************************************************/

//WARNING/TODO C. Pham. Now with wiringPi, successive SPI.transfer() calls is not working anymore!
//Therefore writing/reading directly from buffer is not supported 
  
void SX128XLT::startWriteSXBuffer(uint8_t ptr)
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("startWriteSXBuffer()");
#endif

  _TXPacketL = 0;                   //this variable used to keep track of bytes written
  setMode(MODE_STDBY_RC);
  setBufferBaseAddress(ptr, 0);     //TX,RX
  checkBusy();
  
#ifdef USE_SPI_TRANSACTION        //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif
  
  digitalWrite(_NSS, LOW);
  SPI.transfer(RADIO_WRITE_BUFFER);
  SPI.transfer(ptr);                //address in SX buffer to write to     
  //SPI interface ready for byte to write to buffer
}


uint8_t  SX128XLT::endWriteSXBuffer()
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("endWriteSXBuffer()");
#endif

  digitalWrite(_NSS, HIGH);
  
#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif
  
  return _TXPacketL;
  
}


void SX128XLT::startReadSXBuffer(uint8_t ptr)
{
#ifdef SX128XDEBUG
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


uint8_t SX128XLT::endReadSXBuffer()
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("endReadSXBuffer()");
#endif

  digitalWrite(_NSS, HIGH);
  
#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif
  
  return _RXPacketL;
}


void SX128XLT::writeUint8(uint8_t x)
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("writeUint8()");
#endif

  SPI.transfer(x);

  _TXPacketL++;                     //increment count of bytes written
}

uint8_t SX128XLT::readUint8()
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("readUint8()");
#endif
  byte x;

  x = SPI.transfer(0);

  _RXPacketL++;                      //increment count of bytes read
  return (x);
}


void SX128XLT::writeInt8(int8_t x)
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("writeInt8()");
#endif

  SPI.transfer(x);

  _TXPacketL++;                      //increment count of bytes written
}


int8_t SX128XLT::readInt8()
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("readInt8()");
#endif
  int8_t x;

  x = SPI.transfer(0);

  _RXPacketL++;                      //increment count of bytes read
  return (x);
}


void SX128XLT::writeInt16(int16_t x)
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("writeInt16()");
#endif

  SPI.transfer(lowByte(x));
  SPI.transfer(highByte(x));

  _TXPacketL = _TXPacketL + 2;         //increment count of bytes written
}


int16_t SX128XLT::readInt16()
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("readInt16()");
#endif
  byte lowbyte, highbyte;

  lowbyte = SPI.transfer(0);
  highbyte = SPI.transfer(0);

  _RXPacketL = _RXPacketL + 2;         //increment count of bytes read
  return ((highbyte << 8) + lowbyte);
}


void SX128XLT::writeUint16(uint16_t x)
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("writeUint16()");
#endif

  SPI.transfer(lowByte(x));
  SPI.transfer(highByte(x));

  _TXPacketL = _TXPacketL + 2;         //increment count of bytes written
}


uint16_t SX128XLT::readUint16()
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("writeUint16()");
#endif
  byte lowbyte, highbyte;

  lowbyte = SPI.transfer(0);
  highbyte = SPI.transfer(0);

  _RXPacketL = _RXPacketL + 2;         //increment count of bytes read
  return ((highbyte << 8) + lowbyte);
}


void SX128XLT::writeInt32(int32_t x)
{
#ifdef SX128XDEBUG
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


int32_t SX128XLT::readInt32()
{
#ifdef SX128XDEBUG
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


void SX128XLT::writeUint32(uint32_t x)
{
#ifdef SX128XDEBUG
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


uint32_t SX128XLT::readUint32()
{
#ifdef SX128XDEBUG
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


void SX128XLT::writeFloat(float x)
{
#ifdef SX128XDEBUG
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


float SX128XLT::readFloat()
{
#ifdef SX128XDEBUG
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


uint8_t SX128XLT::transmitSXBuffer(uint8_t startaddr, uint8_t length, uint16_t timeout, int8_t txpower, uint8_t wait)
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("transmitSXBuffer()");
#endif

  setBufferBaseAddress(startaddr, 0);          //TX, RX

  setPacketParams(savedPacketParam1, savedPacketParam2, length, savedPacketParam4, savedPacketParam5, savedPacketParam6, savedPacketParam7);
  setTxParams(txpower, RAMP_TIME);
  setDioIrqParams(IRQ_RADIO_ALL, (IRQ_TX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);   //set for IRQ on TX done and timeout on DIO1
  setTx(timeout);                            //this starts the TX

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


void SX128XLT::writeBuffer(uint8_t *txbuffer, uint8_t size)
{
#ifdef SX128XDEBUG
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


uint8_t SX128XLT::receiveSXBuffer(uint8_t startaddr, uint16_t timeout, uint8_t wait )
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("receiveSXBuffer()");
#endif

  uint16_t regdata;
  uint8_t buffer[2];

  setMode(MODE_STDBY_RC);
  
  setBufferBaseAddress(0, startaddr);               //order is TX RX
  setDioIrqParams(IRQ_RADIO_ALL, (IRQ_RX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);  //set for IRQ on RX done or timeout
  
  setRx(timeout);                                 //no actual RX timeout in this function

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



uint8_t SX128XLT::readBuffer(uint8_t *rxbuffer)
{
#ifdef SX128XDEBUG
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

void SX128XLT::writeBufferChar(char *txbuffer, uint8_t size)
{
#ifdef SX128XDEBUG
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

  SPI.transfer(0);                     //this ensures last byte of buffer writen really is a null (0)

}


uint8_t SX128XLT::readBufferChar(char *rxbuffer)
{
#ifdef SX128XDEBUG
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

void SX128XLT::setSyncWord1(uint32_t syncword)
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("setSyncWord1()");
#endif

  //For FLRC packet type, the SyncWord is one byte shorter and
  //the base address is shifted by one byte
  writeRegister( REG_FLRCSYNCWORD1_BASEADDR, ( syncword >> 24 ) & 0x000000FF );
  writeRegister( REG_FLRCSYNCWORD1_BASEADDR + 1, ( syncword >> 16 ) & 0x000000FF );
  writeRegister( REG_FLRCSYNCWORD1_BASEADDR + 2, ( syncword >> 8 ) & 0x000000FF );
  writeRegister( REG_FLRCSYNCWORD1_BASEADDR + 3, syncword & 0x000000FF );
}

//*******************************************************************************
//Start Ranging routines
//*******************************************************************************


void SX128XLT::setRangingSlaveAddress(uint32_t address)
{
//sets address of ranging slave
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("SetRangingSlaveAddress()");
#endif

  uint8_t buffer[4];

  buffer[0] = (address >> 24u ) & 0xFFu;
  buffer[1] = (address >> 16u) & 0xFFu;
  buffer[2] = (address >>  8u) & 0xFFu;
  buffer[3] = (address & 0xFFu);

  writeRegisters(0x916, buffer, 4 );
}


void SX128XLT::setRangingMasterAddress(uint32_t address)
{
//sets address of ranging master
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("SetRangingMasterAddress()");
#endif

  uint8_t buffer[4];

  buffer[0] = (address >> 24u ) & 0xFFu;
  buffer[1] = (address >> 16u) & 0xFFu;
  buffer[2] = (address >>  8u) & 0xFFu;
  buffer[3] = (address & 0xFFu);

  writeRegisters(0x912, buffer, 4 );
}


void SX128XLT::setRangingCalibration(uint16_t cal)
{
   #ifdef SX128XDEBUG
  PRINTLN_CSTSTR("setRangingCalibration()");
#endif
  savedCalibration = cal;
  writeRegister( REG_LR_RANGINGRERXTXDELAYCAL, ( uint8_t )( ( cal >> 8 ) & 0xFF ) );
  writeRegister( REG_LR_RANGINGRERXTXDELAYCAL + 1, ( uint8_t )( ( cal ) & 0xFF ) );
}


void SX128XLT::setRangingRole(uint8_t role)
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("setRangingRole()");
#endif

  uint8_t buffer[1];

  buffer[0] = role;

  writeCommand(RADIO_SET_RANGING_ROLE, buffer, 1 );
}


uint32_t SX128XLT::getRangingResultRegValue(uint8_t resultType)
{
  uint32_t valLsb = 0;

  setMode(MODE_STDBY_XOSC);
  writeRegister( 0x97F, readRegister( 0x97F ) | ( 1 << 1 ) ); // enable LORA modem clock
  writeRegister( REG_LR_RANGINGRESULTCONFIG, ( readRegister( REG_LR_RANGINGRESULTCONFIG ) & MASK_RANGINGMUXSEL ) | ( ( ( ( uint8_t )resultType ) & 0x03 ) << 4 ) );
  valLsb = ( ( (uint32_t) readRegister( REG_LR_RANGINGRESULTBASEADDR ) << 16 ) | ( (uint32_t) readRegister( REG_LR_RANGINGRESULTBASEADDR + 1 ) << 8 ) | ( readRegister( REG_LR_RANGINGRESULTBASEADDR + 2 ) ) );
  setMode(MODE_STDBY_RC);
  return valLsb;
}


double SX128XLT::getRangingDistance(uint8_t resultType, int32_t regval, float adjust)
{
  float val = 0.0;

  if (regval >= 0x800000)                  //raw reg value at low distance can goto 0x800000 which is negative, set distance to zero if this happens
  {
    regval = 0;
  }

  // Conversion from LSB to distance. For explanation on the formula, refer to Datasheet of SX1280

  switch (resultType)
  {
    case RANGING_RESULT_RAW:
      // Convert the ranging LSB to distance in meter. The theoretical conversion from register value to distance [m] is given by:
      // distance [m] = ( complement2( register ) * 150 ) / ( 2^12 * bandwidth[MHz] ) ). The API provide BW in [Hz] so the implemented
      // formula is complement2( register ) / bandwidth[Hz] * A, where A = 150 / (2^12 / 1e6) = 36621.09
      val = ( double ) regval / ( double ) returnBandwidth(savedModParam2) * 36621.09375;
      break;

    case RANGING_RESULT_AVERAGED:
    case RANGING_RESULT_DEBIASED:
    case RANGING_RESULT_FILTERED:
      PRINT_CSTSTR("??");
      val = ( double )regval * 20.0 / 100.0;
      break;
    default:
      val = 0.0;
      break;
  }
  
  val = val * adjust;
  return val;
}


bool SX128XLT::setupRanging(uint32_t frequency, int32_t offset, uint8_t modParam1, uint8_t modParam2, uint8_t  modParam3, uint32_t address, uint8_t role)
{
 //sequence is frequency, offset, spreading factor, bandwidth, coding rate, calibration, role.  
 #ifdef SX128XDEBUG
  PRINTLN_CSTSTR("setupRanging()");
#endif
 
  setMode(MODE_STDBY_RC);
  setPacketType(PACKET_TYPE_RANGING);
  setModulationParams(modParam1, modParam2, modParam3);  
  setPacketParams(12, LORA_PACKET_VARIABLE_LENGTH, 0, LORA_CRC_ON, LORA_IQ_NORMAL, 0, 0);
  setRfFrequency(frequency, offset);
  setRangingSlaveAddress(address);
  setRangingMasterAddress(address);
  setRangingCalibration(lookupCalibrationValue(modParam1, modParam2));
  setRangingRole(role);
  setHighSensitivity();
  return true;
}



bool SX128XLT::transmitRanging(uint32_t address, uint16_t timeout, int8_t txpower, uint8_t wait)
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("transmitRanging()");
#endif

  if ((_RXEN >= 0) || (_TXEN >= 0))
  {
   return false;
  }    
  
  setMode(MODE_STDBY_RC);
  setRangingMasterAddress(address);
  setTxParams(txpower, RADIO_RAMP_02_US);
  setDioIrqParams(IRQ_RADIO_ALL, (IRQ_TX_DONE + IRQ_RANGING_MASTER_RESULT_VALID + IRQ_RANGING_MASTER_RESULT_TIMEOUT), 0, 0);
  setTx(timeout);                               //this sends the ranging packet
    
  if (!wait)
  {
    return true;
  }

  /**************************************************************************
	Modified by C. Pham - Oct. 2020
  **************************************************************************/
  
#ifdef USE_POLLING

	uint16_t regdata;
	
	do {
		regdata = readIrqStatus();
			
	} while ( !(regdata & IRQ_TX_DONE) && !(regdata & IRQ_RANGING_MASTER_RESULT_VALID) && !(regdata & IRQ_RANGING_MASTER_RESULT_TIMEOUT) );
	
	if (regdata & IRQ_RANGING_MASTER_RESULT_VALID)
#else		 

  while (!digitalRead(_TXDonePin));                               //Wait for DIO1 to go high
  
  if (readIrqStatus() & IRQ_RANGING_MASTER_RESULT_VALID)       //check for timeout  
#endif
  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/
  {
    return true;
  }
  else
  {
    return false;
  }
}


uint8_t SX128XLT::receiveRanging(uint32_t address, uint16_t timeout, int8_t txpower, uint8_t wait)
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("receiveRanging()");
#endif
  
  setTxParams(txpower, RADIO_RAMP_02_US);
  setRangingSlaveAddress(address);
  setDioIrqParams(IRQ_RADIO_ALL, (IRQ_RANGING_SLAVE_RESPONSE_DONE + IRQ_RANGING_SLAVE_REQUEST_DISCARDED), 0, 0);
  setRx(timeout);

  if (!wait)
  {
    return NO_WAIT;                                                            //not wait requested so no packet length to pass
  }

  /**************************************************************************
	Modified by C. Pham - Oct. 2020
  **************************************************************************/
  
#ifdef USE_POLLING

	uint16_t regdata;
	
	do {
		regdata = readIrqStatus();
			
	} while ( !(regdata & IRQ_RANGING_SLAVE_RESPONSE_DONE) && !(regdata & IRQ_RANGING_SLAVE_REQUEST_DISCARDED) );

  setMode(MODE_STDBY_RC);                                                    //ensure to stop further packet reception
  	
	if (regdata & IRQ_RANGING_SLAVE_REQUEST_VALID)
#else		 

  while (!digitalRead(_RXDonePin));

  setMode(MODE_STDBY_RC);                                                    //ensure to stop further packet reception
    
  if (readIrqStatus() & IRQ_RANGING_SLAVE_REQUEST_VALID)       //check for timeout  
#endif
  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/
  {
  return true; 
  }
  else
  { 
  return false;                                                               //so we can check for packet having enough buffer space
  }
}


uint16_t SX128XLT::lookupCalibrationValue(uint8_t spreadingfactor, uint8_t bandwidth)
{
//this looks up the calibration value from the table in SX128XLT_Definitions.hifdef SX128XDEBUG
#ifdef SX128XDEBUG
	PRINTLN_CSTSTR("lookupCalibrationValue()");
#endif

	switch (bandwidth)
  {
    case LORA_BW_0400:
      savedCalibration = RNG_CALIB_0400[(spreadingfactor>>4)-5];
      return savedCalibration;
  
    case LORA_BW_0800:
      savedCalibration = RNG_CALIB_0800[(spreadingfactor>>4)-5];
      return savedCalibration;
  

    case LORA_BW_1600:
     savedCalibration = RNG_CALIB_1600[(spreadingfactor>>4)-5];
     return savedCalibration;

    default:
      return 0xFFFF;

  }
}


uint16_t SX128XLT::getSetCalibrationValue()
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("getCalibrationValue()");
#endif

	return savedCalibration;;
  
}


//*******************************************************************************
//End Ranging routines
//*******************************************************************************


void SX128XLT::setSleep(uint8_t sleepconfig)
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("setSleep()");
#endif
  
  setMode(MODE_STDBY_RC);
  checkBusy();
  
#ifdef USE_SPI_TRANSACTION     //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  //need to save registers to device RAM first
  digitalWrite(_NSS, LOW); 
  SPI.transfer(RADIO_SET_SAVECONTEXT);
  digitalWrite(_NSS, HIGH);
  
  checkBusy();
  
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
  delay(1);           //allow time for shutdown
}


uint16_t SX128XLT::CRCCCITTSX(uint8_t startadd, uint8_t endadd, uint16_t startvalue)
{
  //genrates a CRC of an area of the internal SX buffer

#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("CRCCCITTSX()");
#endif

  uint16_t index, libraryCRC;
  uint8_t j;

  libraryCRC = startvalue;                                  //start value for CRC16

  startReadSXBuffer(startadd);                       //begin the buffer read

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
  
  endReadSXBuffer();                                 //end the buffer read

  return libraryCRC;
}



uint8_t SX128XLT::getByteSXBuffer(uint8_t addr)
{
#ifdef SX128XDEBUG
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


void SX128XLT::printSXBufferHEX(uint8_t start, uint8_t end)
{
#ifdef SX128XDEBUG
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


void SX128XLT::printHEXByte(uint8_t temp)
{
  if (temp < 0x10)
  {
    PRINT_CSTSTR("0");
  }
  PRINT_HEX("%X",temp);
}


void SX128XLT::wake()
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("wake()");
#endif

	digitalWrite(_NSS, LOW);
	delay(1);
	digitalWrite(_NSS, HIGH);
	delay(1);
}


int32_t SX128XLT::getFrequencyErrorRegValue()
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("getFrequencyErrorRegValue()");
#endif
  
  int32_t FrequencyError;
  uint32_t regmsb, regmid, reglsb, allreg;
  
  setMode(MODE_STDBY_XOSC);
  
  regmsb = readRegister( REG_LR_ESTIMATED_FREQUENCY_ERROR_MSB );
  regmsb = regmsb & 0x0F;       //clear bit 20 which is always set
  
  regmid = readRegister( REG_LR_ESTIMATED_FREQUENCY_ERROR_MSB + 1 );
  
  reglsb = readRegister( REG_LR_ESTIMATED_FREQUENCY_ERROR_MSB + 2 );
  setMode(MODE_STDBY_RC);

#ifdef LORADEBUG
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


int32_t SX128XLT::getFrequencyErrorHz()
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("getFrequencyErrorHz()");
#endif
  
  int32_t error, regvalue;
  uint32_t bandwidth;
  float divider;

  bandwidth =   returnBandwidth(savedModParam2);                   //gets the last configured bandwidth
  
  divider = (float) 1625000 / bandwidth;                           //data sheet says 1600000, but bandwidth is 1625000
  regvalue = getFrequencyErrorRegValue();
  error = (FREQ_ERROR_CORRECTION * regvalue) / divider;

  return error;
}

uint8_t SX128XLT::transmitAddressed(uint8_t *txbuffer, uint8_t size, char txpackettype, char txdestination, char txsource, uint32_t timeout, int8_t txpower, uint8_t wait)
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("transmitAddressed()");
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

  if (savedPacketType == PACKET_TYPE_LORA)
  {
   writeRegister(REG_LR_PAYLOADLENGTH, _TXPacketL);                           //only seems to work for lora  
  }  
  else if (savedPacketType == PACKET_TYPE_FLRC)
  {
  	setPacketParams(savedPacketParam1, savedPacketParam2, savedPacketParam3, savedPacketParam4, _TXPacketL, savedPacketParam6, savedPacketParam7);
  }

  setTxParams(txpower, RAMP_TIME);
  setDioIrqParams(IRQ_RADIO_ALL, (IRQ_TX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);   //set for IRQ on TX done and timeout on DIO1
  setTx(timeout);                                                          //this starts the TX

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

#ifdef SX128XDEBUGACK
		PRINTLN_CSTSTR("transmitAddressed is waiting for an ACK");
#endif
#ifdef INVERTIQ_ON_ACK
#ifdef SX128XDEBUGACK
		PRINTLN_CSTSTR("invert IQ for ack reception");
#endif
		invertIQ(true);
#endif
		delay(10);			
		//try to receive the ack
		RXAckPacketL=receiveAddressed(RXBUFFER, RXBUFFER_SIZE, 2000, WAIT_RX);
		
		if (RXAckPacketL) {
#ifdef SX128XDEBUGACK
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
#ifdef SX128XDEBUGACK
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
#ifdef SX128XDEBUGACK
				PRINTLN_CSTSTR("not for me!");
#endif			
			}
		}
		else {
#ifdef SX128XDEBUGACK
		PRINTLN_CSTSTR("received nothing");		 
#endif			
		}
#ifdef INVERTIQ_ON_ACK
#ifdef SX128XDEBUGACK
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


uint8_t SX128XLT::receiveAddressed(uint8_t *rxbuffer, uint8_t size, uint16_t timeout, uint8_t wait)
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("receiveAddressed()");
#endif

  uint8_t index, RXstart, RXend;
  uint16_t regdata;
  uint8_t buffer[2];

  setDioIrqParams(IRQ_RADIO_ALL, (IRQ_RX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);  //set for IRQ on RX done or timeout
  setRx(timeout);

  if (!wait)
  {
    return 0;                                                               //not wait requested so no packet length to pass
  }

  /**************************************************************************
	Modified by C. Pham - Oct. 2020
  **************************************************************************/

#ifdef USE_POLLING

	do {
		regdata = readIrqStatus();
		
		if (regdata & IRQ_HEADER_VALID)
			_RXTimestamp=millis();
			
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
    return 0;                          //packet is errored somewhere so return 0
  }

  readCommand(RADIO_GET_RXBUFFERSTATUS, buffer, 2);
  _RXPacketL = buffer[0];

  RXstart = buffer[1];
  
  checkBusy();
  
#ifdef USE_SPI_TRANSACTION           //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);             //start the burst read

  /**************************************************************************
	Added by C. Pham - Oct. 2020
  **************************************************************************/  

#ifdef SX128XDEBUG1 
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
    _RXPacketL = size;                 //truncate packet if not enough space
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
		
#ifdef SX128XDEBUGACK
  	PRINT_CSTSTR("ACK requested by ");
  	PRINTLN_VALUE("%d", _RXSource);
  	PRINTLN_CSTSTR("ACK transmission turnaround safe wait...");
#endif
#ifdef INVERTIQ_ON_ACK
#ifdef SX128XDEBUGACK
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

#ifdef SX128XDEBUGACK
		if (TXAckPacketL)
  		PRINTLN_CSTSTR("ACK sent");
  	else	
  		PRINTLN_CSTSTR("error when sending ACK"); 
#endif
#ifdef INVERTIQ_ON_ACK
#ifdef SX128XDEBUGACK
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

uint8_t SX128XLT::receiveRTSAddressed(uint8_t *rxbuffer, uint8_t size, uint16_t timeout, uint8_t wait)
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("receiveRTSAddressed()");
#endif

  uint8_t index, RXstart, RXend;
  uint16_t regdata;
  uint8_t buffer[2];
  uint32_t endtimeoutmS;
  bool validHeader=false; 

  setDioIrqParams(IRQ_RADIO_ALL, (IRQ_RX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);  //set for IRQ on RX done or timeout
  setRx(timeout);

  if (!wait)
  {
    return 0;                                                               //not wait requested so no packet length to pass
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

  checkBusy();
  
#ifdef USE_SPI_TRANSACTION           //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);             //start the burst read
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

uint8_t SX128XLT::readRXPacketType()
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("readRXPacketType()");
#endif
	return _RXPacketType;
}

/**************************************************************************
	Added by C. Pham - Oct. 2020
**************************************************************************/
    
uint8_t SX128XLT::readRXDestination()
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("readRXDestination()");
#endif
  return _RXDestination;
}


uint8_t SX128XLT::readRXSource()
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("readRXSource()");
#endif
  return _RXSource;
}

/**************************************************************************
	End by C. Pham - Oct. 2020
**************************************************************************/

uint8_t SX128XLT::readPacket(uint8_t *rxbuffer, uint8_t size)
{
#ifdef SX128XDEBUG
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


uint16_t SX128XLT::addCRC(uint8_t data, uint16_t libraryCRC)
{
  uint8_t j;

  libraryCRC ^= ((uint16_t)data << 8);
  for (j = 0; j < 8; j++)
  {
    if (libraryCRC & 0x8000)
      libraryCRC = (libraryCRC << 1) ^ 0x1021;
    else
      libraryCRC <<= 1;
  }
  return libraryCRC;
}

/**************************************************************************
  Added by C. Pham - Oct. 2020
**************************************************************************/

uint32_t SX128XLT::returnBandwidth()
{
  return (returnBandwidth(savedModParam2));
}

void SX128XLT::setCadParams()
{
  uint8_t buffer[1];
  //hardcoded to use 4 symbols
  buffer[0] = LORA_CAD_04_SYMBOL;
  writeCommand(RADIO_SET_CADPARAMS, buffer, 1);
  
  _cadSymbolNum=4;
}

/*
 Function: Configures the module to perform CAD.
 Returns: Integer that determines if the number of requested CAD have been successfull
   state = 1  --> There has been an error while executing the command
   state = 0  --> No activity detected
   state < 0  --> Activity detected, return mean RSSI computed duuring CAD
*/
int8_t SX128XLT::doCAD(uint8_t counter)
{
#ifdef SX128XDEBUG
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

#ifdef SX128XDEBUG3

void printDouble( double val, byte precision){
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

//this is only for legacy coding rate, i.e. not long interleavng
uint16_t SX128XLT::getToA(uint8_t pl) {
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("getToA()");
#endif

  uint8_t sf;
  uint8_t cr;
  uint32_t airTime = 0;
  uint32_t bw=0;

  bw=returnBandwidth();
  sf=getLoRaSF();
  cr=getLoRaCodingRate();

#ifdef SX128XDEBUG3
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

#ifdef SX128XDEBUG3
  PRINT_CSTSTR("ts is ");
  printDouble(ts,8);
  PRINTLN;
  PRINT_CSTSTR("tPreamble is ");
  printDouble(tPreamble,8);
  PRINTLN;
#endif

#ifdef SX128XDEBUG3
	// for low data rate optimization
  PRINT_CSTSTR("LDRO is ");
  PRINTLN_VALUE("%d",(sf>10)?1:0);
  PRINT_CSTSTR("CRC is ");
  PRINTLN_VALUE("%d",(savedPacketParam4==LORA_CRC_ON)?1:0);  
  PRINT_CSTSTR("header is ");
  PRINTLN_VALUE("%d",(savedPacketParam2==LORA_PACKET_VARIABLE_LENGTH)?1:0);    
#endif

  // Symbol length of payload and time
  double tmp = (8*pl + ((savedPacketParam4==LORA_CRC_ON)?16:0) - 4*sf + ((sf<7)?0:8) + ((savedPacketParam2==LORA_PACKET_VARIABLE_LENGTH)?20:0)) / (double)(4*(sf-2*((sf>10)?1:0)));

	tmp = ( tmp > 0 ) ? tmp : 0;
	
  tmp = ceil(tmp)*(cr + 4);

  double nPayload = 8 + tmp;

#ifdef SX128XDEBUG3
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

#ifdef SX128XDEBUG3
  PRINT_CSTSTR("airTime is ");
  PRINTLN_VALUE("%ld", airTime);
#endif
	return(airTime);
  // return in ms
  //return (ceil(airTime/1000)+1);
}

// need to set cad_number to a value > 0
// we advise using cad_number=3 for a SIFS and DIFS=3*SIFS
#define DEFAULT_CAD_NUMBER    3

void SX128XLT::CarrierSense(uint8_t cs, bool extendedIFS, bool onlyOnce) {
#ifdef SX128XDEBUG1
  PRINTLN_CSTSTR("CarrierSense()");
#endif
  
  if (cs==1)
    CarrierSense1(DEFAULT_CAD_NUMBER, extendedIFS, onlyOnce);

  if (cs==2)
    CarrierSense2(DEFAULT_CAD_NUMBER, extendedIFS);

  if (cs==3)
    CarrierSense3(DEFAULT_CAD_NUMBER);
}

uint16_t SX128XLT::CollisionAvoidance(uint8_t pl, uint8_t ca) {
#ifdef SX128XDEBUG1
  PRINTLN_CSTSTR("CollisionAvoidance()");
#endif
  
  if (ca==1)
  	// we force cad_number to 0 to skip the CAD procedure to test the collision avoidance mechanism
  	// in a real deployment, set back to DEFAULT_CAD_NUMBER
    return(CollisionAvoidance0(pl, 0 /* DEFAULT_CAD_NUMBER */));
    
  return(0);  
}

// need to set cad_number to a value > 0
// we advise using cad_number=3 for a SIFS and cad_number=9 for a DIFS
void SX128XLT::CarrierSense1(uint8_t cad_number, bool extendedIFS, bool onlyOnce) {

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

void SX128XLT::CarrierSense2(uint8_t cad_number, bool extendedIFS) {

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

void SX128XLT::CarrierSense3(uint8_t cad_number) {

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

uint8_t SX128XLT::transmitRTSAddressed(uint8_t pl) {

	uint8_t TXRTSPacketL;
	const uint8_t TXBUFFER_SIZE=1;
	uint8_t TXBUFFER[TXBUFFER_SIZE];
	
#ifdef SX128XDEBUG1
	PRINTLN_CSTSTR("transmitRTSAddressed()");
#endif
			
	TXBUFFER[0]=pl; //length of next data packet

#ifdef SX128XDEBUGRTS
	PRINT_CSTSTR("--> RTS: FOR ");
	PRINT_VALUE("%d", pl);
  PRINTLN_CSTSTR(" Bytes");		
#endif
	
	// use -1 as txpower to use default setting
	TXRTSPacketL=transmitAddressed(TXBUFFER, 1, PKT_TYPE_RTS, 0, _DevAddr, 10000, -1, WAIT_TX);	

#ifdef SX128XDEBUGRTS
	if (TXRTSPacketL)
		PRINTLN_CSTSTR("--> RTS: SENT");
	else	
		PRINTLN_CSTSTR("--> RTS: ERROR"); 
#endif

	return(TXRTSPacketL);
}

uint16_t SX128XLT::CollisionAvoidance0(uint8_t pl, uint8_t cad_number) {

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

#ifdef SX128XDEBUGRTS
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
#ifdef SX128XDEBUGRTS
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

#ifdef SX128XDEBUGRTS
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

#ifdef SX128XDEBUGRTS
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
#ifdef SX128XDEBUGRTS
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
#ifdef SX128XDEBUGRTS
				PRINT_CSTSTR("--> CA: ValidHeader FOR DATA DEFER BY MAX_TOA ");
				PRINTLN_VALUE("%d", getToA(255));			 
#endif  
				return(getToA(255));	
			}
			else {
#ifdef SX128XDEBUGRTS
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
#ifdef SX128XDEBUGRTS
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
#ifdef SX128XDEBUGRTS
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
#ifdef SX128XDEBUGRTS
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
		
#ifdef SX128XDEBUGRTS
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
#ifdef SX128XDEBUGRTS
			PRINTLN_CSTSTR("--> CA: EXIT, ALLOW DATA TRANSMIT");
#endif		
			return(0);
		}	
		// if forceListen==false then this is the first time for nodes starting directly at phase 2
		else {	
#ifdef SX128XDEBUGRTS
			PRINTLN_CSTSTR("--> CA: SEND RTS");
#endif			
		
			// phase 2 where we send an RTS
			transmitRTSAddressed(pl);
		
			// go for another listening period
			forceListen=true;
		}
	}
}	

uint8_t	SX128XLT::invertIQ(bool invert)
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("invertIQ()");
#endif
	if (invert)
		setPacketParams(savedPacketParam1, savedPacketParam2, savedPacketParam3, savedPacketParam4, LORA_IQ_INVERTED, savedPacketParam6, savedPacketParam7);
	else
		setPacketParams(savedPacketParam1, savedPacketParam2, savedPacketParam3, savedPacketParam4, LORA_IQ_NORMAL, savedPacketParam6, savedPacketParam7);	
  return(0);
}

void SX128XLT::setTXSeqNo(uint8_t seqno)
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("SetTXSeqNo()");
#endif
  
  _TXSeqNo=seqno;
}

uint8_t SX128XLT::readTXSeqNo()
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("readTXSeqNo()");
#endif
  
  return(_TXSeqNo);
}

uint8_t SX128XLT::readRXSeqNo()
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("readRXSeqNo()");
#endif
  
  return(_RXSeqNo);
}

uint32_t SX128XLT::readRXTimestamp()
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("readRXTimestamp()");
#endif
  
  return(_RXTimestamp);
}

uint32_t SX128XLT::readRXDoneTimestamp()
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("readRXDoneTimestamp()");
#endif
  
  return(_RXDoneTimestamp);
}

void SX128XLT::setDevAddr(uint8_t addr)
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("setDevAddr()");
#endif
	
	_DevAddr=addr;
}

uint8_t SX128XLT::readDevAddr()
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("readDevAddr()");
#endif

  return(_DevAddr);
}

uint8_t SX128XLT::readAckStatus()
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("readAckStatus()");
#endif

  return(_AckStatus);
}

int8_t SX128XLT::readPacketSNRinACK()
{
#ifdef SX128XDEBUG
  PRINTLN_CSTSTR("readPacketSNRinACK()");
#endif

  return(_PacketSNRinACK);
}

void SX128XLT::setLowPowerFctPtr(void (*pf)(unsigned long))
{
#ifdef SX128XDEBUG
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





