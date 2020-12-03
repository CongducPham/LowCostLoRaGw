/*
* Copyright (C) 2016 Nicolas Bertuol, University of Pau, France
*
* nicolas.bertuol@etud.univ-pau.fr
*
* modified by Congduc Pham, University of Pau, France
*/

#include "DHT22_Temperature.h"

DHT22_Temperature::DHT22_Temperature(char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power):Sensor(nomenclature,is_analog, is_connected, is_low_power, pin_read, pin_power){
  if (get_is_connected()){
    // start library DHT
    //dht = new DHT22(get_pin_read());
    dht = new DHT(get_pin_read(), DHT22);
    
    pinMode(get_pin_power(),OUTPUT);
    
	if(get_is_low_power())
       digitalWrite(get_pin_power(),LOW);
    else
		digitalWrite(get_pin_power(),HIGH);
		
    set_warmup_time(2300);
  }
}

void DHT22_Temperature::update_data()
{
  if (get_is_connected()) {

    double t = dht->readTemperature();

    if (isnan(t))
      set_data((double)-1.0);
    else
      set_data(t);

    /*
    // recover errorCode to know if there was an error or not
    errorCode = dht->readData();
	
	  if(errorCode == DHT_ERROR_NONE){
		  // no error
		  set_data((double)dht->getTemperatureC());
	  }
	  else {
	  	set_data((double)-1.0);
	  }
    */ 
  }
  else {
  	// if not connected, set a random value (for testing)  	
  	if (has_fake_data())	
    	set_data((double)random(-20, 40));
  }

}

double DHT22_Temperature::get_value()
{
  uint8_t retry=4;
  Serial.println("temp");
  
  // if we use a digital pin to power the sensor...
  if (get_is_low_power())
    digitalWrite(get_pin_power(),HIGH);

  dht->begin();
    
  do { 

    // wait
    delay(get_warmup_time());
        
    update_data();
    Serial.println(get_data());

    retry--;
    
  } while (retry && get_data()==-1.0);

  if (get_is_low_power())    
    digitalWrite(get_pin_power(),LOW);  
    
  return get_data();
}
