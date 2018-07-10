/*
* Copyright (C) 2018 C. Pham, University of Pau, France
*
* Congduc.Pham@univ-pau.fr
*
*/

#include "TMP36.h"

TMP36::TMP36(char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power):Sensor(nomenclature, is_analog, is_connected, is_low_power, pin_read, pin_power){
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

void TMP36::update_data()
{
  if (get_is_connected()) {
  	
    double aux_temp = 0;
    double temperature = 0;
    
    // if we use a digital pin to power the sensor...
    if (get_is_low_power())
    	digitalWrite(get_pin_power(),HIGH);     
    
    // wait
    delay(get_wait_time());
   
    for(int i=0; i<get_n_sample(); i++) {
    	
    	// a bit useless to test for analog as we know it is analog
    	// remove this test?
	    if (get_is_analog()) {
	      aux_temp = analogRead(get_pin_read());
	      aux_temp = ((aux_temp*TEMP_SCALE/1024.0)-500)/10;
	    }
    
        temperature += aux_temp;
        delay(10);
    }
    
    if (get_is_low_power())    
        digitalWrite(get_pin_power(),LOW);
  
    // getting the average
    set_data(temperature/(double)get_n_sample()); 
	}
	else {
  		// if not connected, set a random value (for testing)  	
  		if (has_fake_data())
    		set_data((double)random(-10, 30));
	}
}

double TMP36::get_value()
{
  update_data();
  return get_data();
}
