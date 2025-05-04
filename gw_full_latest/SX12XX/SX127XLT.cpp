/*
  Copyright 2019 - Stuart Robinson
  Licensed under a MIT license displayed at the bottom of this document.
  Original published 17/12/19
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
  	- binary constant in form BXXXXXXXX changed to 0bXXXXXXXX
  	- change pow(2,X) to (1 << X)
  	- ...
  - Add ack transaction with transmitAddressed-receiveAddressed, transmitReliable-receiveReliable - done
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
  - change power management in setTXParams() - done.
  	- take into account PA_BOOST
  	- power value is computed differently
  - PA_BOOST setting can be handled separately with setPA_BOOST(bool pa_boost) - done
  - add returnBandwidth() function to get the operating bandwidth value: 7800, ..., 125000, 250000, ...	 - done
  - add getToA(uint8_t pl) function to get the time-on-air of a pl-byte packet according to current LoRa settings - done
  - change the order of header and add a seq number in the header  - done
  	- for transmitAddressed, transmitReliable, receiveAddressed, receiveReliable and readPacketAddressed
  	- new 4-byte header is : destination, packet type, source, sequence number
  - add readRXSeqNo() function to get the RX sequence number in the received packet  - done
  - add TX sequence number management  - done
  	- setTXSeqNo(uint8_t seqno)
  	- readTXSeqNo()
  - add packet receive timestamp information based on millis() counter  - done
  	- readRXTimestamp() when valid header has been detected
  	- readRXDoneTimestamp() when entire packet reception has been detected
  - add CAD and carrier sense mechanism - done.
  	- doCad(uint8_t counter) realizes a number of CAD
  	- CarrierSense(uint8_t cs) performs 3 variants of carrier sense methods 	
  - packet rssi computation in readPacketRSSI() - done
  	- take into account SNR value (positive or negative cases)
  	- take into account low frequency to adapt formula according to Semtech's specs
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

#include <SX127XLT.h>

//use polling on REG_IRQFLAGS instead of DIO0
#define USE_POLLING
//invert IQ in ack transaction
#define INVERTIQ_ON_ACK
/**************************************************************************
End by C. Pham - Oct. 2020
**************************************************************************/

#define LTUNUSED(v) (void) (v)       //add LTUNUSED(variable); in functions to avoid compiler warnings

//#define SX127XDEBUG1               //enable level 1 debug messages
//#define SX127XDEBUG2               //enable level 2 debug messages
//#define SX127XDEBUG3               //enable level 3 debug messages
#define SX127XDEBUG_TEST           //enable test debug messages
#define SX127XDEBUGACK               //enable ack transaction debug messages
#define SX127XDEBUGRTS
//#define SX127XDEBUGCAD
//#define DEBUGPHANTOM               //used to set bebuging for Phantom packets
//#define SX127XDEBUGPINS            //enable pin allocation debug messages
//#define DEBUGFSKRTTY               //enable for FSKRTTY debugging 


SX127XLT::SX127XLT()
{
  /**************************************************************************
	Added by C. Pham - Oct. 2020
  **************************************************************************/  
  
  _PA_BOOST = false;
  _TXSeqNo = 0;
  _RXSeqNo = 0;

  _lowPowerFctPtr=NULL;    
  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/
}

/* Formats for :begin
1 original   > begin(int8_t pinNSS, int8_t pinNRESET, int8_t pinDIO0, int8_t pinDIO1, int8_t pinDIO2, uint8_t device);
2 Simplified > begin(int8_t pinNSS, int8_t pinNRESET, int8_t pinDIO0, uint8_t device);
*/


bool SX127XLT::begin(int8_t pinNSS, int8_t pinNRESET, int8_t pinDIO0, int8_t pinDIO1, int8_t pinDIO2, uint8_t device)
{
//format 1 pins, assign the all available pins 
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("1 begin()");
#endif

#if not defined ARDUINO && not defined USE_ARDUPI
  wiringPiSetup() ;
  PRINTLN_CSTSTR("<<< using wiringPi >>>"); 
#endif

  //assign the passed pins to the class private variabled
  _NSS = pinNSS;
  _NRESET = pinNRESET;
  _DIO0 = pinDIO0;
  _DIO1 = pinDIO1;
  _DIO2 = pinDIO2;
  _Device = device;            //device type needs to be assigned before reset
  _TXDonePin = pinDIO0;        //this is defalt pin for sensing TX done
  _RXDonePin = pinDIO0;        //this is defalt pin for sensing RX done
  
  pinMode(_NSS, OUTPUT);
  digitalWrite(_NSS, HIGH);
  pinMode(_NRESET, OUTPUT);
  digitalWrite(_NRESET, LOW);
  
  if (_DIO0 >= 0)
  {
    pinMode( _DIO0, INPUT);
  }

  if (_DIO1 >= 0)
  {
    pinMode( _DIO1,  INPUT);
  }

  if (_DIO2 >= 0)
  {
    pinMode( _DIO2,  INPUT);
  }

  resetDevice();
     
#ifdef SX127XDEBUGPINS
  PRINTLN_CSTSTR("format 1 begin()");
  PRINTLN_CSTSTR("SX127XLT constructor instantiated successfully");
  PRINT_CSTSTR("NSS ");
  PRINTLN_VALUE("%d",_NSS);
  PRINT_CSTSTR("NRESET ");
  PRINT_CSTSTR("DIO0 ");
  PRINTLN_VALUE("%d",_DIO0);
  PRINT_CSTSTR("DIO1 ");
  PRINTLN_VALUE("%d",_DIO1);
  PRINT_CSTSTR("DIO2 ");
  PRINTLN_VALUE("%d",_DIO2);
#endif

  if (checkDevice())
  {
    return true;
  }

  return false;
}



bool SX127XLT::begin(int8_t pinNSS, int8_t pinNRESET, int8_t pinDIO0, uint8_t device)
{
//format 2 pins, simplified 
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("2 begin()");
#endif

#if not defined ARDUINO && not defined USE_ARDUPI
  wiringPiSetup() ;
  PRINTLN_CSTSTR("<<< using wiringPi >>>");   
#endif

  //assign the passed pins to the class private variabled
  _NSS = pinNSS;
  _NRESET = pinNRESET;
  _DIO0 = pinDIO0;
  _DIO1 = -1;                  //pin not used
  _DIO2 = -1;                  //pin not used 
  _Device = device;            //device type needs to be assigned before reset
  _TXDonePin = pinDIO0;        //this is defalt pin for sensing TX done
  _RXDonePin = pinDIO0;        //this is defalt pin for sensing RX done
  
  pinMode(_NSS, OUTPUT);
  digitalWrite(_NSS, HIGH);
  pinMode(_NRESET, OUTPUT);
  digitalWrite(_NRESET, LOW);
 
  if (_DIO0 >= 0)
  {
    pinMode( _DIO0, INPUT);
  }

  if (_DIO1 >= 0)
  {
    pinMode( _DIO1,  INPUT);
  }

  if (_DIO2 >= 0)
  {
    pinMode( _DIO2,  INPUT);
  }

  resetDevice();
  
  
  #ifdef SX127XDEBUGPINS
  PRINTLN_CSTSTR("format 2 begin()");
  PRINTLN_CSTSTR("SX127XLT constructor instantiated successfully");
  PRINT_CSTSTR("NSS ");
  PRINTLN_VALUE("%d",_NSS);
  PRINT_CSTSTR("NRESET ");
  PRINT_CSTSTR("DIO0 ");
  PRINTLN_VALUE("%d",_DIO0);
  PRINT_CSTSTR("DIO1 ");
  PRINTLN_VALUE("%d",_DIO1);
  PRINT_CSTSTR("DIO2 ");
  PRINTLN_VALUE("%d",_DIO2);
  #endif

  if (checkDevice())
  {
    return true;
  }

  return false;
}


void SX127XLT::resetDevice()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("resetDevice()");
#endif

  if (_Device == DEVICE_SX1272)
  {
    digitalWrite(_NRESET, HIGH);
    delay(2);
    digitalWrite(_NRESET, LOW);
    delay(20);
  }
  else
  {
    digitalWrite(_NRESET, LOW);
    delay(2);
    digitalWrite(_NRESET, HIGH);
    delay(20);
  }
}


void SX127XLT::setMode(uint8_t modeconfig)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("setMode()");
#endif

  uint8_t regdata;

  regdata = modeconfig + _PACKET_TYPE;
  writeRegister(REG_OPMODE, regdata);
}


void SX127XLT::setSleep(uint8_t sleepconfig)
{
  //settings passed via sleepconfig are ignored, feature retained for compatibility with SX1280,SX126x

#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("setSleep()");
#endif

  LTUNUSED(sleepconfig);

  uint8_t regdata;

  regdata = readRegister(REG_OPMODE);
  writeRegister(REG_OPMODE, (regdata & 0xF8));       //clear bits 0,1,2 to set sleepmode
  delay(1);                                         //allow time for shutdown
}


bool SX127XLT::checkDevice()
{
  //check there is a device out there, writes a register and reads back

#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("checkDevice()");
#endif

  uint8_t Regdata1, Regdata2;
  Regdata1 = readRegister(REG_FRMID);               //low byte of frequency setting
  writeRegister(REG_FRMID, (Regdata1 + 1));
  Regdata2 = readRegister(REG_FRMID);               //read changed value back
  writeRegister(REG_FRMID, Regdata1);               //restore register to original value

  if (Regdata2 == (Regdata1 + 1))
  {
    return true;
  }
  else
  {
    return false;
  }
}


void SX127XLT::wake()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("wake()");
#endif
  uint8_t regdata;

  regdata = readRegister(REG_OPMODE);
  writeRegister(REG_OPMODE, (regdata | 0x01));      //set bit 0 to goto stdby mode
}


void SX127XLT::calibrateImage(uint8_t null)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("calibrateImage()");
#endif

  LTUNUSED(null);

  uint8_t regdata, savedmode;
  savedmode = readRegister(REG_OPMODE);
  writeRegister(REG_OPMODE, MODE_SLEEP);
  writeRegister(REG_OPMODE, MODE_STDBY);
  regdata = (readRegister(REG_IMAGECAL) | 0x40);
  writeRegister(REG_IMAGECAL, regdata);
  writeRegister(REG_OPMODE, MODE_SLEEP);
  writeRegister(REG_OPMODE, savedmode);
  delay(10);                              //calibration time 10mS
}



uint16_t SX127XLT::CRCCCITT(uint8_t *buffer, uint8_t size, uint16_t startvalue)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("CRCCCITT()");
#endif

  uint16_t index, libraryCRC;
  uint8_t j;

  libraryCRC = startvalue;                                  //start value for CRC16

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


uint16_t SX127XLT::CRCCCITTSX(uint8_t startadd, uint8_t endadd, uint16_t startvalue)
{
  //genrates a CRC of an area of the internal SX buffer

#ifdef SX127XDEBUG1
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


void SX127XLT::setDevice(uint8_t type)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("setDevice()");
#endif

  _Device = type;
}


void SX127XLT::printDevice()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("printDevice()");
#endif

  switch (_Device)
  {
    case DEVICE_SX1272:
      PRINT_CSTSTR("SX1272");
      break;

    case DEVICE_SX1276:
      PRINT_CSTSTR("SX1276");
      break;

    case DEVICE_SX1277:
      PRINT_CSTSTR("SX1277");
      break;

    case DEVICE_SX1278:
      PRINT_CSTSTR("SX1278");
      break;

    case DEVICE_SX1279:
      PRINT_CSTSTR("SX1279");
      break;

    default:
      PRINT_CSTSTR("Unknown Device");

  }
}


uint8_t SX127XLT::getOperatingMode()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("getOperatingMode()");
#endif

  return readRegister(REG_OPMODE);
}


bool SX127XLT::isReceiveDone()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("isReceiveDone()");
#endif

  return digitalRead(_RXDonePin);
}


bool SX127XLT::isTransmitDone()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("isTransmitDone()");
#endif

  return digitalRead(_TXDonePin);
}


void SX127XLT::writeRegister(uint8_t address, uint8_t value)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("writeRegister()");
#endif

#if defined ARDUINO || defined USE_ARDUPI

#ifdef USE_SPI_TRANSACTION                  //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);                  //set NSS low
  SPI.transfer(address | 0x80);             //mask address for write
  SPI.transfer(value);                      //write the byte
  digitalWrite(_NSS, HIGH);                 //set NSS high

#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif

#else
  uint8_t spibuf[2];
  spibuf[0] = address | 0x80;
  spibuf[1] = value;

  digitalWrite(_NSS, LOW);         //set NSS low
  wiringPiSPIDataRW(SPI_CHANNEL, spibuf, 2);
  digitalWrite(_NSS, HIGH);        //set NSS high
#endif

#ifdef SX127XDEBUG2
  PRINT_CSTSTR("Write register ");
  printHEXByte0x(address);
  PRINT_CSTSTR(" ");
  printHEXByte0x(value);
  PRINTLN;
  FLUSHOUTPUT;
#endif
}


uint8_t SX127XLT::readRegister(uint8_t address)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("readRegister()");
#endif

  uint8_t regdata;

#if defined ARDUINO || defined USE_ARDUPI

#ifdef USE_SPI_TRANSACTION         //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);         //set NSS low
  SPI.transfer(address & 0x7F);    //mask address for read
  regdata = SPI.transfer(0);       //read the byte
  digitalWrite(_NSS, HIGH);        //set NSS high

#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif

#else
	uint8_t spibuf[2];
	spibuf[0] = address & 0x7F;
	spibuf[1] = 0x00;

  digitalWrite(_NSS, LOW);         //set NSS low
	wiringPiSPIDataRW(SPI_CHANNEL, spibuf, 2);   
  digitalWrite(_NSS, HIGH);        //set NSS high
  
  regdata=spibuf[1];
#endif

#ifdef SX127XDEBUG2
  PRINT_CSTSTR("Read register ");
  printHEXByte0x(address);
  PRINT_CSTSTR(" ");
  printHEXByte0x(regdata);
  PRINTLN;
  FLUSHOUTPUT;
#endif

  return regdata;
}


void SX127XLT::printRegisters(uint16_t Start, uint16_t End)
{
  //prints the contents of SX127x registers to serial monitor

#ifdef SX127XDEBUG
  PRINTLN_CSTSTR("printRegisters()");
#endif

  uint16_t Loopv1, Loopv2, RegData;

  PRINT_CSTSTR("Reg    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F");
  PRINTLN;

  for (Loopv1 = Start; Loopv1 <= End;)           //32 lines
  {
    PRINT_CSTSTR("0x");
    if (Loopv1 < 0x10)
    {
      PRINT_CSTSTR("0");
    }
    PRINT_HEX("%X",Loopv1);                 //print the register number
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


void SX127XLT::printOperatingMode()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("printOperatingMode()");
#endif

  uint8_t regdata;

  regdata = getOpmode();

  switch (regdata)
  {
    case 0:
      PRINT_CSTSTR("SLEEP");
      break;

    case 1:
      PRINT_CSTSTR("STDBY");
      break;

    case 2:
      PRINT_CSTSTR("FSTX");
      break;

    case 3:
      PRINT_CSTSTR("TX");
      break;

    case 4:
      PRINT_CSTSTR("FSRX");
      break;

    case 5:
      PRINT_CSTSTR("RXCONTINUOUS");
      break;

    case 6:
      PRINT_CSTSTR("RXSINGLE");
      break;

    case 7:
      PRINT_CSTSTR("CAD");
      break;

    default:
      PRINT_CSTSTR("NOIDEA");
      break;
  }
}


void SX127XLT::printOperatingSettings()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("printOperatingSettings()");
#endif

  printDevice();
  PRINT_CSTSTR(",");

  printOperatingMode();

  PRINT_CSTSTR(",Version_");
  PRINT_HEX("%X",getVersion());

  PRINT_CSTSTR(",PacketMode_");

  if (getPacketMode())
  {
    PRINT_CSTSTR("LoRa");
  }
  else
  {
    PRINT_CSTSTR("FSK");
  }

  if (getHeaderMode())
  {
    PRINT_CSTSTR(",Implicit");
  }
  else
  {
    PRINT_CSTSTR(",Explicit");
  }

  PRINT_CSTSTR(",CRC_");
  if (getCRCMode())
  {
    PRINT_CSTSTR("On");
  }
  else
  {
    PRINT_CSTSTR("Off");
  }


  PRINT_CSTSTR(",AGCauto_");
  if (getAGC())
  {
    PRINT_CSTSTR("On");
  }
  else
  {
    PRINT_CSTSTR("Off");
  }

  PRINT_CSTSTR(",LNAgain_");
  PRINT_VALUE("%d",getLNAgain());

  PRINT_CSTSTR(",LNAboostHF_");
  if (getLNAboostHF())
  {
    PRINT_CSTSTR("On");
  }
  else
  {
    PRINT_CSTSTR("Off");
  }

  PRINT_CSTSTR(",LNAboostLF_");
  if (getLNAboostLF())
  {
    PRINT_CSTSTR("On");
  }
  else
  {
    PRINT_CSTSTR("Off");
  }

}


/**************************************************************************
Modified by C. Pham - Oct. 2020
**************************************************************************/  

void SX127XLT::setTxParams(int8_t txPower, uint8_t rampTime)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("setTxParams()");
#endif

	if (txPower < 0)
		return;

  byte RegPaDacReg=(_Device==DEVICE_SX1272)?0x5A:0x4D;
  
  uint8_t param1, param2;

  writeRegister(REG_PARAMP, rampTime);       //Reg 0x0A this is same for SX1272 and SX1278

  if (txPower == 20) {
    //20dBm needs PA_BOOST
    _PA_BOOST=true;
    //set the high output power config with register REG_PA_DAC
    writeRegister(RegPaDacReg, 0x87);
    param2 = OCP_TRIM_150MA;
  }
  else {
    if (txPower > 14)
      txPower=14;
        
    // disable high power output in all other cases
    writeRegister(RegPaDacReg, 0x84);
    
    if (txPower > 10)
      // set RegOcp for OcpOn and OcpTrim
      // 130mA
      param2 = OCP_TRIM_130MA;
    else
    if (txPower > 5)
      // 100mA
      param2 = OCP_TRIM_100MA;
    else
      param2 = OCP_TRIM_80MA;
  }
  
  if (_Device == DEVICE_SX1272) {
    // Pout = -1 + _power[3:0] on RFO
    // Pout = 2 + _power[3:0] on PA_BOOST
    if (_PA_BOOST) {
      param1 = txPower - 2;
      // we set the PA_BOOST pin
      param1 = param1 | 0b10000000;
    }
    else
      param1 = txPower + 1;

    writeRegister(REG_PACONFIG, param1);	// Setting output power value
  }
  else {
    // for the SX1276
    uint8_t pmax=15;

    // then Pout = Pmax-(15-_power[3:0]) if  PaSelect=0 (RFO pin for +14dBm)
    // so L=3dBm; H=7dBm; M=15dBm (but should be limited to 14dBm by RFO pin)

    // and Pout = 17-(15-_power[3:0]) if  PaSelect=1 (PA_BOOST pin for +14dBm)
    // so x= 14dBm (PA);
    // when p=='X' for 20dBm, value is 0x0F and RegPaDacReg=0x87 so 20dBm is enabled

    if (_PA_BOOST) {
      param1 = txPower - 17 + 15;
      // we set the PA_BOOST pin
      param1 = param1 | 0b10000000;
    }
    else
      param1 = txPower - pmax + 15;

    // set MaxPower to 7 -> Pmax=10.8+0.6*MaxPower [dBm] = 15
    param1 = param1 | 0b01110000;

    writeRegister(REG_PACONFIG, param1);
  }
  
  writeRegister(REG_OCP, param2);
}    

/**************************************************************************
End by C. Pham - Oct. 2020
**************************************************************************/  

/* original code
 ***************

void SX127XLT::setTxParams(int8_t txPower, uint8_t rampTime)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("setTxParams()");
#endif

  uint8_t param1, param2;

  writeRegister(REG_PARAMP, rampTime);       //Reg 0x0A this is same for SX1272 and SX1278

  if (txPower > 20)
  {
    txPower = 20;
  }

  if (txPower < 2)
  {
    txPower = 2;
  }

  if (txPower > 17)
  {
    writeRegister(REG_PADAC, 0x87);          //Reg 0x4D this is same for SX1272 and SX1278
  }
  else
  {
    writeRegister(REG_PADAC, 0x84);          //Reg 0x4D this is same for SX1272 and SX1278
  }

  //now the device specific settings

  if (_Device != DEVICE_SX1272)
  {
    //for all devices apart from SX1272

    if (txPower > 17)
    {
      param1 = (txPower + 0xEB);
      param2 = OCP_TRIM_130MA;
    }

    if (txPower <= 17)
    {
      param1 = txPower + 0xEE;
      param2 = OCP_TRIM_100MA;
    }

    if (txPower <= 5)
    {
      param1 = txPower + 0xEE;
      param2 = OCP_TRIM_80MA;
    }

  }
  else
  {
    //for device SX1272

    if (txPower > 17)
    {
      param1 = (txPower + 0x7F);
      param2 = OCP_TRIM_130MA;
    }

    if (txPower <= 17)
    {
      param1 = txPower + 0x82;
      param2 = OCP_TRIM_100MA;
    }

    if (txPower <= 5)
    {
      param1 = txPower + 0x82;
      param2 = OCP_TRIM_80MA;
    }

  }

  writeRegister(REG_PACONFIG, param1);       //Reg 0x09 this changes for SX1272 and SX1278
  writeRegister(REG_OCP, param2);            //Reg 0x0C this is same for SX1272 and SX1278
}

*/

void SX127XLT::setPacketParams(uint16_t packetParam1, uint8_t  packetParam2, uint8_t packetParam3, uint8_t packetParam4, uint8_t packetParam5)
{
  //format is PreambleLength, Fixed\Variable length packets, Packetlength, CRC mode, IQ mode

#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("SetPacketParams()");
#endif

  uint8_t preambleMSB, preambleLSB, regdata;

  //*******************************************************
  //These changes are the same for SX1272 and SX127X
  //PreambleLength reg 0x20, 0x21
  preambleMSB = packetParam1 >> 8;
  preambleLSB = packetParam1 & 0xFF;
  writeRegister(REG_PREAMBLEMSB, preambleMSB);
  writeRegister(REG_PREAMBLELSB, preambleLSB);

  //TX Packetlength reg 0x22
  writeRegister(REG_PAYLOADLENGTH, packetParam3);                //when in implicit mode, this is used as receive length also

  /**************************************************************************
	Modified by C. Pham - Oct. 2020
  **************************************************************************/  
  
  if (packetParam5==LORA_IQ_INVERTED)
  	invertIQ(true);
  else	
  	invertIQ(false);
  	
  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/  
  
  /* original code
   ***************

  //IQ mode reg 0x33
  
  regdata = ( (readRegister(REG_INVERTIQ)) & 0xBF );             //mask off invertIQ bit 6
  writeRegister(REG_INVERTIQ, (regdata + packetParam5));
  */
  //*******************************************************

  //CRC mode
  _UseCRC = packetParam4;                                       //save CRC status

  if (_Device != DEVICE_SX1272)
  {
    //for all devices apart from SX1272
    //Fixed\Variable length packets
    regdata = ( (readRegister(REG_MODEMCONFIG1)) & (~READ_IMPLCIT_AND_X)); //mask off bit 0
    writeRegister(REG_MODEMCONFIG1, (regdata + packetParam2));             //write out with bit 0 set appropriatly

    //CRC on payload
    regdata = ( (readRegister(REG_MODEMCONFIG2)) & (~READ_HASCRC_AND_X));  //mask off all bits bar CRC on - bit 2
    writeRegister(REG_MODEMCONFIG2, (regdata + (packetParam4 << 2)));      //write out with CRC bit 2 set appropriatly
  }
  else
  {
    //for SX1272
    //Fixed\Variable length packets
    regdata = ( (readRegister(REG_MODEMCONFIG1)) & (~READ_IMPLCIT_AND_2)); //mask off bit 2
    writeRegister(REG_MODEMCONFIG1, (regdata + (packetParam2 << 2)));      //write out with bit 2 set appropriatly

    //CRC on payload
    regdata = ( (readRegister(REG_MODEMCONFIG1)) & (~READ_HASCRC_AND_2));  //mask of all bits bar CRC on - bit 1
    writeRegister(REG_MODEMCONFIG1, (regdata + (packetParam4 << 1)));      //write out with CRC bit 1 set appropriatly
  }
}


void SX127XLT::setModulationParams(uint8_t modParam1, uint8_t modParam2, uint8_t  modParam3, uint8_t  modParam4)
{
  //order is SpreadingFactor, Bandwidth, CodeRate, Optimisation

#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("setModulationParams()");
#endif

  uint8_t regdata, bw;

  //Spreading factor - same for SX1272 and SX127X - reg 0x1D
  regdata = (readRegister(REG_MODEMCONFIG2) & (~READ_SF_AND_X));
  writeRegister(REG_MODEMCONFIG2, (regdata + (modParam1 << 4)));
  
  if (_Device != DEVICE_SX1272)
  {
    //for all devices apart from SX1272

    //bandwidth
    regdata = (readRegister(REG_MODEMCONFIG1) & (~READ_BW_AND_X));
    writeRegister(REG_MODEMCONFIG1, (regdata + modParam2));

    //Coding rate
    regdata = (readRegister(REG_MODEMCONFIG1) & (~READ_CR_AND_X));
    writeRegister(REG_MODEMCONFIG1, (regdata + (modParam3)));

    //Optimisation
    if (modParam4 == LDRO_AUTO)
    {
      modParam4 = returnOptimisation(modParam2, modParam1);
    }

    regdata = (readRegister(REG_MODEMCONFIG3) & (~READ_LDRO_AND_X));
    writeRegister(REG_MODEMCONFIG3, (regdata + (modParam4 << 3)));

  }
  else
  {
    //for SX1272
    //bandwidth
    regdata = (readRegister(REG_MODEMCONFIG1) & (~READ_BW_AND_2));   //value will be LORA_BW_500 128, LORA_BW_250 64, LORA_BW_125 0

    switch (modParam2)
    {
      case LORA_BW_125:
        bw = 0x00;
        break;

      case LORA_BW_500:
        bw = 0x80;
        break;

      case LORA_BW_250:
        bw = 0x40;
        break;

      default:
        bw = 0x00;                       //defaults to LORA_BW_125
    }

    writeRegister(REG_MODEMCONFIG1, (regdata + bw));

    //Coding rate
    regdata = (readRegister(REG_MODEMCONFIG1) & (~READ_CR_AND_2));
    writeRegister(REG_MODEMCONFIG1, (regdata + (modParam3 << 2)));

    //Optimisation
    if (modParam4 == LDRO_AUTO)
    {
      modParam4 = returnOptimisation(modParam2, modParam1);
    }

    regdata = (readRegister(REG_MODEMCONFIG1) & (~READ_LDRO_AND_2));
    writeRegister(REG_MODEMCONFIG1, (regdata + modParam4));

  }
  
  if ( modParam1 == LORA_SF6)
  {
  writeRegister(REG_DETECTOPTIMIZE, 0x05);
  writeRegister(REG_DETECTIONTHRESHOLD, 0x0C);
  }
  else
  {
  writeRegister(REG_DETECTOPTIMIZE, 0x03);
  writeRegister(REG_DETECTIONTHRESHOLD, 0x0A);
  }
 
}


void SX127XLT::setRfFrequency(uint64_t freq64, int32_t offset)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("setRfFrequency()");
#endif

  savedFrequency = freq64;
  savedOffset = offset;
  
  freq64 = freq64 + offset;
  freq64 = ((uint64_t)freq64 << 19) / 32000000;
  _freqregH = freq64 >> 16;
  _freqregM = freq64 >> 8;
  _freqregL = freq64;
    
  writeRegister(REG_FRMSB, _freqregH);
  writeRegister(REG_FRMID, _freqregM);
  writeRegister(REG_FRLSB, _freqregL);
}


uint32_t SX127XLT::getFreqInt()
{
  //get the current set LoRa device frequency, return as long integer
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("getFreqInt()");
#endif

  uint8_t Msb, Mid, Lsb;
  uint32_t uinttemp;
  float floattemp;
  Msb = readRegister(REG_FRMSB);
  Mid = readRegister(REG_FRMID);
  Lsb = readRegister(REG_FRLSB);
  floattemp = ((Msb * 0x10000ul) + (Mid * 0x100ul) + Lsb);
  floattemp = ((floattemp * 61.03515625) / 1000000ul);
  uinttemp = (uint32_t)(floattemp * 1000000);
  return uinttemp;
}


int32_t SX127XLT::getFrequencyErrorRegValue()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("getFrequencyErrorRegValue()");
#endif

  int32_t FrequencyError;
  uint32_t msb, mid, lsb;
  uint32_t allreg;

  setMode(MODE_STDBY_RC);

  msb = readRegister(REG_FEIMSB);
  mid = readRegister(REG_FEIMID);
  lsb = readRegister(REG_FEILSB);

#ifdef SX127XDEBUG1
  PRINTLN;
  PRINT_CSTSTR("Registers ");
  PRINT_HEX("%lX",msb);
  PRINT_CSTSTR(" ");
  PRINT_HEX("%lX",mid);
  PRINT_CSTSTR(" ");
  PRINTLN_HEX("%lX",lsb);
#endif

  allreg = (uint32_t) ( msb << 16 ) | ( mid << 8 ) | lsb;

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


int32_t SX127XLT::getFrequencyErrorHz()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("getFrequencyErrorHz()");
#endif

  uint16_t msb, mid, lsb;
  int16_t freqerr;
  uint8_t bw;
  float bwconst;

  msb = readRegister(REG_FEIMSB);
  mid = readRegister(REG_FEIMID);
  lsb = readRegister(REG_FEILSB);

  if (_Device != DEVICE_SX1272)
  {
    //for all devices apart from SX1272
    bw = (readRegister(REG_MODEMCONFIG1)) & (0xF0);

    switch (bw)
    {
      case LORA_BW_125:           //ordered with most commonly used first
        bwconst = 2.097;
        break;

      case LORA_BW_062:
        bwconst = 1.049;
        break;

      case LORA_BW_041:
        bwconst = 0.6996;
        break;

      case LORA_BW_007:
        bwconst = 0.1309;
        break;

      case LORA_BW_010:
        bwconst = 0.1745;
        break;

      case LORA_BW_015:
        bwconst = 0.2617;
        break;

      case LORA_BW_020:
        bwconst = 0.3490;
        break;

      case LORA_BW_031:
        bwconst = 0.5234;
        break;

      case LORA_BW_250:
        bwconst = 4.194;
        break;

      case LORA_BW_500:
        bwconst = 8.389;
        break;

      default:
        bwconst = 0x00;
    }
  }
  else
  {
    //for the SX1272
    bw = (readRegister(REG_MODEMCONFIG1)) & (0xF0);

    switch (bw)
    {
      case 0:                   //this is LORA_BW_125
        bwconst = 2.097;
        break;

      case 64:                  //this is LORA_BW_250
        bwconst = 4.194;
        break;

      case 128:                 //this is LORA_BW_250
        bwconst = 8.389;
        break;

      default:
        bwconst = 0x00;
    }


  }

  freqerr = msb << 12;          //shift lower 4 bits of msb into high 4 bits of freqerr
  mid = (mid << 8) + lsb;
  mid = (mid >> 4);
  freqerr = freqerr + mid;

  freqerr = (int16_t) (freqerr * bwconst);

  return freqerr;
}


void SX127XLT::setTx(uint32_t timeout)
{
  //There is no TX timeout function for SX127X, the value passed is ignored

#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("setTx()");
#endif

  LTUNUSED(timeout);                                  //unused TX timeout passed for compatibility with SX126x, SX128x

  clearIrqStatus(IRQ_RADIO_ALL);

  /* This function not used on current SX127x
    if (_rxtxpinmode)
    {
    rxEnable();
    }
  */

  writeRegister(REG_OPMODE, (MODE_TX + 0x88));    //TX on LoRa mode
}


void SX127XLT::setRx(uint32_t timeout)
{
  //no timeout in this routine, left in for compatibility
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("setRx()");
#endif

  LTUNUSED(timeout);

  clearIrqStatus(IRQ_RADIO_ALL);

  /* This function not used on current SX127x
    if (_rxtxpinmode)
    {
    rxEnable();
    }
  */

  writeRegister(REG_OPMODE, (MODE_RXCONTINUOUS + 0x80));    //RX on LoRa mode
}


bool SX127XLT::readTXIRQ()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("readTXIRQ()");
#endif

  uint16_t IRQreg;
  IRQreg = readIrqStatus();

  if (IRQreg & IRQ_TX_DONE)
  {
    return true;
  }
  else
  {
    return false;
  }
}


bool SX127XLT::readRXIRQ()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("readRXIRQ()");
#endif

  uint16_t IRQreg;
  IRQreg = readIrqStatus();

  if (IRQreg & IRQ_RX_DONE)
  {
    return true;
  }
  else
  {
    return false;
  }
}


void SX127XLT::setLowPowerReceive()
{
  //set RX power saving mode

#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("setLowPowerReceive()");
#endif

  writeRegister(REG_LNA, 0x20 ); //at HF 100% LNA current
}


void SX127XLT::setHighSensitivity()
{
  //set Boosted LNA for HF mode

#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("setHighSensitivity()");
#endif

  if (_Device != DEVICE_SX1272)
  {
    //for all devices apart from SX1272
    writeRegister(REG_LNA, 0x3B ); //at HF 150% LNA current.
  }
  else
  {
    //for SX1272
    writeRegister(REG_LNA, 0x23 ); //at HF 10% LNA current.
  }
}


void SX127XLT::setRXGain(uint8_t config)
{
  //set RX power saving mode

#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("setRXGain( )");
#endif

  uint8_t regdata;

  if (_Device != DEVICE_SX1272)
  {
    //for all devices apart from SX1272
    regdata = readRegister(REG_MODEMCONFIG3);
    writeRegister(REG_MODEMCONFIG3, (regdata &  (~READ_AGCAUTO_AND_X))); //clear bit AgcAutoOn (2) to ensure RegLNA is controlling gain
    writeRegister(REG_LNA, config);
  }
  else
  {
    regdata = readRegister(REG_MODEMCONFIG2);
    writeRegister(REG_MODEMCONFIG2, (regdata & (~READ_AGCAUTO_AND_2))); //clear bit AgcAutoOn (2) to ensure RegLNA is controlling gain
    writeRegister(REG_LNA, config);
  }
}


uint8_t SX127XLT::getAGC()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("getAGC()");
#endif

  uint8_t regdata;

  if (_Device != DEVICE_SX1272)
  {
    regdata = readRegister(REG_MODEMCONFIG3);
    regdata = (regdata & READ_AGCAUTO_AND_X);
  }
  else
  {
    regdata = readRegister(REG_MODEMCONFIG2);
    regdata = (regdata & READ_AGCAUTO_AND_2);
  }

  return (regdata >> 2);
}


uint8_t SX127XLT::getLNAgain()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("getLNAgain()");
#endif

  uint8_t regdata;
  regdata = readRegister(REG_LNA);
  regdata = (regdata & READ_LNAGAIN_AND_X);
  return (regdata >> 5);
}


uint8_t SX127XLT::getCRCMode()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("getCRCMode()");
#endif

  uint8_t regdata;

  regdata = readRegister(REG_MODEMCONFIG2);

  if (_Device != DEVICE_SX1272)
  {
    regdata = readRegister(REG_MODEMCONFIG2);
    regdata = ((regdata & READ_HASCRC_AND_X) >> 2);
  }
  else
  {
    regdata = readRegister(REG_MODEMCONFIG1);
    regdata = ((regdata & READ_HASCRC_AND_2) >> 1);
  }

  return regdata;
}


uint8_t SX127XLT::getHeaderMode()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("getHeaderMode()");
#endif

  uint8_t regdata;

  regdata = readRegister(REG_MODEMCONFIG1);

  if (_Device != DEVICE_SX1272)
  {
    regdata = (regdata & READ_IMPLCIT_AND_X);
  }
  else
  {
    regdata = ((regdata & READ_IMPLCIT_AND_2) >> 2);
  }

  return regdata;
}


uint8_t SX127XLT::getLNAboostLF()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("getLNAboostLF");
#endif

  uint8_t regdata;

  regdata = readRegister(REG_LNA);

  if (_Device != DEVICE_SX1272)
  {
    regdata = (regdata & READ_LNABOOSTLF_AND_X);
  }
  else
  {
    regdata = (regdata & READ_LNABOOSTLF_AND_X);
  }

  return (regdata >> 3);
}


uint8_t SX127XLT::getLNAboostHF()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("getLNAboostHF()");
#endif

  uint8_t regdata;

  regdata = readRegister(REG_LNA);

  if (_Device != DEVICE_SX1272)
  {
    regdata = (regdata & READ_LNABOOSTHF_AND_X);
  }
  else
  {
    regdata = (regdata & READ_LNABOOSTHF_AND_X);
  }

  return regdata;
}


uint8_t SX127XLT::getOpmode()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("getOpmode()");
#endif

  uint8_t regdata;

  regdata = (readRegister(REG_OPMODE) & READ_OPMODE_AND_X);

  return regdata;
}


uint8_t SX127XLT::getPacketMode()
{
  //its either LoRa or FSK

#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("getPacketMode()");
#endif

  uint8_t regdata;

  regdata = (readRegister(REG_OPMODE) & READ_RANGEMODE_AND_X);

  return (regdata >> 7);
}


uint8_t SX127XLT::readRXPacketL()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("readRXPacketL()");
#endif

  _RXPacketL = readRegister(REG_RXNBBYTES);
  return _RXPacketL;
}



uint8_t SX127XLT::readTXPacketL()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("readTXPacketL()");
#endif

  return _TXPacketL;
}

/**************************************************************************
Modified by C. Pham - Oct. 2020
**************************************************************************/  

int8_t SX127XLT::readPacketRSSI()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("readPacketRSSI()");
#endif

  _PacketRSSI = readRegister(REG_PKTRSSIVALUE);
  
  readPacketSNR();

  if( _PacketSNR < 0 )
  {
      // added by C. Pham, using Semtech SX1272 rev3 March 2015
      // for SX1272 we use -139, for SX1276, we use -157
      // then for SX1276 when using low-frequency (i.e. 433MHz band) then we use -164
      _PacketRSSI = -(OFFSET_RSSI+(_Device==DEVICE_SX1272?0:18)+(savedFrequency<436000000?7:0)) + (double)_PacketRSSI + (double)_PacketSNR*0.25;
  }
  else
  {
      _PacketRSSI= -(OFFSET_RSSI+(_Device==DEVICE_SX1272?0:18)+(savedFrequency<436000000?7:0)) + (double)_PacketRSSI*16.0/15.0;
  }
  
  return _PacketRSSI;
}

/**************************************************************************
End by C. Pham - Oct. 2020
**************************************************************************/  
  
/* original code
 ***************  

uint8_t SX127XLT::readPacketRSSI()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("readPacketRSSI()");
#endif

  _PacketRSSI = (157 - regdata) * (-1);
  return _PacketRSSI;
}
*/

/**************************************************************************
Modified by C. Pham - Oct. 2020
**************************************************************************/ 

int8_t SX127XLT::readPacketSNR()

/**************************************************************************
End by C. Pham - Oct. 2020
**************************************************************************/  
  
/* original code
 *************** 
uint8_t SX127XLT::readPacketSNR()
 */
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("readPacketSNR()");
#endif

  uint8_t regdata;
  regdata = readRegister(REG_PKTSNRVALUE);

  if (regdata > 127)
  {
    _PacketSNR  =  ((255 - regdata) / 4) * (-1);
  }
  else
  {
    _PacketSNR  = regdata / 4;
  }

  return _PacketSNR;
}

bool SX127XLT::readPacketCRCError()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("readPacketCRCError()");
#endif

  uint16_t IRQreg;
  IRQreg = readIrqStatus();

  if (IRQreg & IRQ_CRC_ERROR)
  {
    return true;
  }
  else
  {
    return false;
  }
}


bool SX127XLT::readPacketHeaderValid()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("readPacketHeaderValid()");
#endif

  uint16_t IRQreg;
  IRQreg = readIrqStatus();

  if (IRQreg & IRQ_HEADER_VALID)
  {
    return true;
  }
  else
  {
    return false;
  }
}


uint8_t SX127XLT::packetOK()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("packetOK(");
#endif

  bool packetHasCRC;

  packetHasCRC = (readRegister(REG_HOPCHANNEL) & 0x40);                  //read the packet has CRC bit in RegHopChannel

#ifdef DEBUGPHANTOM
  PRINT_CSTSTR("PacketHasCRC = ");
  PRINTLN_VALUE("%d",packetHasCRC);
  PRINT_CSTSTR("_UseCRC = ");
  PRINTLN_VALUE("%d",_UseCRC);
#endif

  if ( !packetHasCRC && _UseCRC )
  {
    _IRQmsb = _IRQmsb + IRQ_NO_PACKET_CRC;                              //flag the phantom packet
    return 0;
  }

  if ( readIrqStatus() != (IRQ_RX_DONE + IRQ_HEADER_VALID) )
  {
    return 0;                                                           //no RX done and header valid only, could be CRC error
  }

  return 1;                                                             //packet is OK
}


uint8_t SX127XLT::readRXPacketType()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("readRXPacketType()");
#endif

  return _RXPacketType;
}


uint8_t SX127XLT::readRXDestination()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("readRXDestination()");
#endif
  return _RXDestination;
}


uint8_t SX127XLT::readRXSource()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("readRXSource()");
#endif

  return _RXSource;
}


void SX127XLT::setBufferBaseAddress(uint8_t txBaseAddress, uint8_t rxBaseAddress)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("setBufferBaseAddress()");
#endif

  writeRegister(REG_FIFOTXBASEADDR, txBaseAddress);
  writeRegister(REG_FIFORXBASEADDR, rxBaseAddress);
}


void SX127XLT::setPacketType(uint8_t packettype )
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("setPacketType()");
#endif
  uint8_t regdata;

  regdata = (readRegister(REG_OPMODE) & 0x7F);           //save all register bits bar the LoRa\FSK bit 7

  if (packettype == PACKET_TYPE_LORA)
  {
    _PACKET_TYPE = PACKET_TYPE_LORA;
    writeRegister(REG_OPMODE, 0x80);                     //REG_OPMODE, need to set to sleep mode before configure for LoRa mode
    writeRegister(REG_OPMODE, (regdata + 0x80));         //back to original standby mode with LoRa set
  }
  else
  {
    _PACKET_TYPE = PACKET_TYPE_GFSK;
    writeRegister(REG_OPMODE, 0x00);                     //REG_OPMODE, need to set to sleep mode before configure for FSK
    writeRegister(REG_OPMODE, regdata);                  //back to original standby mode with LoRa set
  }

}


void SX127XLT::clearIrqStatus(uint16_t irqMask)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("clearIrqStatus()");
#endif

  uint8_t masklsb;
  uint16_t maskmsb;
  _IRQmsb = _IRQmsb & 0xFF00;                                 //make sure _IRQmsb does not have LSB bits set.
  masklsb = (irqMask & 0xFF);
  maskmsb = (irqMask & 0xFF00);
  writeRegister(REG_IRQFLAGS, masklsb);                       //clear standard IRQs
  _IRQmsb = (_IRQmsb & (~maskmsb));                           //only want top bits set.
}


uint16_t SX127XLT::readIrqStatus()
{
#ifdef SX127XDEBUG1
  PRINT_CSTSTR("readIrqStatus()");
#endif

  bool packetHasCRC;

  packetHasCRC = (readRegister(REG_HOPCHANNEL) & 0x40);        //read the packet has CRC bit in RegHopChannel


#ifdef DEBUGPHANTOM
  PRINT_CSTSTR("PacketHasCRC = ");
  PRINTLN_VALUE("%d",packetHasCRC);
  PRINT_CSTSTR("_UseCRC = ");
  PRINTLN_VALUE("%d",_UseCRC);
#endif
  
  if ( !packetHasCRC && _UseCRC )                                //check if packet header indicates no CRC on packet, byt use CRC set
  {
   bitSet(_IRQmsb, 10);                                          //flag the phantom packet, set bit 10
  }

  return (readRegister(REG_IRQFLAGS) + _IRQmsb);
}


void SX127XLT::setDioIrqParams(uint16_t irqMask, uint16_t dio0Mask, uint16_t dio1Mask, uint16_t dio2Mask)
{
  //note the irqmask contains the bit values of the interrupts that are allowed, so for all interrupts value is 0xFFFF

#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("setDioIrqParams()");
#endif

  uint8_t mask0, mask1, mask2;

  LTUNUSED(dio2Mask);                      //variable left in call for compatibility with other libraries


  switch (dio0Mask)
  {
    case IRQ_RX_DONE:
      mask0 = 0;
      break;

    case IRQ_TX_DONE:
      mask0 = 0x40;
      break;

    case IRQ_CAD_DONE:
      mask0 = 0x80;
      break;

    default:
      mask0 = 0x0C;
  }

  switch (dio1Mask)               //for compatibility with other libraries IRQ_RX_TIMEOUT = IRQ_RX_TX_TIMEOUT
  {
    case IRQ_RX_TIMEOUT:
      mask1 = 0;
      break;

    case IRQ_FSHS_CHANGE_CHANNEL:
      mask1 = 0x10;
      break;

    case IRQ_CAD_ACTIVITY_DETECTED:
      mask1 = 0x20;
      break;

    default:
      mask1 = 0x30;
  }

  mask2 = 0x00;                  //is always IRQ_FSHS_CHANGE_CHANNEL

  writeRegister(REG_IRQFLAGSMASK, ~irqMask);
  writeRegister(REG_DIOMAPPING1, (mask0 + mask1 + mask2));
}


void SX127XLT::printIrqStatus()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("printIrqStatus()");
#endif

  uint8_t _IrqStatus;
  _IrqStatus = readIrqStatus();

  //0x01
  if (_IrqStatus & IRQ_CAD_ACTIVITY_DETECTED)
  {
    PRINT_CSTSTR(",IRQ_CAD_ACTIVITY_DETECTED");
  }

  //0x02
  if (_IrqStatus & IRQ_FSHS_CHANGE_CHANNEL)
  {
    PRINT_CSTSTR(",IRQ_FSHS_CHANGE_CHANNEL");
  }

  //0x04
  if (_IrqStatus & IRQ_CAD_DONE)
  {
    PRINT_CSTSTR(",IRQ_CAD_DONE");
  }

  //0x08
  if (_IrqStatus & IRQ_TX_DONE)
  {
    PRINT_CSTSTR(",IRQ_TX_DONE");
  }

  //0x10
  if (_IrqStatus & IRQ_HEADER_VALID)
  {
    PRINT_CSTSTR(",IRQ_HEADER_VALID");
  }

  //0x20
  if (_IrqStatus & IRQ_CRC_ERROR)
  {
    PRINT_CSTSTR(",IRQ_CRC_ERROR");
  }

  //0x40
  if (_IrqStatus & IRQ_RX_DONE)
  {
    PRINT_CSTSTR(",IRQ_RX_DONE");
  }

  //0x80
  if (_IrqStatus & IRQ_RX_TIMEOUT )
  {
    PRINT_CSTSTR(",IRQ_RX_TIMEOUT ");
  }

  if (_IRQmsb & IRQ_TX_TIMEOUT  )
  {
    PRINT_CSTSTR(",TX_TIMEOUT");
  }

  if (_IRQmsb & IRQ_RX_TIMEOUT  )
  {
    PRINT_CSTSTR(",RX_TIMEOUT");
  }

  if (_IRQmsb & IRQ_NO_PACKET_CRC  )
  {
    PRINT_CSTSTR(",NO_PACKET_CRC");
  }

}


void SX127XLT::printASCIIPacket(uint8_t *buffer, uint8_t size)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("printASCIIPacket()");
#endif

  uint8_t index;

  for (index = 0; index < size; index++)
  {
    Serial.write(buffer[index]);
  }

}


void SX127XLT::printHEXPacket(uint8_t *buffer, uint8_t size)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("printHEXPacket()");
#endif

  uint8_t index;

  for (index = 0; index < size; index++)
  {
    printHEXByte(buffer[index]);
    PRINT_CSTSTR(" ");
  }
}


void SX127XLT::printASCIIorHEX(uint8_t temp)
{
  //prints as ASCII if within range, otherwise as HEX.
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("printASCIIorHEX()");
#endif

  if ((temp < 0x10) || (temp > 0x7E))
  {
    PRINT_CSTSTR(" (");
    printHEXByte(temp);
    PRINT_CSTSTR(") ");
  }
  else
  {
    Serial.write(temp);
  }
}


void SX127XLT::printHEXByte(uint8_t temp)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("printHEXByte()");
#endif

  if (temp < 0x10)
  {
    PRINT_CSTSTR("0");
  }
  PRINT_HEX("%X",temp);
}


void SX127XLT::printHEXByte0x(uint8_t temp)
{
  //print a byte, adding 0x

#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("printHEXByte0x()");
#endif

  PRINT_CSTSTR("0x");
  if (temp < 0x10)
  {
    PRINT_CSTSTR("0");
  }
  PRINT_HEX("%X",temp);
}


bool SX127XLT::isRXdone()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("isRXdone()");
#endif

  return digitalRead(_DIO0);
}

bool SX127XLT::isTXdone()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("isTXdone()");
#endif

  return digitalRead(_DIO0);
}


bool SX127XLT::isRXdoneIRQ()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("isRXdoneIRQ()");
#endif

  return (readRegister(REG_IRQFLAGS) & IRQ_RX_DONE);
}


bool SX127XLT::isTXdoneIRQ()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("isTXdoneIRQ()");
#endif

  return (readRegister(REG_IRQFLAGS) & IRQ_TX_DONE);
}


void SX127XLT::setTXDonePin(uint8_t pin)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("setTXDonePin()");
#endif

  _TXDonePin = pin;
}


void SX127XLT:: setRXDonePin(uint8_t pin)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("setRXDonePin()");
#endif

  _RXDonePin = pin;
}


uint8_t SX127XLT::receive(uint8_t *rxbuffer, uint8_t size, uint32_t rxtimeout, uint8_t wait )
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("receive()");
#endif

  uint16_t index;
  uint32_t endtimeoutmS;
  uint8_t regdata;

  setMode(MODE_STDBY_RC);
  regdata = readRegister(REG_FIFORXBASEADDR);                               //retrieve the RXbase address pointer
  writeRegister(REG_FIFOADDRPTR, regdata);                                  //and save in FIFO access ptr

  setDioIrqParams(IRQ_RADIO_ALL, (IRQ_RX_DONE + IRQ_HEADER_VALID), 0, 0);  //set for IRQ on RX done
  setRx(0);                                                                //no actual RX timeout in this function
  
  if (!wait)
  {
    return 0;                                                              //not wait requested so no packet length to pass
  }

  /**************************************************************************
	Modified by C. Pham - Oct. 2020
  **************************************************************************/
  
  if (rxtimeout == 0)
  {
#ifdef USE_POLLING
    index = readRegister(REG_IRQFLAGS);

    //poll the irq register for ValidHeader, bit 4
    while ((bitRead(index, 4) == 0))
      {
        index = readRegister(REG_IRQFLAGS);
        // adding this small delay decreases the CPU load of the lora_gateway process to 4~5% instead of nearly 100%
        // suggested by rertini (https://github.com/CongducPham/LowCostLoRaGw/issues/211)
        // tests have shown no significant side effects
        delay(1);        
      }    
    
    //_RXTimestamp start when a valid header has been received
    _RXTimestamp=millis();
    
    //poll the irq register for RXDone, bit 6
    while ((bitRead(index, 6) == 0))
      {
        index = readRegister(REG_IRQFLAGS);
      }  
#else  
    while (!digitalRead(_RXDonePin));                  ///Wait for DIO0 to go high, no timeout, RX DONE
#endif    
  }
  else
  {
    endtimeoutmS = (millis() + rxtimeout);
#ifdef USE_POLLING

#ifdef SX127XDEBUG1
  	PRINTLN_CSTSTR("RXDone using polling");
#endif 
    index = readRegister(REG_IRQFLAGS);

    //poll the irq register for ValidHeader, bit 4
    while ((bitRead(index, 4) == 0) && (millis() < endtimeoutmS))
      {
        index = readRegister(REG_IRQFLAGS);
        delay(1);        
      }    

		if (bitRead(index, 4)!=0) {
    	//_RXTimestamp start when a valid header has been received
    	_RXTimestamp=millis();		
#ifdef SX127XDEBUG1		
  		PRINTLN_CSTSTR("Valid header detected");		
#endif

			//update endtimeoutmS to avoid timeout at the middle of a packet reception
			endtimeoutmS = (_RXTimestamp + 10000);
    }
              
    //poll the irq register for RXDone, bit 6
    while ((bitRead(index, 6) == 0) && (millis() < endtimeoutmS))
      {
        index = readRegister(REG_IRQFLAGS);
      }
#else    
    while (!digitalRead(_RXDonePin) && (millis() < endtimeoutmS));
#endif    
  }

  //_RXDoneTimestamp start when the packet been fully received  
  _RXDoneTimestamp=millis();

#ifdef SX127XDEBUG1
  PRINTLN_VALUE("%ld", _RXTimestamp);
	PRINTLN_VALUE("%ld", _RXDoneTimestamp);  		
#endif
    
  setMode(MODE_STDBY_RC);                                            //ensure to stop further packet reception

#ifdef USE_POLLING
  if (bitRead(index, 6) == 0)
#else
  if (!digitalRead(_RXDonePin))                                             //check if DIO still low, is so must be RX timeout
#endif  
  {
    _IRQmsb = IRQ_RX_TIMEOUT;
#ifdef SX127XDEBUG1    
    PRINTLN_CSTSTR("RX timeout");
#endif     
    return 0;
  }

  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/

  if ( readIrqStatus() != (IRQ_RX_DONE + IRQ_HEADER_VALID) )
  {
    return 0;                                                                //no RX done and header valid only, could be CRC error
  }

  _RXPacketL = readRegister(REG_RXNBBYTES);

  if (_RXPacketL > size)                      //check passed buffer is big enough for packet
  {
    _RXPacketL = size;                        //truncate packet if not enough space in passed buffer
  }

#ifdef USE_SPI_TRANSACTION   //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);                    //start the burst read

#if defined ARDUINO || defined USE_ARDUPI  
  SPI.transfer(REG_FIFO);
#endif  

  for (index = 0; index < _RXPacketL; index++)
  {
#if defined ARDUINO || defined USE_ARDUPI
    regdata = SPI.transfer(0);
#else  
    regdata = readRegister(REG_FIFO);
#endif    
    rxbuffer[index] = regdata;
  }
  digitalWrite(_NSS, HIGH);

#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif

  return _RXPacketL;                           //so we can check for packet having enough buffer space
}


uint8_t SX127XLT::receiveAddressed(uint8_t *rxbuffer, uint8_t size, uint32_t rxtimeout, uint8_t wait)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("receiveAddressed()");
#endif

  uint16_t index;
  uint32_t endtimeoutmS;
  uint8_t regdata;

  setMode(MODE_STDBY_RC);
  regdata = readRegister(REG_FIFORXBASEADDR);                                //retrieve the RXbase address pointer
  writeRegister(REG_FIFOADDRPTR, regdata);                                   //and save in FIFO access ptr

  setDioIrqParams(IRQ_RADIO_ALL, (IRQ_RX_DONE + IRQ_HEADER_VALID), 0, 0);   //set for IRQ on RX done
  setRx(0);                                                                 //no actual RX timeout in this function
  
  if (!wait)
  {
    return 0;                                                              //not wait requested so no packet length to pass
  }

  /**************************************************************************
	Modified by C. Pham - Oct. 2020
  **************************************************************************/
  
  if (rxtimeout == 0)
  {
#ifdef USE_POLLING
    index = readRegister(REG_IRQFLAGS);

    //poll the irq register for ValidHeader, bit 4
    while ((bitRead(index, 4) == 0))
      {
        index = readRegister(REG_IRQFLAGS);
        // adding this small delay decreases the CPU load of the lora_gateway process to 4~5% instead of nearly 100%
        // suggested by rertini (https://github.com/CongducPham/LowCostLoRaGw/issues/211)
        // tests have shown no significant side effects
        delay(1);        
      }    

    //_RXTimestamp start when a valid header has been received
    _RXTimestamp=millis();
        
    //poll the irq register for RXDone, bit 6
    while ((bitRead(index, 6) == 0))
      {
        index = readRegister(REG_IRQFLAGS);
      }
#else  
    while (!digitalRead(_RXDonePin));                  ///Wait for DIO0 to go high, no timeout, RX DONE
#endif    
  }
  else
  {
    endtimeoutmS = (millis() + rxtimeout);
#ifdef USE_POLLING

#ifdef SX127XDEBUG1
  	PRINTLN_CSTSTR("RXDone using polling");
#endif 
    index = readRegister(REG_IRQFLAGS);
		
    //poll the irq register for ValidHeader, bit 4
    while ((bitRead(index, 4) == 0) && (millis() < endtimeoutmS))
      {
        index = readRegister(REG_IRQFLAGS);
        delay(1);        
      }    

		if (bitRead(index, 4)!=0) {
    	//_RXTimestamp start when a valid header has been received
    	_RXTimestamp=millis();		
#ifdef SX127XDEBUG1
  		PRINTLN_CSTSTR("Valid header detected"); 		
#endif

			//update endtimeoutmS to avoid timeout at the middle of a packet reception
			endtimeoutmS = (_RXTimestamp + 10000);
    }
              
    //poll the irq register for RXDone, bit 6
    while ((bitRead(index, 6) == 0) && (millis() < endtimeoutmS))
      {
        index = readRegister(REG_IRQFLAGS);
      }
#else    
    while (!digitalRead(_RXDonePin) && (millis() < endtimeoutmS));
#endif    
  }

  //_RXDoneTimestamp start when the packet been fully received  
  _RXDoneTimestamp=millis();

#ifdef SX127XDEBUG1
  PRINTLN_VALUE("%ld", _RXTimestamp);
	PRINTLN_VALUE("%ld", _RXDoneTimestamp);  		
#endif
  
  setMode(MODE_STDBY_RC);                                            //ensure to stop further packet reception

#ifdef USE_POLLING
  if (bitRead(index, 6) == 0)
#else
  if (!digitalRead(_RXDonePin))                                             //check if DIO still low, is so must be RX timeout
#endif  
  {
    _IRQmsb = IRQ_RX_TIMEOUT;
#ifdef SX127XDEBUG1    
    PRINTLN_CSTSTR("RX timeout");
#endif    
    return 0;
  }

  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/

  if ( readIrqStatus() != (IRQ_RX_DONE + IRQ_HEADER_VALID) )
  {
    return 0;                       //no RX done and header valid only, could be CRC error
  }

  _RXPacketL = readRegister(REG_RXNBBYTES);
  
#ifdef SX127XDEBUG1
  PRINT_CSTSTR("Receive ");
  PRINT_VALUE("%d", _RXPacketL);
  PRINTLN_CSTSTR(" bytes");     		
#endif  

#ifdef USE_SPI_TRANSACTION   //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);                    //start the burst read

  /**************************************************************************
	Added by C. Pham - Oct. 2020
  **************************************************************************/  

#if defined ARDUINO || defined USE_ARDUPI
  SPI.transfer(REG_FIFO);
  // we read our header
  _RXDestination = SPI.transfer(0);
  _RXPacketType = SPI.transfer(0);
  _RXSource = SPI.transfer(0);
  _RXSeqNo = SPI.transfer(0);  
#else  
  // we read our header
  _RXDestination = readRegister(REG_FIFO);
  _RXPacketType = readRegister(REG_FIFO);
  _RXSource = readRegister(REG_FIFO);
  _RXSeqNo = readRegister(REG_FIFO);
#endif  

#ifdef SX127XDEBUG1
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
  
  if (_RXPacketL > size)                      //check passed buffer is big enough for packet
  {
    _RXPacketL = size;                          //truncate packet if not enough space in passed buffer
  }  

  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/  
  
  /* original code
   ***************
    
  _RXPacketType = SPI.transfer(0);
  _RXDestination = SPI.transfer(0);
  _RXSource = SPI.transfer(0);
  
  */

  for (index = 0; index < _RXPacketL; index++)
  {
#if defined ARDUINO || defined USE_ARDUPI
    regdata = SPI.transfer(0);
#else  
    regdata = readRegister(REG_FIFO);
#endif
    rxbuffer[index] = regdata;
  }
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
		
#ifdef SX127XDEBUGACK
  	PRINT_CSTSTR("ACK requested by ");
  	PRINTLN_VALUE("%d", _RXSource);
  	PRINTLN_CSTSTR("ACK transmission turnaround safe wait...");
#endif
#ifdef INVERTIQ_ON_ACK
#ifdef SX127XDEBUGACK
		PRINTLN_CSTSTR("invert IQ for ack transmission");
#endif
		invertIQ(true);
#endif			
		delay(100);
		TXBUFFER[0]=2; //length
		TXBUFFER[1]=0; //RFU
		TXBUFFER[2]=readRegister(REG_PKTSNRVALUE); //SNR of received packet
		
		uint8_t saveTXSeqNo=_TXSeqNo;
		_TXSeqNo=_RXSeqNo;
		// use -1 as txpower to use default setting
		TXAckPacketL=transmitAddressed(TXBUFFER, 3, PKT_TYPE_ACK, _RXSource, _DevAddr, 10000, -1, WAIT_TX);	

#ifdef SX127XDEBUGACK
		if (TXAckPacketL)
  		PRINTLN_CSTSTR("ACK sent");
  	else	
  		PRINTLN_CSTSTR("error when sending ACK"); 
#endif
#ifdef INVERTIQ_ON_ACK
#ifdef SX127XDEBUGACK
		PRINTLN_CSTSTR("set back IQ to normal");
#endif
		invertIQ(false);
#endif		
		_TXSeqNo=saveTXSeqNo;
	}

  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/ 	

  return _RXPacketL;                           //so we can check for packet having enough buffer space
}


/**************************************************************************
Added by C. Pham - Dec. 2020
**************************************************************************/

uint8_t SX127XLT::receiveRTSAddressed(uint8_t *rxbuffer, uint8_t size, uint32_t rxtimeout, uint8_t wait)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("receiveRTSAddressed()");
#endif

  uint16_t index;
  uint32_t endtimeoutmS;
  uint8_t regdata;
  bool validHeader=false;

  setMode(MODE_STDBY_RC);
  regdata = readRegister(REG_FIFORXBASEADDR);                                //retrieve the RXbase address pointer
  writeRegister(REG_FIFOADDRPTR, regdata);                                   //and save in FIFO access ptr

  setDioIrqParams(IRQ_RADIO_ALL, (IRQ_RX_DONE + IRQ_HEADER_VALID), 0, 0);   //set for IRQ on RX done
  setRx(0);                                                                 //no actual RX timeout in this function
  
  if (!wait)
  {
    return 0;                                                              //not wait requested so no packet length to pass
  }
  
  if (rxtimeout == 0)
  {
#ifdef USE_POLLING
    index = readRegister(REG_IRQFLAGS);

    //poll the irq register for ValidHeader, bit 4
    while ((bitRead(index, 4) == 0))
      {
        index = readRegister(REG_IRQFLAGS);
        // adding this small delay decreases the CPU load of the lora_gateway process to 4~5% instead of nearly 100%
        // suggested by rertini (https://github.com/CongducPham/LowCostLoRaGw/issues/211)
        // tests have shown no significant side effects
        delay(1);        
      }    

		validHeader=true;
		
    //_RXTimestamp start when a valid header has been received
    _RXTimestamp=millis();
#ifdef SX127XDEBUG1	
  	PRINTLN_CSTSTR("Valid header detected"); 		
#endif
        
    //poll the irq register for RXDone, bit 6
    while ((bitRead(index, 6) == 0))
      {
        index = readRegister(REG_IRQFLAGS);
      }
#else  
    while (!digitalRead(_RXDonePin));                  ///Wait for DIO0 to go high, no timeout, RX DONE
#endif    
  }
  else
  {
    endtimeoutmS = (millis() + rxtimeout);
#ifdef USE_POLLING

#ifdef SX127XDEBUG1
  	PRINTLN_CSTSTR("RXDone using polling");
#endif 
    index = readRegister(REG_IRQFLAGS);
		
    //poll the irq register for ValidHeader, bit 4
    while ((bitRead(index, 4) == 0) && (millis() < endtimeoutmS))
      {
        index = readRegister(REG_IRQFLAGS);
        delay(1);        
      }    

		if (bitRead(index, 4)!=0) {
		
			validHeader=true;
			
    	//_RXTimestamp start when a valid header has been received
    	_RXTimestamp=millis();		
#ifdef SX127XDEBUG1	
  		PRINTLN_CSTSTR("Valid header detected"); 		
#endif

			//update endtimeoutmS to try to get the RXDone in a short time interval
			endtimeoutmS = (_RXTimestamp + 250);
    }
              
    //poll the irq register for RXDone, bit 6
    while ((bitRead(index, 6) == 0) && (millis() < endtimeoutmS))
      {
        index = readRegister(REG_IRQFLAGS);
      }
#else    
    while (!digitalRead(_RXDonePin) && (millis() < endtimeoutmS));
#endif    
  }

  //_RXDoneTimestamp start when the packet been fully received  
  _RXDoneTimestamp=millis();

#ifdef SX127XDEBUG1
  PRINTLN_VALUE("%ld", _RXTimestamp);
	PRINTLN_VALUE("%ld", _RXDoneTimestamp);  		
#endif
  
  setMode(MODE_STDBY_RC);                                            //ensure to stop further packet reception

#ifdef USE_POLLING
  if (bitRead(index, 6) == 0)
#else
  if (!digitalRead(_RXDonePin))                                             //check if DIO still low, is so must be RX timeout
#endif  
  {
    _IRQmsb = IRQ_RX_TIMEOUT;
#ifdef SX127XDEBUG1    
    PRINTLN_CSTSTR("RX timeout");
#endif
		
		// if we got a valid header but no RXDone, it is maybe a long data packet
		// so we return a value greater than 1
		if (validHeader)
			return(2);
		else	    
    	return 0;
  }

  _RXPacketL = readRegister(REG_RXNBBYTES);

#ifdef USE_SPI_TRANSACTION   //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);                    //start the burst read
  
#if defined ARDUINO || defined USE_ARDUPI
  SPI.transfer(REG_FIFO);
  // we read our header
  _RXDestination = SPI.transfer(0);
  _RXPacketType = SPI.transfer(0);
  _RXSource = SPI.transfer(0);
  _RXSeqNo = SPI.transfer(0);  
#else  
  // we read our header
  _RXDestination = readRegister(REG_FIFO);
  _RXPacketType = readRegister(REG_FIFO);
  _RXSource = readRegister(REG_FIFO);
  _RXSeqNo = readRegister(REG_FIFO);
#endif  
  
  //the header is not passed to the user
  _RXPacketL=_RXPacketL-HEADER_SIZE;
  
  if (_RXPacketL > size)                      //check passed buffer is big enough for packet
  {
  	// if _RXPacketL!=1 it is a very short data packet because we have RXDone
  	// so we ignore it
    return(0);
  }  

  for (index = 0; index < _RXPacketL; index++)
  {
#if defined ARDUINO || defined USE_ARDUPI  
    regdata = SPI.transfer(0);
#else
    regdata = readRegister(REG_FIFO);
#endif    
    rxbuffer[index] = regdata;
  }
  digitalWrite(_NSS, HIGH);

#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif

	// check if it is really an RTS packet
	if (_RXPacketType!=PKT_TYPE_RTS)
		return(0);

  return _RXPacketL;                           //so we can check for packet having enough buffer space
}

/**************************************************************************
End by C. Pham - Dec. 2020
**************************************************************************/ 

uint8_t SX127XLT::readPacket(uint8_t *rxbuffer, uint8_t size)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("readPacket()");
#endif

  uint8_t index, regdata;

  if ( readIrqStatus() != (IRQ_RX_DONE + IRQ_HEADER_VALID) )
  {
    return 0;                                 //no RX done and header valid only, could be CRC error
  }

  setMode(MODE_STDBY_RC);
  regdata = readRegister(REG_FIFORXBASEADDR);  //retrieve the RXbase address pointer
  writeRegister(REG_FIFOADDRPTR, regdata);     //and save in FIFO access ptr

  _RXPacketL = readRegister(REG_RXNBBYTES);

  if (_RXPacketL > size)                      //check passed buffer is big enough for packet
  {
    _RXPacketL = size;                          //truncate packet if not enough space
  }

#ifdef USE_SPI_TRANSACTION   //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);                    //start the burst read

#if defined ARDUINO || defined USE_ARDUPI  
  SPI.transfer(REG_FIFO);
#endif

  for (index = 0; index < _RXPacketL; index++)
  {
#if defined ARDUINO || defined USE_ARDUPI  
    regdata = SPI.transfer(0);
#else    
    regdata = readRegister(REG_FIFO);
#endif
    rxbuffer[index] = regdata;   
  }
  digitalWrite(_NSS, HIGH);

#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif

  return _RXPacketL;                           //so we can check for packet having enough buffer space
}


uint8_t SX127XLT::readPacketAddressed(uint8_t *rxbuffer, uint8_t size)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("readPacketAddressed()");
#endif

  uint8_t index, regdata;

  setMode(MODE_STDBY_RC);
  regdata = readRegister(REG_FIFORXBASEADDR);  //retrieve the RXbase address pointer
  writeRegister(REG_FIFOADDRPTR, regdata);     //and save in FIFO access ptr

  _RXPacketL = readRegister(REG_RXNBBYTES);

  if (_RXPacketL > size)                      //check passed buffer is big enough for packet
  {
    _RXPacketL = size;                          //truncate packet if not enough space
  }

#ifdef USE_SPI_TRANSACTION   //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);                    //start the burst read
  
  /**************************************************************************
	Added by C. Pham - Oct. 2020
  **************************************************************************/  

#if defined ARDUINO || defined USE_ARDUPI  
  SPI.transfer(REG_FIFO);
  // we read our header
  _RXDestination = SPI.transfer(0);
  _RXPacketType = SPI.transfer(0);
  _RXSource = SPI.transfer(0);
  _RXSeqNo = SPI.transfer(0);
#else
  // we read our header
  _RXDestination = readRegister(REG_FIFO);
  _RXPacketType = readRegister(REG_FIFO);
  _RXSource = readRegister(REG_FIFO);
  _RXSeqNo = readRegister(REG_FIFO);
#endif

  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/  
  
  /* original code
   ***************
    
  _RXPacketType = SPI.transfer(0);
  _RXDestination = SPI.transfer(0);
  _RXSource = SPI.transfer(0);
  
  */

  for (index = 0; index < _RXPacketL; index++)
  {
#if defined ARDUINO || defined USE_ARDUPI  
    regdata = SPI.transfer(0);
#else
    regdata = readRegister(REG_FIFO);
#endif    
    rxbuffer[index] = regdata;
  }
  digitalWrite(_NSS, HIGH);

#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif

  return _RXPacketL;                           //so we can check for packet having enough buffer space
}


uint8_t SX127XLT::transmit(uint8_t *txbuffer, uint8_t size, uint32_t txtimeout, int8_t txpower, uint8_t wait)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("transmit()");
#endif

  uint8_t index, ptr;
  uint8_t bufferdata;
  uint32_t endtimeoutmS;

  if (size == 0)
  {
    return false;
  }

  setMode(MODE_STDBY_RC);
  ptr = readRegister(REG_FIFOTXBASEADDR);       //retrieve the TXbase address pointer
  writeRegister(REG_FIFOADDRPTR, ptr);          //and save in FIFO access ptr

#ifdef USE_SPI_TRANSACTION                   //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);
  
#if defined ARDUINO || defined USE_ARDUPI  
  SPI.transfer(WREG_FIFO);
#endif

  for (index = 0; index < size; index++)
  {
    bufferdata = txbuffer[index];
#if defined ARDUINO || defined USE_ARDUPI    
    SPI.transfer(bufferdata);
#else
		writeRegister(WREG_FIFO, bufferdata);
#endif    
  }
  digitalWrite(_NSS, HIGH);

#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif

  _TXPacketL = size;
  writeRegister(REG_PAYLOADLENGTH, _TXPacketL);

  setTxParams(txpower, RADIO_RAMP_DEFAULT);            //TX power and ramp time

  setDioIrqParams(IRQ_RADIO_ALL, IRQ_TX_DONE, 0, 0);   //set for IRQ on TX done on first DIO pin
  setTx(0);                                            //TX timeout is not handled in setTX()

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
  
  if (txtimeout == 0)
  {
#ifdef USE_POLLING
    index = readRegister(REG_IRQFLAGS);
    //poll the irq register for TXDone, bit 3
    while ((bitRead(index, 3) == 0))
      {
        index = readRegister(REG_IRQFLAGS);
      }
#else  
    while (!digitalRead(_TXDonePin));                  //Wait for pin to go high, TX finished
#endif    
  }
  else
  {
    endtimeoutmS = (millis() + txtimeout);
#ifdef USE_POLLING

#ifdef SX127XDEBUG1
  	PRINTLN_CSTSTR("TXDone using polling");
#endif  
    index = readRegister(REG_IRQFLAGS);
    //poll the irq register for TXDone, bit 3
    while ((bitRead(index, 3) == 0) && (millis() < endtimeoutmS))
      {
        index = readRegister(REG_IRQFLAGS);
      }
#else    
    while (!digitalRead(_TXDonePin) && (millis() < endtimeoutmS));
#endif    
  }

  setMode(MODE_STDBY_RC);                                            //ensure we leave function with TX off

#ifdef USE_POLLING
  if (bitRead(index, 3) == 0)
#else
  if (!digitalRead(_TXDonePin))
#endif  
  {
    _IRQmsb = IRQ_TX_TIMEOUT;
    return 0;
  }

  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/
  
  return _TXPacketL;                                                     //no timeout, so TXdone must have been set
}


uint8_t SX127XLT::transmitAddressed(uint8_t *txbuffer, uint8_t size, char txpackettype, char txdestination, char txsource, uint32_t txtimeout, int8_t txpower, uint8_t wait )
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("transmitAddressed()");
#endif

  uint8_t index, ptr;
  uint8_t bufferdata;
  uint32_t endtimeoutmS;

  if (size == 0)
  {
    return false;
  }

  setMode(MODE_STDBY_RC);
  ptr = readRegister(REG_FIFOTXBASEADDR);         //retrieve the TXbase address pointer
  writeRegister(REG_FIFOADDRPTR, ptr);            //and save in FIFO access ptr

#ifdef USE_SPI_TRANSACTION                     //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);

  /**************************************************************************
	Added by C. Pham - Oct. 2020
  **************************************************************************/
  
  // we insert our header
#if defined ARDUINO || defined USE_ARDUPI  
  SPI.transfer(WREG_FIFO);
  SPI.transfer(txdestination);                    //Destination node
  SPI.transfer(txpackettype);                     //Write the packet type
  SPI.transfer(txsource);                         //Source node
  SPI.transfer(_TXSeqNo);                           //Sequence number    
#else  
  writeRegister(WREG_FIFO, txdestination);
  writeRegister(WREG_FIFO, txpackettype);
  writeRegister(WREG_FIFO, txsource);
  writeRegister(WREG_FIFO, _TXSeqNo);
#endif  
  
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
#if defined ARDUINO || defined USE_ARDUPI    
    SPI.transfer(bufferdata);
#else
    writeRegister(WREG_FIFO, bufferdata);
#endif    
  }
  digitalWrite(_NSS, HIGH);

#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif

  writeRegister(REG_PAYLOADLENGTH, _TXPacketL);

  setTxParams(txpower, RADIO_RAMP_DEFAULT);             //TX power and ramp time

  setDioIrqParams(IRQ_RADIO_ALL, IRQ_TX_DONE, 0, 0);   //set for IRQ on TX done
  setTx(0);                                            //TX timeout is not handled in setTX()

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
  
  if (txtimeout == 0)
  {
#ifdef USE_POLLING
    index = readRegister(REG_IRQFLAGS);
    //poll the irq register for TXDone, bit 3
    while ((bitRead(index, 3) == 0))
      {
        index = readRegister(REG_IRQFLAGS);
      }
#else  
    while (!digitalRead(_TXDonePin));                  //Wait for pin to go high, TX finished
#endif    
  }
  else
  {
    endtimeoutmS = (millis() + txtimeout);
#ifdef USE_POLLING

#ifdef SX127XDEBUG1
  	PRINTLN_CSTSTR("TXDone using polling");
#endif 
    index = readRegister(REG_IRQFLAGS);
    //poll the irq register for TXDone, bit 3
    while ((bitRead(index, 3) == 0) && (millis() < endtimeoutmS))
      {
        index = readRegister(REG_IRQFLAGS);
      }
#else    
    while (!digitalRead(_TXDonePin) && (millis() < endtimeoutmS));
#endif    
  }

  setMode(MODE_STDBY_RC);                                            //ensure we leave function with TX off

#ifdef USE_POLLING
  if (bitRead(index, 3) == 0)
#else
  if (!digitalRead(_TXDonePin))
#endif  
  {
    _IRQmsb = IRQ_TX_TIMEOUT;
    return 0;
  }

	_AckStatus=0;

	if (txpackettype & PKT_FLAG_ACK_REQ) {
		uint8_t RXAckPacketL;
		const uint8_t RXBUFFER_SIZE=3;
		uint8_t RXBUFFER[RXBUFFER_SIZE];

#ifdef SX127XDEBUGACK
		PRINTLN_CSTSTR("transmitAddressed is waiting for an ACK");
#endif
#ifdef INVERTIQ_ON_ACK
#ifdef SX127XDEBUGACK
		PRINTLN_CSTSTR("invert IQ for ack reception");
#endif
		invertIQ(true);
#endif
		delay(10);			
		//try to receive the ack
		RXAckPacketL=receiveAddressed(RXBUFFER, RXBUFFER_SIZE, 2000, WAIT_RX);
		
		if (RXAckPacketL) {
#ifdef SX127XDEBUGACK
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
#ifdef SX127XDEBUGACK
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
#ifdef SX127XDEBUGACK
				PRINTLN_CSTSTR("not for me!");
#endif			
			}
		}
		else {
#ifdef SX127XDEBUGACK
		PRINTLN_CSTSTR("received nothing");		 
#endif			
		}
#ifdef INVERTIQ_ON_ACK
#ifdef SX127XDEBUGACK
		PRINTLN_CSTSTR("set back IQ to normal");
#endif
		invertIQ(false);
#endif		
	} 
	
  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/

  return _TXPacketL;                                                     //no timeout, so TXdone must have been set
}


void SX127XLT::setupLoRa(uint32_t Frequency, int32_t Offset, uint8_t modParam1, uint8_t modParam2, uint8_t  modParam3, uint8_t modParam4)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("setupLoRa()");
#endif

  setMode(MODE_STDBY_RC);                            //go into standby mode to configure device
  setPacketType(PACKET_TYPE_LORA);                   //use LoRa packets
  setRfFrequency(Frequency, Offset);                 //set the operating frequncy
  calibrateImage(0);                                 //run calibration after setting frequency
  setModulationParams(modParam1, modParam2, modParam3, modParam4);
  setBufferBaseAddress(0x00, 0x00);                  //set bases addresses for TX and RX packets in SX buffer
  setPacketParams(8, LORA_PACKET_VARIABLE_LENGTH, 255, LORA_CRC_ON, LORA_IQ_NORMAL); //set the packet options
  setSyncWord(LORA_MAC_PRIVATE_SYNCWORD);            //syncword to use, 0x12 for standard private, 0x34 for public (LORAWAN)
  setHighSensitivity();
}


uint8_t SX127XLT::getLoRaSF()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("getLoRaSF()");
#endif

  uint8_t regdata;
  regdata = readRegister(REG_MODEMCONFIG2);
  regdata = ((regdata & READ_SF_AND_X) >> 4);       //SX1272 and SX1276 store SF in same place

  return regdata;
}


uint8_t SX127XLT::getLoRaCodingRate()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("getLoRaCodingRate");
#endif

  uint8_t regdata;
  regdata = readRegister(REG_MODEMCONFIG1);

  if (_Device != DEVICE_SX1272)
  {
    //for all devices apart from SX1272
    regdata = (((regdata & READ_CR_AND_X) >> 1) + 4);
  }
  else
  {
    regdata = (((regdata & READ_CR_AND_2) >> 3) + 4);
  }

  return regdata;
}


uint8_t SX127XLT::getOptimisation()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("getOptimisation");
#endif

  uint8_t regdata;

  if (_Device != DEVICE_SX1272)
  {
    //for all devices apart from SX1272
    regdata = readRegister(REG_MODEMCONFIG3);
    regdata = ((regdata & READ_LDRO_AND_X) >> 3);
  }
  else
  {
    regdata = readRegister(REG_MODEMCONFIG1);
    regdata = (regdata & READ_LDRO_AND_2);
  }

  return regdata;
}


uint8_t SX127XLT::getSyncWord()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("getSyncWord");
#endif

  return readRegister(REG_SYNCWORD);
}


uint8_t SX127XLT::getInvertIQ()
{
  //IQ mode reg 0x33

#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("getInvertIQ");
#endif

  uint8_t regdata;
  regdata =  ( (readRegister(REG_INVERTIQ)) & 0x40 );
  return regdata;
}


uint8_t SX127XLT::getVersion()
{
  //IQ mode reg 0x33

#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("getVersion");
#endif

  return readRegister(REG_VERSION);
}


uint16_t SX127XLT::getPreamble()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("getPreamble");
#endif

  uint16_t regdata;
  regdata =  ( (readRegister(REG_PREAMBLEMSB) << 8) + readRegister(REG_PREAMBLELSB) );
  return regdata;
}


uint32_t SX127XLT::returnBandwidth(byte BWregvalue)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("returnBandwidth()");
#endif

  //uint8_t regdata;

  if (_Device == DEVICE_SX1272)
  {
    switch (BWregvalue)
    {
      case 0:
        return 125000;

      case 64:
        return 250000;

      case 128:
        return 500000;

      default:
        return 0xFF;                      //so that a bandwidth invalid entry can be identified ?
    }
  }
  else
  {

    switch (BWregvalue)
    {
      case 0:
        return 7800;

      case 16:
        return 10400;

      case 32:
        return 15600;

      case 48:
        return 20800;

      case 64:
        return 31200;

      case 80:
        return 41700;

      case 96:
        return 62500;

      case 112:
        return 125000;

      case 128:
        return 250000;

      case 144:
        return 500000;

      default:
        return 0xFF;                      //so that a bandwidth invalid entry can be identified ?
    }

  }
}

/*
  uint32_t SX127XLT::returnBandwidth2(byte BWregvalue)
  {
  #ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("returnBandwidth2()");
  #endif

  switch (BWregvalue)
  {
    case 0:
      return 7800;

    case 16:
      return 10400;

    case 32:
      return 15600;

    case 48:
      return 20800;

    case 64:
      return 31200;

    case 80:
      return 41700;

    case 96:
      return 62500;

    case 112:
      return 125000;

    case 128:
      return 250000;

    case 144:
      return 500000;

    default:
      break;
  }
  return 0xFFFF;                      //so that a bandwidth not set can be identified
  }
*/

uint8_t SX127XLT::returnOptimisation(uint8_t Bandwidth, uint8_t SpreadingFactor)
{
  //from the passed bandwidth (bandwidth) and spreading factor this routine
  //calculates whether low data rate optimisation should be on or off

#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("returnOptimisation()");
#endif

  uint32_t tempBandwidth;
  float symbolTime;
  tempBandwidth = returnBandwidth(Bandwidth);
  symbolTime = calcSymbolTime(tempBandwidth, SpreadingFactor);

#ifdef SX127XDEBUG1
  PRINT_CSTSTR("Symbol Time ");
  PRINT_VALUE("%.3f",symbolTime);
  PRINTLN_CSTSTR("mS");
#endif

  if (symbolTime > 16)
  {
#ifdef SX127XDEBUG1
    PRINTLN_CSTSTR("LDR Opt on");
#endif
    return LDRO_ON;
  }
  else
  {
#ifdef SX127XDEBUG1
    PRINTLN_CSTSTR("LDR Opt off");
#endif
    return LDRO_OFF;
  }
}


float SX127XLT::calcSymbolTime(float Bandwidth, uint8_t SpreadingFactor)
{
  //calculates symbol time from passed bandwidth (lbandwidth) and Spreading factor (lSF)and returns in mS

#ifdef SX127XDEBUG1
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


void SX127XLT::printModemSettings()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("printModemSettings()");
#endif

  uint8_t regdata;

  printDevice();
  PRINT_CSTSTR(",");
  PRINT_VALUE("%lu",getFreqInt());
  PRINT_CSTSTR("hz,SF");
  PRINT_VALUE("%d",getLoRaSF());
  PRINT_CSTSTR(",BW");


  if (_Device == DEVICE_SX1272)
  {
    regdata = (readRegister(REG_MODEMCONFIG1) & READ_BW_AND_2);
  }
  else
  {
    regdata = (readRegister(REG_MODEMCONFIG1) & READ_BW_AND_X);
  }

  PRINT_VALUE("%ld",returnBandwidth(regdata));
  PRINT_CSTSTR(",CR4:");
  PRINT_VALUE("%d",getLoRaCodingRate());
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


void SX127XLT::setSyncWord(uint8_t syncword)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("setSyncWord()");
#endif

  writeRegister(REG_SYNCWORD, syncword);
}


uint8_t SX127XLT::receiveSXBuffer(uint8_t startaddr, uint32_t rxtimeout, uint8_t wait )
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("receiveSXBuffer()");
#endif
	
	uint16_t index;
  uint32_t endtimeoutmS;

  setMode(MODE_STDBY_RC);
  writeRegister(REG_FIFORXBASEADDR, startaddr);          //set start address of RX packet in buffer
  setDioIrqParams(IRQ_RADIO_ALL, (IRQ_RX_DONE + IRQ_HEADER_VALID), 0, 0);   //set for IRQ on RX done
  setRx(0);                                                                 //no actual RX timeout in this function

  if (!wait)
  {
    return 0;
  }

  /**************************************************************************
	Modified by C. Pham - Oct. 2020
  **************************************************************************/
  
  if (rxtimeout == 0)
  {
#ifdef USE_POLLING
    index = readRegister(REG_IRQFLAGS);

    //poll the irq register for ValidHeader, bit 4
    while ((bitRead(index, 4) == 0))
      {
        index = readRegister(REG_IRQFLAGS);
        // adding this small delay decreases the CPU load of the lora_gateway process to 4~5% instead of nearly 100%
        // suggested by rertini (https://github.com/CongducPham/LowCostLoRaGw/issues/211)
        // tests have shown no significant side effects
        delay(1);        
      }    
    
    //_RXTimestamp start when a valid header has been received
    _RXTimestamp=millis();
    
    //poll the irq register for RXDone, bit 6
    while ((bitRead(index, 6) == 0))
      {
        index = readRegister(REG_IRQFLAGS);
      }  
#else  
    while (!digitalRead(_RXDonePin));                  ///Wait for DIO0 to go high, no timeout, RX DONE
#endif    
  }
  else
  {
    endtimeoutmS = (millis() + rxtimeout);
#ifdef USE_POLLING

#ifdef SX127XDEBUG1
  	PRINTLN_CSTSTR("RXDone using polling");
#endif 
    index = readRegister(REG_IRQFLAGS);

    //poll the irq register for ValidHeader, bit 4
    while ((bitRead(index, 4) == 0) && (millis() < endtimeoutmS))
      {
        index = readRegister(REG_IRQFLAGS);
        delay(1);        
      }    

    //_RXTimestamp start when a valid header has been received
    _RXTimestamp=millis();
              
    //poll the irq register for RXDone, bit 6
    while ((bitRead(index, 6) == 0) && (millis() < endtimeoutmS))
      {
        index = readRegister(REG_IRQFLAGS);
      }
#else    
    while (!digitalRead(_RXDonePin) && (millis() < endtimeoutmS));
#endif    
  }

  //_RXDoneTimestamp start when the packet been fully received  
  _RXDoneTimestamp=millis();
    
  setMode(MODE_STDBY_RC);                                            //ensure to stop further packet reception

#ifdef USE_POLLING
  if (bitRead(index, 6) == 0)
#else
  if (!digitalRead(_RXDonePin))                                             //check if DIO still low, is so must be RX timeout
#endif  
  {
    _IRQmsb = IRQ_RX_TIMEOUT;
#ifdef SX127XDEBUG1    
    PRINTLN_CSTSTR("RX timeout");
#endif     
    return 0;
  }

  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/

  if ( readIrqStatus() != (IRQ_RX_DONE + IRQ_HEADER_VALID) )
  {
    return 0;                                  //no RX done and header valid only, could be CRC error
  }

  _RXPacketL = readRegister(REG_RXNBBYTES);

  return _RXPacketL;                           //so we can check for packet having enough buffer space
}



uint8_t SX127XLT::transmitSXBuffer(uint8_t startaddr, uint8_t length, uint32_t txtimeout, int8_t txpower, uint8_t wait)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("transmitSXBuffer()");
#endif
	
	uint16_t index;
  uint32_t endtimeoutmS = 0;

  setMode(MODE_STDBY_RC);

  writeRegister(REG_FIFOTXBASEADDR, startaddr);         //set start address of packet in buffer
  writeRegister(REG_PAYLOADLENGTH, length);

  setTxParams(txpower, RADIO_RAMP_DEFAULT);             //TX power and ramp time

  setDioIrqParams(IRQ_RADIO_ALL, IRQ_TX_DONE, 0, 0);    //set for IRQ on TX done
  setTx(0);                                             //TX timeout is not handled in setTX()

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
    return length;
  }

  /**************************************************************************
	Modified by C. Pham - Oct. 2020
  **************************************************************************/
  
  if (txtimeout == 0)
  {
#ifdef USE_POLLING
    index = readRegister(REG_IRQFLAGS);
    //poll the irq register for TXDone, bit 3
    while ((bitRead(index, 3) == 0))
      {
        index = readRegister(REG_IRQFLAGS);
      }
#else  
    while (!digitalRead(_TXDonePin));                  //Wait for pin to go high, TX finished
#endif    
  }
  else
  {
    endtimeoutmS = (millis() + txtimeout);
#ifdef USE_POLLING

#ifdef SX127XDEBUG1
  	PRINTLN_CSTSTR("TXDone using polling");
#endif  
    index = readRegister(REG_IRQFLAGS);
    //poll the irq register for TXDone, bit 3
    while ((bitRead(index, 3) == 0) && (millis() < endtimeoutmS))
      {
        index = readRegister(REG_IRQFLAGS);
      }
#else    
    while (!digitalRead(_TXDonePin) && (millis() < endtimeoutmS));
#endif    
  }

  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/

  setMode(MODE_STDBY_RC);                               //ensure we leave function with TX off

  if (millis() >= endtimeoutmS)                         //flag if TX timeout
  {
    _IRQmsb = IRQ_TX_TIMEOUT;

    return 0;
  }

  return length;                                         //no timeout, so TXdone must have been set
}

void SX127XLT::printSXBufferHEX(uint8_t start, uint8_t end)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("printSXBufferHEX()");
#endif

  uint8_t index, regdata;

  setMode(MODE_STDBY_RC);
  writeRegister(REG_FIFOADDRPTR, start);         //set FIFO access ptr to start

#ifdef USE_SPI_TRANSACTION     //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);                       //start the burst read
  
#if defined ARDUINO || defined USE_ARDUPI  
  SPI.transfer(REG_FIFO);
#endif

  for (index = start; index <= end; index++)
  {
#if defined ARDUINO || defined USE_ARDUPI   
    regdata = SPI.transfer(0);
#else
    regdata = readRegister(REG_FIFO);
#endif    
    printHEXByte(regdata);
    PRINT_CSTSTR(" ");

  }
  digitalWrite(_NSS, HIGH);

#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif

}

void SX127XLT::printSXBufferASCII(uint8_t start, uint8_t end)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("printSXBufferASCII)");
#endif

  uint8_t index, regdata;
  setMode(MODE_STDBY_RC);

  writeRegister(REG_FIFOADDRPTR, start);      //and save in FIFO access ptr

#ifdef USE_SPI_TRANSACTION   //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);                    //start the burst read
  
#if defined ARDUINO || defined USE_ARDUPI  
  SPI.transfer(REG_FIFO);
#endif

  for (index = start; index <= end; index++)
  {
#if defined ARDUINO || defined USE_ARDUPI   
    regdata = SPI.transfer(0);
#else
    regdata = readRegister(REG_FIFO);
#endif   
    Serial.write(regdata);
  }
  digitalWrite(_NSS, HIGH);

#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif

}

void SX127XLT::fillSXBuffer(uint8_t startaddress, uint8_t size, uint8_t character)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("fillSXBuffer()");
#endif
  uint8_t index;

  setMode(MODE_STDBY_RC);
  writeRegister(REG_FIFOADDRPTR, startaddress);     //and save in FIFO access ptr

#ifdef USE_SPI_TRANSACTION                          //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);                          //start the burst write

#if defined ARDUINO || defined USE_ARDUPI  
  SPI.transfer(WREG_FIFO);
#endif

  for (index = 0; index < size; index++)
  {
#if defined ARDUINO || defined USE_ARDUPI  
    SPI.transfer(character);
#else
    writeRegister(WREG_FIFO, character);
#endif        
  }

  digitalWrite(_NSS, HIGH);

#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif

}

uint8_t SX127XLT::getByteSXBuffer(uint8_t addr)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("getByteSXBuffer()");
#endif

  uint8_t regdata;
  setMode(MODE_STDBY_RC);                     //this is needed to ensure we can read from buffer OK.

  writeRegister(REG_FIFOADDRPTR, addr);       //set FIFO access ptr to location

#ifdef USE_SPI_TRANSACTION                    //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);                    //start the burst read
#if defined ARDUINO || defined USE_ARDUPI   
  SPI.transfer(REG_FIFO);
  regdata = SPI.transfer(0);
#else
  regdata = readRegister(REG_FIFO);
#endif  
  digitalWrite(_NSS, HIGH);

#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif

  return regdata;
}

void SX127XLT::writeByteSXBuffer(uint8_t addr, uint8_t regdata)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("writeByteSXBuffer");
#endif

  setMode(MODE_STDBY_RC);                 //this is needed to ensure we can write to buffer OK.

  writeRegister(REG_FIFOADDRPTR, addr);   //set FIFO access ptr to location

#ifdef USE_SPI_TRANSACTION                //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);                //start the burst read
#if defined ARDUINO || defined USE_ARDUPI  
  SPI.transfer(WREG_FIFO);
  SPI.transfer(regdata);
#else
	writeRegister(WREG_FIFO,regdata);
#endif  
  digitalWrite(_NSS, HIGH);

#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif

}

void SX127XLT::startWriteSXBuffer(uint8_t ptr)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("startWriteSXBuffer()");
#endif

  setMode(MODE_STDBY_RC);

  _TXPacketL = 0;                               //this variable used to keep track of bytes written
  writeRegister(REG_FIFOADDRPTR, ptr);          //set buffer access ptr

#ifdef USE_SPI_TRANSACTION                      //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);
#if defined ARDUINO || defined USE_ARDUPI  
  SPI.transfer(WREG_FIFO);
#endif  
}

uint8_t SX127XLT::endWriteSXBuffer()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("endWriteSXBuffer()");
#endif

  digitalWrite(_NSS, HIGH);

#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif

  return _TXPacketL;
}

void SX127XLT::startReadSXBuffer(uint8_t ptr)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("startReadSXBuffer()");
#endif

  setMode(MODE_STDBY_RC);
  _RXPacketL = 0;
  writeRegister(REG_FIFOADDRPTR, ptr);           //set buffer access ptr

#ifdef USE_SPI_TRANSACTION                       //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);                       //start the burst read
  
#if defined ARDUINO || defined USE_ARDUPI  
  SPI.transfer(REG_FIFO);
#endif
  //next line would be data = SPI.transfer(0);
  //SPI interface ready for byte to read from
}

uint8_t SX127XLT::endReadSXBuffer()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("endReadSXBuffer()");
#endif

  digitalWrite(_NSS, HIGH);

#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif

  return _RXPacketL;

}

void SX127XLT::writeUint8(uint8_t x)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("writeUint8()");
#endif

#if defined ARDUINO || defined USE_ARDUPI
  SPI.transfer(x);
#else
	writeRegister(WREG_FIFO, x);
#endif  

  _TXPacketL++;                     //increment count of bytes written
}

uint8_t SX127XLT::readUint8()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("readUint8()");
#endif

  uint8_t x;

#if defined ARDUINO || defined USE_ARDUPI
  x = SPI.transfer(0);
#else
	x = readRegister(REG_FIFO);
#endif

  _RXPacketL++;                      //increment count of bytes read
  return (x);
}


uint8_t SX127XLT::readBytes(uint8_t *rxbuffer,   uint8_t size)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("readBytes()");
#endif

  uint8_t x, index;
  
  for (index = 0; index < size; index++)
  {
#if defined ARDUINO || defined USE_ARDUPI  
  x = SPI.transfer(0);
#else
	x = readRegister(REG_FIFO);
#endif  
  rxbuffer[index] = x;
  }
  
  _RXPacketL = _RXPacketL + size;                      //increment count of bytes read
  return size;
}


void SX127XLT::writeInt8(int8_t x)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("writeInt8()");
#endif

#if defined ARDUINO || defined USE_ARDUPI
  SPI.transfer(x);
#else
	writeRegister(WREG_FIFO, x);
#endif  

  _TXPacketL++;                     //increment count of bytes written
}


int8_t SX127XLT::readInt8()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("readInt8()");
#endif

  int8_t x;

#if defined ARDUINO || defined USE_ARDUPI
  x = SPI.transfer(0);
#else
	x = readRegister(REG_FIFO);
#endif

  _RXPacketL++;                      //increment count of bytes read
  return (x);
}


void SX127XLT::writeChar(char x)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("writeChar()");
#endif

#if defined ARDUINO || defined USE_ARDUPI
  SPI.transfer(x);
#else
	writeRegister(WREG_FIFO, x);
#endif

  _TXPacketL++;                     //increment count of bytes written
}


char SX127XLT::readChar()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("readChar()");
#endif

  char x;

#if defined ARDUINO || defined USE_ARDUPI
  x = SPI.transfer(0);
#else
	x = readRegister(REG_FIFO);
#endif

  _RXPacketL++;                      //increment count of bytes read
  return (x);
}


void SX127XLT::writeUint16(uint16_t x)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("writeUint16()");
#endif

#if defined ARDUINO || defined USE_ARDUPI
  SPI.transfer(lowByte(x));
  SPI.transfer(highByte(x));
#else
	writeRegister(WREG_FIFO, lowByte(x));
	writeRegister(WREG_FIFO, highByte(x));
#endif  

  _TXPacketL = _TXPacketL + 2;         //increment count of bytes written
}


uint16_t SX127XLT::readUint16()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("writeUint16()");
#endif

  uint8_t lowbyte, highbyte;

#if defined ARDUINO || defined USE_ARDUPI
  lowbyte = SPI.transfer(0);
  highbyte = SPI.transfer(0);
#else
  lowbyte = readRegister(REG_FIFO);
  highbyte = readRegister(REG_FIFO);
#endif  

  _RXPacketL = _RXPacketL + 2;         //increment count of bytes read
  return ((highbyte << 8) + lowbyte);
}


void SX127XLT::writeInt16(int16_t x)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("writeInt16()");
#endif

#if defined ARDUINO || defined USE_ARDUPI
  SPI.transfer(lowByte(x));
  SPI.transfer(highByte(x));
#else
	writeRegister(WREG_FIFO, lowByte(x));
	writeRegister(WREG_FIFO, highByte(x));
#endif

  _TXPacketL = _TXPacketL + 2;         //increment count of bytes written
}


int16_t SX127XLT::readInt16()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("readInt16()");
#endif

  uint8_t lowbyte, highbyte;

#if defined ARDUINO || defined USE_ARDUPI
  lowbyte = SPI.transfer(0);
  highbyte = SPI.transfer(0);
#else
  lowbyte = readRegister(REG_FIFO);
  highbyte = readRegister(REG_FIFO);
#endif

  _RXPacketL = _RXPacketL + 2;         //increment count of bytes read
  return ((highbyte << 8) + lowbyte);
}


void SX127XLT::writeUint32(uint32_t x)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("writeUint32()");
#endif

  uint8_t i, j;

  union
  {
    uint8_t b[4];
    uint32_t f;
  } data;
  data.f = x;

  for (i = 0; i < 4; i++)
  {
    j = data.b[i];
#if defined ARDUINO || defined USE_ARDUPI    
    SPI.transfer(j);
#else
	writeRegister(WREG_FIFO, j);
#endif    
  }

  _TXPacketL = _TXPacketL + 4;         //increment count of bytes written
}


uint32_t SX127XLT::readUint32()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("readUint32()");
#endif

  uint8_t i, j;

  union
  {
    uint8_t b[4];
    uint32_t f;
  } readdata;

  for (i = 0; i < 4; i++)
  {
#if defined ARDUINO || defined USE_ARDUPI  
    j = SPI.transfer(0);
#else
		j = readRegister(REG_FIFO);
#endif    
    readdata.b[i] = j;
  }
  _RXPacketL = _RXPacketL + 4;         //increment count of bytes read
  return readdata.f;
}


void SX127XLT::writeInt32(int32_t x)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("writeInt32()");
#endif

  uint8_t i, j;

  union
  {
    uint8_t b[4];
    int32_t f;
  } data;
  data.f = x;

  for (i = 0; i < 4; i++)
  {
    j = data.b[i];
#if defined ARDUINO || defined USE_ARDUPI    
    SPI.transfer(j);
#else
	writeRegister(WREG_FIFO, j);
#endif
  }

  _TXPacketL = _TXPacketL + 4;         //increment count of bytes written
}


int32_t SX127XLT::readInt32()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("readInt32()");
#endif

  uint8_t i, j;

  union
  {
    uint8_t b[4];
    int32_t f;
  } readdata;

  for (i = 0; i < 4; i++)
  {
#if defined ARDUINO || defined USE_ARDUPI  
    j = SPI.transfer(0);
#else
		j = readRegister(REG_FIFO);
#endif  
    readdata.b[i] = j;
  }
  _RXPacketL = _RXPacketL + 4;         //increment count of bytes read
  return readdata.f;
}


void SX127XLT::writeFloat(float x)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("writeFloat()");
#endif

  uint8_t i, j;

  union
  {
    uint8_t b[4];
    float f;
  } data;
  data.f = x;

  for (i = 0; i < 4; i++)
  {
    j = data.b[i];
#if defined ARDUINO || defined USE_ARDUPI    
    SPI.transfer(j);
#else
	writeRegister(WREG_FIFO, j);
#endif
  }

  _TXPacketL = _TXPacketL + 4;         //increment count of bytes written
}


float SX127XLT::readFloat()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("readFloat()");
#endif

  uint8_t i, j;

  union
  {
    uint8_t b[4];
    float f;
  } readdata;

  for (i = 0; i < 4; i++)
  {
#if defined ARDUINO || defined USE_ARDUPI  
    j = SPI.transfer(0);
#else
		j = readRegister(REG_FIFO);
#endif 
    readdata.b[i] = j;
  }
  _RXPacketL = _RXPacketL + 4;         //increment count of bytes read
  return readdata.f;
}


void SX127XLT::writeBuffer(uint8_t *txbuffer, uint8_t size)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("writeBuffer()");
#endif

  uint8_t index, regdata;

  _TXPacketL = _TXPacketL + size;      //these are the number of bytes that will be added

  size--;                              //loose one byte from size, the last byte written MUST be a 0

  for (index = 0; index < size; index++)
  {
    regdata = txbuffer[index];
#if defined ARDUINO || defined USE_ARDUPI     
    SPI.transfer(regdata);
#else
		writeRegister(WREG_FIFO, regdata);
#endif    
  }

#if defined ARDUINO || defined USE_ARDUPI
  SPI.transfer(0);                     //this ensures last byte of buffer writen really is a null (0)
#else
	writeRegister(WREG_FIFO, 0);
#endif    
}



void SX127XLT::writeBufferChar(char *txbuffer, uint8_t size)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("writeBuffer()");
#endif

  uint8_t index, regdata;

  _TXPacketL = _TXPacketL + size;      //these are the number of bytes that will be added

  size--;                              //loose one byte from size, the last byte written MUST be a 0

  for (index = 0; index < size; index++)
  {
    regdata = txbuffer[index];
#if defined ARDUINO || defined USE_ARDUPI     
    SPI.transfer(regdata);
#else
		writeRegister(WREG_FIFO, regdata);
#endif 
  }

#if defined ARDUINO || defined USE_ARDUPI
  SPI.transfer(0);                     //this ensures last byte of buffer writen really is a null (0)
#else
	writeRegister(WREG_FIFO, 0);
#endif 
}


uint8_t SX127XLT::readBuffer(uint8_t *rxbuffer)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("readBuffer()");
#endif

  uint8_t index = 0, regdata;

  do                                     //need to find the size of the buffer first
  {
#if defined ARDUINO || defined USE_ARDUPI  
    regdata = SPI.transfer(0);
#else
		regdata = readRegister(REG_FIFO);
#endif    
    rxbuffer[index] = regdata;           //fill the buffer.
    index++;
  } while (regdata != 0);                //keep reading until we have reached the null (0) at the buffer end
  //or exceeded size of buffer allowed
  
  _RXPacketL = _RXPacketL + index;       //increment count of bytes read
  
  return index;                          //return the actual size of the buffer, till the null (0) detected

}


uint8_t SX127XLT::readBufferChar(char *rxbuffer)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("readBuffer()");
#endif

  uint8_t index = 0, regdata;

  do                                     //need to find the size of the buffer first
  {
#if defined ARDUINO || defined USE_ARDUPI  
    regdata = SPI.transfer(0);
#else
		regdata = readRegister(REG_FIFO);
#endif
    rxbuffer[index] = regdata;           //fill the buffer.
    index++;
  } while (regdata != 0);                //keep reading until we have reached the null (0) at the buffer end
  //or exceeded size of buffer allowed
  
  _RXPacketL = _RXPacketL + index;       //increment count of bytes read
  
  return index;                          //return the actual size of the buffer, till the null (0) detected

}


void SX127XLT::rxtxInit(int8_t pinRXEN, int8_t pinTXEN)
{
  //not used on current SX127x modules

#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("rxtxInit()");
#endif

  _RXEN = pinRXEN;
  _TXEN = pinTXEN;

  pinMode(pinRXEN, OUTPUT);
  digitalWrite(pinRXEN, LOW);           //pins needed for RX\TX switching
  pinMode(pinTXEN, OUTPUT);
  digitalWrite(pinTXEN, LOW);           //pins needed for RX\TX switching
}


void SX127XLT::rxEnable()
{
  //not used on current SX127x modules

#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("rxEnable()");
#endif

  digitalWrite(_RXEN, HIGH);
  digitalWrite(_TXEN, LOW);
}


void SX127XLT::txEnable()
{
  //not used on current SX127x modules

#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("txEnable()");
#endif

  digitalWrite(_RXEN, LOW);
  digitalWrite(_TXEN, HIGH);
}


void SX127XLT::setTXDirect()
{
  //turns on transmitter, in direct mode for FSK and audio  power level is from 2 to 17
#ifdef SX127XDEBUG1
  PRINT_CSTSTR("setTXDirect()");
#endif
  writeRegister(REG_OPMODE, 0x0B);           //TX on direct mode, low frequency mode
}


void SX127XLT::toneFM(uint16_t frequency, uint32_t length, uint32_t deviation, float adjust, int8_t txpower)
{
#ifdef SX127XDEBUG1
  PRINT_CSTSTR("toneFM()");
#endif

  uint16_t index;
  uint32_t ToneDelayus;
  uint32_t registershift;
  uint32_t freqreg;
  uint32_t shiftedfreqregH, shiftedfreqregL;
  uint32_t loopcount;
  uint8_t freqregH, freqregM, freqregL;

  #ifdef USE_SPI_TRANSACTION         //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
  #endif
  
  digitalWrite(_NSS, LOW);           //set NSS low
#if defined ARDUINO || defined USE_ARDUPI  
  SPI.transfer(REG_FRMSB & 0x7F);    //mask address for read
  freqregH = SPI.transfer(0);
  freqregM = SPI.transfer(0);
  freqregL = SPI.transfer(0);
#else
	uint8_t spibuf[4];
	spibuf[0] = REG_FRMSB & 0x7F;
	spibuf[1] = 0x00;
	spibuf[2] = 0x00;
	spibuf[3] = 0x00;		

	wiringPiSPIDataRW(SPI_CHANNEL, spibuf, 4);

  freqregH = spibuf[1];
  freqregM = spibuf[2];
  freqregL = spibuf[3];	
#endif  
  digitalWrite(_NSS, HIGH);          //set NSS high
  
#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif
    
  freqreg = ( ( (uint32_t) freqregH << 16 ) | ( (uint32_t) freqregM << 8 ) | ( freqregL ) );

  registershift = deviation / FREQ_STEP;
  shiftedfreqregH = freqreg + registershift;
  shiftedfreqregL = freqreg - registershift;

  uint8_t ShiftH = shiftedfreqregH >> 16;
  uint8_t ShiftM = shiftedfreqregH >> 8;
  uint8_t ShiftL = shiftedfreqregH;
  uint8_t NoShiftH = shiftedfreqregL >> 16;
  uint8_t NoShiftM = shiftedfreqregL >> 8;
  uint8_t NoShiftL = shiftedfreqregL;

  ToneDelayus = ((500000 / frequency));
  loopcount = (length * 500) / (ToneDelayus);
  ToneDelayus = ToneDelayus * adjust;

#ifdef SX127XDEBUG3
  PRINT_CSTSTR("frequency ");
  PRINTLN_VALUE("%lu",frequency);
  PRINT_CSTSTR("length ");
  PRINTLN_VALUE("%ld",length);

  PRINT_CSTSTR("freqreg ");
  PRINTLN_HEX("%lX",freqreg);
  PRINT_CSTSTR("registershift ");
  PRINTLN_VALUE("%ld",registershift);
  shiftedfreqregH = freqreg + (registershift / 2);
  shiftedfreqregL = freqreg - (registershift / 2);
  PRINT_CSTSTR("shiftedfreqregH ");
  PRINTLN_HEX("%lX",shiftedfreqregH);
  PRINT_CSTSTR("shiftedfreqregL ");
  PRINTLN_HEX("%lX",shiftedfreqregL);

  PRINT_CSTSTR("ShiftedHigh,");
  PRINT_HEX("%X",ShiftH);
  PRINT_CSTSTR(",");
  PRINT_HEX("%X",ShiftM);
  PRINT_CSTSTR(",");
  PRINTLN_HEX("%X",ShiftL);

  PRINT_CSTSTR("ShiftedLow,");
  PRINT_HEX("%X",NoShiftH);
  PRINT_CSTSTR(",");
  PRINT_HEX("%X",NoShiftM);
  PRINT_CSTSTR(",");
  PRINTLN_HEX("%X",NoShiftL);
  PRINT_CSTSTR("ToneDelayus,");
  PRINTLN_VALUE("%ld",ToneDelayus);
  PRINT_CSTSTR("loopcount,");
  PRINTLN_VALUE("%ld",loopcount);
  PRINTLN;
  PRINTLN;
#endif

  writeRegister(REG_PLLHOP, 0xAD);            //set fast hop mode, needed for fast changes of frequency
  
  setTxParams(txpower, RADIO_RAMP_DEFAULT);
  setTXDirect();
 
  #ifdef USE_SPI_TRANSACTION         //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
  #endif
  
  for (index = 1; index <= loopcount; index++)
  {
    digitalWrite(_NSS, LOW);                  //set NSS low
#if defined ARDUINO || defined USE_ARDUPI    
    SPI.transfer(0x86);                       //address for write to REG_FRMSB
    SPI.transfer(ShiftH);
    SPI.transfer(ShiftM);
    SPI.transfer(ShiftL);
#else
	uint8_t spibuf[4];
	spibuf[0] = 0x86;
	spibuf[1] = ShiftH;
	spibuf[2] = ShiftM;
	spibuf[3] = ShiftL;		

	wiringPiSPIDataRW(SPI_CHANNEL, spibuf, 4);
#endif    
    digitalWrite(_NSS, HIGH);                 //set NSS high
    delayMicroseconds(ToneDelayus);

    digitalWrite(_NSS, LOW);                  //set NSS low
#if defined ARDUINO || defined USE_ARDUPI    
    SPI.transfer(0x86);                       //address for write to REG_FRMSB
    SPI.transfer(NoShiftH);
    SPI.transfer(NoShiftM);
    SPI.transfer(NoShiftL);
#else
	spibuf[0] = 0x86;
	spibuf[1] = NoShiftH;
	spibuf[2] = NoShiftM;
	spibuf[3] = NoShiftL;		

	wiringPiSPIDataRW(SPI_CHANNEL, spibuf, 4);
#endif    
    digitalWrite(_NSS, HIGH);                 //set NSS high

    delayMicroseconds(ToneDelayus);
  }
  //now set the frequency registers back to centre
  digitalWrite(_NSS, LOW);                  //set NSS low
#if defined ARDUINO || defined USE_ARDUPI  
  SPI.transfer(0x86);                       //address for write to REG_FRMSB
  SPI.transfer(freqregH);
  SPI.transfer(freqregM);
  SPI.transfer(freqregL);
#else
	spibuf[0] = 0x86;
	spibuf[1] = freqregH;
	spibuf[2] = freqregM;
	spibuf[3] = freqregL;		

	wiringPiSPIDataRW(SPI_CHANNEL, spibuf, 4);
#endif  
  digitalWrite(_NSS, HIGH);                 //set NSS high

#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif

  writeRegister(REG_PLLHOP, 0x2D);          //restore PLLHOP register value
  setMode(MODE_STDBY_RC);                   //turns off carrier
}

void SX127XLT::setupDirect(uint32_t frequency, int32_t offset)
{
  //setup LoRa device for direct modulation mode
#ifdef SX127XDEBUG1
  PRINT_CSTSTR("setupDirect()");
#endif
  _PACKET_TYPE = PACKET_TYPE_GFSK;            //need to swap packet type
  setMode(MODE_SLEEP);                        //can only swap to direct mode in sleepmode
  setMode(MODE_SLEEP);                        //can only swap to direct mode in sleepmode 
  setMode(MODE_STDBY_RC);
  writeRegister(REG_DETECTOPTIMIZE, 0x00);    //set continuous mode
  setRfFrequency(frequency, offset);          //set the operating frequncy
  calibrateImage(0);                          //run calibration after setting frequency
  writeRegister(REG_FDEVLSB, 0);              //We are generating a tone by frequency shift so set deviation to 0
}

int8_t SX127XLT::getDeviceTemperature()
{
  #ifdef SX127XDEBUG1
  PRINT_CSTSTR("getDeviceTemperature()");
  #endif
  
  int8_t temperature;
  uint8_t regdata;
  
  regdata = readRegister(REG_IMAGECAL & 0xFE);
  
  writeRegister(REG_IMAGECAL, 0x00);                //register back to power up default 

  writeRegister(REG_OPMODE, 0x00);                  //goto sleep
  writeRegister(REG_OPMODE, 0x00);                  //make sure switch to FSK mode 
  writeRegister(REG_OPMODE, 0x01);                  //go into FSK standby
  delay(5);                                        //wait for oscillator startup 
  writeRegister(REG_OPMODE, 0x04);                  //put device in FSK RX synth mode
  
  writeRegister(REG_IMAGECAL, 0x00);                //set TempMonitorOff = 0
  delay(1);                                        //wait at least 140uS 
  writeRegister(REG_IMAGECAL, 0x01);                //set TempMonitorOff = 1
  setMode(MODE_STDBY_RC);                          //go back to standby
  
  writeRegister(REG_IMAGECAL, (regdata + 1));       //register back to previous setting, with TempMonitorOff set
    
  temperature = readRegister(REG_TEMP);
  
  if (temperature & 0x80 )                         //The sign bit is 1
  {
    temperature = ( ( ~temperature + 1 ) & 0xFF ); //Invert and divide by 4
  }
  else
  {
    temperature = ( temperature & 0xFF );          //Divide by 4
  }
  
  writeRegister(REG_OPMODE,MODE_STDBY);
   
  return temperature;
}



void SX127XLT::fskCarrierOn(int8_t txpower)
{
#ifdef SX127XDEBUG1
  PRINT_CSTSTR("fskCarrierOn()");
#endif

  writeRegister(REG_PLLHOP, 0xAD);          //set fast hop mode, needed for fast changes of frequency
  setTxParams(txpower, RADIO_RAMP_DEFAULT);
  setTXDirect();
}


void SX127XLT::fskCarrierOff()
{
#ifdef SX127XDEBUG1
  PRINT_CSTSTR("fskCarrierOff()");
#endif

 setMode(MODE_STDBY_RC);                   //turns off carrier
}


void SX127XLT::setRfFrequencyDirect(uint8_t high, uint8_t mid, uint8_t low)
{   
  
#ifdef SX127XDEBUG1
  PRINT_CSTSTR("setRfFrequencyDirect()");
#endif
  
#ifdef USE_SPI_TRANSACTION         //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif
  
  digitalWrite(_NSS, LOW);                  //set NSS low
#if defined ARDUINO || defined USE_ARDUPI  
  SPI.transfer(0x86);                       //address for write to REG_FRMSB
  SPI.transfer(high);
  SPI.transfer(mid);
  SPI.transfer(low);
#else
	uint8_t spibuf[4];
	spibuf[0] = 0x86;
	spibuf[1] = high;
	spibuf[2] = mid;
	spibuf[3] = low;		

	wiringPiSPIDataRW(SPI_CHANNEL, spibuf, 4);
#endif   
  digitalWrite(_NSS, HIGH);                 //set NSS high
  
#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif
}

void SX127XLT::getRfFrequencyRegisters(uint8_t *buff)
{
  //returns the register values for the current set frequency
  
#ifdef SX127XDEBUG1
  PRINT_CSTSTR("getRfFrequencyRegisters()");
#endif

  
#ifdef USE_SPI_TRANSACTION         //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif
  
  digitalWrite(_NSS, LOW);                  //set NSS low
#if defined ARDUINO || defined USE_ARDUPI  
  SPI.transfer(REG_FRMSB & 0x7F);           //mask address for read
  buff[0] = SPI.transfer(0);                //read the byte into buffer
  buff[1] = SPI.transfer(0);                //read the byte into buffer
  buff[2] = SPI.transfer(0);                //read the byte into buffer
#else
	uint8_t spibuf[4];
	spibuf[0] = REG_FRMSB & 0x7F;
	spibuf[1] = 0x00;
	spibuf[2] = 0x00;
	spibuf[3] = 0x00;		

	wiringPiSPIDataRW(SPI_CHANNEL, spibuf, 4);
	
	buff[0] = spibuf[1];
	buff[1] = spibuf[2];
	buff[2] = spibuf[3];		
#endif   
  digitalWrite(_NSS, HIGH);                 //set NSS high
  
#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif
}

void SX127XLT::startFSKRTTY(uint32_t freqshift, uint8_t pips, uint16_t pipPeriodmS, uint16_t pipDelaymS, uint16_t leadinmS)
{
  
  #ifdef SX127XDEBUG1
  PRINT_CSTSTR("startFSKRTTY()");
  #endif
  
  uint8_t freqShiftRegs[3];                        //to hold returned registers for shifted frequency 
  uint32_t setCentreFrequency;                     //the configured centre frequency 
  uint8_t index;
  uint32_t endmS;
  setCentreFrequency = savedFrequency;             //to avoid using the savedFrequency
    
  writeRegister(REG_PLLHOP, 0xAD);                 //set fast hop mode, needed for fast changes of frequency
  
  setRfFrequency((savedFrequency + freqshift), savedOffset);          //temporaily set the RF frequency
  getRfFrequencyRegisters(freqShiftRegs);                             //fill first 3 bytes with current frequency registers
  setRfFrequency(setCentreFrequency, savedOffset);                    //reset the base frequency registers
   
  _ShiftfreqregH = freqShiftRegs[0];
  _ShiftfreqregM = freqShiftRegs[1];
  _ShiftfreqregL = freqShiftRegs[2];
  
  writeRegister(REG_PLLHOP, 0xAD);          //set fast hop mode, needed for fast changes of frequency
  setTxParams(10, RADIO_RAMP_DEFAULT);
  
  for (index = 1; index <= pips; index++)
  {
  setRfFrequencyDirect(_ShiftfreqregH, _ShiftfreqregM, _ShiftfreqregL); //set carrier frequency
  setTXDirect();                                                           //turn on carrier 
  delay(pipPeriodmS);
  setMode(MODE_STDBY_RC);                                                  //turns off carrier
  delay(pipDelaymS);
  }
  
  setRfFrequencyDirect(_ShiftfreqregH, _ShiftfreqregM, _ShiftfreqregL); //set carrier frequency 
  endmS = millis() + leadinmS; 
  setTXDirect();                                                           //turn on carrier 
  while (millis() < endmS);                                                //leave leadin on

}

void SX127XLT::transmitFSKRTTY(uint8_t chartosend, uint8_t databits, uint8_t stopbits, uint8_t parity, uint16_t baudPerioduS, int8_t pin)
{
  //micros() will rollover at 4294967295 or 71mins 35secs
  //assume slowest baud rate is 45 (baud period of 22222us) then with 11 bits max to send if routine starts 
  //when micros() > (4294967295 - (22222 * 11) = 4294722855 = 0xFFFC4525 then it could overflow during send
  //Rather than deal with rolloever in the middle of a character lets wait till it overflows and then
  //start the character  
  

  #ifdef SX127XDEBUG1
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
  setRfFrequencyDirect(_freqregH, _freqregM, _freqregL); //set carrier frequency  (low)
  
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
    setRfFrequencyDirect(_ShiftfreqregH, _ShiftfreqregM, _ShiftfreqregL); //set carrier frequency for a 1 
    }
    else
    {
       if (pin >= 0)
       {
       digitalWrite(pin, LOW); 
       }     
      setRfFrequencyDirect(_freqregH, _freqregM, _freqregL);           //set carrier frequency for a 0
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
         setRfFrequencyDirect(_freqregH, _freqregM, _freqregL);           //set carrier frequency for a 0
         while (micros() < enduS);
		 break;
    
    case ParityOne:
         
         setRfFrequencyDirect(_ShiftfreqregH, _ShiftfreqregM, _ShiftfreqregL);   //set carrier frequency for a 1
         while (micros() < enduS);
		 break;

	case ParityOdd:
         if (bitRead(bitcount, 0))                                         //test odd bit count, i.e. when bit 0 = 1 
         {
         setRfFrequencyDirect(_ShiftfreqregH, _ShiftfreqregM, _ShiftfreqregL);           //set carrier frequency for a 1 
         }
		 else
         {
         setRfFrequencyDirect(_freqregH, _freqregM, _freqregL);           //set carrier frequency for a 0 
         }
         while (micros() < enduS);
		 break;

    case ParityEven:
         if (bitRead(bitcount, 0))                                         //test odd bit count, i.e. when bit 0 = 1 
         {
         setRfFrequencyDirect(_freqregH, _freqregM, _freqregL);           //set carrier frequency for a 0 
         }
		 else
         {
         setRfFrequencyDirect(_ShiftfreqregH, _ShiftfreqregM, _ShiftfreqregL);             //set carrier frequency for a 1 
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
  
  setRfFrequencyDirect(_ShiftfreqregH, _ShiftfreqregM, _ShiftfreqregL); //set carrier frequency
  
  while (micros() < enduS);
  
}


void SX127XLT::transmitFSKRTTY(uint8_t chartosend, uint16_t baudPerioduS, int8_t pin)
{
  //micros() will rollover at 4294967295 or 71mins 35secs
  //assume slowest baud rate is 45 (baud period of 22222us) then with 11 bits max to send if routine starts 
  //when micros() > (4294967295 - (22222 * 11) = 4294722855 = 0xFFFC4525 then it could overflow during send
  //Rather than deal with rolloever in the middle of a character lets wait till it overflows and then
  //start the character
  //This overloaded version of transmitFSKRTTY() uses 1 start bit, 7 data bits, no parity and 2 stop bits. 
  

  #ifdef SX127XDEBUG1
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
  setRfFrequencyDirect(_freqregH, _freqregM, _freqregL); //set carrier frequency  (low)
  
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
    setRfFrequencyDirect(_ShiftfreqregH, _ShiftfreqregM, _ShiftfreqregL); //set carrier frequency for a 1 
    }
    else
    {
       if (pin >= 0)
       {
       digitalWrite(pin, LOW); 
       }     
      setRfFrequencyDirect(_freqregH, _freqregM, _freqregL);           //set carrier frequency for a 0
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
  
  setRfFrequencyDirect(_ShiftfreqregH, _ShiftfreqregM, _ShiftfreqregL); //set carrier frequency
  
  while (micros() < enduS);
  
}



void SX127XLT::printRTTYregisters()
{
  
  #ifdef SX127XDEBUG1
  PRINT_CSTSTR("printRTTYregisters()");
  #endif
   
PRINT_CSTSTR("NoShift Registers ");
PRINT_HEX("%X",_freqregH);
PRINT_CSTSTR(" ");
PRINT_HEX("%X",_freqregM);
PRINT_CSTSTR(" ");
PRINTLN_HEX("%X",_freqregL);

PRINT_CSTSTR("Shifted Registers ");
PRINT_HEX("%X",_ShiftfreqregH);
PRINT_CSTSTR(" ");
PRINT_HEX("%X",_ShiftfreqregM);
PRINT_CSTSTR(" ");
PRINTLN_HEX("%X",_ShiftfreqregL);

}

void SX127XLT::endFSKRTTY()
{
  #ifdef SX127XDEBUG1
  PRINT_CSTSTR("endFSKRTTY()");
  #endif
  
  setMode(MODE_STDBY_RC);

}


void SX127XLT::doAFC()
{
//int32_t frequencyerror;

//frequencyerror = getFrequencyErrorHz();
savedOffset = savedOffset - getFrequencyErrorHz();
setRfFrequency(savedFrequency, savedOffset);     //adjust the operating frequency for AFC
}


int32_t SX127XLT::getOffset()
{
return savedOffset;
}


uint32_t SX127XLT::transmitReliable(uint8_t *txbuffer, uint8_t size, char txpackettype, char txdestination, char txsource, uint32_t txtimeout, int8_t txpower, uint8_t wait )
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("transmitAddressed()");
#endif

  uint16_t libraryCRC = 0xFFFF;                    //start value for CRC calc
  uint8_t index, ptr;
  uint8_t bufferdata;
  uint32_t endtimeoutmS;

  if (size == 0)
  {
    return false;
  }

  setMode(MODE_STDBY_RC);
  ptr = readRegister(REG_FIFOTXBASEADDR);         //retrieve the TXbase address pointer
  writeRegister(REG_FIFOADDRPTR, ptr);            //and save in FIFO access ptr

#ifdef USE_SPI_TRANSACTION                       //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);

  /**************************************************************************
	Added by C. Pham - Oct. 2020
  **************************************************************************/  

#if defined ARDUINO || defined USE_ARDUPI
  SPI.transfer(WREG_FIFO);  
  // we insert our header
  SPI.transfer(txdestination);                    //Destination node
  SPI.transfer(txpackettype);                     //Write the packet type
  SPI.transfer(txsource);                         //Source node
  SPI.transfer(_TXSeqNo);                         //Sequence number  
#else
	writeRegister(WREG_FIFO, txdestination);
	writeRegister(WREG_FIFO, txpackettype);
	writeRegister(WREG_FIFO, txsource);
	writeRegister(WREG_FIFO, _TXSeqNo);
#endif
  
  _TXPacketL = HEADER_SIZE + size;                //we have added 4 header bytes to size
  
  libraryCRC = addCRC(txdestination, libraryCRC);  
  libraryCRC = addCRC(txpackettype, libraryCRC);
  libraryCRC = addCRC(txsource, libraryCRC);  
  libraryCRC = addCRC(_TXSeqNo, libraryCRC);  

  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/  
  
  /* original code
   ***************
    
  SPI.transfer(txpackettype);                     //Write the packet type
  SPI.transfer(txdestination);                    //Destination node
  SPI.transfer(txsource);                         //Source node
  _TXPacketL = 3 + size;                          //we have added 3 header bytes to size

  libraryCRC = addCRC(txpackettype, libraryCRC);
  libraryCRC = addCRC(txdestination, libraryCRC);
  libraryCRC = addCRC(txsource, libraryCRC);
  
  */

  for (index = 0; index <= 127; index++)
  {
    bufferdata = txbuffer[index];
    libraryCRC = addCRC(bufferdata, libraryCRC);
#if defined ARDUINO || defined USE_ARDUPI    
    SPI.transfer(bufferdata);
#else
		writeRegister(WREG_FIFO, bufferdata);
#endif      
  }
  digitalWrite(_NSS, HIGH);

#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif

  writeRegister(REG_PAYLOADLENGTH, _TXPacketL);

  setTxParams(txpower, RADIO_RAMP_DEFAULT);                       //TX power and ramp time

  setDioIrqParams(IRQ_RADIO_ALL, IRQ_TX_DONE, 0, 0);              //set for IRQ on TX done
  setTx(0);                                                       //TX timeout is not handled in setTX()

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
  
  if (txtimeout == 0)
  {
#ifdef USE_POLLING
    index = readRegister(REG_IRQFLAGS);
    //poll the irq register for TXDone, bit 3
    while ((bitRead(index, 3) == 0))
      {
        index = readRegister(REG_IRQFLAGS);
      }
#else  
    while (!digitalRead(_TXDonePin));                  //Wait for pin to go high, TX finished
#endif    
  }
  else
  {
    endtimeoutmS = (millis() + txtimeout);
#ifdef USE_POLLING

#ifdef SX127XDEBUG1
  	PRINTLN_CSTSTR("TXDone using polling");
#endif 
    index = readRegister(REG_IRQFLAGS);
    //poll the irq register for TXDone, bit 3
    while ((bitRead(index, 3) == 0) && (millis() < endtimeoutmS))
      {
        index = readRegister(REG_IRQFLAGS);
      }
#else    
    while (!digitalRead(_TXDonePin) && (millis() < endtimeoutmS));
#endif    
  }

  setMode(MODE_STDBY_RC);                                            //ensure we leave function with TX off

#ifdef USE_POLLING
  if (bitRead(index, 3) == 0)
#else
  if (!digitalRead(_TXDonePin))
#endif  
  {
    _IRQmsb = IRQ_TX_TIMEOUT;
    return 0;
  }
  
	_AckStatus=0;

	if (txpackettype & PKT_FLAG_ACK_REQ) {
		uint8_t RXAckPacketL;
		const uint8_t RXBUFFER_SIZE=3;
		uint8_t RXBUFFER[RXBUFFER_SIZE];

#ifdef SX127XDEBUGACK
		PRINTLN_CSTSTR("transmitReliable is waiting for an ACK");
#endif
#ifdef INVERTIQ_ON_ACK
#ifdef SX127XDEBUGACK
		PRINTLN_CSTSTR("invert IQ for ack reception");
#endif
		invertIQ(true);
#endif
		delay(10);			
		//try to receive the ack
		RXAckPacketL=receiveAddressed(RXBUFFER, RXBUFFER_SIZE, 2000, WAIT_RX);

#ifdef INVERTIQ_ON_ACK
#ifdef SX127XDEBUGACK
		PRINTLN_CSTSTR("set back IQ to normal");
#endif
		invertIQ(false);
#endif
		
		if (RXAckPacketL) {
#ifdef SX127XDEBUGACK
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
#ifdef SX127XDEBUGACK
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
#ifdef SX127XDEBUGACK
				PRINTLN_CSTSTR("not for me!");
#endif			
			}
		}
		else {
#ifdef SX127XDEBUGACK
		PRINTLN_CSTSTR("received nothing");		 
#endif			
		}
	}
	
  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/

#ifdef SX127XDEBUG1
  PRINTLN;
  PRINT_CSTSTR("TXcrc,"); 
  PRINTLN_HEX("%X",libraryCRC);
#endif

  return ( ( (uint32_t) libraryCRC << 16) + (uint8_t) _TXPacketL ); 
}


uint16_t SX127XLT::addCRC(uint8_t data, uint16_t libraryCRC)
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


uint32_t SX127XLT::receiveReliable(uint8_t *rxbuffer, uint8_t size, char packettype, char destination, char source, uint32_t rxtimeout, uint8_t wait )
{
//set packettype, destnode, source to ) for no matching check

#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("receiveReliable()");
#endif

  uint16_t libraryCRC = 0xFFFF;                    //start value for CRC calc
  uint16_t index;
  uint32_t endtimeoutmS;
  uint8_t regdata;

  setMode(MODE_STDBY_RC);
  regdata = readRegister(REG_FIFORXBASEADDR);                               //retrieve the RXbase address pointer
  writeRegister(REG_FIFOADDRPTR, regdata);                                  //and save in FIFO access ptr

  setDioIrqParams(IRQ_RADIO_ALL, (IRQ_RX_DONE + IRQ_HEADER_VALID), 0, 0);  //set for IRQ on RX done
  setRx(0);                                                                //no actual RX timeout in this function
  
  if (!wait)
  {
    return 0;                                                              //not wait requested so no packet length to pass
  }

  /**************************************************************************
	Modified by C. Pham - Oct. 2020
  **************************************************************************/
  
  if (rxtimeout == 0)
  {
#ifdef USE_POLLING
    index = readRegister(REG_IRQFLAGS);

    //poll the irq register for ValidHeader, bit 4
    while ((bitRead(index, 4) == 0))
      {
        index = readRegister(REG_IRQFLAGS);
        // adding this small delay decreases the CPU load of the lora_gateway process to 4~5% instead of nearly 100%
        // suggested by rertini (https://github.com/CongducPham/LowCostLoRaGw/issues/211)
        // tests have shown no significant side effects
        delay(1);        
      }    

		//_RXTimestamp start when a valid header has been received
  	_RXTimestamp=millis();

    //poll the irq register for RXDone, bit 6
    while ((bitRead(index, 6) == 0))
      {
        index = readRegister(REG_IRQFLAGS);
      }
#else  
    while (!digitalRead(_RXDonePin));                  ///Wait for DIO0 to go high, no timeout, RX DONE
#endif    
  }
  else
  {
    endtimeoutmS = (millis() + rxtimeout);
#ifdef USE_POLLING

#ifdef SX127XDEBUG1
  	PRINTLN_CSTSTR("RXDone using polling");
#endif 
    index = readRegister(REG_IRQFLAGS);

    //poll the irq register for ValidHeader, bit 4
    while ((bitRead(index, 4) == 0) && (millis() < endtimeoutmS))
      {
        index = readRegister(REG_IRQFLAGS);
        delay(1);        
      }    

		if (bitRead(index, 4)!=0) {
#ifdef SX127XDEBUG1		
  		PRINTLN_CSTSTR("Valid header detected");
#endif
    	//_RXTimestamp start when a valid header has been received
    	_RXTimestamp=millis();

			//update endtimeoutmS to avoid timeout at the middle of a packet reception
			endtimeoutmS = (_RXTimestamp + rxtimeout);
    }
            
    //poll the irq register for RXDone, bit 6
    while ((bitRead(index, 6) == 0) && (millis() < endtimeoutmS))
      {
        index = readRegister(REG_IRQFLAGS);
      }
#else    
    while (!digitalRead(_RXDonePin) && (millis() < endtimeoutmS));
#endif    
  }

  //_RXDoneTimestamp start when the packet been fully received  
  _RXDoneTimestamp=millis();
      
  setMode(MODE_STDBY_RC);                                            //ensure to stop further packet reception

#ifdef USE_POLLING
  if (bitRead(index, 6) == 0)
#else
  if (!digitalRead(_RXDonePin))                                             //check if DIO still low, is so must be RX timeout
#endif  
  {
    _IRQmsb = IRQ_RX_TIMEOUT;
#ifdef SX127XDEBUG1    
    PRINTLN_CSTSTR("RX timeout");
#endif     
    return 0;
  }

  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/

  if ( readIrqStatus() != (IRQ_RX_DONE + IRQ_HEADER_VALID) )
  {
    return 0;                                                                //no RX done and header valid only, could be CRC error
  }

  _RXPacketL = readRegister(REG_RXNBBYTES);
  
#ifdef USE_SPI_TRANSACTION   //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);                    //start the burst read

  /**************************************************************************
	Added by C. Pham - Oct. 2020
  **************************************************************************/  
  
#if defined ARDUINO || defined USE_ARDUPI
  SPI.transfer(REG_FIFO);  
  // we read our header
  _RXDestination = SPI.transfer(0);
  _RXPacketType = SPI.transfer(0);
  _RXSource = SPI.transfer(0);
  _RXSeqNo = SPI.transfer(0);
#else
  // we read our header
  _RXDestination = readRegister(REG_FIFO);
  _RXPacketType = readRegister(REG_FIFO);
  _RXSource = readRegister(REG_FIFO);
  _RXSeqNo = readRegister(REG_FIFO);
#endif  
  //the header is not passed to the user
  _RXPacketL=_RXPacketL-HEADER_SIZE;
  
  if (_RXPacketL > size)                      //check passed buffer is big enough for packet
  {
    _RXPacketL = size;                        //truncate packet if not enough space in passed buffer
  }  

  libraryCRC = addCRC(_RXDestination, libraryCRC);  
  libraryCRC = addCRC(_RXPacketType, libraryCRC);
  libraryCRC = addCRC(_RXSource, libraryCRC);  
  libraryCRC = addCRC(_RXSeqNo, libraryCRC);
  
  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/  
  
  /* original code
   ***************
    
  _RXPacketType = SPI.transfer(0);
  _RXDestination = SPI.transfer(0);
  _RXSource = SPI.transfer(0);
  
  libraryCRC = addCRC(_RXPacketType, libraryCRC);
  libraryCRC = addCRC(_RXDestination, libraryCRC);
  libraryCRC = addCRC(_RXSource, libraryCRC);
  
  */
  
  for (index = 0; index < _RXPacketL; index++)
  {
#if defined ARDUINO || defined USE_ARDUPI  
    regdata = SPI.transfer(0);
#else
    regdata = readRegister(REG_FIFO);
#endif    
    libraryCRC = addCRC(regdata, libraryCRC);
    rxbuffer[index] = regdata;
  }
  digitalWrite(_NSS, HIGH);

#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif

	if (packettype > 0)
     {
     if (_RXPacketType != packettype)
        {
         return 0x1000;                       //set bit 16 to indicate not matching packet type
        }
     }
  
  if (destination > 0)
     {
     if (_RXDestination != destination)
        {
         return 0x20000;                      //set bit 17 to indicate not matching destintion node
        }
     }
 
  if (source > 0)
     {
     if (_RXSource != source)
        {
         return 0x40000;                      //set bit 18 to indicate not matching source node  
        }
     }  

  /**************************************************************************
	Added by C. Pham - Oct. 2020
  **************************************************************************/ 
  
  //sender request an ACK
	if ( (_RXPacketType & PKT_FLAG_ACK_REQ) && (_RXDestination==_DevAddr) ) {
		uint8_t TXAckPacketL;
		const uint8_t TXBUFFER_SIZE=3;
		uint8_t TXBUFFER[TXBUFFER_SIZE];
		
#ifdef SX127XDEBUGACK
  	PRINT_CSTSTR("ACK requested by ");
  	PRINTLN_VALUE("%d", _RXSource);
  	PRINTLN_CSTSTR("ACK transmission turnaround safe wait...");
#endif			
#ifdef INVERTIQ_ON_ACK
#ifdef SX127XDEBUGACK
		PRINTLN_CSTSTR("invert IQ for ack transmission");
#endif
		invertIQ(true);
#endif			
		delay(100);
		TXBUFFER[0]=2; //length
		TXBUFFER[1]=0; //RFU
		TXBUFFER[2]=readRegister(REG_PKTSNRVALUE); //SNR of received packet
		
		uint8_t saveTXSeqNo=_TXSeqNo;
		_TXSeqNo=_RXSeqNo;
		// use -1 as txpower to use default setting
		TXAckPacketL=transmitAddressed(TXBUFFER, 3, PKT_TYPE_ACK, _RXSource, _DevAddr, 10000, -1, WAIT_TX);	

#ifdef SX127XDEBUGACK
		if (TXAckPacketL)
  		PRINTLN_CSTSTR("ACK sent");
  	else	
  		PRINTLN_CSTSTR("error when sending ACK"); 
#endif
#ifdef INVERTIQ_ON_ACK
#ifdef SX127XDEBUGACK
		PRINTLN_CSTSTR("set back IQ to normal");
#endif
		invertIQ(false);
#endif	
		
		_TXSeqNo=saveTXSeqNo;
	}

  /**************************************************************************
	End by C. Pham - Oct. 2020
  **************************************************************************/ 	

#ifdef SX127XDEBUG1
  PRINTLN;
  PRINT_CSTSTR("RXcrc,"); 
  PRINTLN_HEX("%X",libraryCRC);
#endif

  return ( ( (uint32_t) libraryCRC << 16) + (uint8_t) _RXPacketL ); 
}

uint32_t SX127XLT::receiveFT(uint8_t *rxbuffer, uint8_t size, char packettype, char destination, char source, uint32_t rxtimeout, uint8_t wait )
{
//set packettype, destnode, source to ) for no matching check

#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("receiveReliable()");
#endif

  uint16_t libraryCRC = 0xFFFF;                    //start value for CRC calc
  uint16_t index;
  uint32_t endtimeoutmS;
  uint8_t regdata;

  setMode(MODE_STDBY_RC);
  regdata = readRegister(REG_FIFORXBASEADDR);                               //retrieve the RXbase address pointer
  writeRegister(REG_FIFOADDRPTR, regdata);                                  //and save in FIFO access ptr

  setDioIrqParams(IRQ_RADIO_ALL, (IRQ_RX_DONE + IRQ_HEADER_VALID), 0, 0);  //set for IRQ on RX done
  setRx(0);                                                                //no actual RX timeout in this function

  if (!wait)
  {
    return 0;                                                              //not wait requested so no packet length to pass
  }

  if (rxtimeout == 0)
  {
    while (!digitalRead(_RXDonePin));                                      //Wait for DIO0 to go high, no timeout, RX DONE
  }
  else
  {
    endtimeoutmS = millis() + rxtimeout;
    while (!digitalRead(_RXDonePin) && (millis() < endtimeoutmS));
  }

  setMode(MODE_STDBY_RC);                                                   //ensure to stop further packet reception

  if (!digitalRead(_RXDonePin))                                             //check if DIO still low, is so must be RX timeout
  {
    _IRQmsb = IRQ_RX_TIMEOUT;
    return 0;
  }

  if ( readIrqStatus() != (IRQ_RX_DONE + IRQ_HEADER_VALID) )
  {
    return 0;                                                                //no RX done and header valid only, could be CRC error
  }

  _RXPacketL = readRegister(REG_RXNBBYTES);
  

  //if (_RXPacketL > size)                      //check passed buffer is big enough for packet
  //{
    //_RXPacketL = size;                        //truncate packet if not enough space in passed buffer
  //}

#ifdef USE_SPI_TRANSACTION   //to use SPI_TRANSACTION enable define at beginning of CPP file 
  SPI.beginTransaction(SPISettings(LTspeedMaximum, LTdataOrder, LTdataMode));
#endif

  digitalWrite(_NSS, LOW);                    //start the burst read
  
#if defined ARDUINO || defined USE_ARDUPI  
  SPI.transfer(REG_FIFO);
  
  _RXPacketType = SPI.transfer(0);
  _RXDestination = SPI.transfer(0); 
  _RXSource = SPI.transfer(0);
#else
  _RXPacketType = readRegister(REG_FIFO);
  _RXDestination = readRegister(REG_FIFO);
  _RXSource = readRegister(REG_FIFO);
#endif  
  
  libraryCRC = addCRC(_RXPacketType, libraryCRC);
  libraryCRC = addCRC(_RXDestination, libraryCRC);
  libraryCRC = addCRC(_RXSource, libraryCRC);
  
  //extractsize = _RXPacketL - 3;
  
  for (index = 0; index < size; index++)
  {
#if defined ARDUINO || defined USE_ARDUPI  
    regdata = SPI.transfer(0);
#else
    regdata = readRegister(REG_FIFO);
#endif    
    libraryCRC = addCRC(regdata, libraryCRC);
    rxbuffer[index] = regdata;
  }
  digitalWrite(_NSS, HIGH);

#ifdef USE_SPI_TRANSACTION
  SPI.endTransaction();
#endif


if (packettype > 0)
     {
     if (_RXPacketType != packettype)
        {
         return 0x1000;                       //set bit 16 to indicate not matching packet type
        }
     }
  
  if (destination > 0)
     {
     if (_RXDestination != destination)
        {
         return 0x20000;                      //set bit 17 to indicate not matching destintion node
        }
     }
 
  if (source > 0)
     {
     if (_RXSource != source)
        {
         return 0x40000;                      //set bit 18 to indicate not matching source node  
        }
     }  


#ifdef SX127XDEBUG1
  PRINTLN;
  PRINT_CSTSTR("RXcrc,"); 
  PRINTLN_HEX("%X",libraryCRC);
#endif

  return ( ( (uint32_t) libraryCRC << 16) + (uint8_t) _RXPacketL ); 
}

/**************************************************************************
  Added by C. Pham - Oct. 2020
**************************************************************************/

uint32_t SX127XLT::returnBandwidth()
{
  uint8_t regdata;

  if (_Device == DEVICE_SX1272)
  {
    regdata = (readRegister(REG_MODEMCONFIG1) & READ_BW_AND_2);
  }
  else
  {
    regdata = (readRegister(REG_MODEMCONFIG1) & READ_BW_AND_X);
  }

  return (returnBandwidth(regdata));
}

/*
 Function: Configures the module to perform CAD.
 Returns: Integer that determines if the number of requested CAD have been successfull
   state = 1  --> There has been an error while executing the command
   state = 0  --> No activity detected
   state < 0  --> Activity detected, return mean RSSI computed duuring CAD
*/
int8_t SX127XLT::doCAD(uint8_t counter)
{
#ifdef SX127XDEBUG1
	PRINTLN_CSTSTR("doCAD()");
#endif

  uint8_t state = 1;
	byte value = 0x00;
	unsigned long startCAD, endCAD, startDoCad, endDoCad;
	//unsigned long previous;
	unsigned long exitTime;
	uint16_t wait = 100;
  bool activityDetected=false;
	uint8_t retryCAD = 3;
	uint8_t save_counter;
	byte st0;
	int rssi_count=0;
	int rssi_mean=0;
	bool hasRSSI=false;
	unsigned long startRSSI=0;

	// Symbol rate : time for one symbol (usecs)
  //double ts = 1000000.0 / (double)returnBandwidth() / ( 1 << getLoRaSF());
	double ts = 1000000.0*((double)( 1 << getLoRaSF()) / (double)returnBandwidth());
	
  st0 = readRegister(REG_OPMODE);	// Save the previous status

	save_counter = counter;

	startDoCad=millis();

	setMode(MODE_STDBY_RC);

	do {

		hasRSSI=false;

		clearIrqStatus(IRQ_RADIO_ALL); // Initializing flags

		// wait to CadDone flag
		// previous = millis();
		startCAD = millis();
		exitTime = millis()+(unsigned long)wait;

    writeRegister(REG_OPMODE, 0x87);	// LORA mode - Cad

		startRSSI=micros();

		value = readRegister(REG_IRQFLAGS);
		// Wait until CAD ends (CAD Done flag) or the timeout expires
		while ((bitRead(value, 2) == 0) && (millis() < exitTime))
		{
				// only one reading per CAD
				if (micros()-startRSSI > ts+240 && !hasRSSI) {
						uint8_t regdata=readRegister(REG_RSSIVALUE);
            _RSSI = regdata - (OFFSET_RSSI + ((_Device == DEVICE_SX1272)?0:18));
#ifdef SX127XDEBUGCAD
						PRINT_CSTSTR("REG_RSSIVALUE: ");
						PRINTLN_VALUE("%d", regdata);
						PRINT_CSTSTR("_RSSI: ");
						PRINTLN_VALUE("%d", _RSSI);						
#endif              
            rssi_mean += _RSSI;
						rssi_count++;
						hasRSSI=true;
				}

				value = readRegister(REG_IRQFLAGS);
		}

		state = 1;

		endCAD = millis();

		if( bitRead(value, 2) == 1 )
		{
				state = 0;	// CAD successfully performed
#ifdef SX127XDEBUG1					
				PRINT_CSTSTR("CAD duration ");
				PRINTLN_VALUE("%d",endCAD-startCAD);
				PRINTLN_CSTSTR("CAD successfully performed");
#endif					

				value = readRegister(REG_IRQFLAGS);

				// look for the CAD detected bit
				if( bitRead(value, 0) == 1 )
				{
						// we detected activity
            activityDetected=true;
#ifdef SX127XDEBUG1							
						PRINT_CSTSTR("CAD exits after ");
						PRINTLN_VALUE("%d",save_counter-counter);
#endif							
				}

				counter--;
		}
		else
		{
#ifdef SX127XDEBUG1						
				PRINT_CSTSTR("CAD duration ");
				PRINTLN_VALUE("%d",endCAD-startCAD);
        PRINTLN_CSTSTR("Timeout has expired");
#endif					
				retryCAD--;

        // to many errors, so exit by indicating that channel is not free (state=1)
				if (!retryCAD)
            counter=0;
		}

  } while (counter && !activityDetected);

	rssi_mean = rssi_mean / rssi_count;
  _RSSI = rssi_mean;

  writeRegister(REG_OPMODE, st0);

	endDoCad=millis();

  clearIrqStatus(IRQ_RADIO_ALL); 		// Initializing flags

#ifdef SX127XDEBUG1		
	PRINT_CSTSTR("doCAD duration ");
	PRINTLN_VALUE("%d",endDoCad-startDoCad);
#endif

	//TODO C. Pham. returned value is actually positive?
  if (activityDetected)
      return _RSSI;

	return state;
}

#ifdef SX127XDEBUG3

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

uint16_t SX127XLT::getToA(uint8_t pl) {
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("getToA()");
#endif
  uint8_t DE = 0;
  uint8_t sf;
  uint8_t cr;
  uint32_t airTime = 0;
  uint32_t bw=0;

  bw=returnBandwidth();
  sf=getLoRaSF();
  cr=getLoRaCodingRate()-4;

#ifdef SX127XDEBUG3
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
  double tPreamble=(getPreamble()+4.25)*ts;

#ifdef SX127XDEBUG3
  PRINT_CSTSTR("ts is ");
  printDouble(ts,6);
  PRINTLN;
  PRINT_CSTSTR("tPreamble is ");
  printDouble(tPreamble,6);
  PRINTLN;
#endif

  // for low data rate optimization
  if ((ts*1000.0)>16)
      DE=1;

  // Symbol length of payload and time
  double tmp = (8*pl - 4*sf + 28 + 16*_UseCRC - 20*getHeaderMode()) /
          (double)(4*(sf-2*DE) );

  tmp = ceil(tmp)*(cr + 4);

  double nPayload = 8 + ( ( tmp > 0 ) ? tmp : 0 );

#ifdef SX127XDEBUG3
  PRINT_CSTSTR("nPayload is ");
  PRINTLN_VALUE("%d",nPayload);
#endif

  double tPayload = nPayload * ts;
  // Time on air
  double tOnAir = tPreamble + tPayload;
  // in us secs
  airTime = floor( tOnAir * 1e6 + 0.999 );

  //////

#ifdef SX127XDEBUG3
  PRINT_CSTSTR("airTime is ");
  PRINTLN_VALUE("%d",airTime);
#endif
  // return in ms
  return (ceil(airTime/1000)+1);
}

// need to set cad_number to a value > 0
// we advise using cad_number=3 for a SIFS and DIFS=3*SIFS
#define DEFAULT_CAD_NUMBER    3

void SX127XLT::CarrierSense(uint8_t cs, bool extendedIFS, bool onlyOnce) {
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("CarrierSense()");
#endif
  
  if (cs==1)
    CarrierSense1(DEFAULT_CAD_NUMBER, extendedIFS, onlyOnce);

  if (cs==2)
    CarrierSense2(DEFAULT_CAD_NUMBER, extendedIFS);

  if (cs==3)
    CarrierSense3(DEFAULT_CAD_NUMBER);
}

uint16_t SX127XLT::CollisionAvoidance(uint8_t pl, uint8_t ca) {
#ifdef SX127XDEBUG1
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
void SX127XLT::CarrierSense1(uint8_t cad_number, bool extendedIFS, bool onlyOnce) {

  int e;
  uint8_t retries=3;
  uint8_t DIFSretries=8;
  uint8_t cad_value;
  unsigned long _startDoCad, _endDoCad;

  // symbol time in ms
  double ts = 1000.0 / (returnBandwidth() / ( 1 << getLoRaSF()));

  //approximate duration of a CAD, avoid having 0ms
  cad_value=2*ts+1;

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

void SX127XLT::CarrierSense2(uint8_t cad_number, bool extendedIFS) {

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

  //approximate duration of a CAD, avoid having 0ms
  cad_value=2*ts+1;

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

void SX127XLT::CarrierSense3(uint8_t cad_number) {

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

uint8_t SX127XLT::transmitRTSAddressed(uint8_t pl) {

	uint8_t TXRTSPacketL;
	const uint8_t TXBUFFER_SIZE=1;
	uint8_t TXBUFFER[TXBUFFER_SIZE];
	
#ifdef SX127XDEBUG1
	PRINTLN_CSTSTR("transmitRTSAddressed()");
#endif
			
	TXBUFFER[0]=pl; //length of next data packet

#ifdef SX127XDEBUGRTS
	PRINT_CSTSTR("--> RTS: FOR ");
	PRINT_VALUE("%d", pl);
  PRINTLN_CSTSTR(" Bytes");		
#endif
	
	// use -1 as txpower to use default setting
	TXRTSPacketL=transmitAddressed(TXBUFFER, 1, PKT_TYPE_RTS, 0, _DevAddr, 10000, -1, WAIT_TX);	

#ifdef SX127XDEBUGRTS
	if (TXRTSPacketL)
		PRINTLN_CSTSTR("--> RTS: SENT");
	else	
		PRINTLN_CSTSTR("--> RTS: ERROR"); 
#endif

	return(TXRTSPacketL);
}

uint16_t SX127XLT::CollisionAvoidance0(uint8_t pl, uint8_t cad_number) {

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

#ifdef SX127XDEBUGRTS
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
#ifdef SX127XDEBUGRTS
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

#ifdef SX127XDEBUGRTS
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

#ifdef SX127XDEBUGRTS
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
#ifdef SX127XDEBUGRTS
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
#ifdef SX127XDEBUGRTS
				PRINT_CSTSTR("--> CA: ValidHeader FOR DATA DEFER BY MAX_TOA ");
				PRINTLN_VALUE("%d", getToA(255));			 
#endif  
				return(getToA(255));	
			}
			else {
#ifdef SX127XDEBUGRTS
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
#ifdef SX127XDEBUGRTS
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
#ifdef SX127XDEBUGRTS
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
#ifdef SX127XDEBUGRTS
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
		
#ifdef SX127XDEBUGRTS
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
#ifdef SX127XDEBUGRTS
			PRINTLN_CSTSTR("--> CA: EXIT, ALLOW DATA TRANSMIT");
#endif		
			return(0);
		}	
		// if forceListen==false then this is the first time for nodes starting directly at phase 2
		else {	
#ifdef SX127XDEBUGRTS
			PRINTLN_CSTSTR("--> CA: SEND RTS");
#endif			
		
			// phase 2 where we send an RTS
			transmitRTSAddressed(pl);
		
			// go for another listening period
			forceListen=true;
		}
	}
}

uint8_t	SX127XLT::invertIQ(bool invert)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("invertIQ()");
#endif

  if (invert)
  {
  	writeRegister(REG_INVERTIQ, 0x66);
  	writeRegister(REG_INVERTIQ2, 0x19);
  }
  else
  {
  	writeRegister(REG_INVERTIQ, 0x27);
  	writeRegister(REG_INVERTIQ2, 0x1D);  
  }
  
  return(0);
  	
	/*
  byte st0;
  int8_t state = 2;
  byte config1;
  byte config2;

  st0 = readRegister(REG_OPMODE);		// Save the previous status

  config1=readRegister(REG_INVERTIQ);

  setMode(MODE_STDBY_RC);

  if (invert) {
    if (dir==INVERT_IQ_RX) {
      // invert on RX
      // clear bit 0 related to TX
      config1=config1 & 0xFE;
      // set bit 6 related to RX
      config1=config1 | 0x40;
      writeRegister(REG_INVERTIQ, config1);
    }
    else {
      // invert on TX
      // clear bit 6 related to RX
      config1=config1 & 0xBF;
      // set bit 0 related to TX
      config1=config1 | 0x01;      
      writeRegister(REG_INVERTIQ, config1);
    }

    writeRegister(REG_INVERTIQ2, 0x19);
  }
  else {
    writeRegister(REG_INVERTIQ, config1 & 0B10111110);
    writeRegister(REG_INVERTIQ2, 0x1D);
  }

  config1=readRegister(REG_INVERTIQ);
  config2=readRegister(REG_INVERTIQ2);

  //check I/Q setting

  if (invert) {
    if (dir==INVERT_IQ_RX) {
      // invert on RX
      // check bit 6 related to RX
      if ( (config1 & 0x40 == 0x40) && (config2==0x19) )
        state=0;
      else
        state=1;
    }
    else {
      // invert on TX
      // check bit 0 related to TX
      if ( (config1 & 0x01 == 0x01) && (config2==0x19) )
        state=0;
      else
        state=1;
    }
  }
  else {
    if ( (config1 & 0x41 == 0x00) && (config2==0x1D) )
      state=0;
    else
      state=1;
  }

  if (state==0) {
#ifdef SX127XDEBUG1
    PRINTLN_CSTSTR("## I/Q mode has been successfully set ##");
#endif
  }
  else {
#ifdef SX127XDEBUG1
    PRINTLN_CSTSTR("** There has been an error while configuring I/Q mode **");
#endif
  }

  writeRegister(REG_OPMODE,st0);	// Getting back to previous status
  return state;
  */
}

void SX127XLT::setTXSeqNo(uint8_t seqno)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("SetTXSeqNo()");
#endif
  
  _TXSeqNo=seqno;
}

uint8_t SX127XLT::readTXSeqNo()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("readTXSeqNo()");
#endif
  
  return(_TXSeqNo);
}

uint8_t SX127XLT::readRXSeqNo()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("readRXSeqNo()");
#endif
  
  return(_RXSeqNo);
}

void SX127XLT::setPA_BOOST(bool pa_boost)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("setPA_BOOST()");
#endif

  _PA_BOOST=pa_boost;
} 

uint32_t SX127XLT::readRXTimestamp()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("readRXTimestamp()");
#endif
  
  return(_RXTimestamp);
}

uint32_t SX127XLT::readRXDoneTimestamp()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("readRXDoneTimestamp()");
#endif
  
  return(_RXDoneTimestamp);
}

void SX127XLT::setDevAddr(uint8_t addr)
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("setDevAddr()");
#endif
	
	_DevAddr=addr;
}

uint8_t SX127XLT::readDevAddr()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("readDevAddr()");
#endif

  return(_DevAddr);
}

uint8_t SX127XLT::readAckStatus()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("readAckStatus()");
#endif

  return(_AckStatus);
}

int8_t SX127XLT::readPacketSNRinACK()
{
#ifdef SX127XDEBUG1
  PRINTLN_CSTSTR("readPacketSNRinACK()");
#endif

  return(_PacketSNRinACK);
}

void SX127XLT::setLowPowerFctPtr(void (*pf)(unsigned long))
{
#ifdef SX127XDEBUG
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
