/*
* Copyright (C) 2016 Nicolas Bertuol, University of Pau, France
*
* nicolas.bertuol@etud.univ-pau.fr
*/

#ifndef LM35_H
#define LM35_H
#include "Sensor.h"

#define TEMP_SCALE _BOARD_MVOLT_SCALE 

class LM35 : public Sensor {
  public:    
    LM35(char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power);
    void update_data();
    double get_value();
};

#endif
