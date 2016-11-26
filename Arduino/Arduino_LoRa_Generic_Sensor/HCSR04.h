/*
* Copyright (C) 2016 C. Pham, University of Pau, France
*
* Congduc.Pham@univ-pau.fr
*/

/*	the HC_SR04 is a low-cost ultra sonic sensor. It comes in 2 version, a very low-cost requiring 5V 
	and another one, a bit more expensve, that can work in 3v-5v so suitable for 3.3v boards
*/	

#ifndef HCSR04_H
#define HCSR04_H
#include "Sensor.h"

class HCSR04 : public Sensor {
  public:    
    HCSR04(char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power, int pin_trigger);
    void update_data();
    double get_value();
};

#endif
