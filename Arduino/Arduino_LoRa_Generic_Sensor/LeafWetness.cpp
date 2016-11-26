/*
* Copyright (C) 2016 Nicolas Bertuol, University of Pau, France
*
* nicolas.bertuol@etud.univ-pau.fr
*
* modified by Congduc Pham, University of Pau, France
*/

#include "LeafWetness.h"

LeafWetness::LeafWetness(char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power):Sensor(nomenclature, is_analog, is_connected, is_low_power, pin_read, pin_power){
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

void LeafWetness::update_data()
{
  if (get_is_connected()) {
  	
    // if we use a digital pin to power the sensor...
    if (get_is_low_power())
    	digitalWrite(get_pin_power(),HIGH);  	

    // wait
    delay(get_wait_time());
      
    // TO MODIFY, IT SHALL RETURN VALUE BETWEEN 0 - 15 NOT A VOLTAGE
    if(get_is_analog()){
    	// *LW_SCALE because of 5v or 3.3v depending on arduino and /1024 because of 10bits analog value, so 1024 in decimal
    	set_data(((double)analogRead(get_pin_read()) * LW_SCALE / 1024.0));
    }
    else {
    	set_data(((double)digitalRead(get_pin_read())));
    }
        
    if (get_is_low_power())      
        digitalWrite(get_pin_power(),LOW);
        
  }
  else{
  	// if not connected, set a random value (for testing)  	
  	if (has_fake_data())
    	set_data((double) (random(0, 30) / 2.0));
  }
}

double LeafWetness::get_value(){
  update_data();
  return get_data();
}
