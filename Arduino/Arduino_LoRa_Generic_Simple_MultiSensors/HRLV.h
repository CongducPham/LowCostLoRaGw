/*
* Copyright (C) 2016 Mamour Diop, University of Pau, France
*
* mamour.diop@univ-pau.fr
*/

#ifndef HRLV_H
#define HRLV_H
#include "Sensor.h"

#define TEMP_SCALE _BOARD_MVOLT_SCALE 

class HRLV : public Sensor {
  public:    
    HRLV(char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power);
    void update_data();
    double get_value();
};

#endif
