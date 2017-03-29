/*
* Copyright (C) 2016 Congduc Pham, University of Pau, France
*
* Congduc.Pham@univ-pau.fr
*/


#include "rawAnalog.h"

rawAnalog::rawAnalog(char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power):Sensor(nomenclature, is_analog, is_connected, is_low_power, pin_read, pin_power){
  if (get_is_connected()){

    pinMode(get_pin_read(), INPUT);
    pinMode(get_pin_power(),OUTPUT);
    
	if(get_is_low_power())
       digitalWrite(get_pin_power(),LOW);
    else
		digitalWrite(get_pin_power(),HIGH);
	
    set_wait_time(100);
  }
}

void rawAnalog::update_data()
{	
  if (get_is_connected()) {
  	
    int aux_value = 0;
    int value = 0;
    
    // if we use a digital pin to power the sensor...
    if (get_is_low_power())
    	digitalWrite(get_pin_power(),HIGH);
    	
    // wait
    delay(get_wait_time());
   
    for(int i=0; i<get_n_sample(); i++) {
    	
    	if (get_is_analog()) {
        	aux_value = analogRead(get_pin_read());
      	}
    
        value += aux_value;
        delay(10);
	}
	
    if (get_is_low_power())        
        digitalWrite(get_pin_power(),LOW);
        
    // getting the average
    set_data(value/(double)get_n_sample());        
  }
  else {
  		// if not connected, set a random value (for testing)  	
  		if (has_fake_data())
    		set_data((double)random(0,1024));
  }
}

double rawAnalog::get_value()
{
  update_data();
  return get_data();
}
