/*
* Copyright (C) 2018 C. Pham University of Pau, France
*
* Congduc.Pham@univ-pau.fr
*/

#ifndef SHT_TEMPERATURE_H
#define SHT_TEMPERATURE_H

#include "Sensor.h"
#include "Sensirion.h"

//uncomment if you use a SHT2x
//#define SHT2x

class SHT_Temperature : public Sensor {
  public:
    SHT_Temperature(char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power, uint8_t pin_trigger);
    double get_value();
    double get_value(float *data = NULL, float *data1 = NULL, float *data2 = NULL);
    void update_data();
  private:  
    Sensirion* sht = NULL;  
};

#endif
