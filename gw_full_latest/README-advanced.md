Advanced features and configuration tools for the low-cost LoRa gateway with Raspberry
======================================================================================

**IMPORTANT NOTICE**: We should start first with the simple gateway installation. Then follow the instruction here to have the advanced version.

**NEW**: new cloud management approach: simpler, more generic
=============================================================

https://github.com/CongducPham/LowCostLoRaGw/tree/master/gw_advanced/new_cloud_design

[README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_advanced/new_cloud_design/README-NewCloud.md)

Install this upgrade **before** the downlink upgrade

**NEW**: new downlink features: to send from gateway to end-device
==================================================================

https://github.com/CongducPham/LowCostLoRaGw/tree/master/gw_advanced/downlink

[README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_advanced/downlink/README-downlink.md)

Install this upgrade **after** the new cloud management upgrade

**NEW**: Tutorial videos on YouTube
===================================

There are 2 tutorial videos on YouTube:

- [Build your low-cost, long-range IoT device with WAZIUP](https://www.youtube.com/watch?v=YsKbJeeav_M)
- [Build your low-cost LoRa gateway with WAZIUP](https://www.youtube.com/watch?v=peHkDhiH3lE)

that show in images all the steps to build the whole framework from scratch.

NEW: Zipped SD card image
=========================

[raspberrypi-jessie-WAZIUP-demo.dmg.zip](http://cpham.perso.univ-pau.fr/LORA/WAZIUP/raspberrypi-jessie-WAZIUP-demo.dmg.zip)

- Based on Raspbian Jessie
- Supports Raspberry 1B+, RPI2 and RPI3
- Includes all the advanced features described in the gw_advanced github (have to update for new cloud management and downlink)
- Get the zipped image, unzip it, install it on an 8GB SD card, see [this tutorial](https://www.raspberrypi.org/documentation/installation/installing-images/) from www.raspberrypi.org
- Plug the SD card into your Raspberry
- Connect a radio module (see http://cpham.perso.univ-pau.fr/LORA/RPIgateway.html)
- Power-on the Raspberry
- pi user
	- login: pi
	- password: loragateway
- The LoRa gateway starts automatically when RPI is powered on
- With an RPI3, the Raspberry will automatically act as a WiFi access point
	- SSID=WAZIUP_PI_GW_27EB27F90F
	- password=loragateway
- By default, incoming data are uploaded to the [WAZIUP ThingSpeak demo channel](https://thingspeak.com/channels/123986)
- Works out-of-the-box with the [Arduino_LoRa_Simple_temp sketch](https://github.com/CongducPham/LowCostLoRaGw/tree/master/Arduino/Arduino_LoRa_Simple_temp)

WiFi instructions on RPI1B+ and RPI2
------------------------------------

RPI1 and RPI2 do not come with a built-in WiFi interface therefore a WiFi USB dongle is required. Depending on the dongle, you have to check whether you need a specific driver or not. For instance, many dongles such as those from TP-Link use the Realtek chip. To know on which chip your dongle is based, type:
	
	> lsusb
	
If your dongle cannot set up an access-point, then you probably need to install a new version of hostapd. The provided Jessie image works out-of-the-box with an RPI3 and already has a custom version of hostapd for the TP-Link TL-WN725 dongle. If you have an RPI2 and this dongle you can easily enable the WiFi access point feature with the following steps after connecting to your Raspberry with ssh (by using Ethernet with DHCP server for instance):

	> cd /usr/sbin
	> sudo rm hostapd
	> sudo ln -s hostapd.tplink725.realtek hostapd

Then edit /etc/hostapd/hostapd.conf

	> cd /etc/hostapd
	> sudo nano hostapd.conf

	ctrl_interface_group=0
	beacon_int=100
	interface=wlan0
	### uncomment the "driver=rtl871xdrv" line if using a Realtek chip
	### For instance TP-Link TL-WN725 dongles need the driver line
	#driver=rtl871xdrv
	ssid=WAZIUP_PI_GW_27EB27F90F
	...

uncomment 

	#driver=rtl871xdrv
	
save the file and see below to configure your new gateway.

Configure your new gateway
--------------------------

If you see the WiFi network WAZIUP_PI_GW_27EB27F90F then connect to this WiFi network. The address of the Raspberry is then 192.168.200.1. If you have no WiFi access point, then plug your Raspberry into a DHCP-enabled network to get an IP address or shared your laptop internet connection to make your laptop acting as a DHCP server. We will use in this example 192.168.200.1 for your gateway address (WiFi option)

	> ssh pi@192.168.200.1
	pi@192.168.200.1's password: 
	
	The programs included with the Debian GNU/Linux system are free software;
	the exact distribution terms for each program are described in the
	individual files in /usr/share/doc/*/copyright.
	
	Debian GNU/Linux comes with ABSOLUTELY NO WARRANTY, to the extent
	permitted by applicable law.
	Last login: Thu Aug  4 18:04:41 2016

go to lora_gateway folder

	> cd lora_gateway
	
and follows instructions in section **"Install the advanced version/Configure your gateway with config_gw.sh"**.	

What is added?
==============

A/ New configuration files and startup procedure
------------------------------------------------

A.1. A "global_conf.json" file defines the radio configuration of the gateway and the global features that will be enabled at the post-processing stage. A "local_conf.json" file defines the local configuration such as the gateway ID. This is inspired by the way Semtech's packet_forwarder is configured. The gateway ID is composed of 8 bytes in hexadecimal notation. We use the last 5 bytes of the eth0 interface MAC address: "gateway_ID" : "00000027EBBEDA21". Log file names will use the gateway ID.

A.2. A "start_gw.py" Python script now run the gateway, using the global and local configuration files. Actually, as start_gw.py simply reads the configuration files and launch lora_gateway and post_processing_gw.py with the appropriate arguments, it is just a simpler way to run the gateway. You can still use the corresponding command line. For instance, with the default configuration files:

	> python start_gw.py
	
is equivalent to

	> sudo ./lora_gateway --mode 1 | python ./post_processing_gw.py -t | python ./log_gw	

A.3. Cloud support is now separated into different external Python script file. We provide examples for Firebase and ThingSpeak with FireBase.py and ThinkSpeak.py.

A.4. A cmd.sh script can be used in an interactive way to launch various commands for the gateway.

**If you just want to use the new configuration and startup procedure, you have nothing more to install**

B/ New features
---------------

B.1. We added the NoSQL MongoDB support. Received data can be saved in the local database if this feature is activated.

B.2. We added an Apache web server with basic PHP forms to visualize graphically the received data of the MongoDB with any web browser.

B.3. We added the WiFi access-point to the LoRa gateway. The SSID can be WAZIUP_PI_GW_XXXXXXXXXX where XXXXXXXXXX can be the last 5 hex bytes of gateway ID: WAZIUP_PI_GW_27EBBEDA21. It has IP address 192.168.200.1 and will lease IP addresses in the range of 192.168.200.100 and 192.168.200.120. Just connect to http://192.168.200.1 with a web brower (could be from a smartphone) to get the graphic visualization of the data stored in the gateway's MongoDB database.

B.4. We added the support of Bluetooth connection. A simple Android App running on Android smartphone displays the data stored in the gateway's MongoDB database.

B.5. We added the possibility to have a temperature/humidity sensor connected to the Raspberry gateway in order to periodically monitor the temperature/humidity level inside the gateway casing. These data are saved in the local MongoDB database and can therefore be visualized just like data coming from remote sensors. We currently support the DHT22 temperature/humidity sensor which is a digital (1-wire) sensor. Remember that the Raspberry has no analog input so an analog sensor such as the LM35 is not suitable.

B.6. We added a configuration script (scripts/config_gw.sh) to help you configure the gateway with MongoDB, WiFi and Bluetooth features. It is highly recommended to use this script to set your gateway once all the files have been copied.


Install the advanced version
============================

If you got the entire LowCostLoRaGw github repository you should have the LowCostLoRaGw/gw_advanced folder already on your Raspberry:

	pi@raspberrypi:~ $ ls -l LowCostLoRaGw/
	total 32
	drwxr-xr-x 7 pi pi  4096 Jul 26 15:38 Arduino
	drwxr-xr-x 5 pi pi  4096 Jul 26 15:38 gw_advanced
	drwxr-xr-x 2 pi pi  4096 Jul 26 15:38 Raspberry
	-rw-r--r-- 1 pi pi 15522 Jul 26 15:38 README.md
	drwxr-xr-x 2 pi pi  4096 Jul 26 15:38 tutorials

Otherwise, get the advanced version with:

	> svn checkout https://github.com/CongducPham/LowCostLoRaGw/trunk/gw_advanced

Note that you may have to install svn before being able to use the svn command:

	> sudo apt-get install subversion
		
Then, copy all the content (all the files including sub-folders) of gw_advanced into your lora_gateway folder:

	> cp -R LowCostLoRaGw/gw_advanced/* lora_gateway

or	

	> cp -R gw_advanced/* lora_gateway
	
Some files will be overwritten but it is normal. If you previously run the basic version with some private cloud keys/credentials, you have to save them first. For instance, if you have a ThingSpeak channel write key, you have to put it back into ThingSpeak.py.

global_conf.json and local_conf.json
------------------------------------

If you only want to use the new configuration and startup procedure, you can just stop here and edit both global_conf.json and local_conf.json. In global_conf.json, you either specify the LoRa mode or the (bw,cr,sf) combination. If mode is defined, then the (bw,cr,sf) combination will be discarded. To use the (bw,cr,sf) combination, you have to set mode to -1. 

In local_conf.json, it is important to set the gateway ID as indicated previously. The config_gw.sh script can do it for you, see below.

**If you just want to use the new configuration and startup procedure, you have nothing more to install**

Installing the new features
---------------------------

You need to install some additional packages:

WiFi access-point

    > sudo apt-get install hostapd dnsmasq

Bluetooth

    > sudo apt-get install bluetooth bluez
    > sudo apt-get install python-bluez
    
MongoDB database

    > sudo apt-get install mongodb-server
    > sudo pip install pymongo

Web server, PHP and MongoDB link

    > sudo apt-get install apache2
    > sudo apt-get install build-essential python-dev
    > sudo apt-get install php5 libapache2-mod-php5
    > sudo apt-get install php5-dev php5-cli php-pear
    > sudo pecl install mongo

Configure your gateway with config_gw.sh
----------------------------------------

The config_gw.sh in the scripts folder can help you for the gateway configuration, WiFi and Bluettoth configuration tasks after you've performed all the apt-get commands. If you don't want some features, just skip them. You have to provide the last 5 hex-byte of your eth0 interface.

    > cd scripts
    > ifconfig
    eth0  Link encap:Ethernet  HWaddr b8:27:eb:be:da:21  
          inet addr:192.168.2.8  Bcast:192.168.2.255  Mask:255.255.255.0
          inet6 addr: fe80::ba27:ebff:febe:da21/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:16556 errors:0 dropped:0 overruns:0 frame:0
          TX packets:9206 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:6565141 (6.2 MiB)  TX bytes:1452497 (1.3 MiB)
          
In the example, we have "HWaddr b8:27:eb:be:da:21" then use "27EBBEDA21"

    > ./config_gw.sh 27EBBEDA21
    
**config_gw.sh takes care of:**

- compiling the lora_gateway program, the Raspberry board version will be checked automatically
- creating a "gateway_id.txt" file containing the gateway id (e.g. "00000027EBBEDA21")
- setting in local_cong.json the gateway id: "gateway_ID" : "00000027EBBEDA21"
- creating the /home/pi/Dropbox/LoRa-test folder for log files (if it does not exist) 
- creating a "log" symbolic link in the lora_gateway folder pointing to /home/pi/Dropbox/LoRa-test folder
- configuring /etc/hostapd/hostapd.conf for WiFi (step A)
- configuring /etc/bluetooth/main.conf for Bluetooth (step B)
- activating MongoDB storage (step F)
- compiling DHT22 support (step H)
- configuring the gateway to run the lora_gateway program at boot (step I)

Anyway, check steps A to I as described below and perform all needed tasks that config_gw.sh is is not addressing.

**Even if you installed from the zipped SD card image config_gw.sh is still needed to personalize your gateway:**

- compiling the lora_gateway program for your the Raspberry board version
- configuring /etc/hostapd/hostapd.conf for to advertise a WiFi SSID corresponding to last 5 hex-byte of your eth0 interface (e.g. WAZIUP_PI_GW_27EBBEDA21) 
- compiling DHT22 support if you connected such a sensor to your Raspberry


A/ Install a WiFi access-point
==============================

First, install the required packages

	> sudo apt-get install hostapd dnsmasq

Set your hot-spot name (ssid) in /etc/hostapd/hostapd.conf (i.e. WAZIUP_PI_GW_27EBBEDA21 using the last 5 hex bytes of your eth0 mac address) and the password (wpa_passphrase)

Replace in /etc/default/hostapd #DAEMON_CONF="" by DAEMON_CONF="/etc/hostapd/hostapd.conf"

Add in /etc/dnsmasq.conf

	interface=wlan0 
	dhcp-range=192.168.200.100,192.168.200.120,255.255.255.0,1h

to give IP address between 192.168.200.100 and 192.168.200.120, with the subnet mask 255.255.255.0 and a 1h lease

Copy the interfaces_ap file from the scripts folder into /etc/network

	> sudo cp scripts/interfaces_ap /etc/networks

Actually, this file adds the following info for wlan0

	allow-hotplug wlan0
	iface wlan0 inet static 
	address 192.168.200.1 
	netmask 255.255.255.0 
	network 192.168.200.0 
	broadcast 192.168.200.255
	
At this point, it is safer to reboot:

	> sudo shutdown -r now

When you want configure your Raspberry into an access-point, run the start_access_point.sh script (only once):

	> cd scripts
	> ./start_access_point.sh
	
You should be able to ssh on 192.168.200.1. start_access_point.sh renames the current interfaces file into interfaces_not_ap and then renames interfaces_ap into interfaces and restarts the various networking services. Now, even if your Raspberry reboots it will automatically be an access-point until you run the stop_access_point.sh script as follows:

	> cd scripts
	> ./stop_access_point.sh

On Wheezy with a RPI2 using a WiFi dongle you have to check whether you need a specific driver or not. For instance, many dongles such as those from TP-Link use the Realtek chip. To know on which chip your dongle is based, type:
	
	> lsusb
	
If your dongle cannot set up an access-point, then you probably need to install a new version of hostapd. For instance with the TP-Link TL-WN725 you can follow instructions from here https://www.raspberrypi.org/forums/viewtopic.php?f=91&t=54946	

B/ Add Bluetooth support
========================

Jessie: nothing to do for OS support

Wheezy: using a Bluetooth dongle on a RPI2
 
	> sudo apt-get install bluetooth bluez

Then, in all cases

	> sudo apt-get install python-bluez

/etc/bluetooth/main.conf file may be edited to give a name to your Bluetooth network (i.e. WAZIUP_PI_BT_GW_27EBBEDA21 using the last 5 hex bytes of your eth0 mac address). You should have a Bluetooth interface (MAC address) listed in /var/lib/bluetooth/. If the Bluetooth network name indicated in /etc/bluetooth/main.conf is not taken into account, you can modify in /var/lib/bluetooth/<mac_address> either in config file (Wheezy) or settings file (Jessie) the Bluetooth network name for the interface.

then reboot

	> sudo shutdown -r now

C/ Bluetooth App on Android
===========================

Loot at http://blog.davidvassallo.me/2014/05/11/android-linux-raspberry-pi-bluetooth-communication/

Check that your /etc/bluetooth/main.conf file has the following line

	DisablePlugins = pnat

then, with Jessie:

- Look at https://www.raspberrypi.org/forums/viewtopic.php?f=63&t=133263
- Look at https://www.raspberrypi.org/forums/viewtopic.php?p=919420#p919420
- in file /lib/systemd/system/bluetooth.service
	Add ' -C' at the end of the 'ExecStart=' line, to start the bluetooth daemon in 'compatibility' mode.
	Add a new 'ExecStartPost=' immediately after that line, to add the SP Profile. 
	The two lines should look like this:
		ExecStart=/usr/lib/bluetooth/bluetoothd -C
		ExecStartPost=/usr/bin/sdptool add SP

then, with Wheezy: normally nothing to be done
	 
Then use Android Studio with Android 4.0 API minimum. An Android phone with Android 4.4 (KitKat) or higher is needed. 

May need to run on RPI

	> sudo hciconfig hci0 piscan

Run the rfcomm-server Python script in background to handle Bluetooth connections:

	> sudo python rfcomm-server.py > rfcomm.log &
	
D/ Install a web server
=======================

Look at https://www.raspberrypi.org/documentation/remote-access/web-server/apache.md

	> sudo apt-get install apache2

Give rights to the Apache folder in order to easily administer the website

	> sudo chown -R www-data:pi /var/www
	> sudo chmod -R 770 /var/www
	
Check if the server works by connecting to the IP address of the gateway with a web browser, or

	> sudo wget -O verif_apache.html http://127.0.0.1
	> cat ./verif_apache.html

E/ Install MongoDB
==================

Jessie: look at http://raspbian-france.fr/installer-mongodb-raspberry-pi/

	> sudo apt-get install mongodb-server
	
will install MongoDB v2.4.

Wheezy: need to do install from pre-compiled binaries, see:

- http://www.widriksson.com/wp-content/uploads/2014/02/mongodb-rpi_20140207.zip, or
- http://andyfelong.com/2015/02/mongodb-on-the-raspberry-pi-2/
- http://andyfelong.com/2016/01/mongodb-3-0-9-binaries-for-raspberry-pi-2-jessie/

F/ Python for MongoDB
=====================

Install Python package for MongoDB

	> sudo pip install pymongo
	> sudo apt-get install build-essential python-dev
	
Then, to make the post_processing_gw_py script saving received data to the MongoDB, edit in global_conf.json the line:

	"mongodb" : {
		"is_used" : true,	

G/ PHP + MongoDB
================

Install PHP

	> sudo apt-get install php5 libapache2-mod-php5

Test for PHP

	> sudo rm /var/www/index.html
	> echo "<?php phpinfo() ;?>" > /var/www/index.php
	
Check if PHP works by connecting to the IP address of the gateway with a web browser, or

	> sudo wget -O verif_apache.html http://127.0.0.1
	> cat ./verif_apache.html
	
Remember that with Jessie, the web pages are in /var/www/html instead of /var/www, so change accordingly

	> sudo apt-get install php5-dev php5-cli php-pear
	> sudo pecl install mongo

Add in /etc/php5/apache2/php.ini the following line

	> extension=mongo.so
	
Restart the web server
	
	> sudo service apache2 restart	
	
Copy all files in the php folder into /var/www/html or /var/www depending on whether you are running Jessie or Wheezy:

	> cd php
	> sudo cp * /var/www/html
	
H/ Add the DHT22 sensor for the gateway
=======================================

Installing the Python-GPIO library

	> cd sensors_in_raspi
	> cd PIGPIO
	> make -j4
	> sudo make install
	
If you want the gateway to periodically take the measure, edit in local_conf.json the line:

	"dht22" : 0	

and set to a value in second. For instance 600 will take a measure every 10 minutes.

Use a DHT22 sensor and prepare it with a 10kOhms resistance between pin 1 (VCC) and pin 2 (data). It is also recommended to have a 100nF condensator between pin 1 (VCC) and pin 4 (Ground). Then connect DHT22's pin 1 to RPI's pin 1 (3.3v DC Power), DHT22's pin 2 to RPI's pin 7 (GPIO04) and DHT22's pin 4 to RPI's pin 6 (Ground).
	
I/ Run the gateway at boot
==========================

If you want to run the gateway at boot, you can add the following line:

	/home/pi/lora_gateway/scripts/start_gw.sh
	
in the /etc/rc.local file, before the "exit 0" line

If you use the config_gw.sh script, it can do it for you.

**IMPORTANT NOTICE**: when the gateway is run at boot, it is run under root identity. In the post_processing_gw.py script, the folder path for log files is now hard coded as /home/pi/Dropbox/LoRa-test instead of ~/Dropbox/LoRa-test in previous versions. In this way, even if the gateway is run under root identity, the log files are stored in the pi user account.

**IMPORTANT NOTICE**: To run an operation gateway, it is better to reboot the gateway and let the LoRa gateway program start at boot. Manually lauching the gateway can be usefull for test purposes but we observed that redirections of LoRa gateway output to the post-processing stage can be broken thus leading to a not responding gateway.

USE cmd.sh to interact with the gateway
=======================================

You can use cmd.sh as follows:

	> ./cmd.sh
	=======================================* Gateway 00000027EBBEDA21 *===
	0- sudo python start_gw.py &                                         +
	1- sudo ./lora_gateway --mode 1                                      +
	2- sudo ./lora_gateway --mode 1|python post_processing_gw.py -t -m 2 +
	3- ps aux | grep -e start_gw -e lora_gateway -e post_proc -e log_gw  +
	4- tail --line=15 ../Dropbox/LoRa-test/post-processing_*.log         +
	5- tail -f ../Dropbox/LoRa-test/post-processing_*.log                +
	6- less ../Dropbox/LoRa-test/post-processing_*.log                   +
	------------------------------------------------------* Bluetooth *--+
	a- run: sudo hciconfig hci0 piscan                                   +
	b- run: sudo python rfcomm-server.py                                 +
	c- run: nohup sudo python rfcomm-server.py -bg > rfcomm.log &        +
	d- run: ps aux | grep rfcomm                                         +
	e- run: tail -f rfcomm.log                                           +
	---------------------------------------------------* Connectivity *--+
	f- test: ping www.univ-pau.fr                                        +
	--------------------------------------------------* Configuration *--+
	A- show global_conf.json                                             +
	B- show local_conf.json                                              +
	C- show clouds.json                                                  +
	D- edit global_conf.json                                             +
	E- edit local_conf.json                                              +
	F- edit clouds.json                                                  +
	-----------------------------------------------------------* kill *--+
	K- kill all gateway related processes                                +
	k- kill rfcomm-server process                                        +
	R- reboot gateway                                                    +
	S- shutdown gateway                                                  +
	---------------------------------------------------------------------+
	Q- quit                                                              +
	======================================================================
	Enter your choice: 

cmd.sh needs a file called gateway_id.txt that should contain the ID of your gateway. As indicated previously, the gateway ID is composed of 8 bytes in hexadecimal notation with the last 5 bytes being the last 5 bytes of the gateway eth0 interface MAC address. It is exactely the same ID that the one indicated in local_conf.json. If you start cmd.sh without this gateway_id.txt file, it will prompt you to create such file by entering the last 5 bytes of the gateway eth0 interface MAC address:

	ERROR: gateway_id.txt file not found
	should create it by running echo "000000XXXXXXXXXX" > gateway_id.txt
	where XXXXXXXXXX is the last 5 bytes of your MAC Ethernet interface address
	Example: echo "00000027EBBEDA21" > gateway_id.txt
	Here is your MAC Ethernet interface address:
	-------------------------------------------------------
	    eth0  Link encap:Ethernet  HWaddr b8:27:eb:be:da:21
	-------------------------------------------------------
	Enter the last 5 hex bytes of your MAC Ethernet interface address
	in capital character and without the : separator
	example: HWaddr b8:27:eb:be:da:21 then just enter 27EBBEDA21
	
If you enter 27EBBEDA21, cmd.sh will create the gateway_id.txt file with the following content:

	> cat gateway_id.txt
	00000027EBBEDA21
	
cmd.sh will also set the gateway id in the local_conf.json file: "gateway_ID" : "00000027EBBEDA21". 

Normally, the LoRa gateway starts automatically on boot. To verify that you have a running gateway, use option 3.

	BEGIN OUTPUT
	Check for lora_gateway process
	##############################
	root      4119  0.0  0.3   6780  3184 ?        S    10:21   0:00 sudo python start_gw.py
	root      4123  0.0  0.5   9228  5180 ?        S    10:21   0:00 python start_gw.py
	root      4124  0.0  0.0   1912   364 ?        S    10:21   0:00 sh -c sudo ./lora_gateway --mode 1 --ndl | python post_processing_gw.py | python log_gw.py
	root      4125  0.0  0.3   6780  3188 ?        S    10:21   0:00 sudo ./lora_gateway --mode 1 --ndl
	root      4131 88.5  0.2   3700  2176 ?        R    10:21   3:31 ./lora_gateway --mode 1 --ndl
	pi        4176  0.0  0.2   4276  1948 pts/1    S+   10:25   0:00 grep -e start_gw -e lora_gateway -e post_processing -e log_gw
	##############################
	The gateway is running if you see the lora_gateway process
	END OUTPUT
	Press RETURN/ENTER...

To stop the gateway, use option K. This option can also kill the gateway processes that are run at boot.

**IMPORTANT NOTICE**: Do not launch a new gateway instance with an existing one as there will be conflict on the SPI bus.

You can start manually the gateway in background for test purposes with option 0. This option actually performs the following commands:

	> sudo python ./start_gw.py &
	> disown %1
	
**IMPORTANT NOTICE**: To run an operation gateway, it is better to reboot the gateway and let the LoRa gateway program start at boot. Manually lauching the gateway can be usefull for test purposes but we observed that redirections of LoRa gateway output to the post-processing stage can be broken thus leading to a not responding gateway.

You can then use option 5 to see the logs in real time. To test the simple gateway, use option 1. You can ssh at any time and use option 5 to see the latest packets that have been received. If you have the WiFi access point enabled you can use a smartphone with an ssh apps to log on 192.168.200.1 and launch cmd.sh from your smartphone.	

You can easily add new useful commands to the cmd.sh shell script.
	
Enjoy!
C. Pham	