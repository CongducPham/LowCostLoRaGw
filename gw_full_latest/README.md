Low-cost LoRa gateway features and configuration tools
======================================================

Quick start
-----------

- The current SD card image has everything you need!
- Read the [Low-cost-LoRa-GW leaflet](https://github.com/CongducPham/tutorials/blob/master/Low-cost-LoRa-GW-leaflet.pdf)
- Look at the [Low-cost-LoRa-GW web admin interface tutorial](https://github.com/CongducPham/tutorials/blob/master/Low-cost-LoRa-GW-web-admin.pdf). For most end-users, the web admin interface is sufficient to configure and manage the gateway. **For instance, to configure a new gateway, it is recommended to use the web admin interface to update the gateway software and to run the basic configuration procedure as described [here](https://github.com/CongducPham/LowCostLoRaGw#option-i)**.
- **The rest of this README file contains additional information to better understand the gateway architecture and advanced mechanisms**.
- There is an additional [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-manual-install.md) describing all manual installation steps if you want to install from scratch.

There are additional dedicated README files
-------------------------------------------

- remote access to your gateway from anywhere with `ngrok`
	- [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-remote-access-ngrok.md)
- interfacing with Node-Red
	- [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-NodeRed.md)
- cloud management approach
	- [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-NewCloud.md)
- encryption and native LoRaWAN frame format
	- see [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-aes_lorawan.md)
	- end-device can send native LoRaWAN packets
	- low-level gateway provides raw output for post_processing_gw.py to handle LoRaWAN packets
- downlink features: to send from gateway to end-device
	- [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-downlink.md)
- manual installation procedure
	- [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-manual-install.md)	

WiFi instructions on RPI1B+ and RPI2
------------------------------------

RPI1 and RPI2 do not come with a built-in WiFi interface therefore a WiFi USB dongle is required. Depending on the dongle, you have to check whether you need a specific driver or not. For instance, many dongles such as those from TP-Link use the Realtek chip. To know on which chip your dongle is based, type:
	
	> lsusb
	
If your dongle cannot set up an access-point, then you probably need to install a new version of hostapd. The provided Jessie image works out-of-the-box with an RPI3 and already has a custom version of hostapd for the TP-Link TL-WN725 dongle. If you have an RPI2 and this dongle you can easily enable the WiFi access point feature with the following steps after connecting to your Raspberry with ssh (by using Ethernet with DHCP server for instance):

	> cd /usr/sbin
	> sudo rm hostapd
	> sudo ln -s hostapd.tplink725.realtek hostapd

Then edit `/etc/hostapd/hostapd.conf`. If you don't have the `/etc/hostapd/hostapd.conf` file then you may need to run:

	> zcat /usr/share/doc/hostapd/examples/hostapd.conf.gz | sudo tee -a /etc/hostapd/hostapd.conf
	
Then

	> cd /etc/hostapd
	> sudo nano hostapd.conf

	ctrl_interface_group=0
	beacon_int=100
	interface=wlan0
	### uncomment the "driver=rtl871xdrv" line if using a Realtek chip
	### For instance TP-Link TL-WN725 dongles need the driver line
	#driver=rtl871xdrv
	ssid=WAZIUP_PI_GW_27EB27F90F
	wpa_passphrase=loragateway
	...

uncomment 

	#driver=rtl871xdrv
	
save the file and see below to configure your new gateway.

Connect to your new gateway
---------------------------

The SD card image defines a `pi` user:

	- login: pi
	- password: loragateway
		
With the default gateway configuration, the gateway acts as a WiFi access point. If you see the WiFi network WAZIUP_PI_GW_XXXXXXXXXX then connect to this WiFi network. The WiFi password is `loragateway`. **It is strongly advise to change this WiFi password**. The address of the Raspberry is then 192.168.200.1.  

If you see no WiFi access point (e.g. RP1/RPI2/RPI0 without WiFi dongle), then plug your Raspberry into a DHCP-enabled box/router/network to get an IP address or shared your laptop internet connection to make your laptop acting as a DHCP server. On a Mac, there is a very simple solution [here](https://mycyberuniverse.com/mac-os/connect-to-raspberry-pi-from-a-mac-using-ethernet.html). For Windows, you can follow [this tutorial](http://www.instructables.com/id/Direct-Network-Connection-between-Windows-PC-and-R/) or [this one](https://electrosome.com/raspberry-pi-ethernet-direct-windows-pc/). You can then use [Angry IP Scanner](http://angryip.org/) to determine the assigned IP address for the Raspberry.

We will use in this example 192.168.2.8 for the gateway address (DHCP option in order to have Internet access from the Raspberry)

	> ssh pi@192.168.2.8
	pi@192.168.200.1's password: 
	
	The programs included with the Debian GNU/Linux system are free software;
	the exact distribution terms for each program are described in the
	individual files in /usr/share/doc/*/copyright.
	
	Debian GNU/Linux comes with ABSOLUTELY NO WARRANTY, to the extent
	permitted by applicable law.
	Last login: Thu Aug  4 18:04:41 2016

You should see the text command interface described at the [end of this document](https://github.com/CongducPham/LowCostLoRaGw/tree/master/gw_full_latest#use-cmdsh-to-interact-with-the-gateway). The text command interface can be used in an interactive way to launch various commands for the gateway and can be easily customized.

**IMPORTANT NOTICE**: the LoRa gateway program starts automatically when the Raspberry is powered on. To verify that you have a running gateway, use option `3`.

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

To stop the gateway, use option `K` to kill all the gateway processes that are run at boot.

**IMPORTANT NOTICE**: Do not launch a new gateway instance with an existing one as there will be conflict on the SPI bus. When you need to perform some tests and start the gateway manually, use `K` to kill all processes first.

Use `Q` to quit the text command interface.

Make your gateway a WiFi client
------------------------------

In the default gateway configuration (i.e. from the SD card image) the gateway acts as a WiFi access point and Internet connectivity is provided through the wire Ethernet interface. In case you want to make the gateway a WiFi client so that it connects to the Internet through an existing WiFi network you have to do the following steps. Note that this solution works even if you are logged (`ssh`) on the gateway using the gateway's access point WiFi network.

1. run in the `lora_gateway/scripts` folder the `prepare_wifi_client.sh` script
2. run the following command:
	```
	wpa_passphrase "my_ssid" "my_password" | sudo tee -a /etc/wpa_supplicant/wpa_supplicant.conf > /dev/null
	```
	where `my_ssid` and `my_password` should be replaced by your WiFi SSID and password
	
3. reboot the gateway:
	```
	sudo shutdown -r now
	```
	
At reboot, your gateway is not acting as a WiFi access point anymore and should be connected to your WiFi network. One issue is to know the IP address assigned to your gateway by the WiFi access point. If your WiFi access point can show the list of leased IP address then you can easily determined your gateway IP address. 

Otherwise, one solution is to take a smartphone or a computer that is connected to the WiFi network in order to know the IP network address of the WiFi network advertised by your access point (e.g. 192.168.1.0). Then you can use a network tool such as `Angry IP scanner` available on most platforms (including Android smartphone) to ping and discover all devices on that WiFi network. Once you obtained the IP address of the gateway on that WiFi network, for instance 192.168.1.25, you can then use `ssh` to log into the gateway and use the gateway's web interface to manage your gateway as usual.

Once logged on your gateway through a computer on the same WiFi network, you can use:

	> iwgetid
	wlan0	ESSID:"my_ssid"
	
or 

	> iwconfig
	lo		no wireless extensions.
	
	wlan0	IEEE 802.11  ESSID:"my_ssid"  
			Mode:Managed  Frequency:2.437 GHz  Access Point: 2E:F0:A2:90:30:55
			Bit Rate=24 Mb/s   Tx-Power=31 dBm   
			Retry short limit:7   RTS thr:off   Fragment thr:off
			Power Management:on
			Link Quality=40/70  Signal level=-70 dBm  
			Rx invalid nwid:0  Rx invalid crypt:0  Rx invalid frag:0
			Tx excessive retries:0  Invalid misc:0   Missed beacon:0
	
	eth0	no wireless extensions.	  
	
to verify that the gateway is connected on the WiFi network. You can also try Internet connectivity by pinging a computer on the Internet.

If you are using the text command interface (`cmd.sh`, see end of this document), use option `g` to run `prepare_wifi_client.sh` and option `h` to interactively enter the SSID and password. Then use option `R` to reboot your gateway.

When you want to switch back the gateway into access point mode, then run in the `lora_gateway/scripts` folder the `start_access_point.sh` script (or option `i` of the text command interface).	

**The web admin interface** can also be used to put the gateway into WiFi client mode and to specify a WiFi network and password (equivalent to option `g` and `h`, and do not forget to reboot). In addition, `raspap-webgui` from https://github.com/billz/raspap-webgui has been integrated and can be accessed at http://`gw_ip_address`/raspap-webgui. **Once** the gateway is configured as a WiFi client with the web admin interface, `raspap-webgui` is useful to dynamically discover and configure additional WiFi networks. **However**, note that it is risky to specify several WiFi networks because it becomes very difficult to know on which WiFi network the gateway is connected to.

**The SD card image has `raspap-webgui` already installed**. If you want to install it manually, here are some instructions, mainly taken and adapted from https://github.com/billz/raspap-webgui. We suppose that you have our latest gateway software and all required packages, only additional packages will be described. The installation instructions are adapted to preserve our gateway settings.

Start off by installing `lighttpd`

	> sudo apt-get install lighttpd
	
After that, enable PHP for lighttpd and restart it for the settings to take effect

	> sudo lighttpd-enable-mod fastcgi-php
	> sudo service lighttpd restart	

Regarding the `www-data ALL=(ALL)` line for the `/etc/sudoers` file, we already updated our web admin interface installation script to include all of them. So no need to do this step here.

Then, git clone the files to `/var/www/html`. Note: for older versions of Raspbian (before Jessie, May 2016) use `/var/www` instead.

	> cd /var/www/html 
	> sudo git clone https://github.com/billz/raspap-webgui
	
Set the files ownership to `pi:www-data` user:group

	> sudo chown -R pi:www-data /var/www/html/raspap-webgui
	
Move the RaspAP configuration file to the correct location

	> sudo mkdir /etc/raspap
	> sudo mv /var/www/html/raspap-webgui/raspap.php /etc/raspap/
	> sudo chown -R www-data:www-data /etc/raspap

Move the HostAPD logging scripts to the correct location

	> sudo mkdir /etc/raspap/hostapd
	> sudo mv /var/www/html/raspap-webgui/installers/*log.sh /etc/raspap/hostapd 

Reboot and it should be up and running!

	> sudo reboot

The default username is 'admin' and the default password is 'secret'. RaspAp webgui can then be accessed at http://`gw_ip_address`/raspap-webgui. There is also a link from our web admin interface to RaspAp in the `System` menu.

Compiling the low-level gateway program
---------------------------------------	 	
    
DO NOT modify the `lora_gateway.cpp` file unless you know what you are doing. Check the `radio.makefile` file to indicate whether your radio module uses the PA_BOOST amplifier line or not (which means it uses the RFO line). HopeRF RFM92W/95W or inAir9B or NiceRF1276 or a radio module with +20dBm possibility (the SX1272/76 has +20dBm feature but some radio modules that integrate the SX1272/76 may not have the electronic to support it) need the `-DPABOOST`. Both Libelium SX1272 and inAir9 (not inAir9B) do not use PA_BOOST. You can also define a maximum output power to stay within transmission power regulations of your country. For instance, if you do not define anything, then the output power is set to 14dBm (ETSI european regulations), otherwise use `-DMAX_DBM=10` for 10dBm. Then:

	> make lora_gateway

If you are using a Raspberry v2 or v3:

	> make lora_gateway_pi2

On Raspberry v2 or v3 a symbolic link will be created that will point to `lora_gateway_pi2`.

To launch the low-level gateway (use `K` to kill all processes started at boot first):

	> sudo ./lora_gateway

By default, the low-level gateway runs in LoRa mode 1 and has address 1. With the low-level gateway you can receive LoRa message to test the radio reception of you gateway.

You can have a look at the "Low-cost-LoRa-GW-step-by-step" tutorial in our tutorial repository https://github.com/CongducPham/tutorials.

Adding LoRa gateway's post-processing features
----------------------------------------------

A data post-processing stage can be added after the low-level LoRa gateway program. This is usually how the gateway should be used in order to have advanced data management as a typical post-processing task is to push received data to Internet servers or dedicated (public or private) IoT clouds.

The `post_processing_gw.py` script handles end-device's raw data from the low-level LoRa gateway. The template script already implements data uploading to various public IoT clouds. See this [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-NewCloud.md) to know how to configure the cloud definition. Adding the post-processing stage is realized as follows:

	> sudo ./lora_gateway | python ./post_processing_gw.py

To further log processing output in a file (in `/home/pi/Dropbox/LoRa-test/post_processing.log`)

	> sudo ./lora_gateway | python ./post_processing_gw.py | python ./log_gw
	
**Note that if you want to run and test the above command now**, you have to create a `Dropbox` folder in your home directory with a subfolder `LoRa-test` that will be used locally. Please put attention to the name of the folders: they must be `Dropbox/LoRa-test` because the `post_processing_gw.py` Python script uses these paths. You can mount Dropbox later on if you want: the local folders and contents will be unchanged. **Otherwise, just run the `basic_config_gw.sh` configuration script as described [previously](https://github.com/CongducPham/LowCostLoRaGw#configuring-your-gateway-after-update) (recommended)**.

    > mkdir -p Dropbox/LoRa-test 	
	
Actually, both `lora_gateway` can take additional parameters to configure the radio module. However, it is more convenient to use the `start_gw.py` script that will parse the gateway configuration file, see below, to launch the low-level gateway accordingly:

	> sudo python start_gw.py

This is the command that we recommend and it is actually how the gateway starts when the Raspberry is powered on. To test, just flash a temperature sensor and it should work out-of-the-box on our [LoRa ThingSpeak test channel](https://thingspeak.com/channels/66794).

As `start_gw.py` simply reads the configuration file to launch `lora_gateway` and the `post_processing_gw.py` script, it is just a simpler way to run the gateway. You can however still use the corresponding command line for test purposes. For instance, with the default configuration file `sudo python start_gw.py` is equivalent to:

	> sudo ./lora_gateway --mode 1 | python ./post_processing_gw.py | python ./log_gw	

You can have a look at the "Low-cost-LoRa-GW-step-by-step" tutorial in our tutorial repository https://github.com/CongducPham/tutorials.

![](https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/post-processing.png)

Gateway configuration file
--------------------------

When using the full gateway with post-processing stage, a `gateway_conf.json` file defines the gateway configuration with several sections for radio configuration, local gateway option such as gateway ID, etc. One important field is the gateway ID which is composed of 8 bytes in hexadecimal notation. We use the last 5 bytes of the eth0 interface MAC address: `"gateway_ID" : "00000027EBBEDA21"`. Both `basic_config_gw.sh` and `config_gw.sh` script can do it for you, see below. Starting from November 3rd, 2017, the gateway ID is re-created from the MAC address every time the Raspberry reboots. This is done in order to automatically have a valid gateway id when installing a new gateway with the provided SD card image.

In `gateway_conf.json`, you can either specify the LoRa mode or the (bw,cr,sf) combination. If mode is defined, then the (bw,cr,sf) combination will be discarded. To use the (bw,cr,sf) combination, you have to set mode to -1. 

Cloud support is separated into different external script files (mainly Python but other language can be used). There is for instance an example for ThingSpeak with `CloudThinkSpeak.py`. See the dedicated [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-NewCloud.md) file on cloud management.

Configure your gateway with `basic_config_gw.sh` or `config_gw.sh`
--------------------------------------------------------------

`basic_config_gw.sh` should be sufficient for most of the cases. The configuration script mainly assign the gateway id so that it is uniquely identified (the gateway's WiFi access point SSID is based on that gateway id for instance). The gateway id will use the last 5 bytes of the Raspberry eth0 MAC address (or wlan0 on an RPI0W without Ethernet adapter) and the configuration script will extract this information for you.  

    > ifconfig
    eth0  Link encap:Ethernet  HWaddr b8:27:eb:be:da:21  
          inet addr:192.168.2.8  Bcast:192.168.2.255  Mask:255.255.255.0
          inet6 addr: fe80::ba27:ebff:febe:da21/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:16556 errors:0 dropped:0 overruns:0 frame:0
          TX packets:9206 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:6565141 (6.2 MiB)  TX bytes:1452497 (1.3 MiB)
          
In the example, we have `HWaddr b8:27:eb:be:da:21` then the gateway id will be `00000027EBBEDA21`. There is an additional script called `test_gwid.sh` in the script folder to test whether the gateway id can be easily determined. In the `scripts` folder, simply run `test_gwid.sh`:

	> cd /home/pi/lora_gateway/scripts
	> ./test_gwid.sh
	Detecting gw id as 00000027EBBEDA21
	
If you don't see something similar to 00000027EBBEDA21 (8 bytes in hex format) then you have to explicitly provide the **last 5 bytes** of the gw id to `basic_config_gw.sh`. Otherwise, in the `scripts` folder, simply run `basic_config_gw.sh` to automatically configure your gateway. 

	> cd /home/pi/lora_gateway/scripts
	> ./basic_config_gw.sh
	
or

	> ./basic_config_gw.sh 27EBBEDA21
	
`basic_config_gw.sh` takes care of:

- determining the gateway id
- compiling the `lora_gateway` program, the Raspberry board version will be checked automatically
- creating a `gateway_id.txt` file containing the gateway id (e.g. "00000027EBBEDA21")
- setting in `gateway_cong.json` the gateway id: "gateway_ID" : "00000027EBBEDA21"
- creating the `/home/pi/Dropbox/LoRa-test` folder for log files (if it does not exist) 
- creating a `log` symbolic link in the `lora_gateway` folder pointing to `/home/pi/Dropbox/LoRa-test` folder
- configuring `/etc/hostapd/hostapd.conf` for WiFi access point 
- configuring the gateway to run the `lora_gateway` program at boot

If you need more advanced configuration, then run `config_gw.sh`. for advanced WiFi and Bluettoth configuration tasks (if you use our SD card image, otherwise, you need first to install some required packages). If you don't want some features, just skip them. The configuration script also automatically determines the gateway id like previously. `config_gw.sh` **takes care of:**

- everything that `basic_config_gw.sh` is doing, **plus**
- configuring `/etc/bluetooth/main.conf` for Bluetooth 
- activating MongoDB storage 
- compiling DHT22 support 

**Even if you installed from the zipped SD card image `basic_config_gw.sh` or `config_gw.sh` is still needed to personalize your gateway to:**

- compile the `lora_gateway` program for your the Raspberry board version
- configure `/etc/hostapd/hostapd.conf` to advertise a WiFi SSID corresponding to last 5 hex-byte of your `eth0` interface (e.g. WAZIUP_PI_GW_27EBBEDA21) 

`gateway_conf.json` options
---------------------------

A typical `gateway_conf.json` is shown below:

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
	
["radio_conf"] defines the LoRa radio parameters. This section is read by `start_gw.py` to launch the `lora_gateway` program with the appropriate parameters. You can either specify the LoRa mode or the (bw,cr,sf) combination. If mode is defined, then the (bw,cr,sf) combination will be discarded. To use the (bw,cr,sf) combination, you have to set mode to -1. To specify an ad-hoc frequency, use ["freq" : 433.3] for instance. If you want to use "ch", you have to modify `lora_gateway.cpp` to select the correct frequency band and then only you can use ["ch" : 10]. If BAND900 is selected in lora_gateway.cpp then the channel used will be CH_10_900 which is 924.68MHz (defined in `SX1272.h`).

["gateway_conf"]["gateway_ID""] is composed of 8 bytes in hexadecimal notation. We use the last 5 bytes of the eth0 interface MAC address.

["gateway_conf"]["ref_latitude"] you can put the gateway latitude here. 

["gateway_conf"]["ref_longitude"] you can put the gateway longitude here. For instance, `CloudGpsFile.py` uses the gateway GPS coordinates to calculate the distance of remote GPS device to the gateway. It is also possible to periodically push the gateway GPS coordinates on a cloud platform (similar to what is is done in LoRaWAN network server). For that purpose, `post_processing_gw.py` periodically calls `post_status_processing_gw.py` where customized peridodic tasks can be added.

["gateway_conf"]["wappkey"] when set to true will enable app key enforcement in `post_processing_gw.py`. Add in `key_AppKey.py` the list of appkey that you want to enable.

["gateway_conf"]["raw"] when set to true will make  the `lora_gateway` program to provide raw payload to the post-processing stage. Then, `post_processing_gw.py` will try to dissect the packet which will be in most case a LoRaWAN packet. However, you can use your own packet format and then modify `post_processing_gw.py` accordingly.

["gateway_conf"]["aes"] when set to true will enable local AES decryption in `post_processing_gw.py`. The AES AppSKey and NwkSKey are stored in `loraWAN_config.py`. They must match those used by the end-device. We only support ABP (activitation by personalization) method. Actually, AES is usefull to provide data privacy.

["gateway_conf"]["log_post_processing"] when set to true will make `start_gw.py` to additionally launch `log_gw.py` script to log all the `post_processing_gw.py`'s outputs.

["gateway_conf"]["log_weekly"] when set to true will make `log_gw.py` to create a new log file every week, instead of every month.

["gateway_conf"]["dht22"] indicates the time interval (in second) for `post_processing_gw.py` to trigger a temperature/humidity measure from the DHT22 sensor every N seconds (that you must connect and install, see step H). `post_processing_gw.py` will typically display the following information, that will be logged in the log file.

	2017-03-31T23:42:52.703430> Getting gateway temperature
	2017-03-31T23:42:52.703722> Gateway TC : 26.40 C | HU : 24.90 % at 2017-03-31 23:42:52.703074

["gateway_conf"]["dht22_mongo"] when set to true will further store the temperature/humidity measure in the local MongDB. Then, these measures will be visible on the gateway's web page. You can check with this feature the condition inside the gateway's case in outdoor deployment.

["gateway_conf"]["downlink"] indicates the time interval (in second) for `post_processing_gw.py` to check for a `downlink-post.txt`. See this [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-downlink.md).

["gateway_conf"]["status"] indicates the time interval (in second) for `post_processing_gw.py` to call `post_status_processing_gw.py` for periodic tasks. Currently, `post_status_processing_gw.py` will display a status message to indicate that the script is correctly running in case you don't receive packet for a long time.

	2017-12-27T14:30:17.496030> status: start running
	2017-12-27T14:30:17.496899> status: show current GPS position
	2017-12-27T14:30:17.497074> show GPS: current GPS coordinate: gw lat my_lat long my_long
	2017-12-27T14:30:17.497242> status: exiting

We plan in the future to send appropriate message to a LoRaWAN network server (such as TTN) in the same way the `single_chan_pkt_fwd` program from Thomas Telkamp works (and like most LoRaWAN gateways). 

["gateway_conf"]["aux_radio"] indicates the time interval (in second) for `post_processing_gw.py` to check for a `aux_radio_post.txt` file with data received from other radio interfaces, e.g. IEEE802.15.4, etc. This feature is not currently distributed as it is still in the early stage of development.

["alert_conf"]["use_mail"] when set to true indicates that `post_processing_gw.py` will sent an alerting mail on specific events. There are currently 2 events: when `post_processing_gw.py` is started (which usually means that the gateway has booted and is up) and when the radio module has been reset by the low-level lora_gateway program because of some receive errors.

["alert_conf"]["contact_mail"] is the list of email address recipients: e.g. joejoejoe@gmail.com,jackjackjack@hotmail.com,...

["alert_conf"]["mail_from"] is a valid address email with associated account

["alert_conf"]["mail_server"] is an SMTP server name

["alert_conf"]["mail_passwd"] is the passwd. It is not very secure but there is no other way to the best of my knowledge.

["alert_conf"]["use_sms"] when set to true indicates that `post_processing_gw.py` will sent an alerting SMS on specific events. There are currently 2 events: when `post_processing_gw.py` is started (which usually means that the gateway has booted and is up) and when the radio module has been reset by the low-level lora_gateway program because of some receive errors. We already tested the SMS transmission using gammu with a 3G dongle (see this [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/3GDongle/README.md) that explains how to use a 3G dongle for Internet connectivity).

["alert_conf"]["pin"] is the SIM card pin number if this feature is enabled.

["alert_conf"]["contact_sms"] is the phone number, with country prefix, for the SMS.	

["alert_conf"]["gammurc_file"] is the path to the GAMMU configuration file for the 3G dongle. See the [3G Dongle README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/3GDongle/README.md).

Adding new features
-------------------

The gateway architecture is flexible on purpose for adding or customizing your gateway by modifying the `post_processing_gw.py` script or the `post_status_processing_gw.py` script or by adding new cloud scripts. To add new options, you can either add new fields in one of the current sections in `gateway_conf.json`, or add a brand new section. 

New cloud platforms can be added by writing new cloud scripts and adding them to `clouds.json`. See this [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-NewCloud.md).

Use `cmd.sh` to interact with the gateway
-----------------------------------------

When you connect with `ssh` to the gateway, a text command interface, `cmd.sh`, is launched. If you have the WiFi access point enabled you can use a smartphone with an `ssh` app to log on 192.168.200.1. On iOS we tested with `Termius` and on Android we tested with `JuiceSSH`. You can use `cmd.sh` as follows:

	> ./cmd.sh
	=======================================* Gateway 00000027EBBEDA21 *===
	0- sudo python start_gw.py                                           +
	1- sudo ./lora_gateway --mode 1                                      +
	2- sudo ./lora_gateway --mode 1 | python post_processing_gw.py       +
	3- ps aux | grep -e start_gw -e lora_gateway -e post_proc -e log_gw  +
	4- tail --line=15 ../Dropbox/LoRa-test/post-processing.log           +
	5- tail -f ../Dropbox/LoRa-test/post-processing.log                  +
	6- less ../Dropbox/LoRa-test/post-processing.log                     +
	---------------------------------------------------* Connectivity *--+
	f- test: ping www.univ-pau.fr                                        +
	g- wifi: configure as WiFi client at next reboot                     +
	h- wifi: indicate WiFi SSID and password at next reboot              +
	i- wifi: configure as WiFi access point at next reboot               +	
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
	----------------------------------------------------------* ngrok *--+
	M- get and install ngrok                                             +
	N- ngrok authtoken                                                   +
	O- ngrok tcp 22                                                      + 	
	---------------------------------------------------------* Update *--+
	U- update to latest version on repository                            +
	V- download and install a file                                       +
	W- run a command                                                     +
	-----------------------------------------------------------* kill *--+
	K- kill all gateway related processes                                +
	k- kill rfcomm-server process                                        +
	R- reboot gateway                                                    +
	S- shutdown gateway                                                  +
	---------------------------------------------------------------------+
	Q- quit                                                              +
	======================================================================
	Enter your choice: 

`cmd.sh` needs a file called `gateway_id.txt` that should contain the ID of your gateway. As indicated previously, the gateway ID is composed of 8 bytes in hexadecimal notation with the last 5 bytes being the last 5 bytes of the gateway eth0 interface MAC address. It is exactely the same ID that the one indicated in `gateway_conf.json`. If you start `cmd.sh` without this `gateway_id.txt` file, `cmd.sh` will create such file by determining last 5 bytes of the gateway eth0 interface MAC address:

	> cat gateway_id.txt
	00000027EBBEDA21
	
`cmd.sh` will also set the gateway id in the `gateway_conf.json` file: `"gateway_ID" : "00000027EBBEDA21"`. 

Normally, the LoRa gateway starts automatically on boot. To verify that you have a running gateway, use option `3`.

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

To stop the gateway, use option `K`. This option can also kill the gateway processes that are run at boot.

**IMPORTANT NOTICE**: Do not launch a new gateway instance with an existing one as there will be conflict on the SPI bus.

You can start manually the gateway for test purposes with option `0`. 
	
**IMPORTANT NOTICE**: To run an operation gateway, it is better to reboot the gateway and let the LoRa gateway program start at boot. Manually lauching the gateway can be usefull for test purposes but we observed that redirections of LoRa gateway output to the post-processing stage can be broken thus leading to a not responding gateway.

You can then use option `5` to see the logs in real time. To test the simple low-level gateway, use option `1` (after killing all gateway's processes with option `K`. You can `ssh` at any time and use option `5` to see the latest packets that have been received. 	

You can easily add new useful commands to the `cmd.sh` shell script.
	
Enjoy!
C. Pham	