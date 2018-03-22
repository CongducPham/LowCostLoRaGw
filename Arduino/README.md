Arduino example sketches
========================

This folder contains sketches for Arduino (and compatible) boards. The example sketches will show how simple, yet effective, low-cost LoRa IoT device can be programmed. For instance, they show how LoRa radio modules are configured and how a device can send sensed data to a gateway. They actually serve as template for future developments. On the Uno, Pro Mini, Mini, Nano, Teensy the mapping is as follows:

```
       Arduino      Radio module
         GND----------GND   (ground in)
         3V3----------3.3V  (3.3V in)
  SS pin D10----------NSS   (CS chip select in)
 SCK pin D13----------SCK   (SPI clock in)
MOSI pin D11----------MOSI  (SPI Data in)
MISO pin D12----------MISO  (SPI Data out)
```

On the MEGA, the SPI pin are as follows: 50 (MISO), 51 (MOSI), 52 (SCK). Starting from November 3rd, 2017, the CS pin is always pin number 10 on Arduino and Teensy boards. You can have a look at the [Low-cost-LoRa-IoT-step-by-step](https://github.com/CongducPham/tutorials/blob/master/Low-cost-LoRa-IoT-outdoor-step-by-step.pdf) tutorial in the tutorial repository (https://github.com/CongducPham/tutorials).

All the examples uses so-called LoRa mode 1 (BW125, CR45, SF12) at 865.2 Mhz (`CH_10_868`) to work with the default gateway configuration.

**`Arduino_LoRa_Ping_Pong`** shows a simple ping-pong communication between a LoRa device and a gateway by requesting an acknowlegment for data messages sent to the gateway. This example can serve as a simple range test as the device displays back the SNR of the received packet on the gateway.

**`Arduino_LoRa_Ping_Pong_LCD`** is an extended version that uses an OLED display to show in real-time the results of the range test. The pictures below show our simple range tester built from an Heltec ESP32 WiFi LoRa with OLED display. Plenty of information on this nice board are available from https://robotzero.one/heltec-wifi-lora-32/ and https://github.com/darthm0e/esp32-101 to name a few. 

![](https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/pingpong.png)

We also use a regular Arduino Pro Mini or Arduino Nano with an I2C 0.9inch OLED display (such as [this one]( https://fr.aliexpress.com/item/1pcs-0-96-blue-0-96-inch-OLED-module-New-128X64-OLED-LCD-LED-Display-Module/32643950109.html?spm=a2g0s.9042311.0.0.LbMu7r)). You can connect SDA and SCL to pin A4 and A5 respectively. Of course, connect your LoRa module as usual. For range test prior to deploying the sensor nodes, it is recommended to build the Ping-Pong tester with the case that you will use for your final end-devices because the impact of the case on transmission quality is important (see [this antenna presentation](https://www.youtube.com/watch?v=AhFy4-kForA&feature=youtu.be) from our colleague Fabien Ferrero). On the left part you can see an Arduino Nano using the Modtronix inAir9 radio module with the external antenna. On the right part, it is an Arduino Pro Mini with the integrated antenna provided by a simple PCB (see below the `Arduino_LoRa_GPS` description).

![](https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/pingpong_1.png)

**`Arduino_LoRa_Simple_temp`** illustrates how a simple LoRa device with temperature data can be flashed to an Arduino board. The example illustrates in a simple manner how to implement most of the features of a real IoT device: periodic sensing, transmission to gateway, duty-cycle and low-power mode to run on battery for months.
 
**`Arduino_LoRa_temp`** illustrates a more complex example with AES encryption and the possibility to send LoRaWAN packet. It can also open a receive window after every transmission to wait for downlink message coming from the gateway (uncomment `#define WTH_RCVW`). The template shows for instance how an '/@Ax#' command from the gateway can be parsed to set the node's address to 'x'. It can serve as a template for a more complex LoRa IoT device.

**`Arduino_LoRa_Simple_BeaconCollar`** is a simple beacon system to build a collar device intended for Cattle Rustling applications. The device will periodically send a beacon message with a sequence number to track at the gateway the beacon's RSSI and beacon losses to trigger alarms. Look at this [dedicated tutorial](https://github.com/CongducPham/tutorials/blob/master/Low-cost-LoRa-Collar.pdf) to build such as system.

**`Arduino_LoRa_GPS`** is a more elaborated GPS beacon system where a GPS module (UBlox 6M/7M/M8N) is used to get the coordinates of the collar device. The device will periodically send a beacon message with a sequence number and the GPS coordinates in the following format `\!BC/0/LAT/43.31408/LGT/-0.36362/FXT/4172`. This example can served as a basis for a tracking device that parses in a very light-weight manner the GPGGA NMEA message. Look at this [dedicated tutorial](https://github.com/CongducPham/tutorials/blob/master/Low-cost-LoRa-Collar.pdf) to build such as system. In the tutorial, the GPS device can use an open source PCB board designed by Fabien Ferrero from LEAT laboratory, University of Nice, France, to easily integrate an Arduino Pro Mini and an RFM95W radio module. The PCB has an integrated antenna to avoid external fragile part. The PCB Gerber layout file can be obtained from [https://github.com/FabienFerrero/UCA_Board](https://github.com/FabienFerrero/UCA_Board). Here is a [1-click order link](https://www.pcbway.com/project/shareproject/W48634ASK5_UCA_reverse.html) on PCBWAY.com. The latest PCB version with more holes for more connectivity options can be ordered from [here](https://www.pcbway.com/project/shareproject/UCA_Board.html).

This example also shows how a dedicated "cloud" task, `CloudGpsFile.py`, is implemented and enabled in `clouds.json` to process GPS messages from remote GPS devices and to compute their distance to the gateway. More details are provided in this [dedicated tutorial](https://github.com/CongducPham/tutorials/blob/master/Low-cost-LoRa-Collar.pdf).

**`Arduino_LoRa_Generic_Sensor`** is a very generic sensor template where a large variety of new physical sensors can be added. All physical sensors must be derived from a base Sensor class (defined in `Sensor.cpp` and `Sensor.h`) and should provide a `get_value()` and `get_nomenclature()` function. All the periodic task loop with duty-cycle low-power management is already there as in previous examples. Some predefined physical sensors are also already defined:

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
	
You can look at the provided examples to see how you can write a specific sensor class for a specific physical sensor. The previous simple temperature example `Arduino_LoRa_Simple_temp` can be obtained from the generic example by simply using a single sensor declaration with the LM35 class (i.e. `sensor_ptrs[0]`).	

**`Arduino_LoRa_InteractiveDevice`** is a tool that turns an Arduino board to an interactive device where a user can interactively enter data to be sent to the gateway. There are also many parameters that can dynamically be configured. This example can serve for test and debug purposes as well.

**`Arduino_LoRa_ucamII`** is the image IoT sensor device for multimedia sensing. Read [this specific page](http://cpham.perso.univ-pau.fr/WSN-MODEL/tool-html/imagesensor.html) and this [specific tutorial](https://github.com/CongducPham/tutorials/blob/master/Low-cost-LoRa-ImageIoT-step-by-step.pdf) for more informations on how to build and run the image sensor.

What Arduino boards are supported?
==================================

One main issue for a eligible board is to have a 3.3v pin to power the radio module. Some boards do not have such pin (Arduino Mini or Arduino Pro Mini in the 5V version for instance) and therefore although the software can be compiled and uploaded, connecting the radio module needs an additional voltage regulator that we want to avoid. Therefore, we exclusively use boards with built-in 3.3v pin. The following boards have been tested:

**Small form factor**

- Arduino Pro Mini (8MHz/3.3V version is better for power consumption), Arduino Nano (ATmega328 16MHz/5V but 3.3v pin), Arduino Micro (ATmega32u4 16MHz/5V but 3.3v pin)
- Teensy LC/31/32/35/36
- Ideetron Nexus (which is an ATmega328P at 3.3v, select as Arduino Mini in IDE)  
- Adafruit Feather32u4 and FeatherM0
- Expressif ESP8266 (only the **`Arduino_LoRa_Simple_temp`** has been modified to support the ESP8266)
- Expressif ESP32 (tested but support is not integrated into library, so you need to change the CS pin according to your board model)

**Uno/MEGA form factor**

- Arduino Uno, MEGA2560, Due
- Arduino Zero, M0 (require IDE 1.7.x from http://www.arduino.org)

For low-power and IoT integration desin, it is better to use either Arduino Pro Mini 8MHz/3.3v or the TeensyLC/31/32. On the Pro Mini, AES encryption can be used but then you may run out of RAM memory if you need additional library for specific physical sensors. The TeensyLC/31/32 has much more RAM memory, allowing much bigger project with AES encryption and many physical sensors. If you want to build an interactive device, it is better to use a MEGA2560 or Due or TeensyLC/31/32 because of memory limitation that may cause the lower memory boards to be unstable.

Note that there might be current limitation issues on some boards. The radio module can draw more than 50mA when transmitting (depending on the programmed transmission power) so the current limit of most microcontroller's analog/digital pins is reached. See for instance http://playground.arduino.cc/Main/ArduinoPinCurrentLimitations. It is better to connect the radio module VCC to a dedicated 3.3v pin that usually has higher current limit. However, it may be still not enough on some boards if you use PA_BOOST at 20dBm for your radio module. So be careful. For the MEGA, Due, and Teensy, they have a 3.3v pin with higher current limit so there should not be current issues with these boards.

What about other boards?
------------------------

Other boards may be supported as well with only very few modifications. However, for all the other boards, low-power features may need to use specific libraries depending on the microcontroller. If you have compilation issues, try to compile with low-power and EEPROM disabled:

	//#define LOW_POWER
	//#define WITH_EEPROM

Configuring the device
======================

Frequency
---------

There are a number of configuration options that can be either enabled or disabled by respectively uncommenting or commenting some definition. The first block that you can configure is for the radio regulation and the frequency band. By default, ETSI-based regulations for Short Range Devices (SRD) is used in the 868MHz frequency band. 

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

If you select `ETSI_EUROPE_REGULATION` then available channels are channels `CH_10_868` to `CH_18_868` and the default channel will be `CH_10_868`. If you select `SENEGAL_REGULATION` then available channels are channels `CH_04_868` to `CH_09_868` and the default channel will be `CH_04_868`. There are also regulation concerning the maximum transmit power which 14dBm (25mW) for ETSI and 10dBm (10mW) in Senegal as shown below: 

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
	
Although 900MHz band is supported (mostly for the US ISM band) by using `CH_05_900` as default channel, the library does not implement the frequency hopping mechanism nor the limited dwell time (e.g. 400ms per transmission). It is up to the programmer to ensure, by using appropriate LoRa settings and message size, that the 400ms limit is satisfied. Therefore, for the moment, `FCC_US_REGULATION` is not implemented.

Adapting frequency for your country
-----------------------------------

We can have a better view on the available license-free frequency bands by looking at the current specification of LoRaWAN which defines the set of default frequency channels for EU 863-870MHz ISM Band & EU 433MHz ISM Band, US 902-928MHz ISM Band, China 779-787MHz ISM Band & China 470-510MHz Band, Australia 915-928MHz ISM Band, South Korea 920-923MHz ISM Band and AS923 ISM Band channel frequencies. The last AS923 ISM Band applies to regions where the frequencies [923...923.5MHz] are comprised in the ISM band, which is the case for the following countries:

- Brunei [923-925 MHz]
- Cambodia [923-925 MHz]
- Hong Kong [920-925 MHz]
- Indonesia [923-925 MHz]
- Japan [920-928 MHz]
- Laos [923-925 MHz]
- Malaysia [919-923 MHz]
- New Zealand [915-928 MHz]
- Philippines [918-920 MHz]
- Singapore [920-925 MHz]
- Taiwan [922-928 MHz]
- Thailand [920-925 MHz]
- Vietnam [920-925 MHz]

You can easily redefine the default frequency for your country. For instance, if we take the case of Vietnam, you can select the 900MHz band by uncommenting BAND900:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// please uncomment only 1 choice
	//#define BAND868
	#define BAND900
	//#define BAND433
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	
and change the default channel to CH_08_900=920.36MHz as follows:

	#ifdef BAND868
	#ifdef SENEGAL_REGULATION
	const uint32_t DEFAULT_CHANNEL=CH_04_868;
	#else
	const uint32_t DEFAULT_CHANNEL=CH_10_868;
	#endif
	#elif defined BAND900
	const uint32_t DEFAULT_CHANNEL=CH_08_900; // <------- here
	#elif defined BAND433
	const uint32_t DEFAULT_CHANNEL=CH_00_433;
	#endif

If you want, you can also change for the gateway side in lora_gateway.cpp. First the band:

	// IMPORTANT
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// please uncomment only 1 choice
	//#define BAND868
	#define BAND900
	//#define BAND433
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

then the default channel declaration:

	#ifdef BAND868
	#define MAX_NB_CHANNEL 15
	#define STARTING_CHANNEL 4
	#define ENDING_CHANNEL 18
	#ifdef SENEGAL_REGULATION
	uint8_t loraChannelIndex=0;
	#else
	uint8_t loraChannelIndex=6;
	#endif
	uint32_t loraChannelArray[MAX_NB_CHANNEL]={CH_04_868,CH_05_868,CH_06_868,CH_07_868,CH_08_868,CH_09_868,
												CH_10_868,CH_11_868,CH_12_868,CH_13_868,CH_14_868,CH_15_868,CH_16_868,CH_17_868,CH_18_868};

	#elif defined BAND900 
	#define MAX_NB_CHANNEL 13
	#define STARTING_CHANNEL 0
	#define ENDING_CHANNEL 12
	uint8_t loraChannelIndex=8; // <------- here
	uint32_t loraChannelArray[MAX_NB_CHANNEL]={CH_00_900,CH_01_900,CH_02_900,CH_03_900,CH_04_900,CH_05_900,CH_06_900,CH_07_900,CH_08_900,
												CH_09_900,CH_10_900,CH_11_900,CH_12_900};
	#elif defined BAND433
	#define MAX_NB_CHANNEL 4
	#define STARTING_CHANNEL 0
	#define ENDING_CHANNEL 3
	uint8_t loraChannelIndex=0;
	uint32_t loraChannelArray[MAX_NB_CHANNEL]={CH_00_433,CH_01_433,CH_02_433,CH_03_433};                                              
	#endif

However, it is **highly** advised to rather change the frequency setting at the gateway by editing the `gateway_conf.json` file:

	"radio_conf" : {
		"mode" : 1,
		"bw" : 500,
		"cr" : 5,
		"sf" : 12,
		"ch" : -1,
		"freq" : 920.36
	},	

PA_BOOST
--------

Depending on your radio module, you have to indicate whether PA_BOOST should be used or not. Uncomment if your radio is an HopeRF RFM92W, HopeRF RFM95W, Modtronix inAir9B, NiceRF1276 or if you known from the circuit diagram that output use the PA_BOOST line instead of the RFO line. Many boards uses the RFM95W (Adafruit LoRa Feather boards, Dragino LoRa shield/hat, Loranga board for instance). In this case, they behaves like an RFM95W so they need PA_BOOST.
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// uncomment if your radio is an HopeRF RFM92W, HopeRF RFM95W, Modtronix inAir9B, NiceRF1276
	// or you known from the circuit diagram that output use the PABOOST line instead of the RFO line
	//#define PABOOST
	/////////////////////////////////////////////////////////////////////////////////////////////////////////// 	

LoRa mode
---------

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
| 9    | 500|  8 |
| 10   | 500|  7 |	

Low-power
---------

Low-power management is handled by various low-power libraries. For Arduino boards, it is the `LowPower` library from Rocket Scream. For TeensyLC/31/32, it is the `Snooze` library. As indicated previously, if you use another board, you may need to use a specific low-power library, or disable low-power management. By default in the example, the device will wake up every 10 minutes. You can changed it to another value, expressed in minutes. If low power is not used, the duty-cycle behavior will still be implemented but with the `delay()` function which is not energy-efficient.

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

AES encryption can be enabled in `Arduino_LoRa_temp` and `Arduino_LoRa_Generic_Sensor` with the following define statement:
	
	#define WITH_AES

Please refer to the [README-aes_lorawan](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-aes_lorawan.md) file.

Receive window
--------------

A receive window can be enabled in `Arduino_LoRa_temp` with the following define statement:
	
	#define WITH_RCVW
	
A receive window is opened after every transmission to wait for downlink message coming from the gateway. The template shows for instance how an '/@Ax#' command can be parsed to set the node's address to 'x'.	Note that the node's address can be saved in EEPROM so that a reset of the board will keep the new address. To do so, you have to comment the line:

	//#define FORCE_DEFAULT_VALUE
	
For more information on dynamic configuration of end-device, please read carefully the information in the `Arduino_LoRa_temp` template.	
	
For more information on how to perform downlink transmission from the gateway, please refer to the following [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-downlink.md).

LoRaWAN support
---------------

Limited LoRaWAN support is provided in `Arduino_LoRa_temp` and `Arduino_LoRa_Generic_Sensor` with the following define statements:

	#define LORAWAN
	
Please refer to the [README-aes_lorawan](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-aes_lorawan.md) file.

Using the RadioHead library for the end-device
----------------------------------------------

Although not recommended as you won't benefit from the advanced features proposed by our library, it is possible to use the RadioHead library to build an end-device that can send data to our gateway. The radio driver is `RH_RF95` which is primarily intended for the HopeRF RFM95/96/97/98 but other SX1276/77/78/79-based radio module will work. For instance both the Modtronix inAir9 and inAir9B. Care must be taken to correctly define the PA_BOOST support. Both RFM95W and inAir9B need PA_BOOST while the inAir9 uses the RFO instead. Also, the RadioHead library needs the radio's DIO 0 pin to be connected to an interrupt-enabled pin of the Arduino board. The RadioHead lib can be downloaded from [http://www.airspayce.com/mikem/arduino/RadioHead/](http://www.airspayce.com/mikem/arduino/RadioHead/).

Our library uses the following 4-byte header: `dst(1B)` `ptype(1B)` `src(1B)` `seq(1B)`. `ptype` is decomposed in 2 parts `type(4bits)` and `flags(4bits)`; `type` can take current value of `DATA=0001` and `ACK=0010`. The flags are from left to right: `ack_requested|encrypted|with_appkey|is_binary`. Fortunately the RH lib also use a 4-byte header: `to`, `from`, `id`, `flags`. It is therefore easy to have the following correspondance: `to <-> dst` and `from <-> src`; and use `id` for `ptype` and `flags` for `seq`. However, in RH_RF95, the field insertion order is: `to`, `from`, `id`, `flags` so to avoid changing the RH lib, we chose to use the following mapping:
 
```
   to <-> dst
 from <-> ptype
   id <-> src
flags <-> seq 
``` 

These variables will be set using the `RHDatagram.h` functions: `setHeaderTo`, `setHeaderfrom`, `setHeaderId`, `setHeaderFlags`. The **`Arduino_LoRa_Radiohead_Example`** provides an example sending "hello" using so-called LoRa mode 1 (BW125, CR45, SF12) on 865.2Mhz. PA_BOOST is activated by default and the DI0 pin is connected to Arduino digital pin 2.

