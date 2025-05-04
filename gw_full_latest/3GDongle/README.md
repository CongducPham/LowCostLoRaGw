3G/4G connectivity with 3G/4G USB dongle
==================================

Dongles
-------

The following dongles have been successfully tested:

- https://www.amazon.fr/gp/product/B01G3GLPDU/ref=oh_aui_detailpage_o04_s00?ie=UTF8&psc=1
- https://fr.aliexpress.com/item/Original-Unlocked-HUAWEI-E220-7-2Mbps-3G-USB-dongle-HSDPA-USB-STICK-support-Android-system/32779450595.html

Most of Huawei E220 dongles (or compatible) should work fine and I personally find the USB cable to be more practical. Also, these dongles seems to not suffer from USB disconnection.

It is generally safer to use a 3G/4G router where the dongle can be plugged. We also tested these 2 products that work fine:

- https://www.amazon.fr/gp/product/B003WK62OS/ref=oh_aui_detailpage_o08_s00?ie=UTF8&psc=1
- https://www.amazon.fr/gp/product/B00634PLTW/ref=oh_aui_detailpage_o01_s00?ie=UTF8&psc=1

Many information in this file has been taken from the following original article: https://enavarro.me/raspberry-pi-connection-3g-et-envoi-de-sms-en-meme-temps.html

**If you are using our SD card image** then everything is already set up. You just need to plug your USB 2G/3G/4G dongle with a valid SIM card (with no pin configuration) and run:

	> cd /home/pi/lora_gateway/3GDongle
	> ./enable-3GDongle-internet-on-boot.sh
	> sudo reboot
	
You can also use the web admin interface to enable the dongle on boot (`System`	menu, then `Cellular` tab) and reboot.

Installation on Raspberry (Jessie)
----------------------------------

First, it is safer to have higher current on USB by having "max_usb_current=1" in /boot/config.txt:

	> cat /boot/config.txt
	...
	max_usb_current=1

Restart your RPI then plug-in the USB 3G dongle (it is better to remove the pin code protection on the SIM card, use a telephone/smartphone to do this step):

	> lsusb
	Bus 001 Device 005: ID 05c6:6000 Qualcomm, Inc. Siemens SG75
	Bus 001 Device 003: ID 0424:ec00 Standard Microsystems Corp. SMSC9512/9514 Fast Ethernet Adapter
	Bus 001 Device 002: ID 0424:9514 Standard Microsystems Corp. 
	Bus 001 Device 001: ID 1d6b:0002 Linux Foundation 2.0 root hub

You should see the Qualcomm modem.

Then, we need wvdial to configure and manage the dongle, so first we install the wvdial package:

	> sudo apt-get install wvdial

Then, we use wvdialconf to check and configure the dongle:

	> sudo wvdialconf
	Editing `/etc/wvdial.conf'.

	Scanning your serial ports for a modem.

	ttyUSB0<*1>: ATQ0 V1 E1 -- OK
	ttyUSB0<*1>: ATQ0 V1 E1 Z -- OK
	ttyUSB0<*1>: ATQ0 V1 E1 S0=0 -- OK
	ttyUSB0<*1>: ATQ0 V1 E1 S0=0 &C1 -- OK
	ttyUSB0<*1>: ATQ0 V1 E1 S0=0 &C1 &D2 -- OK
	ttyUSB0<*1>: ATQ0 V1 E1 S0=0 &C1 &D2 +FCLASS=0 -- OK
	ttyUSB0<*1>: Modem Identifier: ATI -- Manufacturer: QUALCOMM INCORPORATED
	ttyUSB0<*1>: Speed 9600: AT -- OK
	ttyUSB0<*1>: Max speed is 9600; that should be safe.
	ttyUSB0<*1>: ATQ0 V1 E1 S0=0 &C1 &D2 +FCLASS=0 -- OK
	ttyUSB1<*1>: ATQ0 V1 E1 -- OK
	ttyUSB1<*1>: ATQ0 V1 E1 Z -- OK
	ttyUSB1<*1>: ATQ0 V1 E1 S0=0 -- OK
	ttyUSB1<*1>: ATQ0 V1 E1 S0=0 &C1 -- OK
	ttyUSB1<*1>: ATQ0 V1 E1 S0=0 &C1 &D2 -- OK
	ttyUSB1<*1>: ATQ0 V1 E1 S0=0 &C1 &D2 +FCLASS=0 -- OK
	ttyUSB1<*1>: Modem Identifier: ATI -- Manufacturer: QUALCOMM INCORPORATED
	ttyUSB1<*1>: Speed 9600: AT -- OK
	ttyUSB1<*1>: Max speed is 9600; that should be safe.
	ttyUSB1<*1>: ATQ0 V1 E1 S0=0 &C1 &D2 +FCLASS=0 -- OK
	ttyUSB2<*1>: ATQ0 V1 E1 -- failed with 2400 baud, next try: 9600 baud
	ttyUSB2<*1>: ATQ0 V1 E1 -- failed with 9600 baud, next try: 9600 baud
	ttyUSB2<*1>: ATQ0 V1 E1 -- and failed too at 115200, giving up.
	ttyUSB3<*1>: ATQ0 V1 E1 -- failed with 2400 baud, next try: 9600 baud
	ttyUSB3<*1>: ATQ0 V1 E1 -- failed with 9600 baud, next try: 9600 baud
	ttyUSB3<*1>: ATQ0 V1 E1 -- and failed too at 115200, giving up.

	Found a modem on /dev/ttyUSB0.
	Modem configuration written to /etc/wvdial.conf.
	ttyUSB0<Info>: Speed 9600; init "ATQ0 V1 E1 S0=0 &C1 &D2 +FCLASS=0"
	ttyUSB1<Info>: Speed 9600; init "ATQ0 V1 E1 S0=0 &C1 &D2 +FCLASS=0"	
	
You should have an minimum /etc/wvdial.conf file

	> cat /etc/wvdial.conf
	[Dialer Defaults]
	Init1 = ATZ
	Init2 = ATQ0 V1 E1 S0=0 &C1 &D2 +FCLASS=0
	Modem Type = Analog Modem
	; Phone = <Target Phone Number>
	ISDN = 0
	; Password = <Your Password>
	New PPPD = yes
	; Username = <Your Login Name>
	Modem = /dev/ttyUSB0
	Baud = 9600	
	
Add the following lines. Username and Password are not important, put whatever you want. The name of the provider is also not very important.

	> sudo nano /etc/wvdial.conf
	[Dialer Defaults]
	Init1 = ATZ
	Init2 = ATQ0 V1 E1 S0=0 &C1 &D2 +FCLASS=0
	Init3 = AT+CGDCONT=1,"IP","bt"
	Stupid mode = 1
	Modem Type = Analog Modem
	ISDN = 0
	New PPPD = yes
	Phone = *99#
	Check Def Route = 1
	Username = bt
	Password = bt	
	Modem = /dev/ttyUSB0
	Baud = 9600
	
Then just run (in background):

	> sudo wvdial &
	[1] 1436
	--> WvDial: Internet dialer version 1.61
	--> Initializing modem.
	--> Sending: ATZ
	ATZ
	OK
	--> Sending: ATZ
	OK
	--> Sending: ATQ0 V1 E1 S0=0 &C1 &D2 +FCLASS=0
	OK
	--> Sending: AT+CGDCONT=1,"IP","bt"
	AT+CGDCONT=1,"IP","bt"
	OK
	--> Modem initialized.
	--> Sending: ATDT*99#
	--> Waiting for carrier.
	ATDT*99#
	CONNECT 28800000
	--> Carrier detected.  Starting PPP immediately.
	--> Starting pppd at Thu Mar 16 09:25:39 2017
	--> Pid of pppd: 20726
	--> Using interface ppp0
	--> local  IP address 10.214.254.75
	--> remote IP address 10.64.64.64
	--> primary   DNS address 62.201.129.99
	--> secondary DNS address 62.201.159.99	
	
We can verify the ppp0 interface:

	> ifconfig
	...
	ppp0	Link encap:Point-to-Point Protocol  
			inet addr:10.214.254.75  P-t-P:10.64.64.64  Mask:255.255.255.255
			UP POINTOPOINT RUNNING NOARP MULTICAST  MTU:1500  Metric:1
			RX packets:107 errors:0 dropped:0 overruns:0 frame:0
			TX packets:121 errors:0 dropped:0 overruns:0 carrier:0 collisions:0 txqueuelen:3 
			RX bytes:29481 (28.7 KiB)  TX bytes:12666 (12.3 KiB)	
		
You can also check the IP routing table:

	> route
	Kernel IP routing table
	Destination     Gateway         Genmask         Flags Metric Ref    Use Iface
	default         *               0.0.0.0         U     0      0        0 ppp0
	10.64.64.64     *               255.255.255.255 UH    0      0        0 ppp0
	link-local      *               255.255.0.0     U     303    0        0 wlan0
	192.168.200.0   *               255.255.255.0   U     0      0        0 wlan0

You should have the default route using the ppp0 interface.
			
Now your RPI gateway has 3G internet connectivity to upload data on IoT clouds.

To shutdown the connection, just kill the process:

	> sudo kill -9 1436

In general, use "ps aux | grep wvdial" to know the process id associated to wvdial.

If you want to have the 3G connectivity on boot, then add the following lines in your /etc/network/interface file:

	auto ppp0
	iface ppp0 inet wvdial
	
Then you can test with:

	> sudo ifup ppp0
	> sudo ifdown ppp0	


Send SMS via the gateway's SMS Service 
===============================================

Gammu
-----

Install gammu:

	> sudo apt-get update
	> sudo apt-get install gammu
	> sudo apt-get install python-gammu
		
Find to which USB port the device is connected to:

	> dmesg | grep tty
	[    11.738624] usb 1-1.2: GSM modem (1-port) converter now attached to ttyUSB0
	[ 	 11.738962] usb 1-1.2: GSM modem (1-port) converter now attached to ttyUSB1
 
Configure gammu for a .gammurc file

	> gammu-detect
	; Configuration file generated by gammu-detect.
	; Please check The Gammu Manual for more information.
	[gammu]
	device = /dev/ttyUSB0
	name = Phone on USB serial port HUAWEI_Technologies HUAWEI_Mobile
	connection = at
	[gammu1]
	device = /dev/ttyUSB1
	name = Phone on USB serial port HUAWEI_Technologies HUAWEI_Mobile
	connection = at
	
Copy command result into .gammurc:

	> gammu-detect > .gammurc
	
This step is very important because the SMS Service uses this file for sending SMS. The path is typically /home/pi/.gammurc.

**IMPORTANT** If you use the 3G dongle for Internet AND for SMS, then you have to use /dev/ttyUSB1 for SMS assuming that /dev/ttyUSB0 is used for 3G connectivity as defined previously in the /etc/wvdial.conf file. Therefore, you can edit your .gammurc file to have /dev/ttyUSB0 and /dev/ttyUSB1 inversed:

	> nano .gammurc
	; Configuration file generated by gammu-detect.
	; Please check The Gammu Manual for more information.
	[gammu]
	device = /dev/ttyUSB1
	name = Phone on USB serial port HUAWEI_Technologies HUAWEI_Mobile
	connection = at
	[gammu1]
	device = /dev/ttyUSB0
	name = Phone on USB serial port HUAWEI_Technologies HUAWEI_Mobile
	connection = at	

Check 3G dongle characteristics:

	> gammu identity
	Device				: /dev/ttyUSB0
	Manufacturer		: Huawei
	Model				: E220 (E220)
	Firmware			: 11.117.10.00.00
	IMEI				: 3XX19301XXXXXX3
	SIM IMSI			: 2XXXX923271XXX1

Gateway's SMS Service 
---------------------

Configure the SMS service in clouds.json
	
		{	
			"name":"SMS Service",
			"script":"python CloudSMS.py",
			"type":"smsservice",
			"gammurc_file":"/home/pi/.gammurc",
			"enabled":true,
			"always_enabled":true
		}
	
| fields         | description        									        |
| -------------- | ------------------------------------------------------------ |
| gammurc_file   | .gammurc file location. By default : /home/pi/.gammurc       |
| enabled        | if set to true, will enabled SMS cloud service				|
| always_enabled | if true, SMS will be sent whenever data is pushed            |
|				 | if false, SMS will be sent only if Internet is not available |

Following the cloud philosophy for storing key or identification information (see [the new cloud README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-NewCloud.md#good-practice-for-storing-keys)), a key_SMS.py file will store the SIM card pin number, the SMS number and a list of allowed source address as follows:

	> cat key_SMS.py
	PIN = "1234"
	
	contacts=["+1XXXXXXXXX"]
	
	source_list=[]
	
| fields         | description        									        |
| -------------- | ------------------------------------------------------------ |
| pin       	 | your pin code, if it is not disabled. For example 1234       |
| contacts       | list of phone numbers to receive SMS		                    |
| source_list    | list of device's address allowed for the SMS service         |
|				 | Ex: ["6","3"]. If set to [], then all devices are accepted   |
	

Now the SMS service is operational.

Enjoy,
C. Pham & M. Diop		
		