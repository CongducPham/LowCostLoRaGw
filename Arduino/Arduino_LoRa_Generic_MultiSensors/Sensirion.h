/* ========================================================================== */
/*  Sensirion.h - Library for Sensirion SHT1x & SHT7x family temperature      */
/*    and humidity sensors                                                    */
/*  Created by Markus Schatzl, November 28, 2008                              */
/*  Released into the public domain                                           */
/*                                                                            */
/*  Revised (v1.1) by Carl Jackson, August 4, 2010                            */
/*  Rewritten (v2.0) by Carl Jackson, December 10, 2010                       */
/*  Rewritten (v3.01) by Thierry Couquillou, June 3, 2016                     */
/*    See README.txt file for details                                         */
/* ========================================================================== */


#ifndef Sensirion_h
#define Sensirion_h

#include <stdint.h>


// Clock pulse timing macros
// Lengthening these may assist communication over long wires
#define PULSE_LONG  delayMicroseconds(30)
#define PULSE_SHORT delayMicroseconds(15)

// User constants DO NOT CHANGE
const uint8_t TEMP     =     0;
const uint8_t HUMI     =     1;

// Status register bit definitions
const uint8_t LOW_RES  =  0x01;  // 12-bit Temp / 8-bit RH (vs. 14 / 12)
const uint8_t NORELOAD =  0x02;  // No reload of calibrarion data
const uint8_t HEAT_ON  =  0x04;  // Built-in heater on
const uint8_t BATT_LOW =  0x40;  // VDD < 2.47V

// Function return code definitions
const int8_t S_Err_TO     = -4; // Timeout
const int8_t S_Err_CRC    = -3; // CRC failure
const int8_t S_Err_NoACK  = -2; // ACK expected but not received
const int8_t S_Err_Param  = -1; // Parameter error in function call
const int8_t S_Meas_Wait  = 0;  // Wait for sensor cooling down
const int8_t S_Temp_Req   = 1;  // Temperature request (pulse)
const int8_t S_Temp_Wait  = 2;  // Wait for temperature measurement
const int8_t S_Humi_Req   = 3;  // Humidity request (pulse) means temperature measurement was successfull
const int8_t S_Humi_Wait  = 4;  // Wait for humidity measurement
const int8_t S_Calc_Run   = 5;  // Calculation in progress
const int8_t S_Meas_Rdy   = 6;  // All measurement was successfull (pulse)


class Sensirion
{
  private:
    uint8_t   _pinData;         // Pin interface
    uint8_t   _pinClock;
    uint8_t   _step;            // Step of read cycle 0/1/2/3/4/5
    uint8_t   _stat_reg;        // Local copy of status register
    uint8_t   _shtaddress;      // Address of SHTxx module
    uint8_t   _shtnohold;       // Hold Master mode for SHT2x/3x module
    uint8_t   _sht1x;           // With Address we can know if it's SHT1x/2x/3x
    uint8_t   _sht2x;
    uint8_t   _sht3x;
    uint8_t   _shtprefix;       // prefix for SHT2x and SHT3x command
    uint32_t  _time;            // For delay and Time out calculation
    uint16_t  _meas[2];         // meas[0] = Temperature, meas[1] = Humidity
    uint8_t   _crc;

    int8_t  getResult(uint8_t cmd);
    int8_t  meas(uint8_t cmd);
    int8_t  measRdy(uint8_t cmd);
    int8_t  putByte(uint8_t value);
    uint8_t getByte(bool ack);
    uint8_t bitrev(uint8_t value);
    void    startTransmission(void);
    void    stopTransmission(void);
    void    resetConnection(void);
    void    calcCRC(uint8_t value, uint8_t *crc);

  public:
    Sensirion(uint8_t dataPin, uint8_t clockPin, uint8_t address = 0x00, bool noholdmaster = false);
    
    int8_t  measure(float *temp = NULL, float *humi = NULL, float *dew = NULL, float temp0 = 0, float *humi0 = NULL);   
    int8_t  writeSR(uint8_t value);
    int8_t  readSR(uint8_t *result);
    int8_t  reset(void);
    float   calcTemp(uint16_t rawData);
    float   calcHumi(uint16_t rawData, float temp);
    //float   calcHumi(float dewpoint, float temp);
    //float   calcDewpoint(float humi, float temp);
};

#endif  // #ifndef Sensirion_h
