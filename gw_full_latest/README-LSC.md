Support for Lightweight Stream Cipher encryption method
=======================================================

Description 
-----------

This module will add the LSC encryption algorithm at gateway level:

- starting from v3.9 `post-processing_gw.py`
	* add support for Lightweight Stream Cipher (LSC)
	* add `LSC_decrypt.py` library file that will be called by `post-processing_gw.py`	
	* in `gateway_conf.json` file, `gateway_conf` section, `"lsc" : true` enables local LSC decryption at gateway
	* `post-processing_gw.py` will try to decrypt an encrypted packet with all enabled encryption modules, e.g. AES, LSC, ...
		* this process is done sequentially, module by module
		* as it is most likely that decryption will succeed only when both encryption and decryption algorithms do match, there is normally only one decryption method that will succeed
		* therefore `post-processing_gw.py` will stop when a decryption module succeeds
		* actually, it is mandatory for an encryption module to implement a Message Integrity Check (MIC) procedure at both the sender and receiver side. A correct MIC means that encryption and decryption algorithms match

- `Arduino_LoRa_temp` has also been updated to be able to send an LSC-encrypted packet
	* comment `WITH_AES` and uncomment `WITH_LSC`
	* the encryption key is defined by `uint8_t LSC_Nonce[16]`
		
- end-device uses our SX1272 4-byte packet header which is described in [section 3 of the low-cost gateway web page](http://cpham.perso.univ-pau.fr/LORA/RPIgateway.html). After the header, the encrypted payload can be found, followed by a 4-byte MIC: `dst[1] | ptype[1] | src[1] | seq[1] | encrypted payload[n] | MIC[4]`. The packet type indicates that it is an encrypted data packet: `PKT_TYPE_DATA | PKT_FLAG_DATA_ENCRYPTED`	

- `post_processing_gw.py` is able to locally check integrity and decrypt the encryted payload if the LSC encryption/decryption key is known at the gateway's post-processing level. `post_processing_gw.py` uses `LSC_decrypt.py` script for that purpose. `LSC_decrypt.py` uses `LSC_Nonce` from `LSC_config.py`.

- in case the key is not stored on the gateway, `post_processing_gw.py` can upload the encrypted data on an Internet cloud. `clouds.json` has an "encrypted_clouds" section to indicate scripts that will be called to upload encrypted messages. 


How to update your gateway
--------------------------
	
There is no extra system module to install, apart from updating the gateway with the latest distribution from our github repository. The update procedure will update `post-processing_gw.py` and add `LSC_decrypt.py` and `LSC_config.py`. You need to add the `"lsc" : true` line, right after the  `"aes" : false` line in `gateway_conf.json` file, `gateway_conf` section.
	
Examples
========

Using output from real sensor device to test LSC features
---------------------------------------------------------

We use:

	uint8_t LSC_Nonce[16] = {
	  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
	  0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C                                
	};

The example uses the `Arduino_LoRa_temp` that is an extended version of `Arduino_LoRa_Simple_temp` updated with both LoRaWAN-like AES and LSC capabilities.

	LoRa temperature sensor, extended version
	Arduino Pro Mini detected
	ATmega328P detected
	SX1276 detected, starting
	SX1276 LF/HF calibration
	...
	Get back previous sx1272 config
	Using packet sequence number of 104
	Forced to use default parameters
	Using node addr of 6
	Using idle period of 30
	Setting Mode: state 0
	Setting Channel: state 0
	Setting Power: state 0
	Setting node addr: state 0
	SX1272 successfully configured
	Mean temp is 22.5
	Sending \!#1#TC/22.50
	Real payload size is 13
	\!#1#TC/22.50
	plain payload hex
	05 06 07 08 5C 21 23 31 23 54 43 2F 32 32 2E 35 30 
	Encrypting
	encrypted payload
	D2 1D 27 3E D2 77 5D B3 64 20 75 CB 05 FA E9 B7 0A 
	[HEADER+CIPHER]:
	01 16 06 68|D2 1D 27 3E D2 77 5D B3 64 20 75 CB 05 FA E9 B7 0A 
	[encrypted HEADER+CIPHER]:
	E4 F5 7F 5E 00 8B 1B 2E 16 05 EA 72 42 2D 84 84 D1 67 BD BD 56 
	[MICv2]:[MIC]:
	63 21 18 A4 
	transmitted packet:
	EncryptedPayload | MIC[4]
	D2 1D 27 3E D2 77 5D B3 64 20 75 CB 05 FA E9 B7 0A|63 21 18 A4 
	[base64 CIPHER+MIC]:0h0nPtJ3XbNkIHXLBfrptwpjIRik
	--> CS1
	--> CAD 549
	OK1
	LoRa pkt size 21
	LoRa pkt seq 104
	LoRa Sent in 1731
	LoRa Sent w/CAD in 2280
	Packet sent, state 0
	Wait for 4000
	Wait for incoming packet
	No packet
	Switch to power saving mode
	Successfully switch LoRa module in sleep mode
	.....	

Use `LSC_Encrypt.py` to decrypt packet in command line mode
-----------------------------------------------------------

Once the encrypted packet is in base64 format it can be decrypted by the final application that knows the encryption key. We provide an exemple in Python with the `LSC_decrypt.py` script that can be called from a terminal command line given a mandatory base64-encoded string argument, a mandatory packet info string (pdata) an optional radio info string (rdata), as parameters:

	>  python LSC_decrypt.py "0h0nPtJ3XbNkIHXLBfrptwpjIRik" "1,22,6,104,21,8,-45" "125,5,12"
	?LSC: received MIC:  [ 99  33  24 164]
	?LSC: computed MIC:  [ 99  33  24 164]
	?LSC: valid MIC
	?LSC: plain payload is \x05\x06\x07\x08\!#1#TC/22.50
	?plain payload is:\!#1#TC/22.50
	^p1,18,6,104,17,8,-45
	^r125,5,12
	?\!#1#TC/22.50	

As you can see the base64 string is obtained here by copy/paste from the `Arduino_LoRa_temp` example. The packet info string is `"1,20,6,99,21,8,-45"` which means:

- destination address is 1 (it is the gateway that is the receiver)
- packet type is 22 which is 0x16 and means  `PKT_TYPE_DATA | PKT_FLAG_DATA_WAPPKEY | PKT_FLAG_DATA_ENCRYPTED`
- source address is 6
- packet sequence number is 104
- packet length is 21 bytes (13 payload + 4 appkey + 4 MIC)
- SNR of received packet at gateway is 8
- RSSI of received packet at gateway is -45

Only the 4 first parameters are useful for the decryption process has the MIC is computed using the packet header ans especially the packet sequence number. Note the 2 '?' in front of the plain data (last line). These are normally the data prefix \xFF\xFE inserted by the `lora_gateway` program and emulated by `LSC_decrypt.py` that cannot be printed on the output. The '?' in front of the other lines indicate to higher level scripts such as `post_processing_gw.py` that these lines should be ignored.

Once the application has the plain payload, it can further push the plain data into other IoT clouds. For instance, a final app running on a Linux machine can simply use `post_processing_gw.py` with the plain data to upload data to IoT clouds.

	> python LSC_decrypt.py "0h0nPtJ3XbNkIHXLBfrptwpjIRik" "1,22,6,104,21,8,-45" "125,5,12" | python post_processing_gw.py
	enable local LSC decryption
	Parsing cloud declarations
	[u'python CloudThingSpeak.py']
	Parsed all cloud declarations
	post_processing_gw.py got cloud list: 
	[u'python CloudThingSpeak.py']
	Parsing cloud declarations
	Parsed all cloud declarations
	post_processing_gw.py got encrypted cloud list: 
	[]
	Parsing cloud declarations
	Parsed all cloud declarations
	post_processing_gw.py got LoRaWAN encrypted cloud list: 
	[]
	Current working directory: /home/pi/lora_gateway
	2019-01-17T14:16:37.720955
	rcv ctrl pkt info (^p): 1,18,6,104,17,8,-45
	splitted in:  [1, 18, 6, 104, 17, 8, -45]
	(dst=1 type=0x12(DATA WAPPKEY) src=6 seq=104 len=17 SNR=8 RSSI=-45)
	rcv ctrl radio info (^r): 125,5,12
	splitted in:  [125, 5, 12]
	(BW=125 CR=5 SF=12)
	got first framing byte
	--> got LoRa data prefix
	--> DATA with_appkey: read app key sequence
	app key is  0x05 0x06 0x07 0x08
	app key disabled
	valid app key: accept data
	number of enabled clouds is 1
	--> cloud[0]
	uploading with python CloudThingSpeak.py
	python CloudThingSpeak.py "#1#TC/22.50" "1,18,6,104,13,8,-45" "125,5,12" "2019-01-17T14:16:37+01:00" "00000027EBEFC4A6"
	ThingSpeak: uploading (multiple)
	rcv msg to log (\!) on ThingSpeak ( default , 1 ): 
	ThingSpeak: will issue curl cmd
	curl -s -k -X POST --data field1=22.50 https://api.thingspeak.com/update?key=SGSH52UGPVAUYG3S
	ThingSpeak: returned code from server is 354484
	--> cloud end

As you see, `LSC_decrypt.py` set the final packet type and final data length to the appropriate value for `post_processing_gw.py` which uses exactly the same cloud configuration than a normal gateway would have. 

Receiving encrypted packets on a full gateway
---------------------------------------------

The gateway is started normally, or manually with:

	> sudo ./lora_gateway --mode 1 | python post_processing_gw.py 
	
or simply with

	> sudo python start_gw.py	
	
to have the full gateway running. Note that both `lora_gateway` and `post_processing_gw.py` are in normal mode, i.e. `["gateway_conf"]["raw"]` is false. Here `post_processing_gw.py` uses both local AES and LSC decryption, i.e. `["gateway_conf"]["aes"]` and `["gateway_conf"]["lsc"]` are both true to show how different decryption methods can be enabled.

	enable local AES decryption
	enable local LSC decryption
	Parsing cloud declarations
	[u'python CloudThingSpeak.py']
	Parsed all cloud declarations
	post_processing_gw.py got cloud list: 
	[u'python CloudThingSpeak.py']
	Parsing cloud declarations
	Parsed all cloud declarations
	post_processing_gw.py got encrypted cloud list: 
	[]
	Parsing cloud declarations
	Parsed all cloud declarations
	post_processing_gw.py got LoRaWAN encrypted cloud list: 
	[]
	Current working directory: /home/pi/lora_gateway
	SX1276 detected, starting.
	SX1276 LF/HF calibration
	...
	**********Power ON: state 0
	Default sync word: 0x12
	LoRa mode 1
	Setting mode: state 0
	Channel CH_10_868: state 0
	Use PA_BOOST amplifier line
	Set LoRa power dBm to 14
	Power: state 0
	Get Preamble Length: state 0
	Preamble Length: 8
	LoRa addr 1: state 0
	SX1272/76 configured as LR-BS. Waiting RF input for transparent RF-serial bridge
	Low-level gw status ON
	--- rxlora. dst=1 type=0x16 src=6 seq=108 len=21 SNR=7 RSSIpkt=-34 BW=125 CR=4/5 SF=12
	2019-01-17T14:13:30.101814
	rcv ctrl pkt info (^p): 1,22,6,108,21,7,-34
	splitted in:  [1, 22, 6, 108, 21, 7, -34]
	(dst=1 type=0x16(DATA WAPPKEY ENCRYPTED) src=6 seq=108 len=21 SNR=7 RSSI=-34)
	rcv ctrl radio info (^r): 125,5,12
	splitted in:  [125, 5, 12]
	(BW=125 CR=5 SF=12)
	rcv timestamp (^t): 2019-01-17T14:13:30.099

	got first framing byte
	--> got LoRa data prefix
	--> DATA encrypted: encrypted payload size is 21
	--> decrypting in AES-CTR mode (LoRaWAN version)
	###BADMIC###
	--> decrypting in LSC mode
	?LSC: received MIC:  [ 33  41 198  16]
	?LSC: computed MIC:  [ 33  41 198  16]
	?LSC: valid MIC
	?LSC: plain payload is \x05\x06\x07\x08\!#1#TC/22.50
	--> plain payload is :  \x05\x06\x07\x08\!#1#TC/22.50
	--> changed packet type to clear data
	--> DATA with_appkey: read app key sequence
	app key is  0x05 0x06 0x07 0x08
	app key disabled
	valid app key: accept data
	number of enabled clouds is 1
	--> cloud[0]
	uploading with python CloudThingSpeak.py
	python CloudThingSpeak.py "#1#TC/22.50" "1,18,6,108,13,7,-34" "125,5,12" "2019-01-17T14:13:30+01:00" "00000027EBEFC4A6"
	ThingSpeak: uploading (multiple)
	rcv msg to log (\!) on ThingSpeak ( default , 1 ): 
	ThingSpeak: will issue curl cmd
	curl -s -k -X POST --data field1=22.50 https://api.thingspeak.com/update?key=SGSH52UGPVAUYG3S
	ThingSpeak: returned code from server is 354479
	--> cloud end	
	
List of new files
=================

- `LSC_decrypt.py`: does the decryption of the LSC packet with encrypted payload for `post_processing_gw.py
- `LSC_config.py`: stores the `LSC_Nonce` encryption key 
- `README-LSC.md`: this README file

Files that will be updated
==========================

- `post_processing_gw.py`: updated with the LSC part

Files that need to be updated
=============================

- `gateway_conf.json`: add/set `"lsc" : true` according to your needs in `gateway_conf` section


Enjoy!
C. Pham 			
	