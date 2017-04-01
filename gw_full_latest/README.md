Low-cost LoRa gateway features and configuration tools
======================================================

Configuration files and startup procedure
-----------------------------------------

A "gateway_conf.json" file defines gateway configuration with several sections for radio configuration, local gateway option such as gateway ID, etc. One important field is the gateway ID which is composed of 8 bytes in hexadecimal notation. We use the last 5 bytes of the eth0 interface MAC address: "gateway_ID" : "00000027EBBEDA21". Log file names will use the gateway ID. The config_gw.sh script can do it for you, see below.

In gateway_conf.json, you can either specify the LoRa mode or the (bw,cr,sf) combination. If mode is defined, then the (bw,cr,sf) combination will be discarded. To use the (bw,cr,sf) combination, you have to set mode to -1. 

A "start_gw.py" Python script runs the gateway, using the gateway configuration file to launch the low-level gateway with appropriate parameters and to log all outputs from the post-processing stage. As start_gw.py simply reads the configuration file to launch lora_gateway and the post_processing_gw.py script, it is just a simpler way to run the gateway. You can still use the corresponding command line. For instance, with the default configuration file:

	> python start_gw.py
	
is equivalent to:

	> sudo ./lora_gateway --mode 1 | python ./post_processing_gw.py | python ./log_gw	

Cloud support is separated into different external Python script file. We provide examples for Firebase and ThingSpeak with FireBase.py and ThinkSpeak.py.

A cmd.sh script can be used in an interactive way to launch various commands for the gateway.

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

Connect to your new gateway
---------------------------

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
	
and follows instructions below.

Configure your gateway with config_gw.sh
----------------------------------------

The config_gw.sh in the scripts folder can help you for the gateway configuration, WiFi and Bluettoth configuration tasks (if you use our SD card image, otherwise, you need first to install some required packages). If you don't want some features, just skip them. You have to provide the last 5 hex-byte of your eth0 interface.

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
- setting in gateway_cong.json the gateway id: "gateway_ID" : "00000027EBBEDA21"
- creating the /home/pi/Dropbox/LoRa-test folder for log files (if it does not exist) 
- creating a "log" symbolic link in the lora_gateway folder pointing to /home/pi/Dropbox/LoRa-test folder
- configuring /etc/hostapd/hostapd.conf for WiFi (step A)
- configuring /etc/bluetooth/main.conf for Bluetooth (step B)
- activating MongoDB storage (step F)
- compiling DHT22 support (step H)
- configuring the gateway to run the lora_gateway program at boot (step I)

Anyway, check steps A to I as described below and perform all needed tasks that config_gw.sh is is not addressing.

**Even if you installed from the zipped SD card image config_gw.sh is still needed to personalize your gateway to:**

- compile the lora_gateway program for your the Raspberry board version
- configure /etc/hostapd/hostapd.conf for to advertise a WiFi SSID corresponding to last 5 hex-byte of your eth0 interface (e.g. WAZIUP_PI_GW_27EBBEDA21) 
- compile DHT22 support if you connected such a sensor to your Raspberry

If you install everything yourself, from a standard Jessie distribution
=========================================================================

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
	
If you want the gateway to periodically take the measure, edit in gateway_conf.json the line:

	"dht22" : 0	

and set to a value in second. For instance 600 will take a measure every 10 minutes.

Use a DHT22 sensor and prepare it with a 10kOhms resistance between pin 1 (VCC) and pin 2 (data). It is also recommended to have a 100nF condensator between pin 1 (VCC) and pin 4 (Ground). Then connect DHT22's pin 1 to RPI's pin 1 (3.3v DC Power), DHT22's pin 2 to RPI's pin 7 (GPIO04) and DHT22's pin 4 to RPI's pin 6 (Ground).
	
I/ Run the gateway at boot
==========================

If you want to run the gateway at boot, you can add the following line:

	/home/pi/lora_gateway/scripts/start_gw.sh
	
in the /etc/rc.local file, before the "exit 0" line

If you use the config_gw.sh script, it can do it for you.

**IMPORTANT NOTICE**: when the gateway is run at boot, it is run under root identity. In the post_processing_gw.py script, the folder path for log files is hard coded as /home/pi/Dropbox/LoRa-test. In this way, even if the gateway is run under root identity, the log files are stored in the pi user account.

**IMPORTANT NOTICE**: To run an operation gateway, it is better to reboot the gateway and let the LoRa gateway program start at boot. Manually lauching the gateway can be usefull for test purposes but we observed that redirections of LoRa gateway output to the post-processing stage can be broken thus leading to a not responding gateway.

gateway_conf.json options
=========================

A typical gateway_conf.json is shown below:

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
	
["radio_conf"] defines the LoRa radio parameters. This section is read by start_gw.py to launch the lora_gateway program with the appropriate parameters. You can either specify the LoRa mode or the (bw,cr,sf) combination. If mode is defined, then the (bw,cr,sf) combination will be discarded. To use the (bw,cr,sf) combination, you have to set mode to -1. To specify an ad-hoc frequency, use ["freq" : 433.3] for instance. If you want to use "ch", you have to modify lora_gateway.cpp to select the correct frequency band and then only you can use ["ch" : 10]. If BAND900 is selected in lora_gateway.cpp then the channel used will be CH_10_900 which is 924.68MHz (defined in SX1272.h).

["gateway_conf"]["gateway_ID""] is composed of 8 bytes in hexadecimal notation. We use the last 5 bytes of the eth0 interface MAC address.

["gateway_conf"]["ref_latitude"] you can put the gateway latitude here. We plan to push this data to LoRaWAN network server in the future.

["gateway_conf"]["ref_longitude"] you can put the gateway longitude here. We plan to push this data to LoRaWAN network server in the future.

["gateway_conf"]["wappkey"] when set to true will enable app key enforcement in post_processing_gw.py. Add there the list of appkey that you want to enable.

["gateway_conf"]["raw"] when set to true will make  the lora_gateway program to provide raw payload to the post-processing stage. Then, post_processing_gw.py will try to dissect the packet which will be in most case a LoRaWAN packet. However, you can use your own packet format and then modify post_processing_gw.py ccordingly.

["gateway_conf"]["aes"] when set to true will enable local AES decryption in post_processing_gw.py. The AES AppSKey and NwkSKey are stored in loraWAN.py. They must match those used by the end-device. We only support ABP (activitation by personalization) method. Actually, AES is usefull to provide data privicy.

["gateway_conf"]["log_post_processing"] when set to true will make start_gw.py to additionally launch log_gw.py script to log all the post_processing_gw.py's outputs.

["gateway_conf"]["log_weekly"] when set to true will make log_gw.py to create a new log file every week, instead of every month.

["gateway_conf"]["dht22"] indicates the time interval (in second) for post_processing_gw.py to trigger a temperature/humidity measure from the DHT22 sensor every N seconds (that you must connect and install, see step H). post_processing_gw.py will typically display the following information, that will be logged in the log file.

	2017-03-31T23:42:52.703430> Getting gateway temperature
	2017-03-31T23:42:52.703722> Gateway TC : 26.40 C | HU : 24.90 % at 2017-03-31 23:42:52.703074

["gateway_conf"]["dht22_mongo"] when set to true will further store the temperature/humidity measure in the local MongDB. Then, these measures will be visible on the gateway's web page. You can check with this feature the condition inside the gateway's case in outdoor deployment.

["gateway_conf"]["downlink"] indicates the time interval (in second) for post_processing_gw.py to check for a downlink-post.txt. See this [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-downlink.md).

["gateway_conf"]["status"] indicates the time interval (in second) for post_processing_gw.py to display a status message to indicate that the script is correctly running in case you don't receive packet for a long time.

	2017-03-31T23:44:00.479800> 2017-03-31 23:44:00.479397
	2017-03-31T23:44:00.480088> post status: gw ON, lat my_lat long my_long 

We plan in the future to send appropriate message to a LoRaWAN network server (such as TTN) in the same way the single_chan_pkt_fwd program from Thomas Telkamp works (and like most LoRaWAN gateways). 

["gateway_conf"]["aux_radio"] indicates the time interval (in second) for post_processing_gw.py to check for a aux_radio_post.txt file with data received from other radio interfaces, e.g. IEEE802.15.4, etc. This feature is not currently distributed as it is still in the early stage of development.

["alert_conf"]["use_mail"] when set to true indicates that post_processing_gw.py will sent an alerting mail on specific events. There are currently 2 events: when post_processing_gw.py is started (which usually means that the gateway has booted and is up) and when the radio module has been reset by the low-level lora_gateway program because of some receive errors.

["alert_conf"]["contact_mail"] is the list of email address recipients: e.g. joejoejoe@gmail.com,jackjackjack@hotmail.com,...

["alert_conf"]["mail_from"] is a valid address email with associated account

["alert_conf"]["mail_server"] is an SMTP server name

["alert_conf"]["mail_passwd"] is the passwd. It is not very secure but there is no other way to the best of my knowledge.

["alert_conf"]["use_sms"] when set to true indicates that post_processing_gw.py will sent an alerting SMS on specific events. This feature will be integrated soon. We already tested the SMS transmission using gammu with a 3G dongle (see this [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/3GDongle/README.md) that explains how to use a 3G dongle for Internet connectivity).

["alert_conf"]["contact_sms"] is the phone number, with country prefix, for the SMS.	

Adding new features
===================

The gateway architecture is flexible on purpose for adding or customizing your gateway by only modifying the post_processing_gw.py script. To add new options, you can either add new fields in one of the current sections in gateway_conf.json, or add a brand new section.

New cloud platforms can be added by writing new cloud scripts and adding them to clouds.json. See this [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-NewCloud.md).

More information on the gateway advanced features
=================================================

- cloud management approach
	- [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-NewCloud.md)
- encryption and native LoRaWAN frame format
	- see [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-aes_lorawan.md)
	- end-device can send native LoRaWAN packets
	- low-level gateway provides raw output for post_processing_gw.py to handle LoRaWAN packets
- downlink features: to send from gateway to end-device
	- [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-downlink.md)

USE cmd.sh to interact with the gateway
=======================================

You can use cmd.sh as follows:

	> ./cmd.sh
	=======================================* Gateway 00000027EBBEDA21 *===
	0- sudo python start_gw.py & ; disown %1                             +
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
	--------------------------------------------------* Filtering msg *--+
	l- List LoRa reception indications                                   +
	m- List radio module reset indications                               +
	n- List boot indications                                             +
	o- List post-processing status                                       +
	p- List low-level gateway status                                     +	
	--------------------------------------------------* Configuration *--+
	A- show gateway_conf.json                                            +
	B- edit gateway_conf.json                                            +
	C- show clouds.json                                                  +
	D- edit clouds.json                                                  +
	-----------------------------------------------------------* kill *--+
	K- kill all gateway related processes                                +
	k- kill rfcomm-server process                                        +
	R- reboot gateway                                                    +
	S- shutdown gateway                                                  +
	---------------------------------------------------------------------+
	Q- quit                                                              +
	======================================================================
	Enter your choice: 

cmd.sh needs a file called gateway_id.txt that should contain the ID of your gateway. As indicated previously, the gateway ID is composed of 8 bytes in hexadecimal notation with the last 5 bytes being the last 5 bytes of the gateway eth0 interface MAC address. It is exactely the same ID that the one indicated in gateway_conf.json. If you start cmd.sh without this gateway_id.txt file, it will prompt you to create such file by entering the last 5 bytes of the gateway eth0 interface MAC address:

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
	
cmd.sh will also set the gateway id in the gateway_conf.json file: "gateway_ID" : "00000027EBBEDA21". 

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