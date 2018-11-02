#ifndef _DHT22_H_
#define _DHT22_H_

#include <inttypes.h>

#define DHT22_ERROR_VALUE -995

typedef enum
{
  DHT_ERROR_NONE = 0,
  DHT_BUS_HUNG,
  DHT_ERROR_NOT_PRESENT,
  DHT_ERROR_ACK_TOO_LONG,
  DHT_ERROR_SYNC_TIMEOUT,
  DHT_ERROR_DATA_TIMEOUT,
  DHT_ERROR_CHECKSUM,
  DHT_ERROR_TOOQUICK
} DHT22_ERROR_t;

class DHT22
{
  private:
    uint8_t _bitmask;
    volatile uint8_t *_baseReg;
    unsigned long _lastReadTime;
    short int _lastHumidity;
    short int _lastTemperature;

  public:
    DHT22(uint8_t pin);
    DHT22_ERROR_t readData();
	short int getHumidityInt();
	short int getTemperatureCInt();
    void clockReset();
#if !defined(DHT22_NO_FLOAT)
    float getHumidity();
    float getTemperatureC();
#endif
};

// Report the humidity in .1 percent increments, such that 635 means 63.5% relative humidity
//
// Converts from the internal integer format on demand, so you might want
// to cache the result.
inline short int DHT22::getHumidityInt()
{
  return _lastHumidity;
}

// Get the temperature in decidegrees C, such that 326 means 32.6 degrees C.
// The temperature may be negative, so be careful when handling the fractional part.
inline short int DHT22::getTemperatureCInt()
{
  return _lastTemperature;
}

#if !defined(DHT22_NO_FLOAT)
// Return the percentage relative humidity in decimal form
inline float DHT22::getHumidity()
{
  return float(_lastHumidity)/10;
}
#endif

#if !defined(DHT22_NO_FLOAT)
// Return the percentage relative humidity in decimal form
//
// Converts from the internal integer format on demand, so you might want
// to cache the result.
inline float DHT22::getTemperatureC()
{
  return float(_lastTemperature)/10;
}
#endif //DHT22_SUPPORT_FLOAT

#endif /*_DHT22_H_*/
