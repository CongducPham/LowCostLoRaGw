/*
* Copyright (C) 2018 C. Pham, University of Pau, France
*
* Congduc.Pham@univ-pau.fr
*
*/

#include "SHT1X.h"
#include "SHT_Humidity.h"

SHT_Humidity::SHT_Humidity(char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power, uint8_t pin_trigger):Sensor(nomenclature, is_analog, is_connected, is_low_power, pin_read, pin_power, pin_trigger){
  if (get_is_connected()){
    // start library DHT
    sht = new SHT1x(get_pin_read(), get_pin_trigger());
    
    pinMode(get_pin_power(),OUTPUT);
    
	if(get_is_low_power())
       digitalWrite(get_pin_power(),LOW);
    else
		digitalWrite(get_pin_power(),HIGH);
		
    set_wait_time(2000);
  }
}

void SHT_Humidity::update_data()
{
  if (get_is_connected()) {
  	
    // if we use a digital pin to power the sensor...
    if (get_is_low_power())
    	digitalWrite(get_pin_power(),HIGH);  	

    // wait
    delay(get_wait_time());

	set_data((double)sht->readHumidity());

    if (get_is_low_power())		
        digitalWrite(get_pin_power(),LOW);   
  }
  else {
  	// if not connected, set a random value (for testing)  	
  	if (has_fake_data()) 	
    	set_data((double)random(15, 90));
  }
}

double SHT_Humidity::get_value()
{
  update_data();
  return get_data();
}
