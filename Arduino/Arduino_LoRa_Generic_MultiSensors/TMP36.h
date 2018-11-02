/*
* Copyright (C) 2018 C. Pham University of Pau, France
*
* Congduc.Pham@univ-pau.fr
*/

#ifndef TMP36_H
#define TMP36_H
#include "Sensor.h"

#define TEMP_SCALE _BOARD_MVOLT_SCALE 

class TMP36 : public Sensor {
  public:    
    TMP36(char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power);
    void update_data();
    double get_value();
};

#endif
