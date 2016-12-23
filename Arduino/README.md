Arduino example sketches
========================

This folder contains sketches for Arduino (and compatible) boards. The example sketches will show how simple, yet effective, low-cost LoRa IoT device can be programmed. For instance, they show how LoRa radio modules are configured and how a device can send sensed data to a gateway. They actually serve as template for future developments. 

**Arduino_LoRa_Ping_Pong** shows a simple ping-pong communication between a LoRa device and a gateway by requesting an acknowlegment for data messages sent to the gateway.

**Arduino_LoRa_Simple_temp** illustrates how a simple LoRa device with temperature data can be flashed to an Arduino board. The example illustrates in a simple manner how to implement most of the features of a real IoT device: periodic sensing, transmission to gateway, duty-cycle and low-power mode to run on battery for months.
 
**Arduino_LoRa_temp** illustrates a more complex example by adding custom Carrier Sense mechanism that you can easily change, AES encryption and the possibility to send LoRaWAN packet. It can serve as a template for a more complex LoRa IoT device.

**Arduino_LoRa_Generic_Sensor** is a very generic sensor template where a large variety of new physical sensors can be added. All physical sensors must be derived from a base Sensor class (defined in Sensor.cpp and Sensor.h) and should provide a get_value() and get_nomenclature() function. All the periodic task loop with duty-cycle low-power management is already there as in previous examples. Some predefined physical sensors are also already defined:

- very simple LM35DZ analog temperature sensor
- digital DHT22 temperature and humidity sensor
- digital DS18B20 temperature sensor
- ultra-sonic HC-SR04 distance sensor
- Davies Leaf Wetness sensor
- general raw analog sensor

You just have to define the number of physical sensors you want to consider:

	// SENSORS DEFINITION 
	//////////////////////////////////////////////////////////////////
	// CHANGE HERE THE NUMBER OF SENSORS, SOME CAN BE NOT CONNECTED
	const int number_of_sensors = 7;
	//////////////////////////////////////////////////////////////////

Add the sensors:

	//////////////////////////////////////////////////////////////////
	// ADD YOUR SENSORS HERE   
	// Sensor(nomenclature, is_analog, is_connected, is_low_power, pin_read, pin_power, pin_trigger=-1)
	  sensor_ptrs[0] = new LM35("tc", IS_ANALOG, IS_CONNECTED, low_power_status, (uint8_t) A0, (uint8_t) 8);
	  sensor_ptrs[1] = new DHT22_Temperature("TC", IS_NOT_ANALOG, IS_CONNECTED, low_power_status, (uint8_t) 3, (uint8_t) 9);
	  sensor_ptrs[2] = new DHT22_Humidity("HU", IS_NOT_ANALOG, IS_CONNECTED, low_power_status, (uint8_t) 3, (uint8_t) 9);
	  sensor_ptrs[3] = new LeafWetness("lw", IS_ANALOG, IS_NOT_CONNECTED, low_power_status, (uint8_t) A2, (uint8_t) 7);
	  sensor_ptrs[4] = new DS18B20("DS", IS_NOT_ANALOG, IS_NOT_CONNECTED, low_power_status, (uint8_t) 4, (uint8_t) 7);
	  sensor_ptrs[5] = new rawAnalog("SH", IS_ANALOG, IS_CONNECTED, low_power_status, (uint8_t) A1, (uint8_t) 6);
	  sensor_ptrs[6] = new HCSR04("DIS", IS_NOT_ANALOG, IS_CONNECTED, low_power_status, (uint8_t) 39, (uint8_t) 41, (uint8_t) 40);
	
	  // for non connected sensors, indicate whether you want some fake data, for test purposes for instance
	  sensor_ptrs[3]->set_fake_data(true);
	  sensor_ptrs[4]->set_fake_data(true); 
	
	//////////////////////////////////////////////////////////////////  	
	
For each physical sensor, you have to indicate a nomemclature string, whether it is analog or not, whether it is connected to your board or not, whether its power pin is regulated by low-power management or not, the read pin, the power pin and an optional trigger pin (see the HCSR04 sensor). See how sensors that are not connected can provide emulated data for test purposes. 

The control loop that is periodically executed is as follows:

	char final_str[80] = "\\!";
	char aux[6] = "";
	
	for (int i=0; i<number_of_sensors; i++) {
	
	    if (sensor_ptrs[i]->get_is_connected() || sensor_ptrs[i]->has_fake_data()) {
	      
	        ftoa(aux, sensor_ptrs[i]->get_value(), 2);
	    
	        if (i==0) {
	            sprintf(final_str, "%s%s/%s", final_str, sensor_ptrs[i]->get_nomenclature(), aux);
	        } 
	        else {
	            sprintf(final_str, "%s/%s/%s", final_str, sensor_ptrs[i]->get_nomenclature(), aux);
	        }
	    }
	}
      
With the declared sensors, the transmitted data string can be as follows:

	\!tc/19.52/TC/19.62/HU/45/lw/3.53/DS/19.8/SH/500.56/DIS/56.78      
	
You can look at the provided examples to see how you can write a specific sensor class for a specific physical sensor. The previous simple temperature example Arduino_LoRa_Simple_temp can be obtained from the generic example by simply using a single sensor declaration with the LM35 class (i.e. sensor_ptrs[0]).	

**Arduino_LoRa_InteractiveDevice** is a tool that turns an Arduino board to an interactive device where a user can interactively enter data to be sent to the gateway. There are also many parameters that can dynamically be configured. This example can serve for test and debug purposes as well.

What Arduino boards are supported?
==================================

One main issue for a eligible board is to have a 3.3v pin to power the radio module. Some boards do not have such pin (Arduino Mini or Arduino Pro Mini in the 5V version for instance) and therefore although the software can be compiled and uploaded, connecting the radio module needs an additional voltage regulator that we want to avoid. Therefore, we exclusively use boards with built-in 3.3v pin. The following boards have been tested:

**Small form factor**

- Arduino Pro Mini (8MHz/3.3V version), Arduino Nano (ATmega328 16MHz/5V but 3.3v pin), Arduino Micro (ATmega32u4 16MHz/5V but 3.3v pin)
- Teensy LC/31/32/35/36
- Ideetron Nexus (which is an ATmega328P at 3.3v, select as Arduino Mini in IDE)  
- Adafruit Feather 32u4 and Feather M0

**Uno/MEGA form factor**

- Arduino Uno, MEGA2560, Due
- Arduino Zero, M0 (require IDE 1.7.x from http://www.arduino.org)

For low-power and IoT integration desin, it is better to use either Arduino Pro Mini 8MHz/3.3v or the TeensyLC/31/32. On the Pro Mini, AES encryption can be used but then you may run out of RAM memory if you need additional library for specific physical sensors. The TeensyLC/31/32 has much more RAM memory, allowing much bigger project with AES encryption and many physical sensors. If you want to build an interactive device, it is better to use a MEGA2560 or Due or TeensyLC/31/32 because of memory limitation that may cause the lower memory boards to be unstable.

Note that there might be current limitation issues on some boards. The radio module can draw more than 50mA when transmitting (depending on the programmed transmission power) so the current limit of most microcontroller's analog/digital pins is reached. See for instance http://playground.arduino.cc/Main/ArduinoPinCurrentLimitations. It is better to connect the radio module VCC to a dedicated 3.3v pin that usually has higher current limit. However, it may be still not enough on some boards if you use PA_BOOST at 20dBm for your radio module. So be careful. For the MEGA, Due, and Teensy, they have a 3.3v pin with higher current limit so there should not be current issues with these boards.

What about other boards?
------------------------

Other boards may be supported as well with only very few modifications. It has been reported that the ESP8266 works fine. However, for all the other boards, low-power features may need to use specific libraries depending on the microcontroller. If you have compilation issues, try to compile with low-power disabled:

	//#define LOW_POWER

Configuring the device
======================

There are a number of configuration options that can be either enabled or disabled by respectively uncommenting or commenting some definition. The first bloc that you can configure is for the radio regulation and the frequency band. By default, ETSI-based regulations for Short Range Devices (SRD) is used in the 868MHz frequency band. 

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// please uncomment only 1 choice
	//
	#define ETSI_EUROPE_REGULATION
	//#define FCC_US_REGULATION
	//#define SENEGAL_REGULATION
	/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// please uncomment only 1 choice
	#define BAND868
	//#define BAND900
	//#define BAND433
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	
Here is a reminder of the pre-defined radio frequency channels in the library:

| ch | F(MHz) | ch | F(MHz) | ch | F(MHz) |
|----|--------|----|--------|----|--------|
| 04 | 863.2* | 00 | 903.08 | 00 | 433.3* |
| 05 | 863.5* | 01 | 905.24 | 01 | 433.6* |
| 06 | 863.8* | 02 | 907.40 | 02 | 433.9* |
| 07 | 864.1* | 03 | 909.56 | 03 | 434.3* |
| 08 | 864.4* | 04 | 911.72 |  - |   -    |
| 09 | 864.7* | 05 | 913.88 |  - |   -    |
| 10 | 865.2  | 06 | 916.04 |  - |   -    |
| 11 | 865.5  | 07 | 918.20 |  - |   -    |
| 12 | 865.8  | 08 | 920.36 |  - |   -    |
| 13 | 866.1  | 09 | 922.52 |  - |   -    |
| 14 | 866.4  | 10 | 924.68 |  - |   -    |
| 15 | 867.7  | 11 | 926.84 |  - |   -    |
| 16 | 867.0  | 12 | 915.00 |  - |   -    |
| 17 | 868.0  |  - |   -    |  - |   -    |
| 18 | 868.1* |  - |   -    |  - |   -    |
|  - |   -    |  - |   -    |  - |   -    |

If you select ETSI_EUROPE_REGULATION then available channels are channel CH_10_868 to CH_18_868 and the default channel will be CH_10_868. If you select SENEGAL_REGULATION then available channels are channel CH_04_868 to CH_09_868 and the default channel will be CH_04_868. There are also regulation concerning the maximum transmit power which 14dBm (25mW) for ETSI and 10dBm (10mW) in Senegal as shown below: 

	#ifdef ETSI_EUROPE_REGULATION
	#define MAX_DBM 14
	#elif defined SENEGAL_REGULATION
	#define MAX_DBM 10
	#endif
	
	#ifdef BAND868
	#ifdef SENEGAL_REGULATION
	const uint32_t DEFAULT_CHANNEL=CH_04_868;
	#else
	const uint32_t DEFAULT_CHANNEL=CH_10_868;
	#endif
	#elif defined BAND900
	const uint32_t DEFAULT_CHANNEL=CH_05_900;
	#elif defined BAND433
	const uint32_t DEFAULT_CHANNEL=CH_00_433;
	#endif
	
Although 900MHz band is supported (mostly for the US ISM band) by using CH_05_900 as default channel, the library does not implement the frequency hopping mechanism nor the limited dwell time (e.g. 400ms per transmission). Therefore, for the moment, FCC_US_REGULATION is not implemented.

Then, depending on your radio module, you have to indicate whether PA_BOOST should be used or not. Uncomment if your radio is an HopeRF RFM92W, HopeRF RFM95W, Modtronix inAir9B, NiceRF1276 or if you known from the circuit diagram that output use the PA_BOOST line instead of the RFO line.
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// uncomment if your radio is an HopeRF RFM92W, HopeRF RFM95W, Modtronix inAir9B, NiceRF1276
	// or you known from the circuit diagram that output use the PABOOST line instead of the RFO line
	//#define PABOOST
	/////////////////////////////////////////////////////////////////////////////////////////////////////////// 	

The next usefull configuration block defines the LoRa mode and the node address. If you deploy several sensor nodes then node address should be different from one sensor to another. For the moment, the address is between 2 and 255 included. 0 is for broadcast and 1 for the gateway.
	
	///////////////////////////////////////////////////////////////////
	// CHANGE HERE THE LORA MODE, NODE ADDRESS 
	#define LORAMODE  1
	#define node_addr 10
	//////////////////////////////////////////////////////////////////
	
Here is a reminder of the pre-defined LoRa modes as defined by the library.

| mode | BW | SF |
|------|----|----|
| 1    | 125| 12 |
| 2    | 250| 12 |
| 3    | 125| 10 |
| 4    | 500| 12 |
| 5    | 250| 10 |
| 6    | 500| 11 |
| 7    | 250|  9 |
| 8    | 500|  9 |
| 9    | 500|  7 |
| 10   | 500|  8 |	

Low-power
---------

Low-power management is handled by various low-power libraries. For Arduino boards, it is the LowPower library from Rocket Scream. For TeensyLC/31/32, it is the Snooze library. As indicated previously, if you use another board, you may need to use a specific low-power library, or disable low-power management. By default in the example, the device will wake up every 10 minutes. You can changed it to another value, expressed in minutes. If low power is not used, the duty-cycle behavior will still be implemented but with the delay() function which is not energy-efficient.

	#define LOW_POWER
	#define LOW_POWER_HIBERNATE

	///////////////////////////////////////////////////////////////////
	// CHANGE HERE THE TIME IN MINUTES BETWEEN 2 READING & TRANSMISSION
	unsigned int idlePeriodInMin = 10;
	///////////////////////////////////////////////////////////////////	
	
	
Using EEPROM
------------	

Most of Arduino boards's microcontroller do have built-in EEPROM, except for the Arduino Due and Zero. When EEPROM is available, then all the examples store the last sequence number so that a reset or a reboot will keep the correct sequence number. You can disable this feature by commenting the EEPROM define statement:

	#define WITH_EEPROM

Also, when your micro controller does not have built-in EEPROM, you should disable this feature to avoid compilation errors.

AES encryption
--------------

AES encryption can be enabled in Arduino_LoRa_temp and Arduino_LoRa_Generic_Sensor with the following define statement:
	
	#define WITH_AES

Please refer to the README file in the gw_advanced/aes_lorawan folder.

LoRaWAN support
---------------

Limited LoRaWAN support is provided in Arduino_LoRa_temp and Arduino_LoRa_Generic_Sensor with the following define statements:

	#define LORAWAN
	
Please refer to the README file in the gw_advanced/aes_lorawan folder.
