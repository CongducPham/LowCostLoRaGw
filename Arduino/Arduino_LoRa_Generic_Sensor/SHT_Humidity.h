/*
* Copyright (C) 2018 C. Pham University of Pau, France
*
* Congduc.Pham@univ-pau.fr
*/

#ifndef SHT_HUMIDITY_H
#define SHT_HUMIDITY_H

#include "Sensor.h"
#include "Sensirion.h"

//uncomment if you use a SHT2x
//#define SHT2x

class SHT_Humidity : public Sensor {
  public:    
    SHT_Humidity(char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power, uint8_t pin_trigger);
    double get_value();
    void update_data();
  private:    
    Sensirion* sht = NULL;
};

#endif
