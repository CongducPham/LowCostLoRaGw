/*
* Copyright (C) 2016 Congduc Pham, University of Pau, France
*
* congduc.pham@univ-pau.fr
*/

#ifndef DS18B20_H
#define DS18B20_H

#include "Sensor.h"
#include "OneWire.h"

#define DS18B20_addr 0x28

class DS18B20 : public Sensor {
  public:
    DS18B20(char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power);
    double get_value();
    void update_data();
    
  private:
    OneWire* ds = NULL;
};

#endif
