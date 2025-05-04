/*
  Copyright 2019 - Stuart Robinson
  Licensed under a MIT license displayed at the bottom of this document.
  Original published 17/12/19
*/

#ifndef SX127XLT_h
#define SX127XLT_h

/**************************************************************************
  Added by C. Pham - Oct. 2020
**************************************************************************/
#ifdef ARDUINO
  #include <Arduino.h>
#else
  #include <stdint.h>
	
	#ifdef USE_ARDUPI
  #include "arduPi.h"	
	#else
  #include "arduinoPi.h"
	#endif
	
  #ifndef inttypes_h
    #include <inttypes.h>
  #endif
#endif

/**************************************************************************
  End by C. Pham - Oct. 2020
**************************************************************************/

#include <SX127XLT_Definitions.h>

/**************************************************************************

  ToDO

  Add ppmoffset to frequency error check program Check this in program 12 LT.writeRegister(RegPpmCorrection,ppmoffset);
  Check sensitivity\current for writeRegister(RegLna, 0x3B );.//at HF 150% LNA current.
  Add packet SF6 support and implicit mode support and examples
  returnBandwidth(byte BWregvalue) does not directly take account of SX1272
  Check void SX127XLT::fillSXBuffer and use of ptr
  Add receive callback

**************************************************************************/

class SX127XLT
{

  public:

    SX127XLT();

    bool begin(int8_t pinNSS, int8_t pinNRESET, int8_t pinDIO0, int8_t pinDIO1, int8_t pinDIO2, uint8_t device);
    bool begin(int8_t pinNSS, int8_t pinNRESET, int8_t pinDIO0, uint8_t device);
    void resetDevice();
    void setMode(uint8_t modeconfig);         //same function as setStandby()
    void setSleep(uint8_t sleepconfig);
    bool checkDevice();
    void wake();
    void calibrateImage(uint8_t null);
    uint16_t CRCCCITT(uint8_t *buffer, uint8_t size, uint16_t startvalue);
    uint16_t CRCCCITTSX(uint8_t startadd, uint8_t endadd, uint16_t startvalue);

    void setDevice(uint8_t type);
    void printDevice();
    uint8_t getOperatingMode();
    bool isReceiveDone();                      //reads first DIO pin defined
    bool isTransmitDone();                     //reads first DIO pin defined

    void writeRegister( uint8_t address, uint8_t value );
    uint8_t readRegister( uint8_t address );
    void printRegisters(uint16_t start, uint16_t end);
    void printOperatingMode();
    void printOperatingSettings();

    void setTxParams(int8_t txPower, uint8_t rampTime);
    void setPacketParams(uint16_t packetParam1, uint8_t  packetParam2, uint8_t packetParam3, uint8_t packetParam4, uint8_t packetParam5);
    void setModulationParams(uint8_t modParam1, uint8_t modParam2, uint8_t  modParam3, uint8_t  modParam4);
    void setRfFrequency(uint64_t freq64, int32_t offset);
    uint32_t getFreqInt();
    int32_t getFrequencyErrorRegValue();
    int32_t getFrequencyErrorHz();

    void setTx(uint32_t timeout);
    void setRx(uint32_t timeout);
    bool readTXIRQ();
    bool readRXIRQ();


    void setLowPowerReceive();
    void setHighSensitivity();
    void setRXGain(uint8_t config);

    uint8_t getAGC();
    uint8_t getLNAgain();
    uint8_t getCRCMode();
    uint8_t getHeaderMode();
    uint8_t getLNAboostHF();
    uint8_t getLNAboostLF();
    uint8_t getOpmode();
    uint8_t getPacketMode();           //LoRa or FSK

    uint8_t readRXPacketL();
    uint8_t readTXPacketL();
    /**************************************************************************
      Modified by C. Pham - Oct. 2020
    **************************************************************************/    
    int8_t readPacketRSSI();
    int8_t readPacketSNR();
   
    //uint8_t readPacketRSSI();
    //uint8_t readPacketSNR();
    /**************************************************************************
      End by C. Pham - Oct. 2020
    **************************************************************************/      
    bool readPacketCRCError();
    bool readPacketHeaderValid();
    uint8_t packetOK();
    uint8_t readRXPacketType();
    uint8_t readRXDestination();
    uint8_t readRXSource();

    void setBufferBaseAddress(uint8_t txBaseAddress, uint8_t rxBaseAddress);
    void setPacketType(uint8_t PacketType);

    void clearIrqStatus( uint16_t irqMask );
    uint16_t readIrqStatus();
    void setDioIrqParams(uint16_t irqMask, uint16_t dio0Mask, uint16_t dio1Mask, uint16_t dio2Mask );
    void printIrqStatus();


    void printASCIIPacket(uint8_t *buff, uint8_t tsize);
    void printHEXPacket(uint8_t *buff, uint8_t tsize);
    void printASCIIorHEX(uint8_t temp);
    void printHEXByte(uint8_t temp);
    void printHEXByte0x(uint8_t temp);

    bool isRXdone();
    bool isTXdone();
    bool isRXdoneIRQ();
    bool isTXdoneIRQ();
    void setTXDonePin(uint8_t pin);
    void setRXDonePin(uint8_t pin);

    //*******************************************************************************
    //Packet Read and Write Routines
    //*******************************************************************************

    uint8_t receive(uint8_t *rxbuffer, uint8_t size, uint32_t rxtimeout, uint8_t wait);
    uint8_t receiveAddressed(uint8_t *rxbuffer, uint8_t size, uint32_t rxtimeout, uint8_t wait);
    uint8_t readPacket(uint8_t *rxbuffer, uint8_t size);                    //needed for when receive used with NO_WAIT
    uint8_t readPacketAddressed(uint8_t *rxbuffer, uint8_t size);           //needed for when receive used with NO_WAIT

    uint8_t transmit(uint8_t *txbuffer, uint8_t size, uint32_t txtimeout, int8_t txPower, uint8_t wait);
    uint8_t transmitAddressed(uint8_t *txbuffer, uint8_t size, char txpackettype, char txdestination, char txsource, uint32_t txtimeout, int8_t txpower, uint8_t wait);

    //*******************************************************************************
    //LoRa specific routines
    //*******************************************************************************

    void setupLoRa(uint32_t Frequency, int32_t Offset, uint8_t modParam1, uint8_t modParam2, uint8_t  modParam3, uint8_t modParam4);

    uint8_t getLoRaSF();
    uint8_t getLoRaCodingRate();
    uint8_t getOptimisation();
    uint8_t getSyncWord();
    uint8_t getInvertIQ();
    uint8_t getVersion();
    uint16_t getPreamble();

    uint32_t returnBandwidth(uint8_t BWregvalue);                            //returns in hz the current set bandwidth
    uint8_t returnOptimisation(uint8_t SpreadingFactor, uint8_t Bandwidth);  //this returns the required optimisation setting
    float calcSymbolTime(float Bandwidth, uint8_t SpreadingFactor);
    void printModemSettings();
    void setSyncWord(uint8_t syncword);
    void setTXDirect();
    void setupDirect(uint32_t frequency, int32_t offset);
    void toneFM(uint16_t frequency, uint32_t length, uint32_t deviation, float adjust, int8_t txpower);
    int8_t getDeviceTemperature();
    void fskCarrierOn(int8_t txpower);
    void fskCarrierOff();
    void setRfFrequencyDirect(uint8_t high, uint8_t mid, uint8_t low);
    void getRfFrequencyRegisters(uint8_t *buff);
    void startFSKRTTY(uint32_t freqshift, uint8_t pips, uint16_t pipDelaymS, uint16_t pipPeriodmS, uint16_t leadinmS);
    void transmitFSKRTTY(uint8_t chartosend, uint8_t databits, uint8_t stopbits, uint8_t parity, uint16_t baudPerioduS, int8_t pin);
    void transmitFSKRTTY(uint8_t chartosend, uint16_t baudPerioduS, int8_t pin);
    void printRTTYregisters();
    void endFSKRTTY();
    void doAFC();
    int32_t getOffset();
    uint32_t transmitReliable(uint8_t *txbuffer, uint8_t size, char txpackettype, char txdestination, char txsource, uint32_t txtimeout, int8_t txpower, uint8_t wait );
    uint16_t addCRC(uint8_t data, uint16_t libraryCRC);
    uint32_t receiveReliable(uint8_t *rxbuffer, uint8_t size, char packettype, char destination, char source, uint32_t rxtimeout, uint8_t wait );
    uint32_t receiveFT(uint8_t *rxbuffer, uint8_t size, char packettype, char destination, char source, uint32_t rxtimeout, uint8_t wait );

    /**************************************************************************
      Added by C. Pham - Oct. 2020
    **************************************************************************/
    uint32_t returnBandwidth();
    int8_t doCAD(uint8_t counter);
    uint16_t getToA(uint8_t pl);
    void CarrierSense(uint8_t cs=1, bool extendedIFS=false, bool onlyOnce=false);
    uint16_t CollisionAvoidance(uint8_t pl, uint8_t ca=1);
    void CarrierSense1(uint8_t cad_number, bool extendedIFS=false, bool onlyOnce=false);
    void CarrierSense2(uint8_t cad_number, bool extendedIFS=false);
    void CarrierSense3(uint8_t cad_number);
    uint8_t transmitRTSAddressed(uint8_t pl);
    uint16_t CollisionAvoidance0(uint8_t pl, uint8_t cad_number);
    uint8_t receiveRTSAddressed(uint8_t *rxbuffer, uint8_t size, uint32_t rxtimeout, uint8_t wait);    
    uint8_t invertIQ(bool invert);
    void setTXSeqNo(uint8_t seqno);
    uint8_t readTXSeqNo();
    uint8_t readRXSeqNo();
    void setPA_BOOST(bool pa_boost);
    uint32_t readRXTimestamp();
    uint32_t readRXDoneTimestamp();
    void setDevAddr(uint8_t addr);
    uint8_t readDevAddr();
    uint8_t readAckStatus();
    int8_t readPacketSNRinACK();
    void setLowPowerFctPtr(void (*pf)(unsigned long));    
    /**************************************************************************
      End by C. Pham - Oct. 2020
    **************************************************************************/

    //*******************************************************************************
    //Read Write SX12xxx Buffer commands, this is the buffer internal to the SX12xxxx
    //*******************************************************************************

    uint8_t receiveSXBuffer(uint8_t startaddr, uint32_t rxtimeout, uint8_t wait);
    uint8_t transmitSXBuffer(uint8_t startaddr, uint8_t length, uint32_t txtimeout, int8_t txpower, uint8_t wait);

    void printSXBufferHEX(uint8_t start, uint8_t end);
    void printSXBufferASCII(uint8_t start, uint8_t end);
    void fillSXBuffer(uint8_t startaddress, uint8_t size, uint8_t character);
    uint8_t getByteSXBuffer(uint8_t addr);
    void writeByteSXBuffer(uint8_t addr, uint8_t regdata);

    void startWriteSXBuffer(uint8_t ptr);
    uint8_t endWriteSXBuffer();
    void startReadSXBuffer(uint8_t ptr);
    uint8_t endReadSXBuffer();

    void writeUint8(uint8_t x);
    uint8_t readUint8();
 
    uint8_t readBytes(uint8_t *rxbuffer,   uint8_t size);

    void writeInt8(int8_t x);
    int8_t readInt8();

    void writeChar(char x);
    char readChar();

    void writeUint16(uint16_t x);
    uint16_t readUint16();

    void writeInt16(int16_t x);
    int16_t readInt16();

    void writeUint32(uint32_t x);
    uint32_t readUint32();

    void writeInt32(int32_t x);
    int32_t readInt32();

    void writeFloat(float x);
    float readFloat();

    void writeBuffer(uint8_t *txbuffer, uint8_t size);
    void writeBufferChar(char *txbuffer, uint8_t size);
    uint8_t readBuffer(uint8_t *rxbuffer);
    uint8_t readBufferChar(char *rxbuffer);
	
    //*******************************************************************************
    //RXTX Switch routines - Not yet tested as of 02/12/19
    //*******************************************************************************

    void rxtxInit(int8_t pinRXEN, int8_t pinTXEN);  //not used on current SX127x modules
    void rxEnable();                                //not used on current SX127x modules
    void txEnable();                                //not used on current SX127x modules


    //*******************************************************************************
    //Library variables
    //*******************************************************************************

  private:

    int8_t _NSS, _NRESET, _DIO0, _DIO1, _DIO2;
    uint8_t _RXPacketL;             //length of packet received
    uint8_t _RXPacketType;          //type number of received packet
    uint8_t _RXDestination;         //destination address of received packet
    uint8_t _RXSource;              //source address of received packet
    int8_t  _PacketRSSI;            //RSSI of received packet
    int8_t  _PacketSNR;             //signal to noise ratio of received packet
    /**************************************************************************
      Modified by C. Pham - Oct. 2020
    **************************************************************************/    
    uint8_t _TXPacketL;             //length of transmitted packet
    //int8_t _TXPacketL;             //length of transmitted packet
    /**************************************************************************
      End by C. Pham - Oct. 2020
    **************************************************************************/     
    uint16_t _IRQmsb;               //for setting additional flags
    uint8_t _Device;                //saved device type
    uint8_t _TXDonePin;             //the pin that will indicate TX done
    uint8_t _RXDonePin;             //the pin that will indicate RX done
    uint8_t _UseCRC;                //when packet parameters set this flag enabled if CRC on packets in use
    int8_t _RXEN, _TXEN;            //not currently used
    uint8_t _PACKET_TYPE;           //used to save the set packet type
    uint8_t _freqregH, _freqregM, _freqregL;  //the registers values for the set frequency
    uint8_t _ShiftfreqregH, _ShiftfreqregM, _ShiftfreqregL;  //register values for shifted frequency, used in FSK
    uint32_t savedFrequency;
    int32_t savedOffset;
    /**************************************************************************
      Added by C. Pham - Oct. 2020
    **************************************************************************/
    int16_t  _RSSI;
    uint8_t _TXSeqNo; //sequence number of transmitted packet
    uint8_t _RXSeqNo; //sequence number of received packet
    bool _PA_BOOST;
    uint32_t _RXTimestamp;
    uint32_t _RXDoneTimestamp;
    uint8_t _DevAddr;
    uint8_t _AckStatus;
    int8_t _PacketSNRinACK;
    void (*_lowPowerFctPtr)(unsigned long);    
    /**************************************************************************
      End by C. Pham - Oct. 2020
    **************************************************************************/
};
#endif


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
