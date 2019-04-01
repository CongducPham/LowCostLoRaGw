#include "my_DHT_sensor_code.h"

///////////////////////////////////////////////////////////////////
// CHANGE HERE THE NOMEMCLATURE, HERE TC WOULD MEAN TEMPERATURE IN CELCIUS FOR INSTANCE
// USE A MAXIMUM OF 3 CHARACTERS 

char nomenclature_str[4]="TC";
//////////////////////////////////////////////////////////////////

DHT dht(PIN_READ, DHTTYPE);

///////////////////////////////////////////////////////////////////
// ADD HERE SOME INITIALIZATION CODE
// HERE WE JUST DECLARE VALUE_PIN_READ AS INPUT PIN

void sensor_Init() {

  // for the temperature sensor
  pinMode(PIN_READ, INPUT);
  pinMode(PIN_POWER, OUTPUT);
  //dht.begin();
}
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// CHANGE HERE THE WAY YOU READ A VALUE FROM YOUR SPECIFIC SENSOR
// HERE IT IS AN EXAMPLE WITH THE DHT22

double sensor_getValue() {

  dht.begin();
  delay(2000);
  
  //read the raw sensor value
  //float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return -1;
  }
  else {         
    Serial.print("Temperature: ");
    Serial.println(t);
    //Serial.print(" degrees Celcius Humidity: ");
    //Serial.print(h);
    //Serial.println("%");
  }    	
    
  return t;
}
///////////////////////////////////////////////////////////////////
