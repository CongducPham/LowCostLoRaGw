Low-cost LoRa gateway with Raspberry
====================================

Please also consult the web page: http://cpham.perso.univ-pau.fr/LORA/RPIgateway.html.

2 tutorial videos on YouTube: video of all the steps to build the whole framework from scratch:

- [Build your low-cost, long-range IoT device with WAZIUP](https://www.youtube.com/watch?v=YsKbJeeav_M)
- [Build your low-cost LoRa gateway with WAZIUP](https://www.youtube.com/watch?v=peHkDhiH3lE)

Upgrade notice
--------------

Starting Apr 2nd 2017, the gateway configuration files have changed. There is now only one configuration file, gateway_conf.json, instead of two, global_conf.json and local_conf.json. If you have a gateway version prior to Apr 2nd, 2017, please read the "Upgrade notice" below.

Features
========

- an alert mail can be sent to a list of contact email addresses to notify when gateway is starting and when the radio module has been reset
- periodic status report to monitor whether the post-processing stage of the gateway is up or not
- encryption and native LoRaWAN frame format
	- see [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-aes_lorawan.md)
	- end-device can send native LoRaWAN packets
	- low-level gateway provides raw output for post_processing_gw.py to handle LoRaWAN packets
- downlink features: to send from gateway to end-device
	- [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-downlink.md)
- simple, flexible and generic cloud management approach
	- [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-NewCloud.md)
- support for an embedded DHT22 temperature/humidity sensor to monitor the condition inside the gateway case
- there is a NoSQL MongoDB support and received data can be saved in the local database if this feature is activated
- there is an Apache web server with basic PHP forms to visualize graphically the received data of the MongoDB with any web browser
- the gateway acts as the WiFi access-point. The SSID is WAZIUP_PI_GW_XXXXXXXXXX where XXXXXXXXXX is the last 5 hex bytes of gateway ID: WAZIUP_PI_GW_27EB27F90F for instance. It has IP address 192.168.200.1 and will lease IP addresses in the range of 192.168.200.100 and 192.168.200.120. Just connect to http://192.168.200.1 with a web brower (could be from a smartphone) to get the graphic visualization of the data stored in the gateway's MongoDB database.	
- there is the support of Bluetooth connection. A simple Android App running on Android smartphone displays the data stored in the gateway's MongoDB database.
- a configuration script (scripts/config_gw.sh) helps you configure the gateway with MongoDB, WiFi and Bluetooth features. It is highly recommended to use this script to set your gateway once all the files have been copied.
- Arduino code for gateway and interactive end-device
	- code for gateway and interactive end-device are now separated in 2 sketches
	- the old version is still in folder Arduino_LoRa_Gateway_1_4 and will not be maintained anymore. It will stay at v1.4
	- Arduino_LoRa_Gateway now contains the gateway code. It is equivalent, in previous version v1.4, to compilation with IS_RCV_GATEWAY.
	- Arduino_LoRa_InteractiveDevice contains the code for an interactive end-device.  It is equivalent, in previous version v1.4, to compilation with IS_SEND_GATEWAY
- get the zipped SD card image (Raspbian Jessie)
	- [raspberrypi-jessie-WAZIUP-demo.dmg.zip](http://cpham.perso.univ-pau.fr/LORA/WAZIUP/raspberrypi-jessie-WAZIUP-demo.dmg.zip)
	- Based on Raspbian Jessie 
	- Supports Raspberry 1B+, RPI2 and RPI3 (WiFi support is for RPI3. For RPI1 and RPI2 see [here](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README.md#wifi-instructions-on-rpi1b-and-rpi2) for modifications to support some WiFi dongles)
	- Get the zipped image, unzip it, install it on an 8GB SD card, see [this tutorial](https://www.raspberrypi.org/documentation/installation/installing-images/) from www.raspberrypi.org
	- Plug the SD card into your Raspberry
	- Connect a radio module (see http://cpham.perso.univ-pau.fr/LORA/RPIgateway.html)
	- Power-on the Raspberry
	- pi user
		- login: pi
		- password: loragateway
		- **it is strongly advise to change the pi user's password**		
	- The LoRa gateway starts automatically when RPI is powered on
	- With an RPI3, the Raspberry will automatically act as a WiFi access point
		- SSID=WAZIUP_PI_GW_27EB27F90F for instance
		- password=loragateway
		- **it is strongly advise to change this WiFi password**
	- Includes most of features described here but a full update with the latest version is **necessary, see below**	
	- By default, incoming data are uploaded to our [LoRa ThingSpeak test channel](https://thingspeak.com/channels/66794)
	- Works out-of-the-box with the [Arduino_LoRa_Simple_temp sketch](https://github.com/CongducPham/LowCostLoRaGw/tree/master/Arduino/Arduino_LoRa_Simple_temp)
	
Latest gateway version 
======================

The full, latest distribution of the low-cost gateway is available in the gw_full_latest folder. It contains all the gateway control and post-processing software. If you use our SD card image and update the gateway from it you don't need to install any additional packages. Otherwise you may need to install required Raspbian Jessie packages as explained in the various README files.

To get directly to the full, latest gateway version, (i) simply download (git clone) the whole repository and copy the entire content of the gw_full_latest folder on your Raspberry, in a folder named lora_gateway or, (ii) get only (svn checkout) the gw_full_latest folder in a folder named lora_gateway. 

First option
------------

Get all the repository:

	> git clone https://github.com/CongducPham/LowCostLoRaGw.git
	
You will get the entire repository:

	pi@raspberrypi:~ $ ls -l LowCostLoRaGw/
	total 32
	drwxr-xr-x 7 pi pi  4096 Apr  1 15:38 Arduino
	-rw-r--r-- 1 pi pi 15522 Apr  1 15:38 README.md	
	drwxr-xr-x 2 pi pi  4096 Apr  1 15:38 gw_full_latest	
	drwxr-xr-x 2 pi pi  4096 Apr  1 15:38 tutorials
	
Create a folder named "lora_gateway" for instance then copy all the files of the LowCostLoRaGw/gw_full_latest folder in it.

    > mkdir lora_gateway
    > cd lora_gateway
    > cp -R ../LowCostLoRaGw/gw_full_latest/* .
    
Or if you want to "move" the LowCostLoRaGw/gw_full_latest folder, simply do (without creating the lora_gateway folder before):

	> mv LowCostLoRaGw/gw_full_latest ./lora_gateway    

Second option
-------------

Get only the gateway part:

	> svn checkout https://github.com/CongducPham/LowCostLoRaGw/trunk/gw_full_latest lora_gateway
	
That will create the lora_gateway folder and get all the file of (GitHub) LowCostLoRaGw/gw_full_latest in it. Then:

	> cd lora_gateway

Note that you may have to install svn before being able to use the svn command:

	> sudo apt-get install subversion

Then, in the script folder, run config_gw.sh to configure your gateway, as described [here](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README.md#configure-your-gateway-with-config_gwsh). After configuration, reboot your Raspberry. 

By default gateway_conf.json configures the gateway with a simple behavior: LoRa mode 1 (BW125SF12), no DHT sensor in gateway (so no MongoDB for DHT sensor), no downlink, no AES, no raw mode. clouds.json enables only the ThingSpeak demo channel (even the local MongoDB storage is disabled). You can customize your gateway later when you have more cloud accounts and when you know better what features you want to enable.

The LoRa gateway starts automatically when RPI is powered on. Then use cmd.sh to execute the main operations on the gateway as described in [here](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README.md#use-cmdsh-to-interact-with-the-gateway).	

With the latest gateway version on the github, you also have in lora_gateway/scripts an update_gw.sh script that updates your gateway with future latest versions. Simple go into lora_gateway/scripts and type:

	> ./update_gw.sh
	
If you have an existing /home/pi/lora_gateway folder, then it will preserve all you existing configuration files (i.e. key_*, global_conf.json, local_conf.json, clouds.json and radio.makefile). As the repository does not have a gateway_id.txt file, it will also preserve your gateway id.

**Note that you can also use this script to install a completely new gateway with the latest gateway version** by downloading from the github the gw_full_latest/scripts/update_gw.sh script (switch in raw mode and save the script on your computer), copy it on your Raspberry gateway (using scp for instance) in /home/pi and then simply run the script (you may need to add execution right with chmod +x update_gw.sh).

Upgrade notice
--------------
Starting Apr 2nd 2017, the gateway configuration files have changed. There is now only one configuration file, gateway_conf.json, instead of two, global_conf.json and local_conf.json. If you have a gateway version prior to Apr 2nd, 2017, then you can still use the update_gw_sh scripts but then you have to manually enter your gateway configuration setting, in global_conf.json and local_conf.json, into the new gateway_conf.json file.

Your global_conf.json file may look like:

	{
		"mode" : 1,
		"bw" : 500,
		"cr" : 5,
		"sf" : 12,
		"ch" : -1,
		"freq" : -1,
		"ignorecomment" : false,
		"loggw" : false,
		"wappkey" : false,
		"raw" : false,
		"aes" : false,
		"log_post_processing" : true
	}

and your local_conf.json may look like:

	{
		"gateway_conf" : {
			"gateway_ID" : "00000027EB27F90F",
			"ref_latitude" : "my_lat",
			"ref_longitude" : "my_long",
			"dht22" : 0,
			"dht22_mongo": false,
			"downlink" : 0,
			"status" : 600,
			"aux_radio" : 0
		},
		"log_conf" : {
			"log_weekly" : false
		}
	}

The format of the new gateway_conf.json file is as follows:

	{
		"radio_conf" : {
			"mode" : 1,
			"bw" : 500,
			"cr" : 5,
			"sf" : 12,
			"ch" : -1,
			"freq" : -1
		},
		"gateway_conf" : {
			"gateway_ID" : "00000027EB27F90F",
			"ref_latitude" : "my_lat",
			"ref_longitude" : "my_long",
			"wappkey" : false,
			"raw" : false,
			"aes" : false,
			"log_post_processing" : true,
			"log_weekly" : false,				
			"dht22" : 0,
			"dht22_mongo": false,
			"downlink" : 0,
			"status" : 600,
			"aux_radio" : 0
		},
		"alert_conf" : {
			"use_mail" : false,
			"contact_mail" : joejoejoe@gmail.com,jackjackjack@hotmail.com",
			"mail_from" : "myorg.gmail.com",
			"mail_server" : "smtp.gmail.com",
			"mail_passwd" : "my_passwd",
			"use_sms" : false,
			"contact_sms" : "+33XXXXXXXXX"
		}	
	}
	
Therefore, to upgrade, you have to (1) replace the whole "gateway_conf" section of gateway_conf.json by the "gateway_conf" section of local_conf_json, (2) report the first 6 fields of global_conf.json into the "radio_conf" section of gateway_conf.json, (3) report the last 4 fields of global_conf.json (thus omitting both "loggw" and "ignorecomment") into the "gateway_conf" section of gateway_conf.json and, (4) report the "log_weekly" field in the "log_conf" of local_conf.json into the "gateway_conf" section of gateway_conf.json.

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
	
Compiling the low-level gateway program
=======================================	 	
    
DO NOT modify the lora_gateway.cpp file unless you know what you are doing. Check the radio.makefile file to indicate whether your radio module uses the PA_BOOST amplifier line or not (which means it uses the RFO line). HopeRF RFM92W/95W or inAir9B or NiceRF1276 or a radio module with +20dBm possibility (the SX1272/76 has +20dBm feature but some radio modules that integrate the SX1272/76 may not have the electronic to support it) need the -DPABOOST. Both Libelium SX1272 and inAir9 (not inAir9B) do not use PA_BOOST. You can also define a maximum output power to stay within transmission power regulations of your country. For instance, if you do not define anything, then the output power is set to 14dBm (ETSI european regulations), otherwise use -DMAX_DBM=10 for 10dBm. Then:

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

A data post-processing stage in added after the low-level LoRa gateway program. The post_processing_gw.py script can be customized to process sensor raw data from the low-level LoRa gateway. A typical processing task is to push received data to Internet servers or dedicated (public or private) IoT clouds. post_processing_gw.py is a template that already implement data uploading to various public IoT clouds. See this [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-NewCloud.md) to know how to configure the cloud definition.

Adding the post-processing stage is done as follows:

	> sudo ./lora_gateway | python ./post_processing_gw.py

To log processing output in a file (in ~/Dropbox/LoRa-test/post_processing.log)

	> sudo ./lora_gateway | python ./post_processing_gw.py | python ./log_gw
	
**Note that if you want to run and test the above command now**, you have to create a "Dropbox" folder in your home directory with a subforder "LoRa-test" that will be used locally. Please put attention to the name of the folders: they must be "Dropbox/LoRa-test" because the "post_processing_gw.py" Python script uses these paths. You can mount Dropbox later on if you want: the local folders and contents will be unchanged. **Otherwise, just run the config_raspbian.sh configurarion script as it will be described later on (recommended)**.

    > mkdir -p Dropbox/LoRa-test 	
	
Actually, both lora_gateway can take additional parameters to configure the radio module. However, it is more convenient to use the start_gw.py script that will parse the gateway configuration file to launch the low-level gateway accordingly:

	> sudo python start_gw.py

This is the command that we recommend. To test, just flash a temperature sensor and it should work out-of-the-box on our [LoRa ThingSpeak test channel](https://thingspeak.com/channels/66794).

You can also customize the post-processing stage (post_processing_gw.py) at your convenience later.

You can have a look at the "Low-cost-LoRa-GW-step-by-step" tutorial in our tutorial repository (https://github.com/CongducPham/tutorials).

Connect a radio module to your end-device
=========================================

To have an end-device, you have to connect a LoRa radio module to an Arduino board. Just connect the corresponding SPI pin (MOSI, MISO, CLK, CS). Of course you also need to provide the power (3.3v) to the radio module. You can have a look at the "Low-cost-LoRa-IoT-step-by-step" tutorial in the tutorial repository (https://github.com/CongducPham/tutorials).

There is an important issue regarding the radio modules. The Semtech SX1272/76 has actually 2 lines of RF power amplification (PA): a high efficiency PA up to 14dBm (RFO) and a high power PA up to 20dBm (PA_BOOST). Setting transmission power to "L" (Low), "H" (High), and "M" (Max) only uses the RFO and delivers 2dBm, 6dBm and 14dBm respectively. "x" (extreme) and "X" (eXtreme) use the PA_BOOST and deliver 14dBm and 20dBm respectively.
However even if the SX1272/76 chip has the PA_BOOST and the 20dBm features, not all radio modules (integrating these SX1272/76) do have the appropriate wiring and circuits to enable these features: it depends on the choice of the reference design that itself is guided by the main intended frequency band usage, and sometimes also by the target country's regulations (such as maximum transmitted power). So you have to check with the datasheet whether your radio module has PA_BOOST (usually check whether the PA_BOOST pin is wired) and 20dBm capability before using "x" or "X". Some other radio modules only wire the PA_BOOST and not the RFO resulting in very bad range when trying to use the RFO mode ("L", "H", and "M"). In this case, one has to use "x" to indicate PA_BOOST usage to get 14dBm.

Practically, we only use either "M" (Max) or "x" (extreme) to have maximum range. They both deliver 14dBm but the difference is whether the RFO pin is used or the PA_BOOST. Therefore, when uploading a sketch on your board, you have to check whether your radio module needs the PA_BOOST in order to get significant output level in which case "x" should be used instead of "M". All the examples start with:

	// IMPORTANT
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// uncomment if your radio is an HopeRF RFM92W, HopeRF RFM95W, Modtronix inAir9B, NiceRF1276
	// or you known from the circuit diagram that output use the PABOOST line instead of the RFO line
	//#define PABOOST
	///////////////////////////////////////////////////////////////////////////////////////////////////////////  

Uncomment PABOOST if you have a HopeRF RFM92W or RFM95W, or a Modtronix inAir9B (if inAir9, leave commented) or a NiceRF1276. If you have another non listed radio module, try first by leaving PABOOST commented, then see whether the packet reception is correct with a reasonably high SNR (such as 6 to 10 dB) at some meters of the gateway. If not, then try with PABOOST uncommented.

First try: a simple Ping-Pong program example
=============================================

As suggested by some people, we provide here a simple Ping-Pong program to upload on an Arduino board. First, install the Arduino IDE 1.6.6. Check that the AVR board library is not above 1.6.9 as there might be some compilation issue otherwise. Then, in your sketch folder, copy the content of the Arduino folder of the distribution.

Run the gateway with:

	> sudo ./lora_gateway
	
With the Arduino IDE, open the Arduino_LoRa_Ping_Pong sketch compile it and upload to an Arduino board. Check your radio module first, see "Connect a radio module to your end-device" above.

The end-device runs in LoRa mode 1 and has address 8. Open the Serial Monitor (38400 bauds) to see the output of the Arduino. It will send "Ping" to the gateway by requesting an ACK every 10s. If the ACK is received then it will display "Pong received from gateway!" otherwise it displays "No Pong!".

Note that in most operational scenarios, requesting ACK from the gateway is costly. Look at the next examples to see how we usually send data without requesting ACK.

**Notice for low-cost/clone Arduino boards**. If you get a low-cost Arduino board, such as those sold by most of Chinese manufacturer, the USB connectivity is probably based on the CH340 or CH341. To make your low-cost Arduino visible to your Arduino IDE, you need the specific driver. Look at http://sparks.gogo.co.nz/ch340.html or http://www.microcontrols.org/arduino-uno-clone-ch340-ch341-chipset-usb-drivers/. For MacOS, you can look at http://www.mblock.cc/posts/run-makeblock-ch340-ch341-on-mac-os-sierra which works for MacOS up to Sierra. For MacOS user that have the previous version of CH34x drivers and encountering kernel panic with Sierra, don't forget to delete previous driver installation: "sudo rm -rf /System/Library/Extensions/usb.kext".


A simple end-device example that periodically sends temperature to the gateway
==============================================================================

See the [video here](https://www.youtube.com/watch?v=YsKbJeeav_M).

First, install the Arduino IDE 1.6.6. Check that the AVR board library is not above 1.6.9 as there might be some compilation issue otherwise. Then, in your sketch folder, copy the content of the Arduino folder of the distribution.

With the Arduino IDE, open the Arduino_LoRa_Simple_temp sketch, compile it and upload to an Arduino board. Check your radio module first, see "Connect a radio module to your end-device" above.

The end-device runs in LoRa mode 1 and has address 8. It will send data to the gateway.

The default configuration uses an application key filter set to [5, 6, 7, 8].

Use a temperature sensor (e.g. LM35DZ) and plugged in pin A0 (analog 0). You can use a power pin to power your temperature sensor if you are not concerned about power saving. Otherwise, you can use digital 8 (the sketch set this pin HIGH when reading value, then sets it back to LOW) and activate low power mode (uncomment #define LOW_POWER), see below. 

For low-power applications the Pro Mini from Sparkfun is certainly a good choice. This board can be either in the 5V or 3.3V version. With the Pro Mini, it is better to really use the 3.3V version running at 8MHz as power consumption will be reduced. Power for the radio module can be obtained from the VCC pin which is powered in 3.3v when USB power is used or when unregulated power is connected to the RAW pin. If you power your Pro Mini with the RAW pin you can use for instance 4 AA batteries to get 6V. If you use a rechargeable battery you can easily find 3.7V Li-Ion packs. In this case, you can inject directly into the VCC pin but make sure that you've unsoldered the power isolation jumper, see Pro Mini schematic on the Arduino web page. To save more power, you can remove the power led.

The current low-power version for Arduino board use the RocketScream Low Power library (https://github.com/rocketscream/Low-Power) and can support most Arduino platforms although the Pro Mini platform will probably exhibit the best energy saving (we measured 124uA current in sleep mode with the power led removed). You can buid the low-power version by uncommenting the LOW_POWER compilation define statement. Then set "int idlePeriodInMin = 10;" to the number of minutes between 2 wake-up. By default it is 10 minutes.

For the special case of Teensy boards (LC/31/32/35/36), the power saving mode uses the excellent work of Collin Duffy with the Snooze library included by the Teensyduino package. You can upgrade the Snooze library from the github (https://github.com/duff2013/Snooze) as version 6 is required to handle the new Teensy 35/36 boards. With the Teensy, you can further use the HIBERNATE mode by uncommenting LOW_POWER_HIBERNATE in the temperature example.

For the special of the Arduino Zero, waking up the board from deep sleep mode is done with the RTC. Therefore the RTCZero library from https://github.com/arduino-libraries/RTCZero is used and you need to install it before being able to compile the example for the Arduino Zero.

Depending on the sensor type, the computation to get the real temperature may be changed accordingly. Here is the instruction for the LM35DZ: http://www.instructables.com/id/ARDUINO-TEMPERATURE-SENSOR-LM35/

The default configuration also use the EEPROM to store the last packet sequence number in order to get it back when the sensor is restarted/rebooted. If you want to restart with a packet sequence number of 0, just comment the line "#define WITH_EEPROM"

Once flashed, the Arduino temperature sensor will send to the gateway the following message \\!#3#20.4 (20.4 is the measured temperature so you may not have the same value) prefixed by the application key every 10 minutes (with some randomization interval). This will trigger at the processing stage of the gateway the logging on the default ThinkSpeak channel (the test channel we provide) in field 3. At the gateway, 20.4 will be recorded on the provided ThingSpeak test channel in field 3 of the channel. If you go to https://thingspeak.com/channels/66794 you should see the reported value. 

The program has been tested on Arduino Uno, Mega2560, Nano, Pro Mini, Mini, Due, Zero.  We also tested on the Teensy3.1/3.2 and the Ideetron Nexus. The SX1272 lib has been modified to change the SPI_SS pin from 2 to 10 when you compile for the Pro Mini, Mini (Nexus), Nano or Teensy. 

**Notice for low-cost/clone Arduino boards**. If you get a low-cost Arduino board, such as those sold by most of Chinese manufacturer, the USB connectivity is probably based on the CH340 or CH341. To make your low-cost Arduino visible to your Arduino IDE, you need the specific driver. Look at http://sparks.gogo.co.nz/ch340.html or http://www.microcontrols.org/arduino-uno-clone-ch340-ch341-chipset-usb-drivers/. For MacOS, you can look at http://www.mblock.cc/posts/run-makeblock-ch340-ch341-on-mac-os-sierra which works for MacOS up to Sierra. For MacOS user that have the previous version of CH34x drivers and encountering kernel panic with Sierra, don't forget to delete previous driver installation: "sudo rm -rf /System/Library/Extensions/usb.kext".


An interactive end-device for sending LoRa messages with the Arduino IDE
========================================================================

With the Arduino IDE, open the Arduino_LoRa_InteractiveDevice sketch. Then compile it and upload to an Arduino board. It is better to use a more powerful Arduino platform for building the interactive device otherwise stability issues can occur (and especially with more RAM memory such as a MEGA, the Uno, ATMega328P, will be very unstable because of the small amount of memory).

By default, the interactive end-device has address 6 and runs in LoRa mode 1.

Enter "\\!SGSH52UGPVAUYG3S#1#21.6" (without the quotes) in the input window and press RETURN

The command will be sent to the gateway and you should see the gateway pushing the data to the ThingSpeak test channel. If you go to https://thingspeak.com/channels/66794 you should see the reported value.

When testing with the interactive end-device, you should not use the --wappkey option for the post_processing_gw.py post-processing python script otherwise your command will not be accepted as only text string without logging services will be received and displayed when --wappkey is set.

	> sudo ./lora_gateway | python ./post_processing_gw.py | python ./log_gw

Use an Arduino as a LoRa gateway
================================

The gateway can also be based on an Arduino board, as described in the web page. With the Arduino IDE, open the Arduino_LoRa_Gateway sketch, compile the code and upload to an Arduino board. Then follow instructions on how to use the Arduino board as a gateway. It is better to use a more powerful (and with more RAM memory such as the MEGA) Arduino platform for building the gateway.

Running in 433MHz band
======================

When your radio module can run in the 433MHz band (for instance when the radio is based on SX1276 or SX1278 chip, such as the inAir4 from Modtronics) then you can test running at 433MHz as follows:

- select line "#define BAND433" in Arduino_LoRa_temp or Arduino_LoRa_Simple_temp
- compile the lora_gateway.cpp with "#define BAND433"
- or simply run your gateway with "lora_gateway --mode 1 --freq 433.3" to be on the same setting than Arduino_LoRa_temp and Arduino_LoRa_Simple_temp
- there are 4 channels in the 433MHz band: 433.3MHz as CH_00_433, 433.6MHz as CH_01_433, 433.9MHz as CH_02_433 and 434.3MHz as CH_03_433 

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

ANNEX.A: LoRa mode and predefined channels
==========================================

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

Pre-defined channels in 868MHz, 915MHz and 433MHz band (most of them from initial Libelium SX1272.h, except those marked with *)

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


ANNEX.B: Troubleshooting
========================

Verify if the low-level gateway program detects your radio module and if the radio module is working by simply run the low-level gateway program with:

	> sudo ./lora_gateway
	
You should see the following output

	SX1276 detected, starting.
	SX1276 LF/HF calibration
	...
	^$**********Power ON: state 0
	^$Default sync word: 0x12
	^$LoRa mode 1
	^$Setting mode: state 0
	^$Channel CH_10_868: state 0
	^$Set LoRa power dBm to 14
	^$Power: state 0
	^$Get Preamble Length: state 0
	^$Preamble Length: 8
	^$LoRa addr 1: state 0
	^$SX1272/76 configured as LR-BS. Waiting RF input for transparent RF-serial bridge	

If one of the state result is different from 0 then it might be a power/current issue. If the Preamble Length is different from 8 then it can also be a power/current issue but also indicate more important failure of the radio module. Get the "faulty" radio module and connect it to an Arduino board running the interactive end-device sketch. If the Preamble Length is now correct, then retry again with the Raspberry gateway. If the problem on the Raspberry persists, try with another radio module.
	
WARNING
=======

- There is currently no control on the transmit time for both gateway and end-device. When using the library to create devices, you have to ensure that the transmit time of your device is not exceeding the legal maximum transmit time defined in the regulation of your country (for instance ETSI define 1% duty-cycle, i.e. 36s/hour).

- Although 900MHz band is supported (mostly for the US ISM band), the library does not implement the frequency hopping mechanism nor the limited dwell time (e.g. 400ms per transmission).


Tutorial materials
=======================

Go to https://github.com/CongducPham/tutorials and look for the "Low-cost-LoRa-GW-step-by-step" tutorial.

Look at our [FAQ](https://github.com/CongducPham/tutorials/blob/master/FAQ.pdf)!


Enjoy!
C. Pham