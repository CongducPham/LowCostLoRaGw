Low-cost LoRa gateway with Raspberry
====================================

Please consult the web page: http://cpham.perso.univ-pau.fr/LORA/RPIgateway.html

**This is the description of the basic version of the low-cost gateway. There is an advanced version that will upgrade some files. Look at the [gw_advanced folder](https://github.com/CongducPham/LowCostLoRaGw/tree/master/gw_advanced) and follow instructions.** With the advanced version, you can then add new updates (such as new cloud management and downlink features) in an incremental way if you want to do so. The update/upgrade steps are as follows: 

- basic version -> gw_advanced: see [gw_advanced folder](https://github.com/CongducPham/LowCostLoRaGw/tree/master/gw_advanced)
- gw_advanced -> new cloud management: see [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_advanced/new_cloud_design/README-NewCloud.md); [How to update](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_advanced/new_cloud_design/README-NewCloud.md#how-to-update-your-gateway) 
- gw_advanced w/new cloud management -> downlink features: see [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_advanced/downlink/README-downlink.md); [How to update](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_advanced/downlink/README-downlink.md#how-to-update-your-gateway)

**NEW** 
=======

- new cloud management approach: simpler, more generic
	- https://github.com/CongducPham/LowCostLoRaGw/tree/master/gw_advanced/new_cloud_design
	- [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_advanced/new_cloud_design/README-NewCloud.md)
- new downlink features: to send from gateway to end-device
	- https://github.com/CongducPham/LowCostLoRaGw/tree/master/gw_advanced/downlink
	- [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_advanced/downlink/README-downlink.md)
- get the zipped SD card image (Raspbian Jessie) with all the advanced features (have to update for new cloud management and downlink) already installed and working out-of-the-box with the Arduino_LoRa_Simple_temp example on a demo ThingSpeak channel.
	- [raspberrypi-jessie-WAZIUP-demo.dmg.zip](http://cpham.perso.univ-pau.fr/LORA/WAZIUP/raspberrypi-jessie-WAZIUP-demo.dmg.zip)
	- Based on Raspbian Jessie 
	- Supports Raspberry 1B+, RPI2 and RPI3
	- Includes all the advanced features described in the gw_advanced github
	- Get the zipped image, unzip it, install it on an 8GB SD card, see [this tutorial](https://www.raspberrypi.org/documentation/installation/installing-images/) from www.raspberrypi.org
	- Plug the SD card into your Raspberry
	- Connect a radio module (see http://cpham.perso.univ-pau.fr/LORA/RPIgateway.html)
	- Power-on the Raspberry
	- The LoRa gateway starts automatically when RPI is powered on
	- By default, incoming data are uploaded to the [WAZIUP ThingSpeak demo channel](https://thingspeak.com/channels/123986)
	- Works out-of-the-box with the [Arduino_LoRa_Simple_temp sketch](https://github.com/CongducPham/LowCostLoRaGw/tree/master/Arduino/Arduino_LoRa_Simple_temp)
- 2 tutorial videos on YouTube: video of all the steps to build the whole framework from scratch
	- [Build your low-cost, long-range IoT device with WAZIUP](https://www.youtube.com/watch?v=YsKbJeeav_M)
	- [Build your low-cost LoRa gateway with WAZIUP](https://www.youtube.com/watch?v=peHkDhiH3lE)

Install Raspbian Wheezy or Jessie
=================================

Fisrt install a Raspberry with Raspbian, Jessie is recommended.

then (you need to have Internet access on your Raspberry):

	> sudo apt-get update
	> sudo apt-get upgrade

Jessie has been tested on RPI1, RPI2 and RPI3, and works great.

Wheezy has been tested on RPI1 and RPI2 and works great. Wheezy on RPI3 is not recommended because built-in WiFi and Bluetooth will not work properly.

We recommend buying either RPI2 or RPI3. RPI3 with Jessie has built-in WiFi and Bluetooth so it is definitely a good choice. In addition RPI3 with Jessie will have a better support lifetime. 

Connect a radio module to Raspberry
===================================

You have to connect a LoRa radio module to the Raspberry's GPIO header. Just connect the corresponding SPI pin (MOSI, MISO, CLK, CS). Of course you also need to provide the power (3.3v) to the radio module. You can have a look at the "Low-cost-LoRa-GW-step-by-step" tutorial in our tutorial repository (https://github.com/CongducPham/tutorials).

Install the low-level LoRa gateway
==================================

Log as **pi** user on your Raspberry using ssh or connect a display and a keyboard. To get all the low-level LoRa gateway files you have 2 options.

First option
------------

Get all the repository:

	> git clone https://github.com/CongducPham/LowCostLoRaGw.git
	
You will get the entire repository:

	pi@raspberrypi:~ $ ls -l LowCostLoRaGw/
	total 32
	drwxr-xr-x 7 pi pi  4096 Jul 26 15:38 Arduino
	drwxr-xr-x 5 pi pi  4096 Jul 26 15:38 gw_advanced
	drwxr-xr-x 2 pi pi  4096 Jul 26 15:38 Raspberry
	-rw-r--r-- 1 pi pi 15522 Jul 26 15:38 README.md
	drwxr-xr-x 2 pi pi  4096 Jul 26 15:38 tutorials
	
Create a folder named "lora_gateway" for instance then copy all the files of the LowCostLoRaGw/Raspberry folder in it.

    > mkdir lora_gateway
    > cd lora_gateway
    > cp -R ../LowCostLoRaGw/Raspberry/* .
    
Or if you want to "move" the LowCostLoRaGw/Raspberry folder, simply do (without creating the lora_gateway folder before):

	> mv LowCostLoRaGw/Raspberry ./lora_gateway    

Second option
-------------

Get only the gateway part:

	> svn checkout https://github.com/CongducPham/LowCostLoRaGw/trunk/Raspberry lora_gateway
	
That will create the lora_gateway folder and get all the file of (GitHub) LowCostLoRaGw/Raspberry in it. Then:

	> cd lora_gateway

Note that you may have to install svn before being able to use the svn command:

	> sudo apt-get install subversion
	
Compiling the low-level gateway program
---------------------------------------	 	
    
DO NOT modify the lora_gateway.cpp file unless you know what you are doing. Check the radio.makefile file to select the radio module that you have. Uncomment only 1 choice or leave all lines commented if you do not have neither an HopeRF92/95 or inAir9B or a radio module with +20dBm possibility (the SX1272/76 has +20dBm feature but some radio modules that integrate the SX1272/76 may not have the electronic to support it). For instance, with both Libelium LoRa and inAir9 (not inAir9B) you should leave all lines commented. Then:

	> make lora_gateway

If you are using a Raspberry v2 or v3 :

	> make lora_gateway_pi2

To launch the gateway

	> sudo ./lora_gateway

On Raspberry v2 or v3 a symbolic link will be created that will point to lora_gateway_pi2.

By default, the gateway runs in LoRa mode 1 and has address 1.

You can have a look at the "Low-cost-LoRa-GW-step-by-step" tutorial in our tutorial repository (https://github.com/CongducPham/tutorials).

Adding LoRa gateway's post-processing features
==============================================

A data post-processing stage in added after the low-level LoRa gateway program. The post_processing_gw.py script can be customized to process sensor raw data from the low-level LoRa gateway. A typical processing task is to push received data to Internet servers or dedicated (public or private) IoT clouds. post_processing_gw.py is a template that already implement data uploading to various public IoT clouds.

For instance, to push data to the provided ThingSpeak WAZIUP test channel

	> sudo ./lora_gateway | python ./post_processing_gw.py -t

To log processing output in a file (in ~/Dropbox/LoRa-test/post_processing.log)

	> sudo ./lora_gateway | python ./post_processing_gw.py -t | python ./log_gw
	
**Note that if you want to run and test the above command now**, you have to create a "Dropbox" folder in your home directory with a subforder "LoRa-test" that will be used locally. Please put attention to the name of the folders: they must be "Dropbox/LoRa-test" because the "post_processing_gw.py" Python script uses these paths. You can mount Dropbox later on if you want: the local folders and contents will be unchanged. **Otherwise, just run the config_raspbian.sh configurarion script as it will be described later on (recommended)**.

    > mkdir -p Dropbox/LoRa-test 	
	
To additionally enforce application key at the gateway post-processing stage

	> sudo ./lora_gateway | python ./post_processing_gw.py -t --wappkey | python ./log_gw

This is the command that we recommend. To test, just flash a temperature sensor and it should work out-of-the-box.

You can customize the post-processing stage (post_processing_gw.py) at your convenience later.

You can have a look at the "Low-cost-LoRa-GW-step-by-step" tutorial in our tutorial repository (https://github.com/CongducPham/tutorials).

Connect a radio module to your end-device
=========================================

To have an end-device, you have to connect a LoRa radio module to an Arduino board. Just connect the corresponding SPI pin (MOSI, MISO, CLK, CS). Of course you also need to provide the power (3.3v) to the radio module. You can have a look at the "Low-cost-LoRa-IoT-step-by-step" tutorial in the tutorial repository (https://github.com/CongducPham/tutorials).

There is an important issue regarding the radio modules. The Semtech SX1272/76 has actually 2 lines of RF power amplification (PA): a high efficiency PA up to 14dBm (RFO) and a high power PA up to 20dBm (PA_BOOST). Setting transmission power to "L" (Low), "H" (High), and "M" (Max) only uses the RFO and delivers 2dBm, 6dBm and 14dBm respectively. "x" (extreme) and "X" (eXtreme) use the PA_BOOST and deliver 14dBm and 20dBm respectively.
However even if the SX1272/76 chip has the PA_BOOST and the 20dBm features, not all radio modules (integrating these SX1272/76) do have the appropriate wiring and circuits to enable these features: it depends on the choice of the reference design that itself is guided by the main intended frequency band usage, and sometimes also by the target country's regulations (such as maximum transmitted power). So you have to check with the datasheet whether your radio module has PA_BOOST (usually check whether the PA_BOOST pin is wired) and 20dBm capability before using "x" or "X". Some other radio modules only wire the PA_BOOST and not the RFO resulting in very bad range when trying to use the RFO mode ("L", "H", and "M"). In this case, one has to use "x" to indicate PA_BOOST usage to get 14dBm.

Practically, we only use either "M" (Max) or "x" (extreme) to have maximum range. They both deliver 14dBm but the difference is whether the RFO pin is used or the PA_BOOST. Therefore, when uploading a sketch on your board, you have to check whether your radio module needs the PA_BOOST in order to get significant output level in which case "x" should be used instead of "M". All the examples start with:

	// IMPORTANT
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// please uncomment only 1 choice
	//
	// uncomment if your radio is an HopeRF RFM92W or RFM95W
	//#define RADIO_RFM92_95
	// uncomment if your radio is a Modtronix inAirB (the one with +20dBm features), if inAir9, leave commented
	//#define RADIO_INAIR9B
	// uncomment if you only know that it has 20dBm feature
	//#define RADIO_20DBM
	/////////////////////////////////////////////////////////////////////////////////////////////////////////// 

Uncomment RADIO_RFM92_95 if you have a HopeRF RFM92W or RFM95W. Uncomment RADIO_INAIR9B if you have a Modtronix inAir9B (if inAir9, leave commented). If you have another non listed radio module, try first by leaving RADIO_20DBM commented, then see whether the packet reception is correct with a reasonably high SNR (such as 6 to 10 dB) at some meters of the gateway. If not, then try with RADIO_20DBM uncommented.

First try: a simple Ping-Pong program example
=============================================

As suggested by some people, we provide here a simple Ping-Pong program to upload on an Arduino board. First, install the Arduino IDE 1.6.6. Then, in your sketch folder, copy the content of the Arduino folder of the distribution.

Run the gateway with:

	> sudo ./lora_gateway
	
With the Arduino IDE, open the Arduino_LoRa_Ping_Pong sketch compile it and upload to an Arduino board. Check your radio module first, see "Connect a radio module to your end-device" above.

The end-device runs in LoRa mode 1 and has address 8. Open the Serial Monitor (38400 bauds) to see the output of the Arduino. It will send "Ping" to the gateway by requesting an ACK every 10s. If the ACK is received then it will display "Pong received from gateway!" otherwise it displays "No Pong!".

Note that in most operational scenarios, requesting ACK from the gateway is costly. Look at the next examples to see how we usually send data without requesting ACK.


An end-device example that periodically sends temperature to the gateway
========================================================================

See the [video here](https://www.youtube.com/watch?v=YsKbJeeav_M).

First, install the Arduino IDE 1.6.6. Then, in your sketch folder, copy the content of the Arduino folder of the distribution.

With the Arduino IDE, open the Arduino_LoRa_temp sketch (or the more simpler Arduino_LoRa_Simple_temp), compile it and upload to an Arduino board. Check your radio module first, see "Connect a radio module to your end-device" above.

The end-device runs in LoRa mode 1 and has address 6 for Arduino_LoRa_temp and address 8 for Arduino_LoRa_Simple_temp. It will send data to the gateway.

The default configuration uses an application key set to [5, 6, 7, 8].

Use a temperature sensor (e.g. LM35DZ) and plugged in pin A0 (analog 0). You can use a power pin to power your temperature sensor if you are not concerned about power saving. Otherwise, you can use digital 8 (the sketch set this pin HIGH when reading value, then sets it back to LOW) and activate low power mode (uncomment #define LOW_POWER), see below. 

For low-power applications the Pro Mini from Sparkfun is certainly a good choice. This board can be either in the 5V or 3.3V version. With the Pro Mini, it is better to really use the 3.3V version running at 8MHz as power consumption will be reduced. Power for the radio module can be obtained from the VCC pin which is powered in 3.3v when USB power is used or when unregulated power is connected to the RAW pin. If you power your Pro Mini with the RAW pin you can use for instance 4 AA batteries to get 6V. If you use a rechargeable battery you can easily find 3.7V Li-Ion packs. In this case, you can inject directly into the VCC pin but make sure that you've unsoldered the power isolation jumper, see Pro Mini schematic on the Arduino web page. To save more power, you can remove the power led.

The current low-power version for Arduino board use the RocketScream Low Power library (https://github.com/rocketscream/Low-Power) and can support most Arduino platforms although the Pro Mini platform will probably exhibit the best energy saving (we measured 124uA current in sleep mode with the power led removed). You can buid the low-power version by uncommenting the LOW_POWER compilation define statement. Then set "int idlePeriodInMin = 10;" to the number of minutes between 2 wake-up. By default it is 10 minutes.

For the special case of Teensy 31/32, the power saving mode uses the Snooze library provided by the Teensyduino package. You can upgrade the Snooze library from their github. With the Teensy, you can further use the HIBERNATE mode by uncommenting "USE_HIBERNATE" in the Snooze.h file of the Snooze libary and uncommenting LOW_POWER_HIBERNATE in the temperature example.

Depending on the sensor type, the computation to get the real temperature may be changed accordingly. Here is the instruction for the LM35DZ: http://www.instructables.com/id/ARDUINO-TEMPERATURE-SENSOR-LM35/

The default configuration also use the EEPROM to store the last packet sequence number in order to get it back when the sensor is restarted/rebooted. If you want to restart with a packet sequence number of 0, just comment the line "#define WITH_EEPROM"

Once flashed, the Arduino temperature sensor will send to the gateway the following message \!#3#20.4 (20.4 is the measured temperature so you may not have the same value) prefixed by the application key every 10 minutes (with some randomization interval). This will trigger at the processing stage of the gateway the logging on the default ThinkSpeak channel (the test channel we provide) in field 3. At the gateway, 20.4 will be recorded on the provided ThingSpeak test channel in field 3 of the channel. If you go to https://thingspeak.com/channels/66794 you should see the reported value. 

The program has been tested on Arduino Uno, Mega2560, Nano, Pro Mini, Mini, Due.  We also tested on the Teensy3.1/3.2 and the Ideetron Nexus. The SX1272 lib has been modified to change the SPI_SS pin from 2 to 10 when you compile for the Pro Mini, Mini (Nexus), Nano or Teensy. 

An interactive end-device for sending LoRa messages with the Arduino IDE
========================================================================

With the Arduino IDE, open the Arduino_LoRa_Gateway sketch and check that "#define IS_SEND_GATEWAY" is uncommented. Then compile it and upload to an Arduino board. It is better to use a more powerful (and with more RAM memory) Arduino platform for building the interactive device otherwise stability issues can occur.

By default, the interactive end-device has address 6 and runs in LoRa mode 1.

Enter "\!SGSH52UGPVAUYG3S#1#21.6" (without the quotes) in the input window and press RETURN

The command will be sent to the gateway and you should see the gateway pushing the data to the ThingSpeak test channel. If you go to https://thingspeak.com/channels/66794 you should see the reported value.

When testing with the interactive end-device, you should not use the --wappkey option for the post_processing_gw.py post-processing python script otherwise your command will not be accepted as only text string without logging services will be received and displayed when --wappkey is set.

	> sudo ./lora_gateway | python ./post_processing_gw.py -t | python ./log_gw

Use an Arduino as a LoRa gateway
================================

The gateway can also be based on an Arduino board, as described in the web page. With the Arduino IDE, open the Arduino_LoRa_Gateway sketch and set the compilation #define to have IS_RCV_GATEWAY and not IS_SEND_GATEWAY. Compile the code and upload to an Arduino board. Then follow instructions on how to use the Arduino board as a gateway. It is better to use a more powerful (and with more RAM memory) Arduino platform for building the gateway.

Running in 433MHz band
======================

When your radio module can run in the 433MHz band (for instance when the radio is based on SX1276 or SX1278 chip) then you can test running at 433MHz as follows:

- uncomment line "e = sx1272.setChannel(0x6C4000);" in Arduino_LoRa_temp or Arduino_LoRa_Simple_temp
- run your gateway with "lora_gateway --mode 1 --freq 433.0" to be on the same setting for Arduino_LoRa_temp and Arduino_LoRa_Simple_temp

Mounting your Dropbox folder
============================

With sshfs:

- look at http://mitchtech.net/dropbox-on-raspberry-pi-via-sshfs/. No need of "sudo gpasswd -a pi fuse" on Jessie.
	
	> sudo apt-get install sshfs
	
- then allow option 'user_allow_other' in /etc/fuse.conf
	
with Dropbox uploader:

- look at http://anderson69s.com/2014/02/18/raspberry-pi-dropbox/
- look at http://raspi.tv/2013/how-to-use-dropbox-with-raspberry-pi
- look at https://github.com/andreafabrizi/Dropbox-Uploader
- but not tested yet and not supported yet	

ANNEX.A
=======

Pre-defined LoRa modes (from initial Libelium SX1272.h)

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

Pre-defined channels in 868MHz and 915MHz band (from initial Libelium SX1272.h)

| ch | F(MHz) | ch | F(MHz) |
|----|--------|----|--------|
| 10 | 865.2  | 00 | 903.08 |
| 11 | 865.5  | 01 | 905.24 |
| 12 | 865.8  | 02 | 907.40 |
| 13 | 866.1  | 03 | 909.56 |
| 14 | 865.4  | 04 | 911.72 |
| 15 | 865.7  | 05 | 913.88 |
| 16 | 866.0  | 06 | 916.04 |
| 17 | 868.0  | 07 | 918.20 |
|  - |   -    | 08 | 920.36 |
|  - |   -    | 09 | 922.52 | 
|  - |   -    | 10 | 924.68 | 
|  - |   -    | 11 | 926.84 | 
|  - |   -    | 12 | 915.00 |


	
WARNING
=======

- There is currently no control on the transmit time for both gateway and end-device. When using the library to create devices, you have to ensure that the transmit time of your device is not exceeding the legal maximum transmit time defined in the regulation of your country.

- Although 900MHz band is supported (mostly for the US ISM band), the library does not implement the frequency hopping mechanism.


NEW: Tutorial materials
=======================

Go to https://github.com/CongducPham/tutorials and look for the "Low-cost-LoRa-GW-step-by-step" tutorial.

Look at our [FAQ](https://github.com/CongducPham/tutorials/blob/master/FAQ.pdf)!

NEW: Get the advanced version of the gateway
============================================

Look at the [gw_advanced folder](https://github.com/CongducPham/LowCostLoRaGw/tree/master/gw_advanced) and follow instructions.


Enjoy!
C. Pham