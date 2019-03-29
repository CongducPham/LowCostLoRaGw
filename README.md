Low-cost LoRa IoT framework developed in the EU H2020 WAZIUP/WAZIHUB projects
=====================================================================

<img src="https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/WAZIUP.png" width="300">
<img src="https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/wazihub.png" width="300">

Quick start
-----------

- **The current SD card image has everything you need!** [zipped SD card image](http://cpham.perso.univ-pau.fr/LORA/WAZIUP/raspberrypi-jessie-WAZIUP-demo.dmg.zip)
- We have a new online [Arduino-Sensor-LoRa tutorial](http://cpham.perso.univ-pau.fr/LORA/WAZIUP/tuto/index.html) and here is the direct link to the [LoRa part](http://cpham.perso.univ-pau.fr/LORA/WAZIUP/tuto/sensors/lora_sensor/Arduino_lora_demo.html)
- Read the [Low-cost-LoRa-device leaflet](https://github.com/CongducPham/tutorials/blob/master/Low-cost-LoRa-device-leaflet.pdf)
- Read the [Low-cost-LoRa-GW leaflet](https://github.com/CongducPham/tutorials/blob/master/Low-cost-LoRa-GW-leaflet.pdf)
- Look at the [Low-cost-LoRa-GW web admin interface tutorial](https://github.com/CongducPham/tutorials/blob/master/Low-cost-LoRa-GW-web-admin.pdf). For most end-users, the web admin interface is sufficient to configure and manage the gateway. **For instance, to configure a new gateway, it is recommended to use the web admin interface to update the gateway software and to run the basic configuration procedure as described [here](https://github.com/CongducPham/LowCostLoRaGw#option-i)**.
- **The rest of this README file contains additional information to better understand the proposed low-cost IoT framework**.

PCBs
----

In order to facilitate connection between an Arduino board or a RaspberryPI and the well-known RFM95 LoRa radio module, we developed simple PCBs for Arduino ProMini, Arduino Nano and RaspberryPI and make them **freely** available. 

![](https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/RFM95Breakout.png)

The first PCB is a simple RFM95 breakout with header pin for both the Raspberry (to make a gateway) and Arduino boards. This PCB can therefore be connected to the GPIO header row of the Raspberry as shown below.

![](https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/RFMonRPI.png)

Or on an Arduino board as it will be explained in Section [connect a radio module to your end-device](https://github.com/CongducPham/LowCostLoRaGw#connect-a-radio-module-to-your-end-device) and illustrated below. On the breakout Arduino header, you can connect RFM95's DIO0 to a digital pin of your Arduino if you want to use another communication library that needs this pin. Our communication library does not need it but we left this possibility open.

![](https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/RFMonArduino.png)

Then, 2 specific PCBs for Arduino Nano and Arduino ProMini are available as shown below. The PCB for Arduino Nano is mainly intended for teaching/training purpose as the Nano is not energy-efficient enough for real deployment. However its main advantage is to avoid the need of an external FTDI breakout cable to program it.

![](https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/NanoLoRaBreakout.png)

The PCB for the Arduino ProMini (3.3v, 8MHz version) can be used for prototyping and even integration purpose. Beware that A4 and A5 (which are usually SDA and SCL pin of the I2C bus) are not connected on the PCB. If you need to use them, use these 2 pins from the Arduino board.

![](https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/ProMiniLoRaBreakout.png)

All the PCBs have footprint for an SMA connector. Both Nano and ProMini PCBs have replicated rows for all the pins. They also have extra VCC and GND rails. They additionally have 2 pads that can be soldered together if you want to connect RFM95's DIO0 to a digital pin of your Arduino to use another communication library that needs this pin. Our communication library does not need it but we left this possibility open. We also indicate clearly which pin of the RFM95 you need to solder on the PCB (for instance `MOSI>`). As you can see on the picture, you can directly solder the Arduino board on the PCB, or, as we did, use intermediate headers so that the Arduino board can be easily plugged and removed.

You can download all the Gerber zipped archive and view them on an [online Gerber viewer](http://www.gerber-viewer.com/).

- RFM95 breakout zipped Gerber archive, 2 layer board of 29x37mm [.zip](https://github.com/CongducPham/LowCostLoRaGw/raw/master/PCBs/RFM95Breakout_2018-10-24.zip)
- Arduino Nano breakout zipped Gerber archive, 2 layer board of 30x81mm [.zip](https://github.com/CongducPham/LowCostLoRaGw/raw/master/PCBs/NanoLoRaBreakout_2018-10-24.zip)
- Arduino ProMini breakout zipped Gerber archive, 2 layer board of 30x77mm [.zip](https://github.com/CongducPham/LowCostLoRaGw/raw/master/PCBs/ProMiniLoRaBreakout_2018-10-24.zip)

You can easily make them produced on many online PCB manufacturers. Usually, you just need to provide the zip archive and both size and number of layers are detected. You can dramatically decrease the price by using "panelize" option. As you can see on the pictures, we use 3x2 for the RFM95 breakout and 3x1 for both the Nano and ProMini breakout. For instance, we ordered them from [JLCPCB](https://jlcpcb.com/) and the cost of 10 panels (i.e. 60 RFM breakout or 30 Nano/ProMini breakout) is about $2!

Tutorial materials
------------------

Also consult the following web page: http://cpham.perso.univ-pau.fr/LORA/RPIgateway.html

3 tutorial videos on YouTube: video of all the steps to build the whole framework from scratch:

- [Build your low-cost, long-range IoT device with WAZIUP](https://www.youtube.com/watch?v=YsKbJeeav_M)
- [Extreme low-cost & low-power LoRa IoT for real-world deployment](https://www.youtube.com/watch?v=2_VQpcCwdd8)
- [Build your low-cost LoRa gateway with WAZIUP](https://www.youtube.com/watch?v=mj8ItKA14PY)

1 live demo video on YouTube: setting up a gateway in less than 5 mins to upload data to various clouds

- [Setting up a LoRa gateway in less than 5mins (live demo)](https://www.youtube.com/watch?v=CJbUFXLpSok)

Go to [https://github.com/CongducPham/tutorials](https://github.com/CongducPham/tutorials) for all tutorials and particularly look for:

- [Low-cost-LoRa-GW leaflet](https://github.com/CongducPham/tutorials/blob/master/Low-cost-LoRa-GW-leaflet.pdf)
- [Low-cost-LoRa-device leaflet](https://github.com/CongducPham/tutorials/blob/master/Low-cost-LoRa-device-leaflet.pdf)
- [Low-cost-LoRa-IoT-step-by-step tutorial](https://github.com/CongducPham/tutorials/blob/master/Low-cost-LoRa-IoT-step-by-step.pdf) 
- [Low-cost-LoRa-GW-step-by-step tutorial](https://github.com/CongducPham/tutorials/blob/master/Low-cost-LoRa-GW-step-by-step.pdf)
- [Low-cost-LoRa-GW web admin interface tutorial](https://github.com/CongducPham/tutorials/blob/master/Low-cost-LoRa-GW-web-admin.pdf)
- [Low-cost-LoRa-IoT-antennaCable tutorial](https://github.com/CongducPham/tutorials/blob/master/Low-cost-LoRa-IoT-antennaCable.pdf)

Look also at our [FAQ](https://github.com/CongducPham/tutorials/blob/master/FAQ.pdf)!

Main features of gateway
------------------------
- **NEW** better support of LoRaWAN and connection to TTN platform
	- see [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-TTN.md) for TTN support and configuration
	- see this [section](https://github.com/CongducPham/LowCostLoRaGw/tree/master/Arduino#lorawan-example-and-support) for the device side
	- see [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-aes_lorawan.md) for updated info on LoRaWAN
- **NEW** remote access to your gateway from anywhere with `remot3.it`
	- [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-remote-access-remot3it.md)
- remote access to your gateway from anywhere with `ngrok`
	- [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-remote-access-ngrok.md)
- a simple, user-friendly web admin interface to configure and update your gateway
	- open a browser and go to http://`gw_ip_address`/admin
	- [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/gw_web_admin/README.md)
	- [Tutorial](https://github.com/CongducPham/tutorials/blob/master/Low-cost-LoRa-GW-web-admin.pdf)
	- `raspap-webgui` from https://github.com/billz/raspap-webgui has been integrated and can be accessed at http://`gw_ip_address`/raspap-webgui. When the gateway is configured as a WiFi client, `raspap-webgui` is especially useful to dynamically discover and configure additional WiFi networks. Read this [section](https://github.com/CongducPham/LowCostLoRaGw/tree/master/gw_full_latest#make-your-gateway-a-wifi-client) prior to use `raspap-webgui`.	
- simple, flexible and generic cloud management approach
	- [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-NewCloud.md)
	- look at the provided cloud script examples to see how IoT clouds such as TTN, ThingSpeak, GroveStreams, MQTT,... are supported
	- a cloud script can be used to generalize the upload of data using SMS, ftp, file, MQTT, Node-Red flow,...
- encryption and native LoRaWAN frame format
	- see [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-aes_lorawan.md)
	- gateway can receive LoRaWAN packets from LoRaWAN devices (including Arduino LMIC based device)
	- low-level gateway can provide raw output for `post_processing_gw.py` to handle LoRaWAN packets and push to TTN using `CloudTTN.py` for instance
- downlink features: to send from gateway to end-device
	- [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-downlink.md) 	 	
- an alert mail can be sent to a list of contact email addresses to notify when gateway is starting and when the radio module has been reset
- periodic status report to monitor whether the post-processing stage of the gateway is up or not
- support for an embedded DHT22 temperature/humidity sensor to monitor the condition inside the gateway case
- there is a NoSQL MongoDB support and received data can be saved in the local database if this feature is activated. See [here](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-NewCloud.md#support-of-mongodb-as-a-cloud-declaration) for more information on the local MongoDB structure.
- the gateway acts as the WiFi access-point. The SSID is WAZIUP_PI_GW_XXXXXXXXXX where XXXXXXXXXX is the last 5 hex bytes of gateway ID: WAZIUP_PI_GW_27EB27F90F for instance. It has IP address 192.168.200.1 and will lease IP addresses in the range of 192.168.200.100 and 192.168.200.120. 
- there is an Apache web server with basic PHP forms to visualize graphically the received data of the MongoDB with any web browser. Just connect to http://192.168.200.1 with a web browser (could be from a smartphone) to get the graphic visualization of the data stored in the gateway's MongoDB database.
- there is the support of Bluetooth connection. A simple Android App running on Android smartphone displays the data stored in the gateway's MongoDB database.
- by default, incoming data are uploaded to our [LoRa ThingSpeak test channel](https://thingspeak.com/channels/66794)
- works out-of-the-box with the [Arduino_LoRa_Simple_temp sketch](https://github.com/CongducPham/LowCostLoRaGw/tree/master/Arduino/Arduino_LoRa_Simple_temp)

Connect a radio module to the Raspberry
=======================================

You have to connect a LoRa radio module to the Raspberry's GPIO header. Just connect the corresponding SPI pin (MOSI, MISO, CLK, CS). 

```
      RPI            Radio module
   GND pin 25----------GND   (ground in)
   3V3 pin 17----------3.3V  (3.3V in)
CS/CE0 pin 24----------NSS   (CS chip select in)
   SCK pin 23----------SCK   (SPI clock in)
  MOSI pin 19----------MOSI  (SPI Data in)
  MISO pin 21----------MISO  (SPI Data out)
```

You can have a look at the "Low-cost-LoRa-GW-step-by-step" tutorial in our tutorial repository https://github.com/CongducPham/tutorials.

![](https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/connect-radio-rpi.png)

**Or you can of course use our RFM95 breakout PCB that can be directly connected to the RPI GPIO header**.

Installing the latest gateway version 
=====================================

The full, latest distribution of the low-cost gateway is available in the `gw_full_latest` folder of the github repository: https://github.com/CongducPham/LowCostLoRaGw. It contains all the gateway control and post-processing software. 

However, the **simplest and recommended way** to install a new gateway is to use [our zipped SD card image](http://cpham.perso.univ-pau.fr/LORA/WAZIUP/raspberrypi-jessie-WAZIUP-demo.dmg.zip) based on the Jessie Raspbian OS and perform a new install of the gateway from this image. In this way you don't need to install the various additional packages that are required (as explained in an additional [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-manual-install.md) describing all manual installation steps if you want to install from scratch). Once you have burnt the SD image on a 8GB (minimum) SD card, insert it in your Raspberry and power it.

The distribution supports Raspberry 1B+, RPI2, RPI3B/B3+, RPI0 and RPI0W. **For RPI1, RPI0 and RPI0W you need to run `make lora_gateway` as the default version is built for RPI2&RPI3**. There is out-of-the-box WiFi support for RPI3B/3B+ and RPI0W. For RPI1 and RPI2 see [here](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README.md#wifi-instructions-on-rpi1b-and-rpi2) for modifications to support some WiFi dongles.


Get our SD card image
---------------------

Download our [zipped SD card image](http://cpham.perso.univ-pau.fr/LORA/WAZIUP/raspberrypi-jessie-WAZIUP-demo.dmg.zip). The current image has everything you need including:

- support for RPI3B+ as well (including the WiFi)
- `remot3.it` tools for remote access
- the simple gateway web admin interface for easy configuration and management
- `mosquitto-clients` package installed to have `mosquitto_pub` and `mosquitto_sub` commands (v1.5)
- Node-Red v0.18.6, Node.js v9.11.1 and `npm` upgraded with `node-red-contrib-thingspeak42` installed
- a ready-to-use Node-Red flow to show how received data can be uploaded to MQTT brokers and ThingSpeak
- MongoDB v3.0.9

The image should work on RPI0 to RPI3B/3B+. For RPI3B+, we performed an update with `rpi-update` on Jessie. Then we followed the procedures in [this link](https://www.linuxquestions.org/questions/slackware-arm-108/raspberry-pi-3-b-wifi-nic-not-found-4175627137/) to copy over the drivers for the 3B+ WiFi radios. The image has already all these features and should work on all RPI models. We however recommend the 3B version unless you need to run other applications on the RPI otherwise the last 3B+ is much more power consuming and heats a lot more.

It is not a MacOS X DMG package as the extension may be misleading, simply unzip the file and burn the dmg file to an SD card. Use an SD card of a minimum of 8GB. Take also a class 10 (it is more than recommended!). If you have bigger SD card, e.g. 16GB, then after boot, use `raspi-config` (see [tutorial here](https://www.raspberrypi.org/documentation/configuration/raspi-config.md)) to resize the partition in order to use the extra space available (you will need to reboot but `raspi-config` will ask you for that). You can use `df -h` to verify that you have more space after reboot.  

You can look at various tutorials on how to burn an image to an SD card. There is one [here from raspberrypi.org](https://www.raspberrypi.org/documentation/installation/installing-images/) and [here from elinux.org](http://elinux.org/RPi_Easy_SD_Card_Setup). We use a Mac to do so and [this is our preferred solution](https://www.raspberrypi.org/documentation/installation/installing-images/mac.md). The [Linux version](https://www.raspberrypi.org/documentation/installation/installing-images/linux.md) is not very different. The `Etcher` tool is also very nice and you don't even need to unzip the SD card image. We also use that solution extensively.

When booting from the provided SD card image
============================================ 

**Important notice**: the LoRa gateway program starts automatically when the Raspberry is powered on. 

**You can try your gateway right away now by building your end-device and jump directly to this [section](https://github.com/CongducPham/LowCostLoRaGw#connect-a-radio-module-to-your-end-device)**.

If you want to connect (`ssh`) to the gateway and perform update procedure, read the rest of the section.
	
Connect to your new gateway
---------------------------

The SD card image defines a `pi` user:

	- login: pi
	- password: loragateway
		
With the default gateway configuration, the gateway acts as a WiFi access point. If you see the WiFi network `WAZIUP_PI_GW_XXXXXXXXXX` then connect to this WiFi network. The WiFi password is `loragateway`. **It is strongly advise to change this WiFi password**. The address of the Raspberry is then 192.168.200.1. Note that it is very convenient to use a smartphone or a tablet to connect to your gateway with `ssh`. On iOS we tested `Termius`  and on Android we tested `JuiceSSH`.

If you see no WiFi access point (e.g. RP1/RPI2/RPI0 without WiFi dongle), then plug your Raspberry into a DHCP-enabled box/router/network to get an IP address or shared your laptop internet connection to make your laptop acting as a DHCP server. On a Mac, there is a very simple solution [here](https://mycyberuniverse.com/mac-os/connect-to-raspberry-pi-from-a-mac-using-ethernet.html). For Windows, you can follow [this tutorial](http://www.instructables.com/id/Direct-Network-Connection-between-Windows-PC-and-R/) or [this one](https://electrosome.com/raspberry-pi-ethernet-direct-windows-pc/). You can then use [Angry IP Scanner](http://angryip.org/) available on Windows/Mac/Linux/Android to determine the assigned IP address for the Raspberry.

We will use in this example 192.168.2.8 for the gateway address (DHCP option in order to have Internet access from the Raspberry)

	> ssh pi@192.168.2.8
	pi@192.168.200.1's password: 
	
	The programs included with the Debian GNU/Linux system are free software;
	the exact distribution terms for each program are described in the
	individual files in /usr/share/doc/*/copyright.
	
	Debian GNU/Linux comes with ABSOLUTELY NO WARRANTY, to the extent
	permitted by applicable law.
	Last login: Thu Aug  4 18:04:41 2016
	
**For the Raspberry Zero**, our SD card image set the RPI in access point mode. However, when in access-point mode, Ethernet over USB with `dtoverlay=dwc2` in `/boot/config.txt` and `modules-load=dwc2,g_ether` in `/boot/cmdline.txt` is not working. As the usage of the Raspberry Zero is mainly with Internet connection through a cellular network (using for instance the LORANGA board from La Fabrica Alegre) the easiest way to have Internet through Ethernet sharing with our SD card image is to use a USB-Ethernet adapter that will add an eth0 interface on the RPI0. These USB-Ethernet adapter are quite cheap and are really useful on the RPI0 as you can then connect it to a DHCP-enabled router/box just like the other RPI boards. Alternatively, you can set your gateway as a WiFi client to connect to an existing WiFi network, see [here](https://github.com/CongducPham/LowCostLoRaGw/tree/master/gw_full_latest#make-your-gateway-a-wifi-client).

Upgrade to the latest gateway version	
-------------------------------------

Once you have your SD card flashed with our image, to get directly to the full, latest gateway version, you can either use (i) the web admin interface, or (ii) the provided update script to be run from the gateway, or (iii) download (git clone) the whole repository and copy the entire content of the gw_full_latest folder on your Raspberry, in a folder named `lora_gateway` or, (iv) get only (`svn checkout`) the `gw_full_latest` folder in a folder named `lora_gateway`. Option (i) is recommended and simple while (ii) is basically an automatization of option (iii) in command line mode. Both options (i) and (ii) however need Internet connectivity on the gateway while option (iii) and (iv) can be done on a computer prior to copy the files on the Raspberry.

Option (i)
----------

**It is the recommended option but the Raspberry must have Internet connection**.

![](https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/gw-web-admin-update.png)

Choose the `Gateway Update` menu on the left. Then, if you install a brand new gateway with our SD card image, use `New installation`. For future updates, use `Full update`.

After `New installation` or `Full update`, run `Basic config`.

When using the web interface, you don't need to perform the manual `Configuring your gateway after update` step below.

It is also recommended to run `Update web admin interface` to update the web interface to the last version, after `New installation` or `Full update`.

Then reboot.

**The full LoRa gateway with post-processing stage starts automatically when the Raspberry is powered on**. By default, incoming data are uploaded to our [LoRa ThingSpeak test channel](https://thingspeak.com/channels/66794). It will work out-of-the-box with the [Arduino_LoRa_Simple_temp sketch](https://github.com/CongducPham/LowCostLoRaGw/tree/master/Arduino/Arduino_LoRa_Simple_temp).

Option (ii)
-----------

If your gateway has Internet connectivity (DHCP with Internet sharing on your laptop for instance), you can use our `update_gw.sh` script. Even if the SD card image has a recent version of the gateway software with the `update_gw.sh` script in the `lora_gateway/scripts` folder it is safer to get the latest version of this script. Simply do:

	> cd /home/pi
	> wget https://raw.githubusercontent.com/CongducPham/LowCostLoRaGw/master/gw_full_latest/scripts/update_gw.sh
	> chmod +x update_gw.sh
	> ./update_gw.sh
	
Note that if you have customized configuration files (i.e. `key_*`, `gateway_conf.json`, `clouds.json` and `radio.makefile`) in the existing `/home/pi/lora_gateway` folder, then `update_gw.sh` will preserve all these configuration files. As the repository does not have a `gateway_id.txt` file, it will also preserve your gateway id.

Otherwise, if it is really the first time you install the gateway, then you can delete the `lora_gateway` folder before running the script:

	> rm -rf lora_gateway
	> ./update_gw.sh

Option (iii)
------------

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
	
Create a folder named `lora_gateway` (or if you already have one, then delete all its content) then copy all the files of the `LowCostLoRaGw/gw_full_latest` folder in it.

    > mkdir lora_gateway
    > cd lora_gateway
    > cp -R ../LowCostLoRaGw/gw_full_latest/* .
    
Or if you want to "move" the `LowCostLoRaGw/gw_full_latest` folder, simply do (without creating the `lora_gateway` folder before):

	> mv LowCostLoRaGw/gw_full_latest ./lora_gateway  
	
If you download the repository from your laptop, then rename `gw_full_latest` into `lora_gateway` and copy the entire `lora_gateway` folder into the Raspberry using `scp` for instance. In the example below, the laptop has wired Internet connectivity and use the gateway's advertised WiFi to connect to the gateway. Therefore the IP address of the gateway is 192.168.200.1.

	> scp -r lora_gateway pi@192.168.200.1:/home/pi
	
If you don't want to use/install `git`, use your laptop to get the .zip file of the [entire github](https://github.com/CongducPham/LowCostLoRaGw) with the "Clone or download" option, unzip the package, rename the `gw_full_latest` folder as `lora_gateway` and perform the `scp` command.		  

Option (iv)
-----------

This upgrade solution can be done on the Raspberry if it has Internet connectivity or on your laptop which is assumed to have Internet connectivity. If you don't have `svn` installed on your laptop, you have to install it first. Then get only the gateway part:

	> cd /home/pi
	> svn checkout https://github.com/CongducPham/LowCostLoRaGw/trunk/gw_full_latest lora_gateway
	
That will create the `lora_gateway` folder and get all the file of (GitHub) `LowCostLoRaGw/gw_full_latest` in it. 
	
To install `svn` on the Raspberry:

	> sudo apt-get install subversion	
	
Here, again, you can do all these steps on your laptop and then use `scp` to copy to the Raspberry.	

Configuring your gateway after update
-------------------------------------

After gateway update with option (ii), (iii) or (iv), you need to configure your new gateway with `basic_config_gw.sh`, that mainly assigns the gateway id so that it is uniquely identified (the gateway's WiFi access point SSID is based on that gateway id for instance). The gateway id will be the last 5 bytes of the Rapberry eth0 MAC address (or wlan0 on an RPI0W without Ethernet adapter) and the configuration script will extract this information for you. There is an additional script called `test_gwid.sh` in the script folder to test whether the gateway id can be easily determined. In the `scripts` folder, simply run `test_gwid.sh`:

	> cd /home/pi/lora_gateway/scripts
	> ./test_gwid.sh
	Detecting gw id as 00000027EBBEDA21
	
If you don't see something similar to 00000027EBBEDA21 (8 bytes in hex format) then you have to explicitly provide the **last 5 bytes**	of the gw id to `basic_config_gw.sh`. Otherwise, in the `scripts` folder, simply run `basic_config_gw.sh` to automatically configure your gateway. 

	> cd /home/pi/lora_gateway/scripts
	> ./basic_config_gw.sh
	
or

	> ./basic_config_gw.sh 27EBBEDA21

If you need more advanced configuration, then run `config_gw.sh` as described [here](https://github.com/CongducPham/LowCostLoRaGw/tree/master/gw_full_latest#configure-your-gateway-with-basic_config_gwsh-or-config_gwsh). However, `basic_config_gw.sh` should be sufficient for most of the cases. The script also compile the low-level gateway program corresponding to you Raspberry model. After configuration, reboot your Raspberry. 

By default `gateway_conf.json` configures the gateway with a simple behavior: LoRa mode 1 (BW125SF12), no DHT sensor in gateway (so no MongoDB for DHT sensor), no downlink, no AES, no raw mode. `clouds.json` enables only the ThingSpeak demo channel (even the local MongoDB storage is disabled). You can customize your gateway later when you have more cloud accounts and when you know better what features you want to enable.

The LoRa gateway starts automatically when RPI is powered on. Then use `cmd.sh` to execute the main operations on the gateway as described [here](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README.md#use-cmdsh-to-interact-with-the-gateway).	


Connect a radio module to your end-device
=========================================

To have an end-device, you have to connect a LoRa radio module to an Arduino board. Just connect the corresponding SPI pin (MOSI, MISO, CLK, CS/SS). On the Uno, Pro Mini, Mini, Nano, Teensy the mapping is as follows:

```
       Arduino      Radio module
         GND----------GND   (ground in)
         3V3----------3.3V  (3.3V in)
  SS pin D10----------NSS   (CS chip select in)
 SCK pin D13----------SCK   (SPI clock in)
MOSI pin D11----------MOSI  (SPI Data in)
MISO pin D12----------MISO  (SPI Data out)
```

**For the Nano and the ProMini, you can of course use our Nano/ProMini breakout PCB that makes connection to an RFM95 radio straightforward**.

On the MEGA, the SPI pin are as follows: 50 (MISO), 51 (MOSI), 52 (SCK). Starting from November 3rd, 2017, the CS pin is always pin number 10 on Arduino and Teensy boards. You can have a look at the [Low-cost-LoRa-IoT-step-by-step](https://github.com/CongducPham/tutorials/blob/master/Low-cost-LoRa-IoT-outdoor-step-by-step.pdf) tutorial in the tutorial repository https://github.com/CongducPham/tutorials.

![](https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/connect-radio-device.png)

There is an important issue regarding the radio modules. The Semtech SX1272/76 has actually 2 lines of RF power amplification (PA): a high efficiency PA up to 14dBm (RFO) and a high power PA up to 20dBm (PA_BOOST). Setting transmission power to `L` (Low), `H` (High), and `M` (Max) only uses the RFO and delivers 2dBm, 6dBm and 14dBm respectively. `x` (extreme) and `X` (eXtreme) use the PA_BOOST and deliver 14dBm and 20dBm respectively.

However even if the SX1272/76 chip has the PA_BOOST and the 20dBm features, not all radio modules (integrating these SX1272/76) do have the appropriate wiring and circuits to enable these features: it depends on the choice of the reference design that itself is guided by the main intended frequency band usage, and sometimes also by the target country's regulations (such as maximum transmitted power). So you have to check with the datasheet whether your radio module has PA_BOOST (usually check whether the PA_BOOST pin is wired) and 20dBm capability before using `x` or `X`. Some other radio modules only wire the PA_BOOST and not the RFO resulting in very bad range when trying to use the RFO mode (`L`, `H`, and `M`). In this case, one has to use `x` to indicate PA_BOOST usage to get 14dBm.

Practically, we only use either `M` (Max) or `x` (extreme) to have maximum range. They both deliver 14dBm but the difference is whether the RFO pin is used or the PA_BOOST. Therefore, when uploading a sketch on your board, you have to check whether your radio module needs the PA_BOOST in order to get significant output level in which case `x` should be used instead of `M`. All the examples start with:

	// IMPORTANT
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// uncomment if your radio is an HopeRF RFM92W, HopeRF RFM95W, Modtronix inAir9B, NiceRF1276
	// or you known from the circuit diagram that output use the PABOOST line instead of the RFO line
	//#define PABOOST
	///////////////////////////////////////////////////////////////////////////////////////////////////////////  

Uncomment `PABOOST` if you have a HopeRF RFM92W or RFM95W or RFM96W, or a Modtronix inAir9B (if inAir9, leave commented) or a NiceRF1276. If you have another non listed radio module, try first by leaving `PABOOST` commented, then see whether the packet reception is correct with a reasonably high SNR (such as 6 to 10 dB) at some meters of the gateway. If not, then try with `PABOOST` uncommented.

First try: a simple Ping-Pong program example
=============================================

As suggested by some people, we provide here a simple Ping-Pong program to upload on an Arduino board. First, install the Arduino IDE. You can use the latest one (we tested with 1.8.6). There have been some issues with the Arduino AVR board library version (some time ago, libraries above 1.6.11 showed some compilation issues because of changes in the GCC AVR compiler) but it seems that these issues have been solved. So now, you can also use the latest Arduino AVR board library (1.6.22 at time writing, included with Arduino 1.8.6 IDE). Then, in your sketch folder, copy the content of the Arduino folder of the distribution.

Run the gateway with:

	> sudo ./lora_gateway
	
With the Arduino IDE, open the `Arduino_LoRa_Ping_Pong` sketch compile it and upload to an Arduino board. Check your radio module first, see `Connect a radio module to your end-device` above.

The end-device runs in LoRa mode 1 and has address 8. Open the Serial Monitor (38400 bauds) to see the output of the Arduino. It will send "Ping" to the gateway by requesting an ACK every 10s. If the ACK is received then it will display "Pong received from gateway!" otherwise it displays "No Pong from gw!". There is a version using an OLED display, `Arduino_LoRa_Ping_Pong_LCD`, that can be compiled for the Heltec ESP32 WiFi LoRa OLED board or a regular Arduino Pro Mini board (we use a Pro Mini or a Nano) with a 0.96inch OLED I2C display. Such device can be used as a very convenient simple range tester.

Note that in most operational scenarios, requesting ACK from the gateway is costly. Look at the next examples to see how we usually send data without requesting ACK.

**Notice for low-cost/clone Arduino boards**. If you get a low-cost Arduino board, such as those sold by most of Chinese manufacturer, the USB connectivity is probably based on the CH340 or CH341. To make your low-cost Arduino visible to your Arduino IDE, you need the specific driver. Sparkfun has a nice web page explaning serial connection to Arduino at https://learn.sparkfun.com/tutorials/serial-basic-hookup-guide. Links to CH341 drivers for Windows, Linux and MacOs can be found in the "Drivers If You Need Them" section. There is also this link http://sparks.gogo.co.nz/ch340.html. For MacOS, you can also look at http://www.mblock.cc/posts/run-makeblock-ch340-ch341-on-mac-os-sierra which works for MacOS up to Sierra. For MacOS user that have the previous version of CH34x drivers and encountering kernel panic with Sierra, don't forget to delete previous driver installation: `sudo rm -rf /System/Library/Extensions/usb.kext`.


A simple end-device example that periodically sends temperature to the gateway
==============================================================================

See the [video here](https://www.youtube.com/watch?v=YsKbJeeav_M).

First, install the Arduino IDE. You can use the latest one (we tested with 1.8.3). But then, check (see the board manager) that the Arduino AVR board library is not above 1.6.11 as there might be some compilation issues because of the change of the GCC AVR compiler. Then, in your sketch folder, copy the content of the Arduino folder of the distribution.

With the Arduino IDE, open the `Arduino_LoRa_Simple_temp` sketch, compile it and upload to an Arduino board. Check your radio module first, see `Connect a radio module to your end-device` above.

The end-device runs in LoRa mode 1 and has address 6. It will send data to the gateway.

The default configuration uses an application key filter set to [5, 6, 7, 8].

Use a temperature sensor (e.g. LM35DZ) and plugged in pin A0 (analog 0). You can use a power pin to power your temperature sensor if you are not concerned about power saving. Otherwise, you can use digital 9 (the sketch set this pin HIGH when reading value, then sets it back to LOW) and activate low power mode (uncomment `#define LOW_POWER`), see below. 

For low-power applications the Pro Mini from Sparkfun is certainly a good choice. This board can be either in the 5V or 3.3V version. With the Pro Mini, it is better to really use the 3.3V version running at 8MHz as power consumption will be reduced. Power for the radio module can be obtained from the VCC pin which is powered in 3.3v when USB power is used or when unregulated power is connected to the RAW pin. If you power your Pro Mini with the RAW pin you can use for instance 4 AA batteries to get 6V. If you use a rechargeable battery you can easily find 3.7V Li-Ion packs. In this case, you can inject directly into the VCC pin but make sure that you've unsoldered the power isolation jumper (Sparkfun Pro Mini) or removed the voltage regulator, see Pro Mini schematic on the Arduino web page. To greatly save power, you should remove the power led.

The current low-power version for Arduino board use the RocketScream Low Power library (https://github.com/rocketscream/Low-Power) and can support most Arduino platforms although the Pro Mini platform will probably exhibit the best energy saving (we measured 54uA current in sleep mode with the power led removed and 12uA when the voltage regulator is removed). You can buid the low-power version by uncommenting the `LOW_POWER` compilation define statement. Then set `int idlePeriodInMin = 10;` to the number of minutes between 2 wake-up. By default it is 10 minutes. There are good web site describing low-power optimization for the pro Mini: http://www.home-automation-community.com/arduino-low-power-how-to-run-atmega328p-for-a-year-on-coin-cell-battery/ or https://andreasrohner.at/posts/Electronics/How-to-modify-an-Arduino-Pro-Mini-clone-for-low-power-consumption/. 

For the special case of Teensy boards (LC/31/32/35/36), the power saving mode uses the excellent work of Collin Duffy with the `Snooze` library included by the Teensyduino package. You can upgrade the Snooze library from the github https://github.com/duff2013/Snooze as version 6 is required to handle the new Teensy 35/36 boards. With the Teensy, you can further use the HIBERNATE mode by uncommenting `LOW_POWER_HIBERNATE` in the temperature example.

For the special of the Arduino Zero, waking up the board from deep sleep mode is done with the RTC. Therefore the `RTCZero` library from https://github.com/arduino-libraries/RTCZero is used and you need to install it before being able to compile the example for the Arduino Zero.

Depending on the sensor type, the computation to get the real temperature may be changed accordingly. Here is the instruction for the LM35DZ: http://www.instructables.com/id/ARDUINO-TEMPERATURE-SENSOR-LM35/

The default configuration also use the EEPROM to store the last packet sequence number in order to get it back when the sensor is restarted/rebooted. If you want to restart with a packet sequence number of 0, just comment the line `#define WITH_EEPROM`

Once flashed, the Arduino temperature sensor will send to the gateway the following message `\!#3#TC/20.4` (TC is used as the nomenclature code and 20.4 is the measured temperature so you may not have the same value) prefixed by the application key every 10 minutes (with some randomization interval). This will trigger at the processing stage of the gateway the logging on the default ThinkSpeak channel (the test channel we provide) in field 3. At the gateway, 20.4 will be recorded on the provided ThingSpeak test channel in field 3 of the channel. If you go to https://thingspeak.com/channels/66794 you should see the reported value. 

At the gateway side, the cloud script that handles the `ThingSpeak` cloud is `CloudThingSpeak.py`. As `ThingSpeak` is a simple and very popular IoT cloud, we enhanced `CloudThingSpeak.py` to be able to assign a specific channel write key and a specific chart (field index) depending on the sensor source address. It is also possible to assign a specific field offset depending on the nomenclature. For more detail, read the examples provided in the [`CloudThingSpeak.py`](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/CloudThingSpeak.py) script. 

The program has been tested on Arduino Uno, Mega2560, Nano, Pro Mini, Mini, Due, Zero.  We also tested on the TeensyLC/3.1/3.2, the Ideetron Nexus and the Feather32u4/M0. Starting from November 3rd, 2017, the SPI_SS pin (CS pin) is always pin number 10 on all Arduino and Teensy boards. 

**Notice for low-cost/clone Arduino boards**. If you get a low-cost Arduino board, such as those sold by most of Chinese manufacturer, the USB connectivity is probably based on the CH340 or CH341. To make your low-cost Arduino visible to your Arduino IDE, you need the specific driver. Look at http://sparks.gogo.co.nz/ch340.html or http://www.microcontrols.org/arduino-uno-clone-ch340-ch341-chipset-usb-drivers/. For MacOS, you can look at http://www.mblock.cc/posts/run-makeblock-ch340-ch341-on-mac-os-sierra which works for MacOS up to Sierra. For MacOS user that have the previous version of CH34x drivers and encountering kernel panic with Sierra, don't forget to delete previous driver installation: `sudo rm -rf /System/Library/Extensions/usb.kext`.

![](https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/iot-node.jpg)


An interactive end-device for sending LoRa messages with the Arduino IDE
========================================================================

With the Arduino IDE, open the `Arduino_LoRa_InteractiveDevice` sketch. Then compile it and upload to an Arduino board. It is better to use a more powerful Arduino platform for building the interactive device otherwise stability issues can occur (and especially with more RAM memory such as a MEGA, the Uno, ATMega328P, will be very unstable because of the small amount of memory).

By default, the interactive end-device has address 6 and runs in LoRa mode 1.

Enter `\!SGSH52UGPVAUYG3S#1#21.6` in the input window and press RETURN

The command will be sent to the gateway and you should see the gateway pushing the data to the ThingSpeak test channel. If you go to https://thingspeak.com/channels/66794 you should see the reported value.

When testing with the interactive end-device, you should not use the `--wappkey` option for the `post_processing_gw.py` post-processing Python script otherwise your command will not be accepted as only text string without logging services will be received and displayed when `--wappkey` is set.

	> sudo ./lora_gateway | python ./post_processing_gw.py | python ./log_gw

Use an Arduino as a LoRa gateway
================================

The gateway can also be based on an Arduino board, as described in the web page. With the Arduino IDE, open the `Arduino_LoRa_Gateway` sketch, compile the code and upload to an Arduino board. Then follow instructions on how to use the Arduino board as a gateway. It is better to use a more powerful (and with more RAM memory such as the MEGA) Arduino platform for building the gateway.

Running in 433MHz and 900MHz band
==================================

When your radio module can run in the 433MHz band (for instance when the radio is based on SX1276 or SX1278 chip, such as the inAir4 from Modtronics) then you can test running at 433MHz as follows:

- select line `#define BAND433` in `Arduino_LoRa_temp` or `Arduino_LoRa_Simple_temp`
- compile the `lora_gateway.cpp` with `#define BAND433`
- or simply run your gateway with `lora_gateway --mode 1 --freq 433.3` to be on the same setting than `Arduino_LoRa_temp` and `Arduino_LoRa_Simple_temp`
- there are 4 channels in the 433MHz band: 433.3MHz as `CH_00_433`, 433.6MHz as `CH_01_433`, 433.9MHz as `CH_02_433` and 434.3MHz as `CH_03_433`. `CH_00_433`=433.3MHz is the default channel in the 433MHz band.

For 900MHz band the procedure is similar:

- select line `#define BAND900` in `Arduino_LoRa_temp` or `Arduino_LoRa_Simple_temp`
- compile the `lora_gateway.cpp` with `#define BAND900`
- or simply run your gateway with `lora_gateway --mode 1 --freq 913.88` to be on the same setting than `Arduino_LoRa_temp` and `Arduino_LoRa_Simple_temp`
- there are 13 channels in the 900MHz band: from `CH_00_900` to `CH_12_900`. `CH_05_900`=913.88MHz is the default channel in the 900MHz band. 


Mounting your Dropbox folder
============================

With `sshfs`:

- look at http://mitchtech.net/dropbox-on-raspberry-pi-via-sshfs/. No need of `sudo gpasswd -a pi fuse` on Jessie.
	
	> sudo apt-get install sshfs
	
- then allow option `user_allow_other` in `/etc/fuse.conf`
	
with Dropbox uploader:

- look at http://anderson69s.com/2014/02/18/raspberry-pi-dropbox/
- look at http://raspi.tv/2013/how-to-use-dropbox-with-raspberry-pi
- look at https://github.com/andreafabrizi/Dropbox-Uploader
- but not tested yet and not supported yet	

ANNEX.A: LoRa mode and predefined channels
==========================================

Pre-defined LoRa modes (from initial Libelium `SX1272.h`)

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

Pre-defined channels in 868MHz, 915MHz and 433MHz band (most of them from initial Libelium `SX1272.h`, except those marked with `*`). Frequencies in bold are those used by default in each band.

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

Specific development
====================

Specific developments can be made from the general, public version on github. The framework has been used for instance in Nestlé's WaterSense project and HygieItalia's WISMART project. If you are interested by such customization (advanced and reliable downlink, dynamic and reconfigurable measure interval, PCBs with integrated antennas,...) you can contact me.

<a href="https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/wismart.jpg"><img src="https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/wismart.jpg" width="400"></a>

Enjoy!
C. Pham