/*
* Copyright (C) 2016 Nicolas Bertuol, University of Pau, France
*
* nicolas.bertuol@etud.univ-pau.fr
*
* modified by Congduc Pham, University of Pau, France
*/

#include "DHT22_Humidity.h"

DHT22_Humidity::DHT22_Humidity(char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power):Sensor(nomenclature, is_analog, is_connected, is_low_power, pin_read, pin_power){
  if (get_is_connected()){
    // start library DHT
    dht = new DHT22(get_pin_read());
    
    pinMode(get_pin_power(),OUTPUT);
    
	if(get_is_low_power())
       digitalWrite(get_pin_power(),LOW);
    else
		digitalWrite(get_pin_power(),HIGH);
		
    set_wait_time(2000);
  }
}

void DHT22_Humidity::update_data()
{
  if (get_is_connected()) {
  	
    // if we use a digital pin to power the sensor...
    if (get_is_low_power())
    	digitalWrite(get_pin_power(),HIGH);  	

    // wait
    delay(get_wait_time());
        
    // recover errorCode to know if there was an error or not
    errorCode = dht->readData();
	
	if(errorCode == DHT_ERROR_NONE){
		// no error
		set_data((double)dht->getHumidity());
	}
	else {
		set_data((double)-1.0);
	}

    if (get_is_low_power())		
        digitalWrite(get_pin_power(),LOW);   
  }
  else {
  	// if not connected, set a random value (for testing)  	
  	if (has_fake_data()) 	
    	set_data((double)random(15, 90));
  }
}

double DHT22_Humidity::get_value()
{
  update_data();
  return get_data();
}
