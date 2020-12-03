/*
* Copyright (C) 2018 C. Pham, University of Pau, France
*
* Congduc.Pham@univ-pau.fr
*
*/

#include "SHT_Temperature.h"

SHT_Temperature::SHT_Temperature(char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power, uint8_t pin_trigger):Sensor(nomenclature,is_analog, is_connected, is_low_power, pin_read, pin_power, pin_trigger){
  if (get_is_connected()){

#ifdef SHT2x
    sht = new Sensirion(get_pin_read(), get_pin_trigger(), 0x40);
#else    
    sht = new Sensirion(get_pin_read(), get_pin_trigger());    
#endif
    
    pinMode(get_pin_power(),OUTPUT);
    
	if(get_is_low_power())
       digitalWrite(get_pin_power(),LOW);
    else
		digitalWrite(get_pin_power(),HIGH);
		
    set_warmup_time(1000);
  }
}

void SHT_Temperature::update_data()
{
  if (get_is_connected()) {
  	
    // if we use a digital pin to power the sensor...
    if (get_is_low_power())
    	digitalWrite(get_pin_power(),HIGH);  	

    // wait
    delay(get_warmup_time());
        
    float t;
    float h;
    int8_t ret=0;
    unsigned long mTime;
    unsigned long endTime;

    mTime=millis();
    endTime=mTime+1000;

    while ( (ret != S_Meas_Rdy) && (millis()<endTime) ) {
      ret=sht->measure(&t, &h);
    }

    if (ret == S_Meas_Rdy) 
      set_data((double)t);
    else 
      set_data((double)-100.0);
      
    if (get_is_low_power())		
        digitalWrite(get_pin_power(),LOW);   
  }
  else {
  	// if not connected, set a random value (for testing)  	
  	if (has_fake_data())	
    	set_data((double)random(-20, 40));
  }

}

double SHT_Temperature::get_value(){
  update_data();
  return get_data();
}
