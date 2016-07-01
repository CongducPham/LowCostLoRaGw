Advanced features and configuration tools for the low-cost LoRa gateway with Raspberry
======================================================================================


IMPORTANT: We should start first with the simple gateway installation. Then follow the instruction here to have the advanced version.

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

B.6. We added simple 128-bit AES decryption capabilities at the gateway in the post_processing_gw.py script. The end-device can encrypt using an 128-bit AES library. The common AES key must be known by both the end-device and the gateway. It is just a demonstration for simple deployment usage.

B.7. We added a configuration script (script/config_raspbian.sh) to help you configure the gateway with MongoDB, WiFi and Bluetooth features


Install the advanced version
============================

Copy all the files (including sub-folders) of the gw_advanced folder in your lora_gateway folder on the Raspberry. Some files will be overwritten but it is normal. If you previously run the basic version with some private cloud keys/credentials, you have to save them first. For instance, if you have a ThingSpeak channel write key, you have to put it back into ThingSpeak.py.

global_conf.json and local_conf.json
------------------------------------
If you only want to use the new configuration and startup procedure, you can just stop here and edit both global_conf.json and local_conf.json. In global_conf.json, you either specify the LoRa mode or the (bw,cr,sf) combination. If mode is defined, then the (bw,cr,sf) combination will be discarded. To use the (bw,cr,sf) combination, you have to set mode to -1.

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
    
AES encryption

    > sudo pip install pycrypto

The config_raspbian.sh in the scripts folder can help you for WiFi and Bluettoth configuration tasks after you've performed all the apt-get commands. If you don't want some features, just skip them. You have to provide the last 5 hex-byte of your eth0 interface.

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
          
In the example, use 27EBBEDA21

    > ./config_raspbian.sh 27EBBEDA21

Then check steps A to I as described below. **config_raspbian.sh takes care of configuring steps A and B only**.

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
	
I/ AES encryption
=================

Installing Python AES encryption package

	> sudo pip install pycrypto

USE cmd.sh to interact with the gateway
=======================================

You can use cmd.sh as follows:

	> ./cmd.sh
	========================================================* Gateway *===
	0- nohup python start_gw.py &                                        +
	1- sudo ./lora_gateway --mode 1                                      +
	2- sudo ./lora_gateway --mode 1|python post_processing_gw.py -t -m 2 +
	3- ps aux | grep -e start_gw -e lora_gateway -e post_proc -e log_gw  +
	4- tail --line=15 ../Dropbox/LoRa-test/post-processing_*.log         +
	5- tail -f ../Dropbox/LoRa-test/post-processing_*.log                +
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
	-----------------------------------------------------------* kill *--+
	K- kill all gateway related processes                                +
	k- kill rfcomm-server process                                        +
	---------------------------------------------------------------------+
	Q- quit                                                              +
	======================================================================
	Enter your choice: 
	
To run an operational gateway, use option 0. Then use option 3 to verify whether all the processes have been launched. You can then use option 5 to see the logs in real time. To test the simple gateway, use option 1. If you access your gateway with ssh, using option 0 allows you to quit the ssh session and leave your gateway running. You can ssh at any time and use option 5 to see the latest packets that have been received. If you have the WiFi access point enabled you can use a smartphone with an ssh apps to log on 192.168.200.1 and launch cmd.sh from your smartphone.	

To stop the gateway, use option K
	
