Support for AES encryption and basic LoRaWAN with the low-cost LoRa platform
============================================================================

Decription 
----------

This module will add AES encryption as well as limited LoRaWAN support for both end-devices and post-processing at gateway level:

- end-device can use LoRaWAN-like encryption (AES-CTR) to encrypt the payload, but still using non-LoRaWAN packet format. This case is referred to as encapsulated LoRaWAN packet. The packet can be received natively by our low-cost gateway (see our packet header format in [section 3 of the low-cost gateway web site](http://cpham.perso.univ-pau.fr/LORA/RPIgateway.html)).
- end-device can additionally decide to forge a simple LoRaWAN packet with encrypted payload and MIC computation as described by LoRaWAN specification. This case is referred to as native LoRaWAN packet. The packet can be received by our low-cost gateway (see below) as well as by a LoRaWAN gateway.
- in both cases, both AppSKey and NwkSKey need to be known by the end-device, therefore the Activation by Personalization (ABP) approach is used here. 
- post_processing_gw.py is able to locally check integrity and decrypt the encryted payload or/and the LoRaWAN packet if both NwkSKey and AppSKey are also known at the gateway's post-processing level. post_processing_gw.py uses loraWAN.py script for that purpose. loraWAN.py stores both NwkSKey and AppSKey.
- in case these keys are not stored on the gateway, post_processing_gw.py can upload the encrypted data (actually the whole LoRaWAN frame with necessary information for decryption provided that AppSKey and NwkSKey are known) on an Internet cloud like a traditional LoRaWAN gateway would do. We provide 2 examples using Firebase, CloudFireBaseAES.py and CloudFireBaseLWAES.py, to store the encryted frame.
- CloudFireBaseAES.py uploads an encapsulated encrypted LoRaWAN packet.
- CloudFireBaseLWAES.py uploads a native encrypted LoRaWAN packet.
- clouds.json has 2 additional cloud sections, "encrypted_clouds" and "lorawan_encrypted_clouds", to indicate scripts that will be called to decrypt respectively encapsulated and native LoRaWAN packets. In the provided examples, "python CloudFireBaseAES.py" will be used for "encrypted_clouds" and "python CloudFireBaseLWAES.py" will be used for "lorawan_encrypted_clouds".
- the LoRaWAN encryption/decryption and MIC integrity functionalities at the gateway (post_processing_gw.py) are provided by the LoRaWAN python library by Jeroen Nijhof. See "How to update your gateway" section.
- the LoRaWAN encryption and packet forging procedure use the AES library and examples provided by Gerben den Hartog at https://github.com/Ideetron/RFM95W_Nexus/tree/master/LoRaWAN_V31. Arduino_LoRa_temp.ino has been updated with these encryption and simple LoRaWAN capabilities. To enable payload encryption, uncomment #define WITH_AES. To further use only LoRaWAN packet format uncomment #define LORAWAN.
- the lora_gateway program can provide raw data to post_processing_gw.py by using the --raw option (start_gw.py will use --raw for the lora_gateway program if ["gateway_conf"]["raw"] is true). This is required to handle native LoRaWAN packets. 
- post_processing_gw.py can also detect, decode and handle LoRaWAN packet with the ["gateway_conf"]["raw"] option.
- you can use raw format for both lora_gateway and post_processing_gw.py and still be able to use the non-LoRaWAN mode because post_processing_gw.py can distinguish between non-LoRaWAN packet format and LoRaWAN packet format. Therefore if you have mixed scenarios you can use the ["gateway_conf"]["raw"] to be able to handle both cases: encapsulated and native LoRaWAN packet.

**Important**: when end-device is using encryption (#define WITH_AES) we use the LoRaWAN packet format and encryption procedure because we simply want to reuse the LoRaWAN python library for decryption. Therefore the 4-byte appkey is not used. Besides, with encryption this simple packet filtering mechanism becomes obsolete as MIC checking is better.

How to update your gateway
==========================
	
You may need the python-crypto package

	> sudo apt-get install python-crypto	
	
You can also get the LoRaWAN python package from https://github.com/jeroennijhof/LoRaWAN. Get the .zip and copy the content into /usr/lib/python2.7/dist-packages:

	> unzip LoRaWAN-master.zip
	> sudo cp -r LoRaWAN-master /usr/lib/python2.7/dist-packages/LoRaWAN
	
Example 1: using output from real sensor to test LoRaWAN packet format and encryption
=====================================================================================

Here we applied full LoRaWAN encrytion at the end-device (using AES lib from Gerben den Hartog from Ideetron). The procedure is then to encrypt the payload wih AppSKey, then put encrypted payload in a LoRaWAN frame and calculate 4-byte MIC on the whole frame with NwkSKey. We use:

	unsigned char AppSkey[16] = {
	  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
	  0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
	};
	
	unsigned char NwkSkey[16] = {
	  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
	  0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
	};
	
and DevAddr is:

	unsigned char DevAddr[4] = {
	  0x02, 0x01, 0x05, 0x04
	};		

We use the AES-CTR encrypting mode of LoRaWAN with FCnt=0 and Direction=0 (uplink). The example uses the Arduino_LoRa_temp that is an extended version of Arduino_LoRa_Simple_temp updated with AES and LoRaWAN capabilities.

	LoRa temperature sensor
	Teensy31/32 detected
	SX1276 detected, starting
	SX1276 LF/HF calibration
	...
	Get back previous sx1272 config
	Using packet sequence number of 77
	Setting Mode: state 0
	Setting Channel: state 0
	Setting Power: state 0
	Setting node addr: state 0
	SX1272 successfully configured
	Reading 233
	(Temp is 75.8
	Sending \!#3#TC/75.8
	Real payload size is 12
	[AppKey][r_size]\!#3#TC/75.8
	plain payload hex
	5C 21 23 33 23 54 43 2F 37 35 2E 38	
	Encrypting
	encrypted payload
	E4 85 FD 1F 77 91 B9 D9 34 FF 1F F9 
	calculate MIC with NwkSKey
	transmitted LoRaWAN-like packet:
	MHDR[1] | DevAddr[4] | FCtrl[1] | FCnt[2] | FPort[1] | EncryptedPayload | MIC[4]
	40 04 05 01 02 00 00 00 01 E4 85 FD 1F 77 91 B9 D9 34 FF 1F F9 A8 BC 3D 7E 
	--> CAD duration 547
	OK1
	--> waiting for 1 CAD = 62
	--> CAD duration 547
	OK2
	--> RSSI -126
	LoRa pkt seq 77
	LoRa Sent in 1897
	LoRa Sent w/CAD in 9255
	Packet sent, state 0	

decryption test with an interactive python shell
------------------------------------------------

In this example, we have AppSKey, NwkSKey and packet payload stored as string therefore we have additional steps. For instance:

	>>> AppSKey = '2B7E151628AED2A6ABF7158809CF4F3C'
	>>> appskey=bytearray.fromhex(AppSKey)
	>>> appskey
	bytearray(b'+~\x15\x16(\xae\xd2\xa6\xab\xf7\x15\x88\t\xcfO<')
	>>> appskeylist=[]
	>>> for i in range (0,len(appskey)):
	...     appskeylist.append(appskey[i])
	... 
	>>> appskeylist
	[43, 126, 21, 22, 40, 174, 210, 166, 171, 247, 21, 136, 9, 207, 79, 60]
	
Actually, the bytearray step is sufficient for the encryption keys. For the packet payload, you really need to get it into a list. If you already have them in list such as:

	>>> appskeylist = [0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c]

then you are ready to use them without the additional steps. 

	>>> import LoRaWAN
	>>> from LoRaWAN.MHDR import MHDR
	>>> AppSKey = '2B7E151628AED2A6ABF7158809CF4F3C'
	>>> appskey=bytearray.fromhex(AppSKey)
	>>> appskey
	bytearray(b'+~\x15\x16(\xae\xd2\xa6\xab\xf7\x15\x88\t\xcfO<')
	>>> appskeylist=[]
	>>> for i in range (0,len(appskey)):
	...     appskeylist.append(appskey[i])
	... 
	>>> appskeylist
	[43, 126, 21, 22, 40, 174, 210, 166, 171, 247, 21, 136, 9, 207, 79, 60]
	>>> NwkSKey = '2B7E151628AED2A6ABF7158809CF4F3C'
	>>> nwkskey=bytearray.fromhex(NwkSKey)
	>>> nwkskey
	bytearray(b'+~\x15\x16(\xae\xd2\xa6\xab\xf7\x15\x88\t\xcfO<')
	>>> nwkskeylist=[]
	>>> for i in range (0,len(nwkskey)):
	...     nwkskeylist.append(netskey[i])
	... 
	>>> nwkskeylist
	[43, 126, 21, 22, 40, 174, 210, 166, 171, 247, 21, 136, 9, 207, 79, 60]
	>>> PKT='40 04 05 01 02 00 00 00 01 E4 85 FD 1F 77 91 B9 D9 34 FF 1F F9 A8 BC 3D 7E'
	>>> pkt=bytearray.fromhex(PKT)
	>>> pkt
	bytearray(b'@\x04\x05\x01\x02\x00\x00\x00\x01\xe4\x85\xfd\x1fw\x91\xb9\xd94\xff\x1f\xf9\xa8\xbc=~')
	>>> pktlist=[]
	>>> for i in range (0,len(pkt)):
	...     pktlist.append(pkt[i])
	... 
	>>> pktlist
	[64, 4, 5, 1, 2, 0, 0, 0, 1, 228, 133, 253, 31, 119, 145, 185, 217, 52, 255, 31, 249, 168, 188, 61, 126]
	>>> lorawan = LoRaWAN.new(nwkskeylist)
	>>> lorawan.read(pktlist)
	>>> print lorawan.get_mhdr().get_mtype()
	64
	>>> print lorawan.get_mic()
	[168, 188, 61, 126]
	>>> print lorawan.compute_mic()
	[168, 188, 61, 126]
	>>> print lorawan.valid_mic()
	True
	>>> lorawan = LoRaWAN.new(appskeylist)
	>>> lorawan.read(pktlist)	
	>>> print bytearray(lorawan.get_payload())
	\!#3#TC/75.8
	
Example 2: post-processing_gw.py handling a fake LoRaWAN packet for local decryption and processing
===================================================================================================

post_processing_gw.py will be run with the --raw option to accept packets in raw format and --aes to locally decrypt. post_processing_gw.py will then detect a LoRaWAN packet format and starts the packet processing. Here, we will show the result of a test using test-loraWAN-2.py that emulates an output from the low-level lora_gateway program.

test-loraWAN-2.py:

	import sys
	print "^p0,0,0,0,26,8,-45"
	print "^r125,5,12"
	print "++\x40\x06\x00\x00\x00\x00\x00\x00\x01\x4F\x29\xE7\x49\x77\xA1\xC2\xE7\x81\xE6\x16\xA6\x68\xFD\x75\x68\x74"
	sys.stdout.flush()
	
Then we issued the following command:

	> python test-loraWAN-2.py | python post_processing_gw.py --raw --aes
	
Output is shown below:

	Parsing cloud declarations
	[u'python CloudThingSpeak.py']
	Parsed all cloud declarations
	post_processing_gw.py got cloud list: 
	[u'python CloudThingSpeak.py']
	raw output from gateway. post_processing_gw will handle packet format
	enable local AES decryption
	
	Current working directory: /home/pi/lora_gateway/test
	2016-10-19T22:09:58.750002
	rcv ctrl pkt info (^p): 0,0,0,0,26,8,-45
	splitted in:  [0, 0, 0, 0, 26, 8, -45]
	rawFormat(len=25 SNR=8 RSSI=-45)
	rcv ctrl radio info (^r): 125,5,12
	splitted in:  [125, 5, 12]
	(BW=125 CR=5 SF=12)
	got first framing byte
	--> got data prefix
	raw format from gateway
	LoRaWAN?
	loraWAN: valid MIC
	loraWAN: plain payload is \!#3#TC/22.50
	plain payload is : \!#3#TC/22.50
	valid app key: accept data
	number of enabled clouds is 1
	--> cloud[0]
	uploading with python CloudThingSpeak.py
	ThingSpeak: uploading
	rcv msg to log (\!) on ThingSpeak ( default , 3 ): 22.50
	ThingSpeak: will issue curl cmd
	curl -s -k -X POST --data field3=22.50&field7=0 https://api.thingspeak.com/update?key=ORE0DIFZIIPT61DO
	ThingSpeak: returned code from server is 201
	--> cloud end

When a LoRaWAN packet is received, post_processing_gw.py will convert the raw packet content into a list of bytes, lorapkt, and call loraWAN_process_pkt:

	from loraWAN import loraWAN_process_pkt
	plain_payload=loraWAN_process_pkt(lorapkt)
	
loraWAN_process_pkt() is provided by loraWAN.py:

	import LoRaWAN
	from LoRaWAN.MHDR import MHDR
	
	AppSKey = '2B7E151628AED2A6ABF7158809CF4F3C'
	NwkSKey = '2B7E151628AED2A6ABF7158809CF4F3C'
	
	def loraWAN_process_pkt(lorapkt):
	
		appskey=bytearray.fromhex(AppSKey)
		appskeylist=[]
	
		for i in range (0,len(appskey)):
			appskeylist.append(appskey[i])
	
		nwkskey=bytearray.fromhex(NwkSKey)
		nwkskeylist=[]
		for i in range (0,len(nwkskey)):
			nwkskeylist.append(nwkskey[i])
	
		lorawan = LoRaWAN.new(nwkskeylist)
		lorawan.read(lorapkt)
		lorawan.compute_mic()
		if lorawan.valid_mic():
			print "loraWAN: valid MIC"
			lorawan = LoRaWAN.new(appskeylist)
			lorawan.read(lorapkt)	
			plain_payload = ''.join(chr(x) for x in lorawan.get_payload())
			print "loraWAN: plain payload is "+plain_payload
			return plain_payload
		else:
			return "###BADMIC###"	
	
If post_processing_gw.py cannot decrypt the payload because encryption keys are not available then you can use a json cloud (such as Firebase) to store the encrypted payload in base64 format. Note that NwkSkey is not available, then MIC computation can not be performed therefore you are not sure at the gateway level that the encrypted payload comes from the right sensor or not. LoRaWAN gateways have the same issue as NwkSKey is stored in LoRaWAN network server.

Example 3: full example with real sensor and the low-cost gateway: no native LoRaWAN format
===========================================================================================

The end-device will use LoRaWAN packet format only to ease the encryption procedure. In the Arduino_LoRa_temp example, only #define WITH_AES is uncommented. Therefore both lora_gateway and post_processing_gw.py run in normal mode (with packet header) to process an encapsulated encrypted LoRaWAN-like packet. 

We use the following settings for our temperature sensor:

	unsigned char AppSkey[16] = {
	  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
	  0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
	};
	
	unsigned char MwkSkey[16] = {
	  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
	  0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
	};
	
and DevAddr is:

	unsigned char DevAddr[4] = {
	  0x00, 0x00, 0x00, 0x06
	};	


Sensor output
-------------

	LoRa temperature sensor
	Teensy31/32 detected
	SX1276 detected, starting
	SX1276 LF/HF calibration
	...
	Get back previous sx1272 config
	Using packet sequence number of 104
	Setting Mode: state 0
	Setting Channel: state 0
	Setting Power: state 0
	Setting node addr: state 0
	SX1272 successfully configured
	Reading 70
	(Temp is 22.55
	Sending \!#3#TC/22.50
	Real payload size is 13
	\!#3#TC/22.50
	plain payload hex
	5C 21 23 33 23 54 43 2F 32 32 2E 35 30 
	Encrypting
	encrypted payload
	4F 29 E7 49 77 A1 C2 E7 81 E6 16 A6 68 
	calculate MIC with NwkSKey
	transmitted LoRaWAN-like packet:
	MHDR[1] | DevAddr[4] | FCtrl[1] | FCnt[2] | FPort[1] | EncryptedPayload | MIC[4]
	40 06 00 00 00 00 00 00 01 4F 29 E7 49 77 A1 C2 E7 81 E6 16 A6 68 FD 75 68 74 
	end-device uses encapsulated LoRaWAN packet format only for encryption
	--> CAD duration 547
	OK1
	--> waiting for 1 CAD = 62
	--> CAD duration 547
	OK2
	--> RSSI -124
	LoRa pkt size 26
	LoRa pkt seq 104
	LoRa Sent in 1897
	LoRa Sent w/CAD in 9255
	Packet sent, state 0

Gateway output
--------------

The gateway is started with:

	> sudo ./lora_gateway --mode 1 | python post_processing_gw.py --aes
	
Note that both lora_gateway and post_processing_gw.py are in normal mode. post_processing_gw.py uses local AES decryption.

	Parsing cloud declarations
	[u'python CloudThingSpeak.py']
	Parsed all cloud declarations
	post_processing_gw.py got cloud list: 
	[u'python CloudThingSpeak.py']
	enable local AES decryption
		
	Current working directory: /home/pi/lora_gateway
	SX1276 detected, starting.
	SX1276 LF/HF calibration
	...
	**********Power ON: state 0
	Default sync word: 0x12
	LoRa mode 1
	Setting mode: state 0
	Channel CH_10_868: state 0
	Set LoRa Power to M
	Power: state 0
	Get Preamble Length: state 0
	Preamble Length: 8
	LoRa addr 1: state 0
	SX1272/76 configured as LR-BS. Waiting RF input for transparent RF-serial bridge
	--- rxlora. dst=1 type=0x14 src=6 seq=104 len=26 SNR=-16 RSSIpkt=-111 BW=125 CR=4/5 SF=12
	2016-10-20T12:09:25.282381
	rcv ctrl pkt info (^p): 1,20,6,104,26,-16,-111
	splitted in:  [1, 20, 6, 104, 26, -16, -111]
	(dst=1 type=0x14(DATA ENCRYPTED) src=6 seq=104 len=26 SNR=-16 RSSI=-111)
	rcv ctrl radio info (^r): 125,5,12
	splitted in:  [125, 5, 12]
	(BW=125 CR=5 SF=12)
	rcv timestamp (^t): 2016-10-20T12:34:22.419
	
	got first framing byte
	--> got data prefix
	--> DATA encrypted: encrypted payload size is 26
	--> decrypting in AES-CTR mode (LoRaWAN version)
	loraWAN: valid MIC
	loraWAN: plain payload is \!#3#TC/22.50
	plain payload is : \!#3#TC/22.50
	valid app key: accept data
	number of enabled clouds is 1
	--> cloud[0]
	uploading with python CloudThingSpeak.py
	ThingSpeak: uploading
	rcv msg to log (\!) on ThingSpeak ( default , 3 ): 22.50
	ThingSpeak: will issue curl cmd
	curl -s -k -X POST --data field3=22.50&field7=104 https://api.thingspeak.com/update?key=ORE0DIFZIIPT61DO
	ThingSpeak: returned code from server is 210
	--> cloud end


Example 4: full example with real sensor and low-cost gateway: native LoRaWAN format
====================================================================================

The end-device will use LoRaWAN packet format (no additional header). In the Arduino_LoRa_temp example, both #define WITH_AES and #define LORAWAN are uncommented. Therefore both lora_gateway and post_processing_gw.py are in raw mode format to transparently process native LoRaWAN packet.

We use the following settings for our temperature sensor:

	unsigned char AppSkey[16] = {
	  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
	  0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
	};
	
	unsigned char MwkSkey[16] = {
	  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
	  0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
	};
	
and DevAddr is:

	unsigned char DevAddr[4] = {
	  0x00, 0x00, 0x00, 0x06
	};	

Sensor output
-------------

	LoRa temperature sensor
	Teensy31/32 detected
	SX1276 detected, starting
	SX1276 LF/HF calibration
	...
	Get back previous sx1272 config
	Using packet sequence number of 100
	Setting Mode: state 0
	Setting Channel: state 0
	Setting Power: state 0
	Setting node addr: state 0
	SX1272 successfully configured
	Reading 70
	(Temp is 22.55
	Sending \!#3#TC/22.50
	Real payload size is 13
	\!#3#TC/22.50
	plain payload hex
	5C 21 23 33 23 54 43 2F 32 32 2E 35 30 
	Encrypting
	encrypted payload
	4F 29 E7 49 77 A1 C2 E7 81 E6 16 A6 68 
	calculate MIC with NwkSKey
	transmitted LoRaWAN-like packet:
	MHDR[1] | DevAddr[4] | FCtrl[1] | FCnt[2] | FPort[1] | EncryptedPayload | MIC[4]
	40 06 00 00 00 00 00 00 01 4F 29 E7 49 77 A1 C2 E7 81 E6 16 A6 68 FD 75 68 74 
	end-device uses native LoRaWAN packet format
	--> CAD duration 547
	OK1
	--> waiting for 1 CAD = 62
	--> CAD duration 547
	OK2
	--> RSSI -126
	LoRa pkt size 26
	LoRa pkt seq 100
	LoRa Sent in 1733
	LoRa Sent w/CAD in 9091
	Packet sent, state 0
	
Gateway output
--------------

The gateway is started with:

	> sudo ./lora_gateway --mode 1 --raw | python post_processing_gw.py --raw --aes
	
Note that both lora_gateway and post_processing_gw.py are in raw mode format to transparently process native LoRaWAN packet. Again, post_processing_gw.py uses local AES decryption.

	Parsing cloud declarations
	[u'python CloudThingSpeak.py']
	Parsed all cloud declarations
	post_processing_gw.py got cloud list: 
	[u'python CloudThingSpeak.py']
	raw output from gateway. post_processing_gw will handle packet format
	enable local AES decryption
		
	Current working directory: /home/pi/lora_gateway
	SX1276 detected, starting.
	SX1276 LF/HF calibration
	...
	**********Power ON: state 0
	Default sync word: 0x12
	LoRa mode 1
	Setting mode: state 0
	Channel CH_10_868: state 0
	Set LoRa Power to M
	Power: state 0
	Get Preamble Length: state 0
	Preamble Length: 8
	LoRa addr 1: state 0
	Raw format, not assuming any header in reception
	SX1272/76 configured as LR-BS. Waiting RF input for transparent RF-serial bridge
	--- rxlora. dst=0 type=0x00 src=0 seq=0 len=26 SNR=-16 RSSIpkt=-110 BW=125 CR=4/5 SF=12
	2016-10-20T12:34:22.420292
	rcv ctrl pkt info (^p): 0,0,0,0,26,-16,-110
	splitted in:  [0, 0, 0, 0, 26, -16, -110]
	rawFormat(len=26 SNR=-16 RSSI=-110)
	rcv ctrl radio info (^r): 125,5,12
	splitted in:  [125, 5, 12]
	(BW=125 CR=5 SF=12)
	rcv timestamp (^t): 2016-10-20T12:09:25.281
	
	got first framing byte
	--> got data prefix
	raw format from gateway
	LoRaWAN?
	loraWAN: valid MIC
	loraWAN: plain payload is \!#3#TC/22.50
	plain payload is : \!#3#TC/22.50
	valid app key: accept data
	number of enabled clouds is 1
	--> cloud[0]
	uploading with python CloudThingSpeak.py
	ThingSpeak: uploading
	rcv msg to log (\!) on ThingSpeak ( default , 3 ): 22.50
	ThingSpeak: will issue curl cmd
	curl -s -k -X POST --data field3=22.50&field7=0 https://api.thingspeak.com/update?key=ORE0DIFZIIPT61DO
	ThingSpeak: returned code from server is 210
	--> cloud end

Example 5: full example with LoRaWAN mDot and low-cost gateway: native LoRaWAN format
=====================================================================================

The gateway is launched as follows:
	
	> sudo ./lora_gateway --mode 1 --raw --sw 34 --freq 868.1| python post_processing_gw.py --raw --aes
	
which additionally sets the sync word to 0x34 for LoRaWAN and the frequency to one the pre-defined LoRaWAN uplink frequency (i.e. 868.1MHz).	

	
Gateway output
--------------

	Parsing cloud declarations
	Parsed all cloud declarations
	post_processing_gw.py got cloud list: 
	[]
	raw output from gateway. post_processing_gw will handle packet format
	enable local AES decryption
	
	Current working directory: /home/pi/lora_gateway
	SX1276 detected, starting.
	SX1276 LF/HF calibration
	...
	**********Power ON: state 0
	Default sync word: 0x12
	Set sync word to 0x34
	LoRa sync word: state 0
	LoRa mode 1
	Setting mode: state 0
	Frequency 868.100000: state 0
	Set LoRa Power to M
	Power: state 0
	Get Preamble Length: state 0
	Preamble Length: 8
	LoRa addr 1: state 0
	Raw format, not assuming any header in reception
	SX1272/76 configured as LR-BS. Waiting RF input for transparent RF-serial bridge

mDot configuration
------------------
	
The mDot is a LoRaWAN radio module from MultiTech. The mdot will be configured by ABP, with the AppSKey and NwkSKey matching those declared in loraWAN.py. Here is the configuration commands issued with a serial tool. Baud rate is 115200. Lines with # are comment lines that were added for clarity purposes.

	# Network Joining mode : manual
	AT+NJM=0
	OK
	
	# Network address (devAddr in LoraMac) (4 bytes)
	AT+NA=0000A58F
	OK
	
	# Data session encryption key (16 bytes)
	AT+DSK=2B7E151628AED2A6ABF7158809CF4F3C
	OK
	
	# Network session encryption key (16 bytes)
	AT+NSK=2B7E151628AED2A6ABF7158809CF4F3C
	OK
	 
	# Set the Tx data rate for all channels : spread factor 12
	AT+TXDR=12
	OK
	
	# Set the Tx power for all channels : 14dB
	AT+TXP=14
	OK
	# Save changes
	AT&W
	OK
	
	# Restart
	ATZ
	OK
	#	
	
	# no ACK
	AT+ACK=0
	OK
	
	Check that the settings are correct
	AT&V

	Device ID:				00:80:00:00:00:00:a5:8f
	Frequency Band:			FB_868
	Frequency Sub Band:		7
	Public Network:			on
	Start Up Mode:			COMMAND
	Network Address:		0000a58f
	Network ID:       
	Network ID Passphrase:   
	Network Key:       
	Network Key Passphrase:     
	Network Session Key:	2b.7e.15.16.28.ae.d2.a6.ab.f7.15.88.09.cf.4f.3c
	Data Session Key:		2b.7e.15.16.28.ae.d2.a6.ab.f7.15.88.09.cf.4f.3c
	Network Join Mode:		MANUAL
	Network Join Retries:	2
	Join Byte Order:		LSB
	Link Check Threshold:	off
	Link Check Count:		off
	Error Correction:		1 bytes
	ACK Retries:			off
	Encryption:				on
	CRC:					on
	Adaptive Data Rate:		off
	Command Echo:			on
	Verbose Response:		off
	Tx Frequency:			0
	Tx Data Rate:			SF_12
	Tx Power:				14
	Tx Wait:				on
	Tx Inverted Signal:		off
	Rx Frequency:			869525000
	Rx Data Rate:			SF_9
	Rx Inverted Signal:		on
	Rx Output Style:		HEXADECIMAL
	Debug Baud Rate:		115200
	Serial Baud Rate:		115200
	Wake Mode:				INTERVAL
	Wake Interval:			10 s
	Wake Delay:				100 ms
	Wake Timeout:			20 ms
	Log Level:				0 	

Sending with mDot
-----------------

	AT+SEND=HelloWorld 
	OK	
	
	AT+SEND=HelloWorld_1 
	OK	

Gateway output, continued
-------------------------

	...
	--- rxlora. dst=0 type=0x00 src=0 seq=0 len=23 SNR=7 RSSIpkt=-68 BW=125 CR=4/5 SF=12
	2016-10-24T11:13:51.119549
	rcv ctrl pkt info (^p): 0,0,0,0,23,7,-68
	splitted in:  [0, 0, 0, 0, 23, 7, -68]
	rawFormat(len=23 SNR=7 RSSI=-68)
	rcv ctrl radio info (^r): 125,5,12
	splitted in:  [125, 5, 12]
	(BW=125 CR=5 SF=12)
	rcv timestamp (^t): 2016-10-24T11:13:51.118

	got first framing byte
	--> got data prefix
	raw format from gateway
	LoRaWAN?
	loraWAN: valid MIC
	loraWAN: plain payload is HelloWorld
	plain payload is : HelloWorld
	HelloWorld
	--- rxlora. dst=0 type=0x00 src=0 seq=0 len=25 SNR=6 RSSIpkt=-80 BW=125 CR=4/5 SF=12
	2016-10-24T11:30:34.333976
	rcv ctrl pkt info (^p): 0,0,0,0,25,6,-80
	splitted in:  [0, 0, 0, 0, 25, 6, -80]
	rawFormat(len=25 SNR=6 RSSI=-80)
	rcv ctrl radio info (^r): 125,5,12
	splitted in:  [125, 5, 12]
	(BW=125 CR=5 SF=12)
	rcv timestamp (^t): 2016-10-24T11:30:34.333

	got first framing byte
	--> got data prefix
	raw format from gateway
	LoRaWAN?
	loraWAN: valid MIC
	loraWAN: plain payload is HelloWorld_1
	plain payload is : HelloWorld_1
	HelloWorld_1

Example 6: use CloudFireBaseLWAES.py to upload a native encrypted LoRaWAN packet
=================================================================================

clouds.json contains the following section:

	"lorawan_encrypted_clouds" : [
		{
			"name":"Firebase cloud",
			"script":"python CloudFireBaseLWAES.py",
			"type":"iotcloud",			
			"write_key":"",
			"enabled":true			
		}
	]
	
post_processing_gw.py will not decrypt locally the encrypted packet (no --aes option). This is the case when NwkSKey and AppSKey are not available on the gateway. post_processing_gw.py will call "python CloudFireBaseLWAES.py" with appropriate parameters. See the new cloud management design [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_advanced/new_cloud_design/README-NewCloud.md).

	> python test-loraWAN-2.py | python post_processing_gw.py --raw	
	
output
------

	Parsing cloud declarations
	[u'python CloudThingSpeak.py']
	Parsed all cloud declarations
	post_processing_gw.py got cloud list: 
	[u'python CloudThingSpeak.py']
	Parsing cloud declarations
	[u'python CloudFireBaseAES.py']
	Parsed all cloud declarations
	post_processing_gw.py got encrypted cloud list: 
	[u'python CloudFireBaseAES.py']
	Parsing cloud declarations
	[u'python CloudFireBaseLWAES.py']
	Parsed all cloud declarations
	post_processing_gw.py got LoRaWAN encrypted cloud list: 
	[u'python CloudFireBaseLWAES.py']
	raw output from gateway. post_processing_gw will handle packet format
	
	Current working directory: /home/pi/lora_gateway/aes_lorawan
	2016-10-26T22:14:12.985467
	rcv ctrl pkt info (^p): 0,0,0,0,26,8,-45
	splitted in:  [0, 0, 0, 0, 26, 8, -45]
	rawFormat(len=26 SNR=8 RSSI=-45)
	rcv ctrl radio info (^r): 125,5,12
	splitted in:  [125, 5, 12]
	(BW=125 CR=5 SF=12)
	got first framing byte
	--> got data prefix
	raw format from gateway
	LoRaWAN?
	--> DATA encrypted: local aes not activated
	--> FYI base64 of LoRaWAN frame w/MIC: QAYAAAAAAAABTynnSXehwueB5hamaP11aHQ=
	--> number of enabled clouds is 1
	--> LoRaWAN encrypted cloud[0]
	uploading with python CloudFireBaseLWAES.py
	FireBase: uploading
	Firebase: upload success
	--> LoRaWAN encrypted cloud end
	
on Firebase
-----------

	sensor0x00000006
		msg0x0000
			FCnt: "0x0000"
			FPort: "0x01"
			MIC: "0xfd756874"
			cr: 5
			data_b64: "TynnSXehwueB5hamaA=="
			datarate: "SF12BW125"
			devAddr: "0x00000006"
			frame_b64: "QAYAAAAAAAABTynnSXehwueB5hamaP11aHQ="
			gateway_eui: "000000XXXXXXXXXX"
			len: 26
			pdata:"0,0,0,0,26,8,-45"			
			ptype: "0x40"
			ptypestr: "unconfirmed data up"
			rdata:"125,5,12"			
			rssi: -45
			snr: 8
			time: "2016-10-26T20:14:13.110056"		

A LoRaWAN packet has the following format:

	MHDR[1] | DevAddr[4] | FCtrl[1] | FCnt[2] | FPort[1] | EncryptedPayload | MIC[4]
		
Here, in the native LoRaWAN format, only LoRaWAN packet fields are stored (FCnt, FPort, MIC and devAddr) as it is a native LoRaWAN packet without additional header.

test-loraWAN-2.py injects the following LoRaWAN packet:

	| \x40 | \x06\x00\x00\x00 | \x00 | \x00\x00 | \x01 | \x4F\x29\xE7\x49\x77\xA1\xC2\xE7\x81\xE6\x16\xA6\x68 | \xFD\x75\x68\x74 |
	^ start of LoRaWAN packet						   ^ start of LoRaWAN payload--------------------------->

without any additional packet header.

Example 7: use CloudFireBaseAES.py to upload an encapsulated encrypted LoRaWAN packet
=====================================================================================

clouds.json contains the following section:

	"encrypted_clouds" : [
		{
			"name":"Firebase cloud",
			"script":"python CloudFireBaseAES.py",
			"type":"iotcloud",			
			"write_key":"",
			"enabled":true			
		}
	]

post_processing_gw.py will not decrypt locally the encrypted packet (no --aes option). This is the case when NwkSKey and AppSKey are not available on the gateway. post_processing_gw.py will call "python CloudFireBaseAES.py" with appropriate parameters. See the new cloud management design [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_advanced/new_cloud_design/README-NewCloud.md).

	> python test-loraWAN-3.py | python post_processing_gw.py --raw
	
output
------

	Parsing cloud declarations
	[u'python CloudThingSpeak.py']
	Parsed all cloud declarations
	post_processing_gw.py got cloud list: 
	[u'python CloudThingSpeak.py']
	Parsing cloud declarations
	[u'python CloudFireBaseAES.py']
	Parsed all cloud declarations
	post_processing_gw.py got encrypted cloud list: 
	[u'python CloudFireBaseAES.py']
	Parsing cloud declarations
	[u'python CloudFireBaseLWAES.py']
	Parsed all cloud declarations
	post_processing_gw.py got LoRaWAN encrypted cloud list: 
	[u'python CloudFireBaseLWAES.py']
	raw output from gateway. post_processing_gw will handle packet format
	
	Current working directory: /home/pi/lora_gateway/aes_lorawan
	2016-10-26T22:02:22.590531
	rcv ctrl pkt info (^p): 1,20,6,0,26,8,-45
	splitted in:  [1, 20, 6, 0, 26, 8, -45]
	rawFormat(len=26 SNR=8 RSSI=-45)
	rcv ctrl radio info (^r): 125,5,12
	splitted in:  [125, 5, 12]
	(BW=125 CR=5 SF=12)
	got first framing byte
	--> got data prefix
	raw format from gateway
	Header[dst=1 ptype=0x14 src=6 seq=0]
	--> DATA encrypted: encrypted payload size is 26
	--> DATA encrypted: local aes not activated
	--> FYI base64 of LoRaWAN frame w/MIC: QAYAAAAAAAABTynnSXehwueB5hamaP11aHQ=
	--> number of enabled clouds is 1
	--> encrypted cloud[0]
	uploading with python CloudFireBaseAES.py
	FireBase: uploading
	Firebase: upload success
	--> encrypted cloud end
	
on Firebase
-----------

	sensor6
		msg0
			FCnt: "0x0000"
			MIC: "0xfd756874"
			cr: 5
			data_b64: "TynnSXehwueB5hamaA=="
			datarate: "SF12BW125"
			devAddr: "0x00000006"
			frame_b64: "QAYAAAAAAAABTynnSXehwueB5hamaP11aHQ="
			gateway_eui: "000000XXXXXXXXXX"
			len: 26
			pdata:"1,20,6,0,26,8,-45"
			ptype: "0x14"
			ptypestr: "DATA ENCRYPTED"
			rdata:"125,5,12"
			rssi: -45
			seq: 0
			snr: 8
			src: 6
			time: "2016-10-26T20:02:22.718648"		

A LoRaWAN packet has the following format:

	MHDR[1] | DevAddr[4] | FCtrl[1] | FCnt[2] | FPort[1] | EncryptedPayload | MIC[4]
	
The encapsulated LoRaWAN format means that the LoRaWAN packet is considered as a payload in our packet format which adds a 4-byte header (see our packet header format in [section 3 of the low-cost gateway web site](http://cpham.perso.univ-pau.fr/LORA/RPIgateway.html)):

	dst[1] | ptype[1] | src[1] | seq[1] | payload	
	
On Firebase, LoRaWAN specific fields that are implied in the AES encrypting/integrity process are stored (FCnt, MIC and devAddr) as well as our packet format header fields (ptype, src and seq). The packet has our defined packet type which is 0x14 for "DATA ENCRYPTED". Note that sequence number can be different than FCnt.

test-loraWAN-3.py injects the following LoRaWAN packet:

	| \x40 | \x06\x00\x00\x00 | \x00 | \x00\x00 | \x01 | \x4F\x29\xE7\x49\x77\xA1\xC2\xE7\x81\xE6\x16\xA6\x68 | \xFD\x75\x68\x74 |
	^ start of LoRaWAN packet						   ^ start of LoRaWAN payload--------------------------->
	
which is encapsulated with our packet format to give:

	| \x01 | \x14 | \x06 | \x00 | \x40 | \x06\x00\x00\x00 | \x00 | \x00\x00 | \x01 | \x4F\x29\xE7\x49\x77\xA1\xC2\xE7\x81\xE6\x16\xA6\x68 | \xFD\x75\x68\x74 |
								^ start of LoRaWAN packet						   ^ start of LoRaWAN payload--------------------------->
								
frame_b64 contains the encrypted LoRaWAN packet in base64 format while data_b64 only contains the encryted LoRaWAN payload in base64 format.

Example 8: use loraWAN.py to decrypt both encapsulated and native encryted LoRaWAN packet
=========================================================================================

Once the LoRaWAN packet is stored in base64 format (frame_b64) in the cloud, it can be decrypted by the final application that knows both NwkSKey and AppSKey. We provide an exemple in python with the loraWAN.py script that can be called from the command line given a mandatory base64-encoded string argument (frame_b64) and 2 optional arguments, the packet info string (pdata) and the radio info string (rdata), as parameters:

	> python loraWAN.py "QAYAAAAAAAABTynnSXehwueB5hamaP11aHQ=" "1,20,6,0,26,8,-45" "125,5,12"
	?loraWAN: valid MIC
	?loraWAN: plain payload is \!#3#TC/22.50
	?plain payload is : \!#3#TC/22.50
	^p1,16,6,0,13,8,-45
	^r125,5,12
	??\!#3#TC/22.50	

Once the application has the plain payload, it can further push the plain data into other IoT clouds. For instance, a final app running on a Linux machine can simply use post_processing_gw.py with the plain data to upload data to IoT clouds just as a gateway with NwkSKey and AppSKey would do.

	> python loraWAN.py "QAYAAAAAAAABTynnSXehwueB5hamaP11aHQ=" "1,20,6,0,26,8,-45" "125,5,12" | python post_processing_gw.py
	Parsing cloud declarations
	[u'python CloudThingSpeak.py']
	Parsed all cloud declarations
	post_processing_gw.py got cloud list: 
	[u'python CloudThingSpeak.py']
	Parsing cloud declarations
	[u'python CloudFireBaseAES.py']
	Parsed all cloud declarations
	post_processing_gw.py got encrypted cloud list: 
	[u'python CloudFireBaseAES.py']
	Parsing cloud declarations
	[u'python CloudFireBaseLWAES.py']
	Parsed all cloud declarations
	post_processing_gw.py got LoRaWAN encrypted cloud list: 
	[u'python CloudFireBaseLWAES.py']
	
	Current working directory: /home/pi/lora_gateway/aes_lorawan
	2016-10-27T22:18:45.079058
	rcv ctrl pkt info (^p): 1,16,6,0,13,8,-45
	splitted in:  [1, 16, 6, 0, 13, 8, -45]
	(dst=1 type=0x10(DATA) src=6 seq=0 len=13 SNR=8 RSSI=-45)
	rcv ctrl radio info (^r): 125,5,12
	splitted in:  [125, 5, 12]
	(BW=125 CR=5 SF=12)
	got first framing byte
	--> got data prefix
	valid app key: accept data
	number of enabled clouds is 1
	--> cloud[0]
	uploading with python CloudThingSpeak.py
	ThingSpeak: uploading
	rcv msg to log (\!) on ThingSpeak ( default , 3 ): 22.50
	ThingSpeak: will issue curl cmd
	curl -s -k -X POST --data field3=22.50&field7=0 https://api.thingspeak.com/update?key=ORE0DIFZIIPT61DO
	ThingSpeak: returned code from server is 218
	--> cloud end

If you use this feature where post_processing_gw.py takes its input from loraWAN.py, **you have to pass at least 2 arguments**: the base64-encoded string and the packet info string. loraWAN.py set the packet type and the data length to the appropriate value for post_processing_gw.py. post_processing_gw.py uses exactely the same cloud configuration than a normal gateway would have. Note the 2 '?' in front of the plain data. These are normally the data prefix \xFF\xFE inserted by the lora_gateway program and emulated by loraWAN.py. The '?' in front of the other lines indicate to post_processing_gw.py that these lines should be ignored.

List of new files
=================

- loraWAN.py: does the decryption of the LoRaWAN packet with encrypted payload for post_processing_gw.py
- test-loraWAN-1.py: shows how to decrypt a LoRaWAN packet with encrypted payload
	- usage: python test-loraWAN-1.py
- test-loraWAN-2.py: injects a fake native LoRaWAN packet into post_processing_gw.py
	- usage with local decryption: python test-loraWAN-2.py | python post_processing_gw.py --raw --aes
- test-loraWAN-3.py: injects a fake encapsulated LoRaWAN packet into post_processing_gw.py
	- usage with local decryption: python test-loraWAN-3.py | python post_processing_gw.py --raw --aes	
- README-aes_lorawan.md: this README file

Files that will be updated
==========================

- post_processing_gw.py: updated with the AES part

Files that need to be updated
=============================

- global_conf.json: set "raw" and "aes" to true according to your needs so that start_gw.py can start the gateway with the correct settings

		

Enjoy!
C. Pham 			
	