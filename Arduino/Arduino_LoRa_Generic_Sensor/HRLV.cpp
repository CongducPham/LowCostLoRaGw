/*
* Copyright (C) 2016 Mamour Diop, University of Pau, France
*
* mamour.diop@univ-pau.fr
*/

#include "HRLV.h"

HRLV::HRLV(char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power):Sensor(nomenclature, is_analog, is_connected, is_low_power, pin_read, pin_power){
  if (get_is_connected()){

    pinMode(get_pin_read(), INPUT);
    pinMode(get_pin_power(),OUTPUT);
    
	if(get_is_low_power())
       digitalWrite(get_pin_power(),LOW);
    else
		digitalWrite(get_pin_power(),HIGH);
	
    set_wait_time(500);
  }
}

void HRLV::update_data()
{
  if (get_is_connected()) {
  	
    double aux_dist = 0;
    double distance = 0;
    
    // if we use a digital pin to power the sensor...
    if (get_is_low_power())
    	digitalWrite(get_pin_power(),HIGH);     
    
    // wait
    delay(get_wait_time());
    
    // you can decice to have less samples
    uint8_t nb_sample=5;
   
    for(int i=0; i<nb_sample; i++) {
    	
	    if (get_is_analog()) {
	      aux_dist = analogRead(get_pin_read()) * 5; // mm
	      //aux_dist /= 25.4; // inches
	    }
    
        distance += aux_dist;
        delay(10);
    }
    
    if (get_is_low_power())    
        digitalWrite(get_pin_power(),LOW);
  
    	// getting the average
		set_data(distance / (double)nb_sample);
	}
	else {
  		// if not connected, set a random value (for testing)  	
  		if (has_fake_data())
    		set_data((double)random(30, 100));
	}
}

double HRLV::get_value()
{
  update_data();
  return get_data();
}
