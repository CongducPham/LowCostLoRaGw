3G connectivity with 3G USB dongle
==================================

Dongles
-------

These 2 dongles have been successfully tested and work without any issues:

- https://www.amazon.fr/gp/product/B01G3GLPDU/ref=oh_aui_detailpage_o04_s00?ie=UTF8&psc=1
- https://fr.aliexpress.com/item/Unlocked-Huawei-Lte-Modem-Adapter-E220-3g-Wwan-Hsdpa-Utms-Usb-Modem-7-2m-Win7-Android/32329684728.html?spm=2114.13010608.0.0.S9UqH7

Generally, most of the Huawei E220 dongles should work fine and I personally find the USB cable to be more practical.

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
	
You should have an /etc/wvdial.conf file

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
	
In my case, the provider (Bouygues Telecom) was not correctly detected so I added the following lines. Username and Password are not important, put whatever you want. The name of the provider is also not very important.

	> sudo nano /etc/wvdial.conf
	[Dialer bt]
	Init2 = ATZ
	Init3 = ATQ0 V1 E1 S0=0 &C1 &D2
	Init4 = AT+CGDCONT=1,"IP","bt"
	Stupid mode = 1
	Phone = *99#
	New PPPD = yes
	Check Def Route = 1
	Username = bt
	Password = bt	
	
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
	
Then just run (in background):

	> sudo wvdial bt &
	[1] 1436
	--> WvDial: Internet dialer version 1.61
	--> Initializing modem.
	--> Sending: ATZ
	ATZ
	OK
	--> Sending: ATZ
	OK
	--> Sending: ATQ0 V1 E1 S0=0 &C1 &D2
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
		
		
Now your RPI gateway has 3G internet connectivity to upload data on IoT clouds.

To shutdown the connection, just kill the process:

	> sudo kill -9 1436

In general, use "ps aux | grep wvdial" to know the process id associated to wvdial.


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
	
This step is very important because the SMS Service uses this file for sending sms.

Check 3g dongle characteristics:

	> gammu identity
	Device               : /dev/ttyUSB0
	Manufacturer         : Huawei
	Model                : E220 (E220)
	Firmware             : 11.117.10.00.00
	IMEI                 : 3XX19301XXXXXX3
	SIM IMSI 			 : 2XXXX923271XXX1

Gateway's SMS Service 
---------------------

Configure the SMS service in clouds.json
	
	>	{	
			"name":"SMS Service",
			"script":"python CloudSMS.py",
			"type":"smsservice",
			"pin":0,   
			"contacts":["+336XXXXXXXX","+336XXXXXXXX","+337XXXXXXXX"],
			"gammurc_file":"/home/pi/.gammurc",
			"enabled":true,
			"always_enabled":true
		}
	
|                |         												        |
| -------------  |: ----------------------------------------------------------: |
| pin       	 |your pin code, if it is not removed. For example 1234         |
| contacts       |expected phone numbers to receive sms                         |
| gammurc_file   |.gammurc file location. By default : /home/pi/.gammurc        |
| enabled        |always true, don't change it     								|
| always_enabled |if true, sms will be sent whenever data is pushed.            |
|				 |If false, sms will be sent only if internet not available.	|

Now the SMS service is operational.

Enjoy,
C. Pham & M. Diop		
		