#include "my_demo_sensor_code.h"

///////////////////////////////////////////////////////////////////
// CHANGE HERE THE NOMEMCLATURE, HERE TC WOULD MEAN TEMPERATURE IN CELCIUS FOR INSTANCE
// USE A MAXIMUM OF 3 CHARACTERS 

char nomenclature_str[4]="TC";
//////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// ADD HERE SOME INITIALIZATION CODE
// HERE WE JUST DECLARE VALUE_PIN_READ AS INPUT PIN

void sensor_Init() {

  // for the temperature sensor
  pinMode(PIN_READ, INPUT);
  pinMode(PIN_POWER, OUTPUT);
}
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// CHANGE HERE THE WAY YOU READ A VALUE FROM YOUR SPECIFIC SENSOR
// HERE IT IS AN EXAMPLE WITH THE LM35DZ SIMPLE ANALOG TEMPERATURE SENSOR

double sensor_getValue() {

  //power the sensor
  digitalWrite(PIN_POWER, HIGH);
  // wait a bit
  delay(500);

  //read the raw sensor value
	int value = analogRead(PIN_READ);

  //power down the sensor
  digitalWrite(PIN_POWER, LOW);
	
	Serial.print(F("Reading "));
	Serial.println(value);

  double sensor_value;
  
  // change here how the temperature should be computed depending on your sensor type
  //  
  //LM35DZ
  sensor_value = (value*VOLTAGE_SCALE/1024.0)/10;

  //TMP36
  //the TMP36 can work with supply voltage of 2.7v-5.5v
  //can be used on 3.3v board
  //we use a 0.95 factor when powering with less than 3.3v, e.g. 3.1v in the average for instance
  //this setting is for 2 AA batteries           
	//sensor_value=((value*0.95*VOLTAGE_SCALE/1024.0)-500)/10;
	
  //set a defined value for testing
  //sensor_value = 22.5;	
    
  return sensor_value;
}
///////////////////////////////////////////////////////////////////
