/*
* Copyright (C) 2018 C. Pham University of Pau, France
*
* Congduc.Pham@univ-pau.fr
*/

#ifndef SHT_TEMPERATURE_H
#define SHT_TEMPERATURE_H

#include "Sensor.h"
#include "SHT1X.h"

class SHT_Temperature : public Sensor {
  public:
    SHT_Temperature(char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power, uint8_t pin_trigger);
    double get_value();
    void update_data();
    
  private:
    SHT1x* sht = NULL;  
};

#endif
