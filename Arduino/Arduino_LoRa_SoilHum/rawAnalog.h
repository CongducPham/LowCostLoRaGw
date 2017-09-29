/*
* Copyright (C) 2016 Congduc Pham, University of Pau, France
*
* Congduc.Pham@univ-pau.fr
*/

#ifndef RAWANALOG_H
#define RAWANALOG_H
#include "Sensor.h"

#define RAWANALOG_SCALE _BOARD_MVOLT_SCALE

class rawAnalog : public Sensor {
  public:    
    rawAnalog(char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power);
    void update_data();
    double get_value();
};

#endif
