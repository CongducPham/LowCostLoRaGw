------------------------------------
Low-cost LoRa gateway with Raspberry
------------------------------------

---------------------------------
Install Raspbian Wheezy or Jessie
---------------------------------

Fisrt install a Raspberry with Raspbian, Jessie is recommended. 

then

	> sudo apt-get update
	> sudo apt-get upgrade

Jessie has been tested on RPI2 and RPI3 and works great.

Wheezy has been tested on RPI1 and RPI2 and works great. Wheezy on RPI3 is not recommended because built-in WiFi and Bluetooth will not work properly.

We recommend buying either RPI2 or RPI3. RPI3 with Jessie has built-in WiFi and Bluetooth so it is definitely a good choice. In addition RPI3 with Jessie will have a better support lifetime. 

Create a "Dropbox" folder in your home directory with a subforder "LoRa-test" that will be used locally. Please put attention to the name of the folder: they must be "Dropbox/LoRa-test" because the "post_processing_gw.py" Python script uses these paths. You can mount Dropbox later on (see below) if you want: the local folders and contents will be unchanged.

Create a folder named "lora_gateway" for instance then copy all the files of the distrib's Raspberry folder in it. DO NOT modify the lora_gateway.cpp file unless you know what you are doing. Check the radio.makefile file to select the radio module that you have. Uncomment only 1 choice or leave all lines commented if you do not have neither an HopeRF92/95 or inAir9B or a radio module with +20dBm possibility (the SX1272/76 has +20dBm feature but some radio modules that integrate the SX1272/76 may not have the electronic to support it). For instance, with both Libelium LoRa and inAir9 (not inAir9B) you should leave all lines commented. Then:

	> make lora_gateway

If you are using a Raspberry 2:

	> make lora_gateway_pi2

To launch the gateway

	> sudo ./lora_gateway

On Raspberry 2 a symbolic link will be created that will point to lora_gateway_pi2.

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

With the Arduino IDE, open the Arduino_LoRa_temp sketch (or the more simpler Arduino_LoRa_Simple_temp), compile it and upload to an Arduino board.

The end-device has address 6 and run in LoRa mode 4 for Arduino_LoRa_temp and address 8 with LoRa mode 1 for Arduino_LoRa_Simple_temp. It will send data to the gateway.

The default configuration uses an application key set to [5, 6, 7, 8].

Use a temperature sensor (e.g. LM35DZ) and plugged in pin A0 (analog 0). You can use a power pin to power your temperature sensor if you are not concerned about power saving. Otherwise, you can use digital 8 (the sketch set this pin HIGH when reading value, then sets it back to LOW) and activate low power mode (uncomment #define LOW_POWER), see below. 

For low-power applications the Pro Mini from Sparkfun is certainly a good choice. This board can be either in the 5V or 3.3V version. With the Pro Mini, it is better to really use the 3.3V version running at 8MHz as power consumption will be reduced. Power for the radio module can be obtained from the VCC pin which is powered in 3.3v when USB power is used or when unregulated power is connected to the RAW pin. If you power your Pro Mini with the RAW pin you can use for instance 4 AA batteries to get 6V. If you use a rechargeable battery you can easily find 3.7V Li-Ion packs. In this case, you can inject directly into the VCC pin but make sure that you've unsoldered the power isolation jumper, see Pro Mini schematic on the Arduino web page. To save more power, you can remove the power led.

The current low-power version for Arduino board use the RocketScream Low Power library (https://github.com/rocketscream/Low-Power) and can support most Arduino platforms although the Pro Mini platform will probably exhibit the best energy saving (we measured 124uA current in sleep mode with the power led removed). You can buid the low-power version by uncommenting the LOW_POWER compilation define statement. Then set "int idlePeriodInMin = 10;" to the number of minutes between 2 wake-up. By default it is 10 minutes.

For the special case of Teensy 31/32, the power saving mode uses the Snooze library provided by the Teensyduino package. You can upgrade the Snooze library from their github. With the Teensy, you can further use the HIBERNATE mode by uncommenting "USE_HIBERNATE" in the Snooze.h file of the Snooze libary and uncommenting LOW_POWER_HIBERNATE in the temperature example.

Depending on the sensor type, the computation to get the real temperature may be changed accordingly. Here is the instruction for the LM35DZ: http://www.instructables.com/id/ARDUINO-TEMPERATURE-SENSOR-LM35/

The default configuration also use the EEPROM to store the last packet sequence number in order to get it back when the sensor is restarted/rebooted. If you want to restart with a packet sequence number of 0, just comment the line "#define WITH_EEPROM"

Once flashed, the Arduino temperature sensor will send to the gateway the following message \!#3#20.4 (20.4 is the measured temperature so you may not have the same value) prefixed by the application key every 10 minutes (with some randomization interval). This will trigger at the processing stage of the gateway the logging on the default ThinkSpeak channel (the test channel we provide) in field 3. At the gateway, 20.4 will be recorded on the provided ThingSpeak test channel in field 3 of the channel. If you go to https://thingspeak.com/channels/66794 you should see the reported value. 

The program has been tested on Arduino Uno, Mega2560, Nano, Pro Mini, Mini, Due.  We also tested on the Teensy3.1/3.2 and the Ideetron Nexus. The SX1272 lib has been modified to change the SPI_SS pin from 2 to 10 when you compile for the Pro Mini, Mini (Nexus), Nano or Teensy. 

------------------------------------------------------------------------
An interactive end-device for sending LoRa messages with the Arduino IDE
------------------------------------------------------------------------

With the Arduino IDE, open the Arduino_LoRa_Gateway sketch and check that "#define IS_SEND_GATEWAY" is uncommented. Then compile it and upload to an Arduino board. It is better to use a more powerful (and with more RAM memory) Arduino platform for building the interactive device otherwise stability issues can occur.

By default, the end-device have address 6 and runs in LoRa mode 4.

Enter "\!SGSH52UGPVAUYG3S#1#21.6" (without the quotes) in the input window and press RETURN

The command will be sent to the gateway and you should see the gateway pushing the data to the ThingSpeak test channel. If you go to https://thingspeak.com/channels/66794 you should see the reported value.

When testing with the interactive end-device, you should not use the --wappkey option for the post_processing_gw.py post-processing python script otherwise your command will not be accepted as only text string without logging services will be received and displayed when --wappkey is set.

	> sudo ./lora_gateway | python ./post_processing_gw.py -t | python ./log_gw

--------------------------------
Use an Arduino as a LoRa gateway
--------------------------------

The gateway can also be based on an Arduino board, as described in the web page. With the Arduino IDE, open the Arduino_LoRa_Gateway sketch and set the compilation #define to have IS_RCV_GATEWAY and not IS_SEND_GATEWAY. Compile the code and upload to an Arduino board. Then follow instructions on how to use the Arduino board as a gateway. It is better to use a more powerful (and with more RAM memory) Arduino platform for building the gateway.

----------------------
Running in 433MHz band
----------------------

When your radio module can run in the 433MHz band (for instance when the radio is based on SX1276 or SX1278 chip) then you can test running at 433MHz as follows:

- uncomment line "e = sx1272.setChannel(0x6C4000);" in Arduino_LoRa_temp or Arduino_LoRa_Simple_temp
- run your gateway with "lora_gateway --mode n --freq 433.0" where n is the LoRa mode you want to use (e.g. 4 with Arduino_LoRa_temp and 1 with Arduino_LoRa_Simple_temp)

----------------------------
Mounting your Dropbox folder
----------------------------

With sshfs:

	look at http://mitchtech.net/dropbox-on-raspberry-pi-via-sshfs/
	(no need of "sudo gpasswd -a pi fuse" on Jessie)
	
	> sudo apt-get install sshfs
	
	then allow option 'user_allow_other' in /etc/fuse.conf
	
with Dropbox uploader:

	look at http://anderson69s.com/2014/02/18/raspberry-pi-dropbox/
	look at http://raspi.tv/2013/how-to-use-dropbox-with-raspberry-pi
	look at https://github.com/andreafabrizi/Dropbox-Uploader
	(not tested yet and not supported)	
	
-------
WARNING
-------

A/ There is currently no control on the transmit time for both gateway and end-device. When using the library to create devices, you have to ensure that the transmit time of your device is not exceeding the legal maximum transmit time defined in the regulation of your country.

B/ Although 900MHz band is supported (mostly for the US ISM band), the library does not implement the frequency hopping mechanism.


Enjoy!
C. Pham