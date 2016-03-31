------------------------------------
Low-cost LoRa gateway with Raspberry
------------------------------------

Fisrt install a Raspberry with Raspbian. Then install python packages such as requests, python-firebase,… as needed

Follow procedure to have Dropbox mounted on your Raspberry if you want to use this feature, otherwise, just create a Dropbox folder with a subforder LoRa-test that will be used locally. You can mount Dropbox later on if you want: the local folders and contents will be unchanged.

Create a folder named lora_gateway for instance then copy all the files of the distrib's Raspberry folder in it. DO NOT modify the lora_gateway.cpp file unless you know what you are doing. Check the radio.makefile file to select the radio module that you have. Uncomment only 1 choice or leave all lines commented if you do not have neither an HopeRF92/95 or inAir9B or a radio module with +20dBm possibility (the SX1272/76 has +20dBm feature but some radio modules that integrate the SX1272/76 may not have the electronic to support it). For instance, with both Libelium LoRa and inAir9 (not inAir9B) you should leave all lines commented. Then:

	> make lora_gateway

If you are using a Raspberry 2:

	> make lora_gateway_pi2

To launch the gateway

	> sudo ./lora_gateway

(or sudo ./lora_gateway_pi2)

By default, the gateway runs in LoRa mode 4 and has address 1.

To use post-processing with the provided ThingSpeak test channel

	> sudo ./lora_gateway | python ./post_processing_gw.py -t

To log processing output in a file (in ~/Dropbox/LoRa-test/post_processing_1.log)

	> sudo ./lora_gateway | python ./post_processing_gw.py -t | python ./log_gw
	
To additionally enforce application key at the gateway post-processing stage

	> sudo ./lora_gateway | python ./post_processing_gw.py -t --wappkey | python ./log_gw

This is the command that we recommend. To test, just flash a temperature sensor as described below and it should work out-of-the-box.

You can customize the post-processing stage (post_processing_gw.py) at your convenience later.

------------------------------------------------------------------------
An end-device example that periodically sends temperature to the gateway
------------------------------------------------------------------------

First, install the Arduino IDE 1.6.6. Then, in your sketch folder, copy the content of the Arduino folder of the distribution.

With the Arduino IDE, open the Arduino_LoRa_temp sketch, compile it and upload to an Arduino board.

The end-device has address 6 and run in LoRa mode 4. It will send data to the gateway.

The default configuration uses an application key set to [5, 6, 7, 8].

Use a temperature sensor (e.g. LM35DZ) and plugged in pin A0 (analog 0). You can use a power pin to power your temperature sensor if you are not concerned about power saving. Otherwise, you can use digital 8 (the sketch set this pin HIGH when reading value, then sets it back to LOW) and activate low power mode (uncomment #define LOW_POWER). You will need the LowPower library from RocketScream (https://github.com/rocketscream/Low-Power). Of course, this low power configuration is still very simple but it already can save lot's of energy for battery-operating mode. The radio module can be plugged in a 3V3 pin if available (which is always the case on MEGA, Due, Uno and Nano) and still be powered but it should drain only small amount of current in sleep mode. 

For low-power applications the Pro Mini from Sparkfun is certainly a good choice. This board can be either in the 5V or 3.3V version. With the Pro Mini, it is better to really use the 3.3V version running at 8MHz as power consumption will be reduced. Power for the radio module can take the output of a free digital pin that will be set to HIGH to power the radio module. In the temperature example, we use digital pin 9 set to HIGH to power the radio module with 3.3V (we have the 3.3V version) but you can use another pin. If you power your Pro Mini with the RAW pin you can use for instance 4 AA batteries to get 6V. Then you can get 3.3V on the VCC pin (which is powered by the on-board voltage regulator) instead of using a digital pin. If you use a rechargeable battery you can easily find 3.7V Li-Ion packs. In this case, you can inject directly into the VCC pin but make sure that you've unsoldered the power isolation jumper.

Depending on the sensor type, the computation to get the real temperature may be changed accordingly. Here is the instruction for the LM35DZ: http://www.instructables.com/id/ARDUINO-TEMPERATURE-SENSOR-LM35/

The default configuration also use the EEPROM to store the last packet sequence number in order to get it back when the sensor is restarted/rebooted. If you want to restart with a packet sequence number of 0, just comment the line "#define WITH_EEPROM"

Once flashed, the Arduino temperature sensor will send to the gateway the following message \!#3#20.4 (20.4 is the measured temperature so you may not have the same value) prefixed by the application key every 10 minutes (with some randomization interval). This will trigger at the processing stage of the gateway the logging on the default ThinkSpeak channel (the test channel we provide) in field 3. At the gateway, 20.4 will be recorded on the provided ThingSpeak test channel in field 3 of the channel. If you go to https://thingspeak.com/channels/66794 you should see the reported value. 

The program has been tested on Arduino Mega and Due with the Libelium Multi-Protocol radio shield to connect the LoRa radio module. We also tested on the Pro Mini, Nano and Teensy3.1/3.2 in which case we do not use the radio shield but simply connect the SPI pins of the radio module to the ones of the board. The SX1272 lib has been modified to change the SPI_SS pin from 2 to 10 when you compile for the Pro Mini, Nano or Teensy. Check on the web page our Pro Mini, Nano and Teensy version of the LoRa temperature sensor.

------------------------------------------------------------------------
An interactive end-device for sending LoRa messages with the Arduino IDE
------------------------------------------------------------------------

With the Arduino IDE, open the Arduino_LoRa_Gateway sketch and check that "#define IS_SEND_GATEWAY" is uncommented. Then compile it and upload to an Arduino board. It is better to use a more powerful (and with more RAM memory) Arduino platform for building the interactive device.

By default, the end-device have address 6 and runs in LoRa mode 4.

Enter "\!SGSH52UGPVAUYG3S#1#21.6" (without the quotes) in the input window and press RETURN

The command will be sent to the gateway and you should see the gateway pushing the data to the ThingSpeak test channel. If you go to https://thingspeak.com/channels/66794 you should see the reported value.

When testing with the interactive end-device, you should not use the --wappkey option for the post_processing_gw.py post-processing python script otherwise your command will not be accepted as only text string without logging services will be received and displayed when --wappkey is set.

	> sudo ./lora_gateway | python ./post_processing_gw.py -t | python ./log_gw

--------------------------------
Use an Arduino as a LoRa gateway
--------------------------------

The gateway can also be based on an Arduino board, as described in the web page. With the Arduino IDE, open the Arduino_LoRa_Gateway sketch and set the compilation #define to have IS_RCV_GATEWAY and not IS_SEND_GATEWAY. Compile the code and upload to an Arduino board. Then follow instructions on how to use the Arduino board as a gateway. It is better to use a more powerful (and with more RAM memory) Arduino platform for building the gateway.

-------
WARNING
-------

A/ There is currently no control on the transmit time for both gateway and end-device. When using the library to create devices, you have to ensure that the transmit time of your device is not exceeding the legal maximum transmit time defined in the regulation of your country.

B/ Although 900MHz band is supported (mostly for the US ISM band), the library does not implement the frequency hopping mechanism.


Enjoy!
C. Pham