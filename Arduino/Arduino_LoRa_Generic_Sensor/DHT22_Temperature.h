/*
* Copyright (C) 2016 Nicolas Bertuol, University of Pau, France
*
* nicolas.bertuol@etud.univ-pau.fr
*/

#ifndef DHT22_TEMPERATURE_H
#define DHT22_TEMPERATURE_H

#include "Sensor.h"
#include "DHT22.h"

class DHT22_Temperature : public Sensor {
  public:
    DHT22_Temperature(char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power);
    double get_value();
    void update_data();
    
  private:
    DHT22* dht = NULL;
    DHT22_ERROR_t errorCode;
    uint8_t _dht_type;
    
};

#endif
