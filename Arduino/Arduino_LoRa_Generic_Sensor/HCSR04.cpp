/*
* Copyright (C) 2016 Congduc Pham, University of Pau, France
*
* Congduc.Pham@univ-pau.fr
*
*/

#include "HCSR04.h"

HCSR04::HCSR04(char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power, int pin_trigger):Sensor(nomenclature, is_analog, is_connected, is_low_power, pin_read, pin_power, pin_trigger){
  if (get_is_connected()){

    pinMode(get_pin_read(), INPUT);
    pinMode(get_pin_power(),OUTPUT);
    
	if (get_is_low_power())
    	digitalWrite(get_pin_power(),LOW);
   	else
		digitalWrite(get_pin_power(),HIGH);
  	
  	if (has_pin_trigger())
  		pinMode(get_pin_trigger(),OUTPUT);
  		
    set_wait_time(500);
  }
}

void HCSR04::update_data()
{
  if (get_is_connected()) {
  	
  	long echo;
    double aux_distance = 0;
    double distance = 0;
    
    // if we use a digital pin to power the sensor...
    if (get_is_low_power())
    	digitalWrite(get_pin_power(),HIGH);     
    
    // wait
    delay(get_wait_time());
    
    // you can decice to have less samples
    uint8_t nb_sample=5;
   
    for(int i=0; i<nb_sample; i++) {
    	
	   	digitalWrite(get_pin_trigger(), HIGH);
	   	// at least 10us
		delayMicroseconds(12);
		digitalWrite(get_pin_trigger(), LOW);
		
		echo = pulseIn(get_pin_read(), HIGH);
				
		aux_distance = echo / 58.0; 

        distance += aux_distance;
        delay(10);
    }
    
    if (get_is_low_power())    
        digitalWrite(get_pin_power(),LOW);
  
    // getting the average
    set_data(distance / (double)nb_sample);
  }
  else {
  	if (has_fake_data())
  		// if not connected, set a random value (for testing)  	
    	set_data((double)random(30, 100));
  }
}

double HCSR04::get_value()
{
  update_data();
  return get_data();
}
