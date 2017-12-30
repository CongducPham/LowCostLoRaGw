Low-cost LoRa gateway manual installation guide
===============================================

Quick start
-----------

- The current SD card image has everything you need!
- Read the [Low-cost-LoRa-GW leaflet](https://github.com/CongducPham/tutorials/blob/master/Low-cost-LoRa-GW-leaflet.pdf)
- Look at the [Low-cost-LoRa-GW web admin interface tutorial](https://github.com/CongducPham/tutorials/blob/master/Low-cost-LoRa-GW-web-admin.pdf). For most end-users, the web admin interface is sufficient to configure and manage the gateway. For instance, to configure a new gateway, just use the web admin interface to update the gateway software and to run the basic configuration procedure as described [here](https://github.com/CongducPham/LowCostLoRaGw#option-i).
- **The rest of this README file contains additional information for manual installation from scratch.**

If you install everything yourself, from a standard Jessie (or newer) distribution
==================================================================================

Install Raspbian Wheezy or Jessie
---------------------------------

Fisrt install a Raspberry with Raspbian, Jessie is recommended.

then (you need to have Internet access on your Raspberry):

	> sudo apt-get update
	> sudo apt-get upgrade

Jessie has been tested on RPI1, RPI2 and RPI3, and works great.

Wheezy has been tested on RPI1 and RPI2 and works great. Wheezy on RPI3 is not recommended because built-in WiFi and Bluetooth will not work properly.

We recommend buying either RPI2 or RPI3. RPI3 with Jessie has built-in WiFi and Bluetooth so it is definitely a good choice. In addition RPI3 with Jessie will have a better support lifetime. 

Install additional packages
---------------------------

You need to install some additional packages:

Python `pip` installer

	> sudo apt-get install python-pip
	
Python requests package

	> sudo pip install requests	
	
Python dateutil package

	> sudo pip install python-dateutil	
	
Python encryption

    > sudo apt-get install python-crypto	

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

Subversion

	> sudo apt-get install subversion
    
2G/3G communication

    > sudo apt-get install python-gammu gammu wvdial
    
json

    > sudo apt-get install jq

serial com

	> sudo apt-get install minicom picocom

MQTT

	> sudo apt-get install mosquitto mosquitto-clients python-mosquitto
	> sudo pip install paho-mqtt

`npm`

	> sudo apt-get install npm
	> sudo npm i -g npm@2.x

Update Node-Red

	> update-nodejs-and-nodered

gps

	> sudo apt-get install gpsd gpsd-clients

`raspap-webgui` (https://github.com/billz/raspap-webgui)

	> sudo apt-get install lighttpd
	> sudo lighttpd-enable-mod fastcgi-php
	> sudo service lighttpd restart	
	
A/ Install a WiFi access-point
==============================

First, install the required packages

	> sudo apt-get install hostapd dnsmasq

Set your hot-spot name (ssid) in `/etc/hostapd/hostapd.conf` (i.e. WAZIUP_PI_GW_27EBBEDA21 using the last 5 hex bytes of your eth0 mac address) and the password (wpa_passphrase)

Replace in `/etc/default/hostapd` `#DAEMON_CONF=""` by `DAEMON_CONF="/etc/hostapd/hostapd.conf"`

Add in `/etc/dnsmasq.conf`

	interface=wlan0 
	dhcp-range=192.168.200.100,192.168.200.120,255.255.255.0,1h

to give IP address between 192.168.200.100 and 192.168.200.120, with the subnet mask 255.255.255.0 and a 1h lease

Copy the `interfaces_ap` file from the `scripts` folder into `/etc/network`

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

When you want configure your Raspberry into an access-point, run the `start_access_point.sh` script (only once):

	> cd scripts
	> ./start_access_point.sh
	
You should be able to ssh on 192.168.200.1. `start_access_point.sh` renames the current interfaces file into `interfaces_not_ap` and then renames `interfaces_ap` into `interfaces` and restarts the various networking services. Now, even if your Raspberry reboots it will automatically be an access-point until you run the `stop_access_point.sh` script as follows:

	> cd scripts
	> ./stop_access_point.sh

On Wheezy with a RPI2 using a WiFi dongle you have to check whether you need a specific driver or not. For instance, many dongles such as those from TP-Link use the Realtek chip. To know on which chip your dongle is based, type:
	
	> lsusb
	
If your dongle cannot set up an access-point, then you probably need to install a new version of `hostapd`. For instance with the TP-Link TL-WN725 you can follow instructions from [https://www.raspberrypi.org/forums/viewtopic.php?f=91&t=54946](https://www.raspberrypi.org/forums/viewtopic.php?f=91&t=54946)	

B/ Add Bluetooth support
========================

Jessie: nothing to do for OS support

Wheezy: using a Bluetooth dongle on a RPI2
 
	> sudo apt-get install bluetooth bluez

Then, in all cases

	> sudo apt-get install python-bluez

`/etc/bluetooth/main.conf` file may be edited to give a name to your Bluetooth network (i.e. WAZIUP_PI_BT_GW_27EBBEDA21 using the last 5 hex bytes of your eth0 mac address). You should have a Bluetooth interface (MAC address) listed in `/var/lib/bluetooth/`. If the Bluetooth network name indicated in `/etc/bluetooth/main.conf` is not taken into account, you can modify in `/var/lib/bluetooth/<mac_address>`, either in config file (Wheezy) or settings file (Jessie), the Bluetooth network name for the interface.

then reboot

	> sudo shutdown -r now

C/ Bluetooth App on Android
===========================

Loot at http://blog.davidvassallo.me/2014/05/11/android-linux-raspberry-pi-bluetooth-communication/

Check that your `/etc/bluetooth/main.conf` file has the following line

	DisablePlugins = pnat

then, with Jessie:

- Look at https://www.raspberrypi.org/forums/viewtopic.php?f=63&t=133263
- Look at https://www.raspberrypi.org/forums/viewtopic.php?p=919420#p919420
- in file /lib/systemd/system/bluetooth.service

	Add ' -C' at the end of the 'ExecStart=' line, to start the bluetooth daemon in 'compatibility' mode.
	
    Add a new 'ExecStartPost=' immediately after that line, to add the SP Profile.
    
    The two lines should look like this:
    ```
	ExecStart=/usr/lib/bluetooth/bluetoothd -C
	ExecStartPost=/usr/bin/sdptool add SP
    ```

then, with Wheezy: normally nothing to be done
	 
Then use Android Studio with Android 4.0 API minimum. An Android phone with Android 4.4 (KitKat) or higher is needed. 

May need to run on RPI

	> sudo hciconfig hci0 piscan

Run the `rfcomm-server` Python script in background to handle Bluetooth connections:

	> sudo python rfcomm-server.py > rfcomm.log &
	
D/ Install a web server
=======================

Look at https://www.raspberrypi.org/documentation/remote-access/web-server/apache.md

	> sudo apt-get install apache2

Give rights to the Apache folder in order to easily administer the website

	> sudo chown -R pi:www-data /var/www
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

	> sudo apt-get install python-pip
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
	
Remember that starting with Jessie, the web pages are in `/var/www/html` instead of `/var/www`, so change accordingly

	> sudo apt-get install php5-dev php5-cli php-pear
	> sudo pecl install mongo

Add in `/etc/php5/apache2/php.ini` the following line

	> extension=mongo.so
	
Restart the web server
	
	> sudo service apache2 restart	
	
Copy all files in the `lora_gateway/php` folder into `/var/www/html` or `/var/www` depending on whether you are running Jessie or Wheezy:

	> cd php
	> sudo cp * /var/www/html
	> sudo chown -R pi:www-data /var/www
	
	
H/ Add the DHT22 sensor for the gateway
=======================================

Installing the Python-GPIO library

	> cd sensors_in_raspi
	> cd PIGPIO
	> make -j4
	> sudo make install
	
If you want the gateway to periodically take the measure, edit in `gateway_conf.json` the line:

	"dht22" : 0	

and set to a value in second. For instance 600 will take a measure every 10 minutes.

Use a DHT22 sensor and prepare it with a 10kOhms resistance between pin 1 (VCC) and pin 2 (data). It is also recommended to have a 100nF condensator between pin 1 (VCC) and pin 4 (Ground). Then connect DHT22's pin 1 to RPI's pin 1 (3.3v DC Power), DHT22's pin 2 to RPI's pin 7 (GPIO04) and DHT22's pin 4 to RPI's pin 6 (Ground).
	
I/ Run the gateway at boot
==========================

If you want to run the gateway at boot, you can add the following line:

	/home/pi/lora_gateway/scripts/start_gw.sh
	
in the `/etc/rc.local` file, before the `exit 0` line

If you use the `config_gw.sh` script, it can do it for you.

**IMPORTANT NOTICE**: when the gateway is run at boot, it is run under root identity. In the `post_processing_gw.py` script, the folder path for log files is hard coded as `/home/pi/Dropbox/LoRa-test`. In this way, even if the gateway is run under root identity, the log files are stored in the pi user account.

**IMPORTANT NOTICE**: To run an operation gateway, it is better to reboot the gateway and let the LoRa gateway program start at boot. Manually lauching the gateway can be usefull for test purposes but we observed that redirections of LoRa gateway output to the post-processing stage can be broken thus leading to a not responding gateway.
	
Enjoy!
C. Pham	