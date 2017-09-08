Low-cost LoRa gateway with Raspberry
====================================

Tutorial materials
------------------

Please also consult the web page: http://cpham.perso.univ-pau.fr/LORA/RPIgateway.html.

2 tutorial videos on YouTube: video of all the steps to build the whole framework from scratch:

- [Build your low-cost, long-range IoT device with WAZIUP](https://www.youtube.com/watch?v=YsKbJeeav_M)
- [Build your low-cost LoRa gateway with WAZIUP](https://www.youtube.com/watch?v=peHkDhiH3lE)

Go to [https://github.com/CongducPham/tutorials](https://github.com/CongducPham/tutorials) for all tutorials and particularly look for:

- [Low-cost-LoRa-IoT-step-by-step tutorial](https://github.com/CongducPham/tutorials/blob/master/Low-cost-LoRa-GW-step-by-step.pdf) 
- [Low-cost-LoRa-GW-step-by-step tutorial](https://github.com/CongducPham/tutorials/blob/master/Low-cost-LoRa-IoT-step-by-step.pdf)
- [Low-cost-LoRa-IoT-antennaCable tutorial](https://github.com/CongducPham/tutorials/blob/master/Low-cost-LoRa-IoT-antennaCable.pdf)

Look also at our [FAQ](https://github.com/CongducPham/tutorials/blob/master/FAQ.pdf)!

Get our SD card image
---------------------

Download our [zipped SD card image](http://cpham.perso.univ-pau.fr/LORA/WAZIUP/raspberrypi-jessie-WAZIUP-demo.dmg.zip). It is not a MacOS X DMG package as the extension may be misleading, simply unzip the file and burn the dmg file to an SD card. Use an SD card of a minimum of 8GB. Take also a class 10. If you have bigger SD card, e.g. 16GB, then after boot, use raspi-config (see [tutorial here](https://www.raspberrypi.org/documentation/configuration/raspi-config.md)) to resize the partition in order to use the extra space available (you will need to reboot but raspi-config will ask you for that). You can use "df -h" to verify that you have more space after reboot.  

You can look at various tutorials on how to burn an image to an SD card. There is one [here from raspberrypi.org](https://www.raspberrypi.org/documentation/installation/installing-images/) and [here from elinux.org](http://elinux.org/RPi_Easy_SD_Card_Setup). I use a Mac to do so and [this is my preferred solution](https://www.raspberrypi.org/documentation/installation/installing-images/mac.md). The [Linux version](https://www.raspberrypi.org/documentation/installation/installing-images/linux.md) is not very different.

Upgrade notice
--------------

Starting Apr 2nd 2017, the gateway configuration files have changed. There is now only one configuration file, gateway_conf.json, instead of two, global_conf.json and local_conf.json. If you have a gateway version prior to Apr 2nd, 2017, please read the "Upgrade notice" below.

Features
========

- a simple, user-friendly web admin interface to configure and update your gateway
	- [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/gw_web_admin/README.md)
	- [Tutorial](https://github.com/CongducPham/tutorials/blob/master/Low-cost-LoRa-GW-web-admin.pdf)
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
- there is a NoSQL MongoDB support and received data can be saved in the local database if this feature is activated. See [here](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-NewCloud.md#support-of-mongodb-as-a-cloud-declaration) for more information on the local MongoDB structure.
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
	- Supports Raspberry 1B+, RPI2, RPI3, RPI0 and RPI0W (out-of-box WiFi support is for RPI3 and RPI0W. For RPI1 and RPI2 see [here](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README.md#wifi-instructions-on-rpi1b-and-rpi2) for modifications to support some WiFi dongles)
	- Get the zipped image, unzip it, install it on an **8GB** SD card (or bigger), see [this tutorial](https://www.raspberrypi.org/documentation/installation/installing-images/) from www.raspberrypi.org
	- Plug the SD card into your Raspberry
	- Connect a radio module (see http://cpham.perso.univ-pau.fr/LORA/RPIgateway.html)
	- Power-on the Raspberry
	- pi user
		- login: pi
		- password: loragateway
		- **it is strongly advise to change the pi user's password**		
	- The LoRa gateway starts automatically when RPI is powered on
	- With an RPI3 and RPI0W, the Raspberry will automatically act as a WiFi access point
		- SSID=WAZIUP_PI_GW_27EB27F90F for instance
		- password=loragateway
		- **it is strongly advise to change this WiFi password**
	- Includes most of features described here but a full update with the latest version is **highly recommended, see below**	
	- By default, incoming data are uploaded to our [LoRa ThingSpeak test channel](https://thingspeak.com/channels/66794)
	- Works out-of-the-box with the [Arduino_LoRa_Simple_temp sketch](https://github.com/CongducPham/LowCostLoRaGw/tree/master/Arduino/Arduino_LoRa_Simple_temp)

		
Installing the latest gateway version 
=====================================

The full, latest distribution of the low-cost gateway is available in the gw_full_latest folder of the github. It contains all the gateway control and post-processing software. The **simplest and recommended way** to install a new gateway is to use [our zipped SD card image](http://cpham.perso.univ-pau.fr/LORA/WAZIUP/raspberrypi-jessie-WAZIUP-demo.dmg.zip) and perform a new install of the gateway from this image. In this way you don't need to install the various additional packages that are required (as explained in the various README files). Once you have burnt the SD image on a 8GB (minimum) SD card, insert it in your Raspberry and power it. 

Connect to your new gateway
---------------------------

If you see the WiFi network WAZIUP_PI_GW_XXXXXXXXXX then connect to this WiFi network. The address of the Raspberry is then 192.168.200.1. If you see no WiFi access point (e.g. RP1/RPI2/RPI0 without WiFi dongle), then plug your Raspberry into a DHCP-enabled box/router/network to get an IP address or shared your laptop internet connection to make your laptop acting as a DHCP server. On a Mac, there is a very simple solution [here](https://mycyberuniverse.com/mac-os/connect-to-raspberry-pi-from-a-mac-using-ethernet.html). For Windows, you can follow [this tutorial](http://www.instructables.com/id/Direct-Network-Connection-between-Windows-PC-and-R/) or [this one](https://electrosome.com/raspberry-pi-ethernet-direct-windows-pc/). You can then use [Angry IP Scanner](http://angryip.org/) to determine the assigned IP address for the Raspberry.

We will use in this example 192.168.2.8 for the gateway address (DHCP option in order to have Internet access from the Raspberry)

	> ssh pi@192.168.2.8
	pi@192.168.200.1's password: 
	
	The programs included with the Debian GNU/Linux system are free software;
	the exact distribution terms for each program are described in the
	individual files in /usr/share/doc/*/copyright.
	
	Debian GNU/Linux comes with ABSOLUTELY NO WARRANTY, to the extent
	permitted by applicable law.
	Last login: Thu Aug  4 18:04:41 2016
	
**For the Raspberry Zero**, our SD card image set the RPI in access point mode. However, when in access-point mode, Ethernet over USB with dtoverlay=dwc2 in /boot/config.txt and modules-load=dwc2,g_ether /boot/cmdline.txt is not working. As the usage of the Raspberry Zero is mainly with Internet connection through a cellular network (using for instance the LORANGA board from La Fabrica Alegre) the easiest way to have Internet through Ethernet sharing with our SD card image is to use a USB-Ethernet adapter that will add an eth0 interface on the RPI0. These USB-Ethernet adapter are quite cheap and are really useful on the RPI0 as you can then connect it to a DHCP-enabled router/box just like the other RPI boards.

Upgrade to the latest gateway version	
-------------------------------------

Once you have your SD card flashed with our image, to get directly to the full, latest gateway version, you can either (i) use the provided update script to be run from the gateway, or (ii) download (git clone) the whole repository and copy the entire content of the gw_full_latest folder on your Raspberry, in a folder named lora_gateway or, (iii) get only (svn checkout) the gw_full_latest folder in a folder named lora_gateway. Option (i) is preferable and is basically an automatization of option (iii), however it needs Internet connectivity on the gateway.

First option
------------

If your gateway has Internet connectivity (DHCP with Internet sharing for instance), you can use our update_gw.sh script. Even if the SD card image has a recent version of the gateway software the update_gw.sh script in the lora_gateway/scripts folder it is safer to get the latest version of this script. Simply do:

	> cd /home/pi
	> wget https://raw.githubusercontent.com/CongducPham/LowCostLoRaGw/master/gw_full_latest/scripts/update_gw.sh
	> chmod +x update_gw.sh
	> ./update_gw.sh
	
Note that if you have customized configuration files (i.e. key_*, gateway_conf.json, clouds.json and radio.makefile) in the existing /home/pi/lora_gateway folder, then update_gw.sh will preserve all these configuration files. As the repository does not have a gateway_id.txt file, it will also preserve your gateway id.

Otherwise, if it is really the first time you install the gateway, then you can delete the lora_gateway folder before running the script:

	> rm -rf lora_gateway
	> ./update_gw.sh

Second option
-------------

This upgrade solution can be done on the Raspberry if it has Internet connectivity or on your laptop which is assumed to have Internet connectivity. If you don't have git installed on your laptop, you have to install it first. Then get all the repository:

	> cd /home/pi
	> git clone https://github.com/CongducPham/LowCostLoRaGw.git
	
You will get the entire repository:

	pi@raspberrypi:~ $ ls -l LowCostLoRaGw/
	total 32
	drwxr-xr-x 7 pi pi  4096 Apr  1 15:38 Arduino
	-rw-r--r-- 1 pi pi 15522 Apr  1 15:38 README.md	
	drwxr-xr-x 2 pi pi  4096 Apr  1 15:38 gw_full_latest	
	drwxr-xr-x 2 pi pi  4096 Apr  1 15:38 tutorials
	
Create a folder named "lora_gateway" (or if you already have one, then delete all its content) then copy all the files of the LowCostLoRaGw/gw_full_latest folder in it.

    > mkdir lora_gateway
    > cd lora_gateway
    > cp -R ../LowCostLoRaGw/gw_full_latest/* .
    
Or if you want to "move" the LowCostLoRaGw/gw_full_latest folder, simply do (without creating the lora_gateway folder before):

	> mv LowCostLoRaGw/gw_full_latest ./lora_gateway  
	
If you download the repository from your laptop, then rename gw_full_latest into lora_gateway and copy the entire lora_gateway folder into the Raspberry using scp for instance. In the example below, the laptop has wired Internet connectivity and use the gateway's advertised WiFi to connect to the gateway. Therefore the IP address of the gateway is 192.168.200.1.

	> scp -r lora_gateway pi@192.168.200.1:/home/pi
	
If you don't want to use/install git, use your laptop to get the .zip file of the [entire github](https://github.com/CongducPham/LowCostLoRaGw) with the "Clone or download", unzip the package, rename the gw_full_latest folder as lora_gateway and perform the scp command.		  

Third option
------------

This upgrade solution can be done on the Raspberry if it has Internet connectivity or on your laptop which is assumed to have Internet connectivity. If you don't have svn installed on your laptop, you have to install it first. Then get only the gateway part:

	> cd /home/pi
	> svn checkout https://github.com/CongducPham/LowCostLoRaGw/trunk/gw_full_latest lora_gateway
	
That will create the lora_gateway folder and get all the file of (GitHub) LowCostLoRaGw/gw_full_latest in it. 
	
To install svn on the Raspberry:

	> sudo apt-get install subversion	
	
Here, again, you can do all these steps on your laptop and then use scp to copy to the Raspberry.	

Configuring your gateway after update
-------------------------------------

After gateway update, you need to configure your new gateway with basic_config_gw.sh, that mainly assigns the gateway id so that it is uniquely identified (the gateway's WiFi access point SSID is based on that gateway id for instance). The gateway id will be the last 5 bytes of the Rapberry eth0 MAC address (or wlan0 on an RPI0W without Ethernet adapter) and the configuration script will extract this information for you. There is an additional script called test_gwid.sh in the script folder to test whether the gateway id can be easily determined. In the script folder, simply run test_gwid.sh:

	> cd /home/pi/lora_gateway/scripts
	> ./test_gwid.sh
	Detecting gw id as 00000027EBBEDA21
	
If you don't see something similar to 00000027EBBEDA21 (8 bytes in hex format) then you have to explicitly provide the **last 5 bytes**	of the gw id to basic_config_gw.sh. Otherwise, in the script folder, simply run basic_config_gw.sh to automatically configure your gateway. 

	> cd /home/pi/lora_gateway/scripts
	> ./basic_config_gw.sh
	
or

	> ./basic_config_gw.sh 27EBBEDA21

If you need more advanced configuration, then run config_gw.sh as described [here](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README.md#configure-your-gateway-with-config_gwsh). However, basic_config_gw.sh should be sufficient for most of the cases. The script also compile the gateway program. After configuration, reboot your Raspberry. 

By default gateway_conf.json configures the gateway with a simple behavior: LoRa mode 1 (BW125SF12), no DHT sensor in gateway (so no MongoDB for DHT sensor), no downlink, no AES, no raw mode. clouds.json enables only the ThingSpeak demo channel (even the local MongoDB storage is disabled). You can customize your gateway later when you have more cloud accounts and when you know better what features you want to enable.

The LoRa gateway starts automatically when RPI is powered on. Then use cmd.sh to execute the main operations on the gateway as described in [here](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README.md#use-cmdsh-to-interact-with-the-gateway).	


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
			"pin": "0000",
			"contact_sms":["+33XXXXXXXXX","+33XXXXXXXXX"],
			"gammurc_file":"/home/pi/.gammurc"
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
	
**Note that if you want to run and test the above command now**, you have to create a "Dropbox" folder in your home directory with a subfolder "LoRa-test" that will be used locally. Please put attention to the name of the folders: they must be "Dropbox/LoRa-test" because the "post_processing_gw.py" Python script uses these paths. You can mount Dropbox later on if you want: the local folders and contents will be unchanged. **Otherwise, just run the basic_config_gw.sh configuration script as described [previously](https://github.com/CongducPham/LowCostLoRaGw#configuring-your-gateway-after-update) (recommended)**.

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

As suggested by some people, we provide here a simple Ping-Pong program to upload on an Arduino board. First, install the Arduino IDE. You can use the latest one (we tested with 1.8.3). But then, check (see the board manager) that the Arduino AVR board library is not above 1.6.11 as there might be some compilation issues because of the change of the GCC AVR compiler. Then, in your sketch folder, copy the content of the Arduino folder of the distribution.

Run the gateway with:

	> sudo ./lora_gateway
	
With the Arduino IDE, open the Arduino_LoRa_Ping_Pong sketch compile it and upload to an Arduino board. Check your radio module first, see "Connect a radio module to your end-device" above.

The end-device runs in LoRa mode 1 and has address 8. Open the Serial Monitor (38400 bauds) to see the output of the Arduino. It will send "Ping" to the gateway by requesting an ACK every 10s. If the ACK is received then it will display "Pong received from gateway!" otherwise it displays "No Pong!".

Note that in most operational scenarios, requesting ACK from the gateway is costly. Look at the next examples to see how we usually send data without requesting ACK.

**Notice for low-cost/clone Arduino boards**. If you get a low-cost Arduino board, such as those sold by most of Chinese manufacturer, the USB connectivity is probably based on the CH340 or CH341. To make your low-cost Arduino visible to your Arduino IDE, you need the specific driver. Look at http://sparks.gogo.co.nz/ch340.html or http://www.microcontrols.org/arduino-uno-clone-ch340-ch341-chipset-usb-drivers/. For MacOS, you can look at http://www.mblock.cc/posts/run-makeblock-ch340-ch341-on-mac-os-sierra which works for MacOS up to Sierra. For MacOS user that have the previous version of CH34x drivers and encountering kernel panic with Sierra, don't forget to delete previous driver installation: "sudo rm -rf /System/Library/Extensions/usb.kext".


A simple end-device example that periodically sends temperature to the gateway
==============================================================================

See the [video here](https://www.youtube.com/watch?v=YsKbJeeav_M).

First, install the Arduino IDE. You can use the latest one (we tested with 1.8.3). But then, check (see the board manager) that the Arduino AVR board library is not above 1.6.11 as there might be some compilation issues because of the change of the GCC AVR compiler. Then, in your sketch folder, copy the content of the Arduino folder of the distribution.

With the Arduino IDE, open the Arduino_LoRa_Simple_temp sketch, compile it and upload to an Arduino board. Check your radio module first, see "Connect a radio module to your end-device" above.

The end-device runs in LoRa mode 1 and has address 8. It will send data to the gateway.

The default configuration uses an application key filter set to [5, 6, 7, 8].

Use a temperature sensor (e.g. LM35DZ) and plugged in pin A0 (analog 0). You can use a power pin to power your temperature sensor if you are not concerned about power saving. Otherwise, you can use digital 8 (the sketch set this pin HIGH when reading value, then sets it back to LOW) and activate low power mode (uncomment #define LOW_POWER), see below. 

For low-power applications the Pro Mini from Sparkfun is certainly a good choice. This board can be either in the 5V or 3.3V version. With the Pro Mini, it is better to really use the 3.3V version running at 8MHz as power consumption will be reduced. Power for the radio module can be obtained from the VCC pin which is powered in 3.3v when USB power is used or when unregulated power is connected to the RAW pin. If you power your Pro Mini with the RAW pin you can use for instance 4 AA batteries to get 6V. If you use a rechargeable battery you can easily find 3.7V Li-Ion packs. In this case, you can inject directly into the VCC pin but make sure that you've unsoldered the power isolation jumper, see Pro Mini schematic on the Arduino web page. To save more power, you can remove the power led.

The current low-power version for Arduino board use the RocketScream Low Power library (https://github.com/rocketscream/Low-Power) and can support most Arduino platforms although the Pro Mini platform will probably exhibit the best energy saving (we measured 124uA current in sleep mode with the power led removed). You can buid the low-power version by uncommenting the LOW_POWER compilation define statement. Then set "int idlePeriodInMin = 10;" to the number of minutes between 2 wake-up. By default it is 10 minutes. There are good web site describing low-power optimization for the pro Mini: http://www.home-automation-community.com/arduino-low-power-how-to-run-atmega328p-for-a-year-on-coin-cell-battery/ or https://andreasrohner.at/posts/Electronics/How-to-modify-an-Arduino-Pro-Mini-clone-for-low-power-consumption/. 

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

Running in 433MHz and 900MHz band
==================================

When your radio module can run in the 433MHz band (for instance when the radio is based on SX1276 or SX1278 chip, such as the inAir4 from Modtronics) then you can test running at 433MHz as follows:

- select line "#define BAND433" in Arduino_LoRa_temp or Arduino_LoRa_Simple_temp
- compile the lora_gateway.cpp with "#define BAND433"
- or simply run your gateway with "lora_gateway --mode 1 --freq 433.3" to be on the same setting than Arduino_LoRa_temp and Arduino_LoRa_Simple_temp
- there are 4 channels in the 433MHz band: 433.3MHz as CH_00_433, 433.6MHz as CH_01_433, 433.9MHz as CH_02_433 and 434.3MHz as CH_03_433. CH_00_433=433.3MHz is the default channel in the 433MHz band.

For 900MHz band the procedure is similar:

- select line "#define BAND900" in Arduino_LoRa_temp or Arduino_LoRa_Simple_temp
- compile the lora_gateway.cpp with "#define BAND900"
- or simply run your gateway with "lora_gateway --mode 1 --freq 913.88" to be on the same setting than Arduino_LoRa_temp and Arduino_LoRa_Simple_temp
- there are 13 channels in the 900MHz band: from CH_00_900 to CH_12_900. CH_05_900=913.88MHz is the default channel in the 900MHz band. 


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

Pre-defined channels in 868MHz, 915MHz and 433MHz band (most of them from initial Libelium SX1272.h, except those marked with *). Frequencies in bold are those used by default in each band.

| ch | F(MHz) | ch | F(MHz) | ch | F(MHz) |
|----|--------|----|--------|----|--------|
| 04 | 863.2* | 00 | 903.08 | **00** | **433.3*** |
| 05 | 863.5* | 01 | 905.24 | 01 | 433.6* |
| 06 | 863.8* | 02 | 907.40 | 02 | 433.9* |
| 07 | 864.1* | 03 | 909.56 | 03 | 434.3* |
| 08 | 864.4* | 04 | 911.72 |  - |   -    |
| 09 | 864.7* | **05** | **913.88** |  - |   -    |
| **10** | **865.2**  | 06 | 916.04 |  - |   -    |
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


Enjoy!
C. Pham