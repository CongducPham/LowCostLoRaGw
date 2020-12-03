/* ========================================================================== */
/*  Sensirion.cpp - Library for Sensirion SHT1x & SHT7x family temperature    */
/*    and humidity sensors                                                    */
/*  Created by Markus Schatzl, November 28, 2008                              */
/*  Released into the public domain                                           */
/*                                                                            */
/*  Revised (v1.1) by Carl Jackson, August 4, 2010                            */
/*  Rewritten (v2.0) by Carl Jackson, December 10, 2010                       */
/*  Rewritten (v3.01) by Thierry Couquillou, June 3, 2016                     */
/*    See README.txt file for details                                         */
/* ========================================================================== */


/******************************************************************************
 * Includes
 ******************************************************************************/

extern "C"
{
  // AVR LibC Includes
  #include <stddef.h>
  #include <stdint.h>
  //#include <math.h>

  // Wiring Core Includes
  #if (ARDUINO < 100)
    #include <WConstants.h>
  #endif
}

#if (ARDUINO >= 100)
  #include <Arduino.h>
#endif

#include "Sensirion.h"


/******************************************************************************
 * Definitions
 ******************************************************************************/

#define PULSE_step  _step = ((error>=0)?_step+1:0)                  // Next step if no error (>=0)else Step 0
#define WAIT_step   _step = ((error>0)?_step+1:((error<0)?0:_step)) // Wait if 0. Next step if no error (>0) else Step 0


// Sensirion command definitions:        adr comm r/w
const uint8_t MEAS_TEMP     = 0x03;   // 000 0001 1 - Trigger T measurement hold master
const uint8_t MEAS_HUMI     = 0x05;   // 000 0010 1 - Trigger RH measurement hold master
// const uint8_t MEAS_TEMP_NH  = 0x13;   // 000 1001 1 - Trigger T measurement no hold master
// const uint8_t MEAS_HUMI_NH  = 0x15;   // 000 1010 1 - Trigger RH measurement no hold master
const uint8_t STAT_REG_W    = 0x06;   // 000 0011 0 - Write user register
const uint8_t STAT_REG_R    = 0x07;   // 000 0011 1 - Read user register
const uint8_t SOFT_RESET    = 0x1e;   // 000 1111 0 - Soft reset

// Status register writable bits
const uint8_t SR_MASK     = 0x07;

// getByte flags
const bool noACK  = false;
const bool ACK    = true;



/******************************************************************************
 * Constructors
 ******************************************************************************/

Sensirion::Sensirion(uint8_t dataPin, uint8_t clockPin, uint8_t address, bool noholdmaster)
{
  // Initialize private storage for library functions
  _pinData      = dataPin;
  _pinClock     = clockPin;

  _shtaddress   = address<<1;           // Address depends of SHT1x/2x/3x. LSb is reserved for R/W
  _shtnohold    = (noholdmaster!=0)<<4;
  _sht1x        = (address==0x00);      // SHT1x address = 0x00
  _sht2x        = (address==0x40);      // SHT2x address = 0x40
  _sht3x        = (address==0x44 || address==0x45); // SHT3x address = 0x44 or 0x45

  if(_sht1x)                            // SHT1x have 3 first bits of command used by address and = 0
    _shtprefix  = 0x00;
  else                                  // SHT2x/3x have 3 first bits of command always = 1
    _shtprefix  = 0xE0;

  _time         = millis();
  _stat_reg     = 0x00;                 // Sensor status register default state


  // Return sensor to default state
  resetConnection();                // Reset communication link with sensor

  startTransmission();
  putByte(SOFT_RESET);              // Send soft reset command
  stopTransmission();
}


/******************************************************************************
 * User functions
 ******************************************************************************/

// S_Meas_Wait  = 0;  // Wait for sensor cooling down
// S_Temp_Req   = 1;  // Temperature request (pulse)
// S_Temp_Wait  = 2;  // Wait for temperature measurement
// S_Humi_Req   = 3;  // Humidity request (pulse) means temperature measurement was successfull
// S_Humi_Wait  = 4;  // Wait for humidity measurement
// S_Calc_Run   = 5;  // calculation in progress
// S_Meas_Rdy   = 6;  // Temperature measurement was successfull (pulse) means all measurement successfull

// All-in-one : measure(&temperature, &humidity, &dewpoint, temp0, &humi0 [=equivalent humidity@temp0])
// All parameters are optionals eg: measure(&temperature) if only need temp. Non blocking. Return S_Meas_Rdy every data refresh.
int8_t Sensirion::measure(float *temp, float *humi, float *dew, float temp0, float *humi0)
{
  int8_t error = 0;

  if (!_sht1x && !_sht2x && !_sht3x)
    return S_Err_Param;

  if (_pinData==_pinClock)
    return S_Err_Param;

  switch(_step)
  {
    case 0:
      if(((millis()-_time)>3000) || ((millis()-_time)>1000 && (_sht2x || _sht3x)))  // Wait 3s for SHT1x, 1s for SHT2x/3x to cool down
        _step++;
      break;

    case 1:                                     // Temperature request (pulse)
    case 3:                                     // Humidity request (pulse)
      _time = millis();                         // Reset time for time-out and delay
      error = meas(_step>>1);                   // Request measurement 0=Temp. 1=Humi.
      PULSE_step;                               // Next step if no error (>=0)else Step 0
      break;

    case 2:                                     // Wait for temperature measurement
    case 4:                                     // Wait for humidity measurement
      error = measRdy((_step-1)>>1);            // Read measurement if available 0=Temp. 1=Humi.
      if(((millis()-_time)>500) || ((millis()-_time)>100 && (_sht2x||_sht3x)))
        error = S_Err_TO;
      WAIT_step;                                // Wait if 0. Next step if no error (>0) else Step 0
      break;

    case 5:
      _time = millis();                         // Reset time for time-out and delay
      if (temp)
      {
        *temp = calcTemp(_meas[TEMP]);          // Temperature calculation
        if (humi)
        {
          *humi = calcHumi(_meas[HUMI], *temp); // Humidity calculation
          /*
          if (dew)
          {
            *dew = calcDewpoint(*humi, *temp);  // Dewpoint calculation
            if (humi0)
            {
              *humi0 = calcHumi(*dew, temp0);   // Equivalent humidity at temp0 (usefull for hum/temp close control loop
            }
          }
          */
        }
      }
      _step = (error>=0)?_step+1:0;             // Next step if no error else step 0
      break;

    case 6:
      _step = 0;                                // Last step. Just one pulse. Measurement is available
      break;

    default:
      _step = 0;
  }

  return (error<0)?error:_step;
}

// Initiate measurement.
int8_t Sensirion::meas(uint8_t cmd)     // Step 1 & 3
{
  int8_t error = 0;

//  pinMode(4, OUTPUT);                   // Trig scope
//  digitalWrite(4, HIGH);                // Trig scope
//  PULSE_SHORT;                          // Trig scope
//  digitalWrite(4, LOW);                 // Trig scope

  if(_sht1x)
  {
    readSR(&_stat_reg);                 // Update sensor status register state
    _crc = bitrev(_stat_reg & SR_MASK); // Initialize CRC calculation
  }

  startTransmission();

  if(!_sht1x)
  {
    if (error = putByte(_shtaddress+0)) // +0 = write bit.
    {
      stopTransmission();
      return error;
    }
  }

  if (cmd == TEMP)
    cmd = _shtprefix | _shtnohold | MEAS_TEMP;
  else
    cmd = _shtprefix | _shtnohold | MEAS_HUMI;

  if (error = putByte(cmd))
  {
    stopTransmission();
    return error;
  }

  if(_sht1x)
    calcCRC(cmd, &_crc);                // Include command byte in CRC calculation

  if(!_sht1x)
  {
    if(_shtnohold)
    {
      PULSE_LONG;
      stopTransmission();
    }
    else
    {
      startTransmission();
      if (error = putByte(_shtaddress+1)) // +1 = read bit. Should return ACK
      {
        stopTransmission();
        return error;
      }

      pinMode(_pinClock, INPUT_PULLUP);   // Return clock line to input mode pullup resistor 20kohm
      PULSE_SHORT;
    }
  }

  return 0;
}

// Check if non-blocking measurement has completed
// Non-zero return indicates complete (with or without error)
int8_t Sensirion::measRdy(uint8_t cmd)    // Step 2 & 4
{
  int8_t error = 0;

  if(_sht1x)
  {
    if (digitalRead(_pinData))            // If _pinData HIGH measurement not ready ...
      return 0;                           // ... will retry next time
  }
  else if(!_shtnohold)
  {
    if (!digitalRead(_pinClock))          // If _pinClk LOW measurement not ready ...
      return 0;                           // ... will retry next time
  }
  else                                    // Need to poll to know if ready
  {
    startTransmission();
    if (putByte(_shtaddress+1))           // +1 = read bit. return !=0 -> ACK not received (measurement not ready)
    {
      stopTransmission();                 // No ACK. Stop transmission and ...
      return 0;                           // ... will retry next time
    }
  }

//  pinMode(4, OUTPUT);                   // Trig scope
//  digitalWrite(4, HIGH);                // Trig scope
//  PULSE_SHORT;                          // Trig scope
//  digitalWrite(4, LOW);                 // Trig scope

  error = getResult(cmd);                 // return 1 if OK, <0 = Error. Only possible error is S_Err_CRC

  stopTransmission();

  return error;                           // return 1 = OK, 0 = Retry next time, <0 = Error.
}

// Get measurement result from sensor (plus CRC, if enabled)
int8_t Sensirion::getResult(uint8_t cmd)
{
  uint8_t val;

  if(!_sht1x)
    _crc = 0;                         // Initialize CRC calculation

  val = getByte(ACK);
  calcCRC(val, &_crc);
  _meas[cmd] = val;
  val = getByte(ACK);
  calcCRC(val, &_crc);
  _meas[cmd] = (_meas[cmd] << 8) | val;
  val = getByte(noACK);

  if(_sht1x)
    val = bitrev(val);

  if (val != _crc)
    return S_Err_CRC;

  return 1;
}

// Write status register
int8_t Sensirion::writeSR(uint8_t value)
{
  int8_t error = 0;

  value &= SR_MASK;                 // Mask off unwritable bits
  _stat_reg = value;                // Save local copy

  startTransmission();

  if(!_sht1x)
  {
    if (error = putByte(_shtaddress+0)) // +0 = write bit.
    {
      stopTransmission();
      return error;
    }
  }

  if (error = putByte(_shtprefix | STAT_REG_W))
  {
    stopTransmission();
    return error;
  }

  error = putByte(value);

  stopTransmission();
  return error;
}

// Read status register
int8_t Sensirion::readSR(uint8_t *result)
{
  uint8_t val;
  int8_t error = 0;

//  pinMode(4, OUTPUT);                   // Trig scope
//  digitalWrite(4, HIGH);                // Trig scope
//  PULSE_SHORT;                          // Trig scope
//  digitalWrite(4, LOW);                 // Trig scope

  if(_sht1x)
    _crc = bitrev(_stat_reg & SR_MASK);  // Initialize CRC calculation
  else
    _crc = 0;                         // Initialize CRC calculation

  startTransmission();

  if(!_sht1x)
  {
    if (error = putByte(_shtaddress+1)) // +1 = read bit.
    {
      stopTransmission();
      return error;
    }
  }

  if (error = putByte(_shtprefix | STAT_REG_R))
  {
    stopTransmission();
    return error;
  }

  calcCRC(_shtprefix | STAT_REG_R, &_crc);  // Include command byte in CRC calculation

  _stat_reg = getByte(ACK);                 // Update sensor status register state
  *result = _stat_reg;                      // Return sensor status register state

  calcCRC(*result, &_crc);

  val = getByte(noACK);

  if(_sht1x)
    val = bitrev(val);

  if (val != _crc)
    error = S_Err_CRC;

  stopTransmission();

  return error;
}

// Public reset function
// Note: Soft reset returns sensor status register to default values
int8_t Sensirion::reset(void)
{
  int8_t error = 0;

  _stat_reg = 0x00;                 // Sensor status register default state
  resetConnection();                // Reset communication link with sensor

  startTransmission();

  error = putByte(SOFT_RESET);      // Send soft reset command & return status

  stopTransmission();
  
  return error;
}


/******************************************************************************
 * Sensirion data communication
 ******************************************************************************/

// Write byte to sensor and check for acknowledge
int8_t Sensirion::putByte(uint8_t value)
{
  uint8_t mask;
  int8_t i;
  int8_t error = 0;

  pinMode(_pinClock, OUTPUT);         // Set clock line to output mode
  pinMode(_pinData, OUTPUT);          // Set data line to output mode

  mask = 0x80;                        // Bit mask to transmit MSB first
  for (i = 8; i > 0; i--)
  {
    digitalWrite(_pinData, value & mask);
    PULSE_SHORT;
    digitalWrite(_pinClock, HIGH);    // Generate clock pulse
    PULSE_LONG;
    digitalWrite(_pinClock, LOW);
    PULSE_SHORT;
    mask >>= 1;                       // Shift mask for next data bit
  }

  pinMode(_pinData, INPUT_PULLUP);    // Return data line to input mode pullup resistor 20kohm
  PULSE_SHORT;

  digitalWrite(_pinClock, HIGH);      // Clock #9 for ACK
  PULSE_LONG;
  if (digitalRead(_pinData))          // Verify ACK ('0') received from sensor
    error = S_Err_NoACK;
  PULSE_SHORT;
  digitalWrite(_pinClock, LOW);       // Finish with clock in low state
  PULSE_SHORT;

  return error;
}

// Read byte from sensor and send acknowledge if "ack" is true
uint8_t Sensirion::getByte(bool ack)
{
  int8_t i;
  uint8_t result = 0;

  pinMode(_pinClock, OUTPUT);         // Set clock line to output mode

  for (i = 8; i > 0; i--)
  {
    result <<= 1;                     // Shift received bits towards MSB
    digitalWrite(_pinClock, HIGH);    // Generate clock pulse
    PULSE_SHORT;
    result |= digitalRead(_pinData);  // Merge next bit into LSB position
    digitalWrite(_pinClock, LOW);
    PULSE_SHORT;
  }
  pinMode(_pinData, OUTPUT);
  digitalWrite(_pinData, !ack);       // Assert ACK ('0') if ack == 1
  PULSE_SHORT;
  digitalWrite(_pinClock, HIGH);      // Clock #9 for ACK / noACK
  PULSE_LONG;
  digitalWrite(_pinClock, LOW);       // Finish with clock in low state
  PULSE_SHORT;

  pinMode(_pinData, INPUT_PULLUP);    // Return data line to input mode pullup resistor 20kohm
  PULSE_SHORT;

  return result;
}


/******************************************************************************
 * Sensirion signaling
 ******************************************************************************/

// Generate Sensirion-specific transmission start sequence
// This is where Sensirion SHT1x does not conform to the I2C standard and is
// the main reason why the AVR TWI hardware support can not be used.
//           ___     ___
// SCK : ___|   |___|   |______
//       _____         ________
// DATA:      |_______|
//
// SHT2x/3x follow I2C standard.
//       _________
// SCK :          |______
//       _____
// DATA:      |__________


void Sensirion::startTransmission(void)
{
  if(_sht1x)
  {
    digitalWrite(_pinData, HIGH);   // Set data register high before turning on
    pinMode(_pinData, OUTPUT);      // output driver (avoid possible low pulse)
    digitalWrite(_pinClock, LOW);   // Set clk register low before turning on (avoid possible high pulse)
    pinMode(_pinClock, OUTPUT);     // Clock turned to output on 1st transmission

    PULSE_SHORT;                    // Start transmission sequence
    digitalWrite(_pinClock, HIGH);
    PULSE_SHORT;
    digitalWrite(_pinData, LOW);
    PULSE_SHORT;
    digitalWrite(_pinClock, LOW);
    PULSE_LONG;
    digitalWrite(_pinClock, HIGH);
    PULSE_SHORT;
    digitalWrite(_pinData, HIGH);
    PULSE_SHORT;
    digitalWrite(_pinClock, LOW);
    PULSE_SHORT;
  }
  else
  {
    digitalWrite(_pinData, HIGH);   // Set data register high before turning on
    pinMode(_pinData, OUTPUT);      // output driver (avoid possible low pulse)
    digitalWrite(_pinClock, HIGH);  // Set clk register high before turning on
    pinMode(_pinClock, OUTPUT);     // output driver (avoid possible low pulse)

    PULSE_SHORT;                    // Start transmission sequence
    digitalWrite(_pinData, LOW);
    PULSE_SHORT;
    digitalWrite(_pinClock, LOW);
    PULSE_SHORT;
  }
}

void Sensirion::stopTransmission(void)
{
  if(!_sht1x)
  {
    digitalWrite(_pinData, LOW);        // Set data register low before turning on
    pinMode(_pinData, OUTPUT);          // output driver (avoid possible high pulse)
    digitalWrite(_pinClock, LOW);       // Set clk register low before turning on
    pinMode(_pinClock, OUTPUT);         // output driver (avoid possible high pulse)

    PULSE_SHORT;                        // Stop transmission sequence
    digitalWrite(_pinClock, HIGH);
    PULSE_SHORT;
    digitalWrite(_pinData, HIGH);
    PULSE_SHORT;

    pinMode(_pinData, INPUT_PULLUP);    // Return clock line to input mode pullup resistor 20kohm
    pinMode(_pinClock, INPUT_PULLUP);   // Return clock line to input mode pullup resistor 20kohm
    PULSE_SHORT;
  }
}

// Communication link reset
// At least 9 SCK cycles with DATA=1, followed by transmission start sequence
//          _    _    _    _    _    _    _    _    _        ___     ___
// SCK : __| |__| |__| |__| |__| |__| |__| |__| |__| |______|   |___|   |______
//      ______________________________________________________         ________
// DATA:                                                      |_______|

void Sensirion::resetConnection(void)
{
  int8_t i;

  if (_sht1x)
  {
    digitalWrite(_pinData, HIGH);  // Set data register high before turning on
    pinMode(_pinData, OUTPUT);     // output driver (avoid possible low pulse)
    PULSE_LONG;
    for (i = 0; i < 9; i++)
    {
      digitalWrite(_pinClock, HIGH);
      PULSE_LONG;
      digitalWrite(_pinClock, LOW);
      PULSE_LONG;
    }

    startTransmission();
  }
}


/******************************************************************************
 * Helper Functions
 ******************************************************************************/

// Calculates temperature in degrees C from raw sensor data
float Sensirion::calcTemp(uint16_t rawData)
{
  if (_sht1x)
  {
    if (_stat_reg & LOW_RES)
      return -40.1 + 0.04 * rawData;
    else
      return -40.1 + 0.01 * rawData;
  }
  else
    return -46.85 + 175.72 / 65536.0 * rawData;
}


//  const float C1  = -4.0000;        // for V3 sensors
//  const float C2l =  0.6480;        // for V3 sensors, 8-bit precision
//  const float C3l = -7.2000E-4;     // for V3 sensors, 8-bit precision
//  const float C2h =  0.0405;        // for V3 sensors, 12-bit precision
//  const float C3h = -2.8000E-6;     // for V3 sensors, 12-bit precision

//  const float C1  = -2.0468;        // for V4 and V5 sensors
//  const float C2l =  0.5872;        // for V4 and V5 sensors, 8-bit precision
//  const float C3l = -4.0845E-4;     // for V4 and V5 sensors, 8-bit precision
//  const float C2h =  0.0367;        // for V4 and V5 sensors, 12-bit precision
//  const float C3h = -1.5955E-6;     // for V4 and V5 sensors, 12-bit precision

// Calculates relative humidity from raw sensor data (with temperature compensation)
float Sensirion::calcHumi(uint16_t rawData, float temp)
{
  float humi;

  if (_sht1x)
  {
    if (_stat_reg & LOW_RES)
    {
      // for V3 sensors
//      humi = -4.0000 + 0.6480 * rawData + -7.2000E-4 * rawData * rawData;
//      humi = (temp - 25.0) * (0.01 + 0.00128 * rawData) + humi;

      // for V4 and V5 sensors
      humi = -2.0468 + 0.5872 * rawData + -4.0845E-4 * rawData * rawData;
      humi = (temp - 25.0) * (0.01 + 0.00128 * rawData) + humi;
    }
    else
    {
      // for V3 sensors
//      humi = -4.0000 + 0.0405 * rawData + -2.8000E-6 * rawData * rawData;
//      humi = (temp - 25.0) * (0.01 + 0.00008 * rawData) + humi;

      // for V4 and V5 sensors
      humi = -2.0468 + 0.0367 * rawData + -1.5955E-6 * rawData * rawData;
      humi = (temp - 25.0) * (0.01 + 0.00008 * rawData) + humi;
    }
  }
  else
    humi = -6.0 + 125.0 / 65536.0 * rawData;

  if (humi > 100.0) humi = 100.0;
  if (humi < 0.1) humi = 0.1;

  return humi;
}

/*
// RH: =100*(EXP((17.625*TD)/(243.04+TD))/EXP((17.625*T)/(243.04+T)))
// TD: =243.04*(LN(RH/100)+((17.625*T)/(243.04+T)))/(17.625-LN(RH/100)-((17.625*T)/(243.04+T)))
//  T: =243.04*(((17.625*TD)/(243.04+TD))-LN(RH/100))/(17.625+LN(RH/100)-((17.625*TD)/(243.04+TD)))

// Calculates dew point in degrees C from humidity
float Sensirion::calcDewpoint(float humi, float temp)
{
  float k;
  k = log(humi/100) + (17.625 * temp) / (243.04 + temp);
  return 243.04 * k / (17.625 - k);
}

// Calculates humidity in degrees C from dew point
float Sensirion::calcHumi(float dewpoint, float temp)
{
  float k1,k2;
  k1 = exp((17.625*dewpoint)/(243.04+dewpoint));
  k2 = exp((17.625*temp)/(243.04+temp));
  return 100*k1/k2;
}

*/

// Calculate CRC for a single byte
void Sensirion::calcCRC(uint8_t value, uint8_t *crc)
{
  const uint8_t POLY = 0x31;   // Polynomial: x**8 + x**5 + x**4 + 1
  int8_t i;
  *crc ^= value;
  for (i = 8; i > 0; i--)
  {
    if (*crc & 0x80)
      *crc = (*crc << 1) ^ POLY;
    else
      *crc = (*crc << 1);
  }
}

// Bit-reverse a byte (for CRC calculations)
uint8_t Sensirion::bitrev(uint8_t value)
{
  int8_t i;
  uint8_t result = 0;
  for (i = 8; i > 0; i--)
  {
    result = (result << 1) | (value & 0x01);
    value >>= 1;
  }
  return result;
}
