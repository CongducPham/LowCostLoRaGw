3G connectivity with 3G USB dongle
==================================

Dongles
-------

It is actually the same, but from different vendors on Amazon.

- https://www.amazon.com/TOOGOO-USB2-0-7-2Mbps-Wireless-Network/dp/B00ORLBDPE
- https://www.amazon.com/Sunrise-7-2MBPS-Wireless-Computer-Navigation/dp/B01F92CDBM/ref=pd_lpo_147_bs_t_2?_encoding=UTF8&psc=1&refRID=T1407SB31NTQCPCPHHCY
- https://www.amazon.com/Pumpkin-Android-System-Wireless-navigation/dp/B00PQ6HUNS/ref=pd_lpo_147_tr_t_3?_encoding=UTF8&psc=1&refRID=T1407SB31NTQCPCPHHCY
- https://www.amazon.fr/Pumpkin-Adaptateur-7-2Mbps-Autoradio-Soutient/dp/B01G3GLPDU/ref=pd_cp_107_1?_encoding=UTF8&psc=1&refRID=QRFWTCDR7QZS7SFVH2XN
- https://www.amazon.fr/dp/B01K5P0WCA?psc=1
- https://www.amazon.fr/Hsdpa-7-2Mbps-Adaptateur-R%C3%A9seau-Dongle/dp/B01K5P0WCA/ref=pd_sbs_107_1?_encoding=UTF8&psc=1&refRID=X494DFM8X4ZPNDMMC3BP

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
	
In my case, the provider (Bouygues Telecom) was not correctly detected so I added the following lines. Username and Password are not important, put whatever you want.

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

Enjoy,
C. Pham		
		