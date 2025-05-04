Using the LORANGA 2G/3G board
=============================

The [LORANGA](https://www.alegrefactory.com/loranga) board from "La Fabrica Alegre" company embeds an RFM95W as the LoRa module. This module needs PABOOST so in order to use the module you first have to edit `radio.makefile` and make sure that PABOOST is enabled:

	CFLAGS=-DPABOOST -DMAX_DBM=14
	
Then, run the `scripts/basic_config_gw.sh` to both configure your gateway and compile the `lora_gateway` program.	

**If you are using our SD card image** then everything is already set up. You just need to plug your Loranga board with a valid SIM card (with no pin configuration) and run:

	> cd /home/pi/lora_gateway/3GDongle/loranga
	> ./enable-loranga-internet-on-boot.sh
	> sudo reboot
	
You can also use the web admin interface to enable the Loranga board on boot (`System`	menu, then `Cellular` tab) and reboot.

Configuring Internet connection
-------------------------------

The support of [LORANGA](https://www.alegrefactory.com/loranga) board has been realized in collaboration with "La Fabrica Alegre" development team.

They have the following [test github](https://github.com/edu986/test_ppp) to explain the installation of the support of the LORANGA board. Note that the `wvdial` method explained for 3G USB dongles is currently not working with the LORANGA board.

The `ppp-creator.py` script basically creates:

- `/etc/chatscripts` folder
- `/etc/ppp/peers` folder
- `/etc/chatscripts/quectel-chat-connect` script
- `/etc/chatscripts/quectel-chat-disconnect` script
- `/etc/ppp/peers/gprs` configuration file

We have a slightly modified `ppp-creator.py` script for the `/etc/chatscripts/quectel-chat-disconnect` script. Here are example of what you should obtained after running `ppp-creator.py` with 2 arguments. The first one being your provider APN and the second one being the serial port used by the Loranga board which is typically `ttyS0` on an RPI3.

	> sudo ./ppp-creator.sh mmsbouygtel.com ttyS0

`/etc/chatscripts/quectel-chat-connect` script:

	ABORT "BUSY"
	ABORT "NO CARRIER"
	ABORT "NO DIALTONE"
	ABORT "ERROR"
	ABORT "NO ANSWER"
	TIMEOUT 30
	"" AT
	OK ATE0
	OK ATI;+CSUB;+CSQ;+CPIN?;+COPS?;+CGREG?;&D2
	# Insert the APN provided by your network operator, default apn is mmsbouygtel.com
	OK AT+CGDCONT=1,"IP","\T",,0,0
	OK ATD*99#
	CONNECT	

`/etc/chatscripts/quectel-chat-disconnect` script:

	ABORT           'BUSY'
	ABORT           'ERROR'
	ABORT           'NO DIALTONE'
	TIMEOUT         30
	''              '+++\c'
	SAY             " + sending break"
	'NO CARRIER'    'ATH'
	SAY             "\n + dropping data connection"
	OK              'AT+CGATT=0'
	SAY             "\n + disconnecting from GPRS"
	OK              '\c'
	SAY             "\n + disconnected."

`/etc/ppp/peers/gprs` configuration file:

	/dev/ttyS0 115200
	# The chat script, customize your APN in this file
	connect 'chat -s -v -f /etc/chatscripts/quectel-chat-connect -T mmsbouygtel.com'
	# The close script
	disconnect 'chat -s -v -f /etc/chatscripts/quectel-chat-disconnect'
	# Hide password in debug messages
	hide-password
	# The phone is not required to authenticate
	noauth
	# Debug info from pppd
	debug
	# If you want to use the HSDPA link as your gateway
	defaultroute
	# pppd must not propose any IP address to the peer
	noipdefault
	# No ppp compression
	novj
	novjccomp
	noccp
	ipcp-accept-local
	ipcp-accept-remote
	local
	# For sanity, keep a lock on the serial line
	lock
	modem
	dump
	nodetach
	# Hardware flow control
	nocrtscts
	remotename 3gppp
	ipparam 3gppp
	ipcp-max-failure 30
	# Ask the peer for up to 2 DNS server addresses
	usepeerdns

If you use our latest version of SD card image (from September 2017) then everything is set up to use the LORANGA board. You just have to run:

	> cd /home/pi/lora_gateway/3GDongle/loranga
	> ./start-internet.sh
	
However, you will probably need	to change the APN information for those from your mobile operator. This is done in the `/etc/ppp/peers/gprs` configuration file

	> sudo nano /etc/ppp/peers/gprs
	
and change `mmsbouygtel.com` by your APN information (for instance SFR uses `sl2sfr`.

Prior to launch the script, you also need to indicate whether you have the Loranga 2G or the Loranga 3G board. This is done by running `select-loranga3G-board.sh` if you have the 3G version. The script basically creates the `loranga3G.txt` file which existence will be checked by `start-internet.sh` to run the appropriate Pythin script to power the Loranga modem.

To start the 2G/3G Internet connection at boot, run:

	> ./enable-loranga-internet-on-boot.sh
	
This simple script simply creates a file called `use_loranga_internet_on_boot.txt`. 

The `start_gw.sh` script, called at boot time, now checks for this file and will run the `start-internet.sh` script before launching the gateway program and post-processing stage. However, it is safer to test it first in interactive mode to see how the ppp connection is going. Once you are supposed to have Internet, try to ping some remote computers or use the web admin interface to see whether the gateway has Internet or not. 

You can provide a parameter to `start-internet.sh` when testing in interactive mode. With no parameters, `pppd` logs are sent to `stdout` which is not very convenient. You can use:

	> ./start-internet.sh "logfile pppd.log"
	
To redirect `pppd` logs to `pppd.log` (in the `/home/pi/lora_gateway/3GDongle/loranga` folder). If you don't want any logs, run:

	> ./start-internet.sh nolog	

When `pppd` is started at boot for an operational gateway, `start-internet.sh` is run without any parameters but it is not important because the logs will not appear on `stdout`. We did not want to have logs in a file because we want to avoid having a log file that can grow quite fast. However, if you want to have the logs saved in a file, you can just edit `/home/pi/lora_gateway/scripts/start_gw.sh` and add `logfile pppd.log` after `./start-internet.sh`.

Use `stop-internet.sh` to stop the pppd process and power-off the LORANGA board. Also, you can disable the usage of LORANGA on boot with:

	> ./disable-loranga-internet-on-boot.sh
	
which simply deletes the `use_loranga_internet_on_boot.txt` file. 	

Sending SMS
-----------

In order to use the LORANGA board for sending SMS with `CloudSMS.py`, you just have to edit (or create) in your `/home/pi` folder the `.gammurc` file:

	[gammu]
	device = /dev/ttyS0
	name = loranga
	connection = at
	
That should work, even if `gammu-detect` is not returning anything. If you want to try if SMS support is working, first kill all gateway processes (option `K` of `cmd.sh`) then edit `clouds.json` to enable `CloudSMS.py`:

	{	
		"name":"SMS Service",
		"script":"python CloudSMS.py",
		"type":"smsservice",
		"gammurc_file":"/home/pi/.gammurc",
		"enabled":true,
		"always_enabled":true
	},

Then edit `key_SMS.py` to indicate the phone number that will receive the SMS (it is better to disable the SIM card PIN with your mobile phone prior to insert the SIM card in the LORANGA board):

	contacts=["+33xxxxxxxxx"]
		
Then start manually the gateway:

	> sudo python start_gw.py
	
And use an end-device (our interactive device for instance) to send `\!TC/22.5` to the gateway. You should see something like:

	--- rxlora. dst=1 type=0x10 src=6 seq=2 len=9 SNR=6 RSSIpkt=-57 BW=125 CR=4/5 SF=12
	2017-09-02T09:47:19.960920> 2017-09-02T09:47:19.960399
	2017-09-02T09:47:19.961140> rcv ctrl pkt info (^p): 1,16,6,2,9,6,-57
	2017-09-02T09:47:19.961329> splitted in:  [1, 16, 6, 2, 9, 6, -57]
	2017-09-02T09:47:19.961516> (dst=1 type=0x10(DATA) src=6 seq=2 len=9 SNR=6 RSSI=-57)
	2017-09-02T09:47:19.961701> rcv ctrl radio info (^r): 125,5,12
	2017-09-02T09:47:19.961889> splitted in:  [125, 5, 12]
	2017-09-02T09:47:19.962099> (BW=125 CR=5 SF=12)
	2017-09-02T09:47:19.962445> rcv timestamp (^t): 2017-09-02T09:47:19.959
	2017-09-02T09:47:19.962670> 
	2017-09-02T09:47:19.962868> got first framing byte
	2017-09-02T09:47:19.963050> --> got data prefix
	2017-09-02T09:47:19.963252> valid app key: accept data
	2017-09-02T09:47:19.963462> number of enabled clouds is 1
	2017-09-02T09:47:19.963661> --> cloud[0]
	2017-09-02T09:47:19.963860> uploading with python CloudSMS.py
	2017-09-02T09:47:38.567588> rcv msg to send via the SMS Service: SensorData Sensor6 SNR 6 RSSI -57 SN 2 BW 125 CR 5 SF 12 GWID 00000027EBBEDA21 TC 22.5
	2017-09-02T09:47:38.567983> Sent to +33xxxxxxxxx successfully!
	2017-09-02T09:47:38.568188> Sending SMS done
	2017-09-02T09:47:38.579110> python CloudSMS.py "TC/22.5" "1,16,6,2,9,6,-57" "125,5,12" "2017-09-02T09:47:19.959" "00000027EBBEDA21"
	2017-09-02T09:47:38.579381> --> cloud end

However, currently, **it is not possible to have Internet connectivity and SMS support at the same time**. Therefore, if you want to enable the LORANGA SMS feature at boot time, run:

	> ./enable-loranga-SMS-on-boot.sh
	
This simple script simply creates a file called `use_loranga_SMS_on_boot.txt`. The `start_gw.sh` script, called at boot time, now also checks for this file and will power on the modem before launching the gateway program and post-processing stage. You can disable the usage of Loranga SMS on boot with:

	> ./disable-loranga-SMS-on-boot.sh
	
which simply deletes the `use_loranga_SMS_on_boot.txt` file. Note that if you have both `use_loranga_Internet_on_boot.txt` and `use_loranga_SMS_on_boot.txt` then the priority is the Internet connection. Remember to use `disable-loranga-internet-on-boot.sh` if you want only the SMS features.	

Raspberry PI Zero
-----------------
	
If you use the latest image, it also works on the new Raspberry Zero W (with the default behavior to act as a WiFi access point so you can easily use `ssh pi@192.168.200.1` once you are connected to the gateway's WiFi). It can therefore be used to build and even lower-cost and more compact LoRa gateway. Note that regarding the serial port, the Raspberry PI Zero W is similar to the Raspberry PI 3, so `/dev/ttyS0` is used, and not `/dev/ttyAMA0`.		

Enjoy,
C. Pham
		