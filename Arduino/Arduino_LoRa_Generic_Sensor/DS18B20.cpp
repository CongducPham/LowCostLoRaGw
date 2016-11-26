/*
* Copyright (C) 2016 Congduc Pham, University of Pau, France
*
* Congduc.Pham@univ-pau.fr
*/

#include "DS18B20.h"

DS18B20::DS18B20(char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power):Sensor(nomenclature, is_analog, is_connected, is_low_power, pin_read, pin_power){
  if (get_is_connected()){
    // start OneWire
    ds = new OneWire(get_pin_read());

    // to power the DS18B20
    pinMode(get_pin_power(),OUTPUT);
      
	if(get_is_low_power())
       digitalWrite(get_pin_power(),LOW);
    else
		digitalWrite(get_pin_power(),HIGH);
      
      set_wait_time(1000);
  }
}

void DS18B20::update_data()
{	
  double temp;
  	
  if (get_is_connected()) {
  	
    // if we use a digital pin to power the sensor...
    if (get_is_low_power())
    	digitalWrite(get_pin_power(),HIGH);  	

    // wait
    delay(get_wait_time());
        
	// copied & adapted from https://skyduino.wordpress.com/2012/04/26/arduino-capteur-de-temperature-ds18b20/
	    
	byte data[9], addr[8];
	// data : data read from the scratchpad
	// addr : 1-Wire addr of the module
	
	if (!ds->search(addr)) { // search for a 1-wire module
	  ds->reset_search();    // reinit the search procedure
	  temp=-1.0;         	// error
	}
	 
	if (OneWire::crc8(addr, 7) != addr[7]) // check that the addr has been correctly received
	  temp=-1.0;                        
	
	if (addr[0] != DS18B20_addr) // check that it is our DS18B20
	  temp=-1.0;         
	  	
	ds->reset();             // reset the 1-Wire
	ds->select(addr);        // select our DS18B20
	 
	ds->write(0x44, 1);      // start the measure
	delay(800);             // wait for the measure
		 
	ds->reset();             // reset the 1-Wire
	ds->select(addr);        // select our DS18B20
	ds->write(0xBE);         // read from scratchpad
	
	for (byte i = 0; i < 9; i++) // read the content of the scratchpad
	  data[i] = ds->read();       // and store the bytes
	 
	// Temp in Celsius computation
	temp = ((data[1] << 8) | data[0]) * 0.0625; 
      
    if (get_is_low_power())
        digitalWrite(get_pin_power(),LOW);
        
	set_data(temp);
  }
  else { 
  	// if not connected, set a random value (for testing)  	
  	if (has_fake_data())
  		set_data((double)random(-20, 40));
  }
}

double DS18B20::get_value()
{
  update_data();
  return get_data();
}
