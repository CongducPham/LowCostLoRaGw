# Downlink transmission with single-channel gateway

This document describes a simple support for downlink transmission requests for the Raspberry-based single-channel gateway **only**. Iif you are using the multi-channel version, downlinks will be taken care by the legacy `lora_pkt_fwd` program (see dedicated [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/scripts/rak2245-rak831/README.md).

For downlink support, there are no changes in the way the gateway (`lora_gateway` program) is launched or interact with the post-processing stage (`post_processing_gw.py`). The single-channel downlink transmission mechanism works as follows:

- create a `downlink` folder in your `lora_gateway` folder, **ALL** downlink related files will be stored in this `downlink` folder
- the `lora_gateway` program will check for a `downlink.txt` file after each LoRa packet reception. This behavior can be disable with `--ndl` option
- `lora_gateway` checks 3 times for `downlink.txt after a packet reception at t_r (to work with Network Server sending PULL_RESP in just-in-time mode) 
	- first, after a delay of `DELAY_DNWFILE`  (typically set to 700ms) to target RX1. There is no reception possible
	- second, after t_r+DELAY_DNWFILE+DELAY_RX2 (additional delay of 1000ms, no reception possible) to target RX2
	- last, after t_r+DELAY_DNWFILE+DELAY_RX2+RCV_TIMEOUT_JACC2 (radio in reception mode for 3500ms) to target either RX1 or RX2 for join-accept
		* if a new packet is received during these 3500ms then it has priority
		* in which case `lora_gateway` then starts a new cycle of downlink checking attempts for this new message 
- `downlink/downlink.txt` should contain ONLY one line in JSON format:
	- non-LoRaWAN downlink: e.g. `{"status":"send_request","dst":3,"data":"hello"}`
	- LoRaWAN downlink: e.g. `{"txpk":{"imme":false,"rfch":0,"powe":14,"ant":0,"brd":0,"tmst":1580280908,"freq":868.1,"modu":"LORA","datr":"SF12BW125","codr":"4/5","ipol":true,"size":19,"data":"YCEXASYHKAAFANKthAgBt6sC6g=="}}`	
- after reading `downlink/downlink.txt`, the file will be deleted by `lora_gateway`
- `downlink.txt` file can be renamed as `downlink-backup-2020-01-20T13:42:23.txt` (with the current date indicated) if `lora_gateway` is compiled with `#define KEEP_DOWNLINK_BACKUP_FILE`
- there is no reliability mechanism implemented
- there are some differences between the non-LoRaWAN downlink and the LoRaWAN downlink that are described in the next paragraphs


# LoRaWAN downlink

For LoRaWAN downlink, downlink messages come from the Network Server, following the LoRaWAN specification and the initial protocol using UDP to communicate with the Network Server. Devices can be programmed using the LMIC LoRaWAN stack which has been ported to Arduino. As the gateway is single-channel, the device must only use one uplink frequency at a fixed SF value that need to match those of the single-channel gateway. The [Arduino_LoRa_LMIC_ABP_BASIC](https://github.com/CongducPham/LowCostLoRaGw/tree/master/Arduino/Arduino_LoRa_LMIC_ABP_BASIC) example shows how to program an Arduino with a simple LoRaWAN device. A more elaborated example is [Arduino_LoRa_LMIC_ABP_temp](https://github.com/CongducPham/LMIC_low_power/tree/master/Arduino_LoRa_LMIC_ABP_temp) with low-power operation mode support. However you need a slightly modified version of LMIC as described [here](https://github.com/CongducPham/LMIC_low_power/blob/master/README.md). Again, as the gateway is single-channel, the support of join operations for OTAA mode requires some modifications in the LMIC code to fix the join frequency and datarate to the one used by uplink packets (and matching those of the single-channel gateway). More details will be provided later on. The LoRaWAN downlink mechanism on the single-channel gateway works as follows:

- a LoRaWAN Network Server needs to be defined (e.g. TTN or ChirpStack for instance. For ChirpStack, a local instance is possible)
- uplink messages use a LoRaWAN cloud script (e.g. `CloudTTN.py` or `CloudChirpStack.py` for instance)
- the gateway and the devices need to be registered on the LoRaWAN Network Server, see the [README-TTN.md](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-TTN.md) for instance
- `["gateway_conf"]["downlink_lorawan"]=true` enables periodic PULL_DATA on Network Server to get downlink messages
- `["gateway_conf"]["downlink_network_server"]="127.0.0.1"` indicates a LoRaWAN Network Server, here, it is the local ChirpStack
- `start_gw.py` will run a python script in background (`scripts/lorawan_stats/downlink_lorawan.py`) to perform periodic `PULL_DATA` to Network Server and get `PULL_RESP` with downlink message. Downlink message (json) will then be written directly to `downlink/downlink.txt` file for `lora_gateway` program
	- `{"txpk":{"imme": false, "rfch": 0, "powe": 14, "ant": 0, "brd": 0, "tmst": 1580596494, "freq": 868.1, "modu": "LORA", "datr": "SF12BW125", "codr": "4/5", "ipol": true, "size": 25, "data": "YCQfBCYCtAAIAQG5APUOXbA2QvlB76KdaQ=="}}`
- `lora_gateway` will use the `tmst` field in the downlink message to decide which receive window to use: normally a difference of 1s between `tmst` of the downlink message and the time the uplink packet was received means received window 1 (RX1, using same parameter as uplink) while a value of 2s means receive window 2 (RX2, using 869.5235MHz and SF12)
- for join-request procedure, the RX1 window is 5s and the RX2 window is 6s
- `lora_gateway` will include the received packet's timestamp `tmst` in the time information string: `^t2020-01-21T17:33:27.740034*4155747970`. The value after the '*' is the local timestamp in microsecond that will be passed to `post_processing_gw.py` and then to cloud scripts when uploading the LoRaWAN packet to the Network Server
- when issuing a downlink message, the Network Server will normally indicate (in case of class A devices) the time at which the downlink packet should be sent. The uplink `tmst` is usually increased either by the value of RX1 window delay, i.e. 1s or 5s, or by the value of RX2 window delay, i.e. 2s or 6s.
	
## Example

Configure the single-channel gateway in LoRaWAN mode:

- set `["radio_conf"]["mode"]` to 11 in `gateway_conf.json` to configure for native (encrypted) LoRaWAN reception on single channel. The single-channel gateway will be configured with BW=125MHz, CR=4/5, SF=12 by default. The frequency is set by default to 868.1MHz for BAND868, 923.2MHz for BAND900 and 433.175 for BAND433 
- set `["gateway_conf"]["raw"]` to true in `gateway_conf.json` 
- set `["gateway_conf"]["aes_lorawan"]` to false as encrypted data will be pushed to TTN or ChirpStack
- set `["gateway_conf"]["downlink_lorawan"]` to true to enable periodic PULL_DATA and get PULL_RESP (downlink messages) from Network Server
- indicate in `["gateway_conf"]["downlink_network_server"]` the address of the Network Server for downlink messages 
- enable `python CloudTTN.py` or `python CloudChirpStack.py` in the LoRaWAN encrypted cloud section of `clouds.json`. Enable one by setting `"enabled"` to `true`

```
	"lorawan_encrypted_clouds" : [
		{	
			"name":"ChirpStack network server",
			"script":"python CloudChirpStack.py",
			"type":"lorawan",			
			"enabled":false
		},	
		...	
	]
```	

It is actually recommended to use the web admin interface and use the easy `Configure for LoRaWAN` functionality that automatize the whole process to set the gateway in LoRaWAN mode.

Start the gateway

	> sudo python start_gw.py
	
Output at gateway when starting, 868.1MHZ SF12BW125, `CloudChirpStack.py` is used to transmit all uplink messages to ChirpStack Network Server

	raw output from low-level gateway. post_processing_gw will handle packet format
	post_processing_gw.py found an alert_conf section
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
	[u'python CloudChirpStack.py']
	Parsed all cloud declarations
	post_processing_gw.py got LoRaWAN encrypted cloud list: 
	[u'python CloudChirpStack.py']
	Starting thread to perform periodic gw status/tasks
	2020-01-21 09:37:51.309005
	post status: gw ON
	post status: executing periodic tasks
	post status: start running
	cs_stats: gw id B827EBFFFFCE23C2
	cs_stats: Opening UDP socket to 127.0.0.1 (127.0.0.1) port 1700...
	cs_stats: Try to send UDP packet: ???'????#?{"stat": {"rxok": 0, "txnb": 0, "ackr": 0, "dwnb": 0, "alti": 0, "time": "2020-01-21 09:37:51 GMT", "rxnb": 0, "rxfw": 0, "lati": 10.0, "long": 20.0}}
	post status: exiting
	Starting thread to perform fast statistics tasks

	Current working directory: /home/pi/lora_gateway
	SX1276 detected, starting.
	SX1276 LF/HF calibration
	...
	**********Power ON: state 0
	LoRa CR 5: state 0
	LoRa SF 12: state 0
	LoRa BW 125: state 0
	Configuring for LoRaWAN
	Set frequency to 868.1MHz: state 0
	Use PA_BOOST amplifier line
	Set LoRa power dBm to 14
	Power: state 0
	Get Preamble Length: state 0
	Preamble Length: 8
	LoRa addr 1: state 0
	Raw format, not assuming any header in reception
	SX1272/76 configured as LR-BS. Waiting RF input for transparent RF-serial bridge
	Default sync word: 0x12
	Set sync word to 0x34
	LoRa sync word: state 0
	Low-level gateway has downlink support
	2020-01-21T17:33:08.861087
	Low-level gw status ON 
	...

Use [Arduino_LoRa_LMIC_ABP_BASIC](https://github.com/CongducPham/LowCostLoRaGw/tree/master/Arduino/Arduino_LoRa_LMIC_ABP_BASIC) as LoRAWAN device, but only enabling 868.1MHz frequency. The device is sending `Hello from LMIC`. Then go on your Network Server console to schedule a downlink. Here we use local ChirpStack to schedule `helloworld`, i.e. `aGVsbG93b3JsZA==` in base64 format. Be sure to schedule the downlink before starting the device which will send almost immediately.
	
Gateway output, continued, when receiving the packet from the LoRaWAN device

	...
	--- rxlora. dst=0 type=0x00 src=0 seq=0 len=28 SNR=7 RSSIpkt=-19 BW=125 CR=4/5 SF=12
	2020-01-21T17:33:27.741324
	rcv ctrl pkt info (^p): 0,0,0,0,28,7,-19
	splitted in:  [0, 0, 0, 0, 28, 7, -19]
	rawFormat(len=28 SNR=7 RSSI=-19)
	rcv ctrl radio info (^r): 125,5,12,868100
	splitted in:  [125, 5, 12, 868100]
	(BW=125 CR=5 SF=12)
	rcv timestamp (^t): 2020-01-21T17:33:27.740034*4155747970

	adding time zone info
	new rcv timestamp (^t): 2020-01-21T17:33:27.740034+01:00
	got first framing byte
	--> got LoRa data prefix
	raw format from LoRa gateway
	LoRaWAN?
	update ctrl pkt info (^p): 256,64,0x26041F24,0,15,7,-19
	+++ rxlora[868100]. lorawan type=0x40 src=0x26041F24 seq=0 len=15 SNR=7 RSSIpkt=-19 BW=125 CR=4/5 SF=12
	--> number of LoRaWAN enabled clouds is 1
	--> LoRaWAN encrypted cloud[0]
	uploading with python CloudChirpStack.py
	python CloudChirpStack.py "QCQfBCaAAAABMHpb8pE1UwwjMG/l7IoiMC8T+A==" "256,64,637804324,0,15,7,-19" "125,5,12,868100" "2020-01-21T17:33:27.740034+01:00*4155747970" "0000B827EBCE23C2"
	CloudChirpStack: gw id: B827EBFFFFCE23C2
	CloudChirpStack: Opening UDP socket to 127.0.0.1 (127.0.0.1) port 1700...
	--> LoRaWAN encrypted cloud end
	--> DATA encrypted: local AES LoRaWAN not activated

	-----------------------------------------------------
	Check for downlink requests 2020-01-21T17:33:28.439246
	Read downlink: 204
	Downlink entry: {"txpk":{"imme":false,"rfch":0,"powe":14,"ant":0,"brd":0,"tmst":4156747970,"freq":868.1,"modu":"LORA","datr":"SF12BW125","codr":"4/5","ipol":true,"size":25,"data":"YCQfBCYC9QAIAQF+eOi0BvnZlM6hkIt/wA=="}}
	Process downlink request
	LoRaWAN downlink request
	Target RX1
	Packet sent, state 0
	-----------------------------------------------------
	...
	
At LMIC device in ABP, with debug output activated to give more details

	Starting
	RXMODE_RSSI
	354: engineUpdate, opmode=0x808
	414: Uplink data pending
	460: Considering band 0, which is available at 345
	1170: Considering band 3, which is available at 0
	1990: No channel found in band 3
	2518: Considering band 0, which is available at 345
	3372: No channel found in band 0
	3899: Considering band 1, which is available at 345
	4752: Airtime available at 345 (channel duty limit)
	5588: Ready for uplink
	6770: Updating info for TX at 413, airtime will be 102919. Setting available time for band 1 to 207159296
	7762: TXMODE, freq=868100000, len=28, SF=12, BW=125, CR=4/5, IH=0
	Packet queued
	FREQ=868100000
	110088: irq: dio: 0x0 flags: 0x8
	110170: Scheduled job 0x254, cb 0x184a ASAP
	110306: Running job 0x254, cb 0x184a, deadline 0
	111113: Scheduled job 0x254, cb 0x185d at 163245
	163249: Running job 0x254, cb 0x185d, deadline 163245
	163412: RXMODE_SINGLE, freq=868100000, SF=12, BW=125, CR=4/5, IH=0
	275799: irq: dio: 0x0 flags: 0x40
	275905: Scheduled job 0x254, cb 0x1865 ASAP
	276033: Running job 0x254, cb 0x1865, deadline 0
	277621: Received downlink, window=RX1, port=1, ack=0
	277720[4443]: EV_TXCOMPLETE (includes waiting for RX windows)
	Received 
	10
	 bytes of payload
	68656C6C6F776F726C64279591: Scheduled job 0x208, cb 0x1b68 at 19029586
	280416: engineUpdate, opmode=0x800
	...
	
You can see that the 10 bytes of downlink data have been received in the RX1 window. It is also possible to target RX2 window as shown in the following gateway output

	...
	--- rxlora. dst=0 type=0x00 src=0 seq=0 len=28 SNR=-6 RSSIpkt=-29 BW=125 CR=4/5 SF=12
	2020-01-21T17:36:07.881986
	rcv ctrl pkt info (^p): 0,0,0,0,28,-6,-29
	splitted in:  [0, 0, 0, 0, 28, -6, -29]
	rawFormat(len=28 SNR=-6 RSSI=-29)
	rcv ctrl radio info (^r): 125,5,12,868100
	splitted in:  [125, 5, 12, 868100]
	(BW=125 CR=5 SF=12)
	rcv timestamp (^t): 2020-01-21T17:36:07.880541*20921181

	adding time zone info
	new rcv timestamp (^t): 2020-01-21T17:36:07.880541+01:00
	got first framing byte
	--> got LoRa data prefix
	raw format from LoRa gateway
	LoRaWAN?
	update ctrl pkt info (^p): 256,64,0x26041F24,0,15,-6,-29
	+++ rxlora[868100]. lorawan type=0x40 src=0x26041F24 seq=0 len=15 SNR=-6 RSSIpkt=-29 BW=125 CR=4/5 SF=12
	--> number of LoRaWAN enabled clouds is 1
	--> LoRaWAN encrypted cloud[0]
	uploading with python CloudChirpStack.py
	python CloudChirpStack.py "QCQfBCaAAAABMHpb8pE1UwwjMG/l7IoiMC8T+A==" "256,64,637804324,0,15,-6,-29" "125,5,12,868100" "2020-01-21T17:36:07.880541+01:00*20921181" "0000B827EBCE23C2"
	CloudChirpStack: gw id: B827EBFFFFCE23C2
	CloudChirpStack: Opening UDP socket to 127.0.0.1 (127.0.0.1) port 1700...
	--> LoRaWAN encrypted cloud end
	--> DATA encrypted: local AES LoRaWAN not activated

	-----------------------------------------------------
	Check for downlink requests 2020-01-21T17:36:08.580379
	Read downlink: 202
	Downlink entry: {"txpk":{"imme":false,"rfch":0,"powe":14,"ant":0,"brd":0,"tmst":22921181,"freq":868.1,"modu":"LORA","datr":"SF12BW125","codr":"4/5","ipol":true,"size":25,"data":"YCQfBCYC9gAIAQGKNMDN8rwMbcvyYfH2bQ=="}}
	Process downlink request
	LoRaWAN downlink request
	Target RX2
	Packet sent, state 0
	Set back frequency: state 0
	Set back SF: state 0
	-----------------------------------------------------
	...
	
At the LMIC device, we can see that the downlink data is received in RX2 window.

	Starting
	RXMODE_RSSI
	439: engineUpdate, opmode=0x808
	498: Uplink data pending
	545: Considering band 0, which is available at 430
	1253: Considering band 3, which is available at 0
	2073: No channel found in band 3
	2601: Considering band 0, which is available at 430
	3457: No channel found in band 0
	3983: Considering band 1, which is available at 430
	4830: Airtime available at 430 (channel duty limit)
	5672: Ready for uplink
	6853: Updating info for TX at 497, airtime will be 102919. Setting available time for band 1 to 212664320
	7846: TXMODE, freq=868100000, len=28, SF=12, BW=125, CR=4/5, IH=0
	Packet queued
	FREQ=868100000
	110046: irq: dio: 0x0 flags: 0x8
	110129: Scheduled job 0x254, cb 0x184a ASAP
	110264: Running job 0x254, cb 0x184a, deadline 0
	111071: Scheduled job 0x254, cb 0x185d at 163203
	163208: Running job 0x254, cb 0x185d, deadline 163203
	163370: RXMODE_SINGLE, freq=868100000, SF=12, BW=125, CR=4/5, IH=0
	197542: irq: dio: 0x1 flags: 0x80
	197626: Scheduled job 0x254, cb 0x1865 ASAP
	197776: Running job 0x254, cb 0x1865, deadline 0
	198583: Scheduled job 0x254, cb 0x187c at 213415
	213419: Running job 0x254, cb 0x187c, deadline 213415
	213582: RXMODE_SINGLE, freq=869525000, SF=12, BW=125, CR=4/5, IH=0
	337719: irq: dio: 0x0 flags: 0x40
	337825: Scheduled job 0x254, cb 0x17f0 ASAP
	337953: Running job 0x254, cb 0x17f0, deadline 0
	339541: Received downlink, window=RX2, port=1, ack=0
	339640[5434]: EV_TXCOMPLETE (includes waiting for RX windows)
	Received 
	10
	 bytes of payload
	68656C6C6F776F726C64341511: Scheduled job 0x208, cb 0x1b68 at 19091506
	342336: engineUpdate, opmode=0x800
	...
	
If you look at the gateway output, you can see that the received packet's timestamps at the gateway is 20921181 (`rcv timestamp (^t): 2020-01-21T17:36:07.880541*20921181`). The downlink message from the Network Server indicates the time at which the downlink data should be sent back to the device: `{"txpk":{"imme":false,"rfch":0,"powe":14,"ant":0,"brd":0,"tmst":22921181,...`. You can see that the timestamp is 22921181 which indicates a time difference of 2s, thus the usage of RX2 window. For the device, according to LoRaWAN specification, if data is received in RX1, then RX2 window is not open. Here, as the gateway is using RX2, the device receives nothing in RX1 and will then open RX2 to try to receive a downlink data.

The downlink support for the single-channel gateway also allows for OTAA mode. However, OTAA with a single-channel gateway needs to fix the join frequency and datarate to the one used by uplink packets (and matching those of the single-channel gateway). Therefore you need to use our slightly modified LMIC stack (the one that includes the low-power mode) and uncomment in `lmic/config.h` the `#define LMIC_SCG` statement. This will avoid LMIC to use SF7 for join-request and will use the same datarate than for uplinks (the one set with `LMIC_setDrTxpow(DR_SF12, 14);` for instance). 

Here we show an LMIC device in OTAA mode, with debug output activated to give more details. You can see that the assigned device address is 0x005C8ADC.

	Starting
	RXMODE_RSSI
	410: engineUpdate, opmode=0x8
	482: Scheduled job 0x27e, cb 0x155b ASAP
	Packet queued
	880: Running job 0x27e, cb 0x155b, deadline 0
	1626[26ms]: EV_JOINING
	2015: engineUpdate, opmode=0xc
	2521: Uplink join pending
	2942: Airtime available at 235280 (previously determined)
	3885: Uplink delayed until 235280
	4442: Scheduled job 0x27e, cb 0x434 at 235155
	235159: Running job 0x27e, cb 0x434, deadline 235155
	235261: engineUpdate, opmode=0xc
	235515: Uplink join pending
	235969: Airtime available at 235280 (previously determined)
	236944: Ready for uplink
	237892: Updating info for TX at 235514, airtime will be 92678. Setting available time for band 0 to 3211395072
	239669: TXMODE, freq=868100000, len=23, SF=12, BW=125, CR=4/5, IH=0
	331597: irq: dio: 0x0 flags: 0x8
	331680: Scheduled job 0x27e, cb 0x1823 ASAP
	331815: Running job 0x27e, cb 0x1823, deadline 0
	332898: Scheduled job 0x27e, cb 0x17f8 at 616322
	616327: Running job 0x27e, cb 0x17f8, deadline 616322
	616490: RXMODE_SINGLE, freq=868100000, SF=12, BW=125, CR=4/5, IH=0
	726986: irq: dio: 0x0 flags: 0x40
	727091: Scheduled job 0x27e, cb 0x1a87 ASAP
	727221: Running job 0x27e, cb 0x1a87, deadline 0
	728818[11661ms]: EV_JOINED
	netid: 0
	devaddr: 5C8ADC
	artKey: F9-AA-FE-19-B7-11-C9-E0-F0-ED-6E-83-72-74-53-EA
	nwkKey: EC-44-A5-5D-84-0D-21-BA-0C-45-6B-1F-3D-0C-7C-F1
	730763: engineUpdate, opmode=0x808
	731328: Uplink data pending
	731783: Considering band 0, which is available at 728500
	732710: Considering band 3, which is available at 0
	733562: No channel found in band 3
	734123: Considering band 0, which is available at 728500
	735059: No channel found in band 0
	735619: Considering band 1, which is available at 728500
	736552: Airtime available at 728500 (channel duty limit)
	737470: Ready for uplink
	738683: Updating info for TX at 731327, airtime will be 102919. Setting available time for band 1 to 863698944
	739757: TXMODE, freq=868100000, len=26, SF=12, BW=125, CR=4/5, IH=0
	841957: irq: dio: 0x0 flags: 0x8
	842040: Scheduled job 0x27e, cb 0x17e9 ASAP
	842175: Running job 0x27e, cb 0x17e9, deadline 0
	842981: Scheduled job 0x27e, cb 0x17f4 at 901258
	901263: Running job 0x27e, cb 0x17f4, deadline 901258
	901425: RXMODE_SINGLE, freq=868100000, SF=12, BW=125, CR=4/5, IH=0
	923273: irq: dio: 0x1 flags: 0x80
	923357: Scheduled job 0x27e, cb 0x17fc ASAP
	923507: Running job 0x27e, cb 0x17fc, deadline 0
	924313: Scheduled job 0x27e, cb 0x1813 at 957614
	957619: Running job 0x27e, cb 0x1813, deadline 957614
	957782: RXMODE_SINGLE, freq=869525000, SF=12, BW=125, CR=4/5, IH=0
	991954: irq: dio: 0x1 flags: 0x80
	992038: Scheduled job 0x27e, cb 0x178f ASAP
	992187: Running job 0x27e, cb 0x178f, deadline 0
	992984[15887ms]: EV_TXCOMPLETE (includes waiting for RX windows)
	995111: Scheduled job 0x237, cb 0x43e at 4745105
	995210: engineUpdate, opmode=0x900
	...

At the gateway we have the following output. Note the RX1 window for join-accept set to 5s: packet reception timestamp is 4149760124 (us) and downlink message timestamp is 4154760124 (us) which means 5s of delay.

	...
	--- rxlora. dst=0 type=0x00 src=0 seq=0 len=23 SNR=10 RSSIpkt=-21 BW=125 CR=4/5 SF=12
	2020-01-22T15:01:51.165045
	rcv ctrl pkt info (^p): 0,0,0,0,23,10,-21
	splitted in:  [0, 0, 0, 0, 23, 10, -21]
	rawFormat(len=23 SNR=10 RSSI=-21)
	rcv ctrl radio info (^r): 125,5,12,868100
	splitted in:  [125, 5, 12, 868100]
	(BW=125 CR=5 SF=12)
	rcv timestamp (^t): 2020-01-22T15:01:51.163516*4149760124

	adding time zone info
	new rcv timestamp (^t): 2020-01-22T15:01:51.163516+01:00
	got first framing byte
	--> got LoRa data prefix
	raw format from LoRa gateway
	LoRaWAN?
	update ctrl pkt info (^p): 256,0,0xD0019A7C,46037,10,10,-21
	+++ rxlora[868100]. lorawan type=0x00 src=0xD0019A7C seq=46037 len=10 SNR=10 RSSIpkt=-21 BW=125 CR=4/5 SF=12
	--> number of LoRaWAN enabled clouds is 1
	--> LoRaWAN encrypted cloud[0]
	uploading with python CloudChirpStack.py
	python CloudChirpStack.py "AHyaAdB+1bNwsqjUyJAaiwAXoHP6x4o=" "256,0,3489766012,46037,10,10,-21" "125,5,12,868100" "2020-01-22T15:01:51.163516+01:00*4149760124" "0000B827EBCE23C2"
	CloudChirpStack: gw id: B827EBFFFFCE23C2
	CloudChirpStack: Opening UDP socket to 127.0.0.1 (127.0.0.1) port 1700...
	--> LoRaWAN encrypted cloud end
	--> DATA encrypted: local AES LoRaWAN not activated

	-----------------------------------------------------
	Check for downlink requests 2020-01-22T15:01:51.863006
	Read downlink: 192
	Downlink entry: {"txpk":{"imme":false,"rfch":0,"powe":14,"ant":0,"brd":0,"tmst":4154760124,"freq":868.1,"modu":"LORA","datr":"SF12BW125","codr":"4/5","ipol":true,"size":17,"data":"ILgw53xkkcEsiZL+UC8AWvE="}}
	Process downlink request
	LoRaWAN downlink request
	Target RX1
	Packet sent, state 0
	-----------------------------------------------------
	...
	
After sending the downlink data (join-accept), the gateway receives the first data packet from the newly activated device and checks for downlink message. In the example, there are no.
	
	...
	--- rxlora. dst=0 type=0x00 src=0 seq=0 len=26 SNR=9 RSSIpkt=-24 BW=125 CR=4/5 SF=12
	2020-01-22T15:01:59.334796
	rcv ctrl pkt info (^p): 0,0,0,0,26,9,-24
	splitted in:  [0, 0, 0, 0, 26, 9, -24]
	rawFormat(len=26 SNR=9 RSSI=-24)
	rcv ctrl radio info (^r): 125,5,12,868100
	splitted in:  [125, 5, 12, 868100]
	(BW=125 CR=5 SF=12)
	rcv timestamp (^t): 2020-01-22T15:01:59.333712*4157930320

	adding time zone info
	new rcv timestamp (^t): 2020-01-22T15:01:59.333712+01:00
	got first framing byte
	--> got LoRa data prefix
	raw format from LoRa gateway
	LoRaWAN?
	update ctrl pkt info (^p): 256,64,0x005C8ADC,0,13,9,-24
	+++ rxlora[868100]. lorawan type=0x40 src=0x005C8ADC seq=0 len=13 SNR=9 RSSIpkt=-24 BW=125 CR=4/5 SF=12
	--> number of LoRaWAN enabled clouds is 1
	--> LoRaWAN encrypted cloud[0]
	uploading with python CloudChirpStack.py
	python CloudChirpStack.py "QNyKXACAAAABOoNDLqQpr03fViE93BHetKY=" "256,64,6064860,0,13,9,-24" "125,5,12,868100" "2020-01-22T15:01:59.333712+01:00*4157930320" "0000B827EBCE23C2"
	CloudChirpStack: gw id: B827EBFFFFCE23C2
	CloudChirpStack: Opening UDP socket to 127.0.0.1 (127.0.0.1) port 1700...
	--> LoRaWAN encrypted cloud end
	--> DATA encrypted: local AES LoRaWAN not activated

	-----------------------------------------------------
	Check for downlink requests 2020-01-22T15:02:00.033007
	NO NEW DOWNLINK ENTRY
	...

# non-LoRaWAN downlink

The non-LoRaWAN downlink mechanism is for non-LoRaWAN scenarios where there is no LoRaWAN Network Server. It is ONLY intended for non-LoRaWAN devices and does not have downlink data encryption feature. The [Arduino_LoRa_temp](https://github.com/CongducPham/LowCostLoRaGw/tree/master/Arduino/Arduino_LoRa_temp) is an example to have a simple non-LoRaWAN device receiving the non-LoRaWAN downlink. The non-LoRaWAN downlink mechanism works as follows:

- `downlink/downlink.txt` will be generated by `post_processing_gw.py` 
- `post_processing_gw.py` periodically check for `downlink/downlink-post.txt` and will build a queue of downlink requests
- `["gateway_conf"]["downlink"]` indicates the time interval (in second) for `post_processing_gw.py` to check for a `downlink-post.txt` file
	- for instance: "downlink" : `["gateway_conf"]["downlink"]=60`
	- set to 0 to disable the `post_processing_gw.py` periodically checking
	- set to -1 to disable the `lora_gateway` downlink checking (`start_gw.py` will launch `lora_gateway` with `--ndl` option, i.e. no downlink)
- `post_processing_gw.py` reads `downlink-post.txt` and generates a new `downlink-post-queued.txt` file containing all queued/pending requests
- `post_processing_gw.py` then deletes `downlink-post.txt`	
- it means that if you want to send a message to a device, as the device will open a receive window only after its own data transmission, it is advised to post the downlink message in advance	
- when a LoRa packet from device i is processed by `post_processing_gw.py`, it will check for a pending downlink request for device i
- if it is the case, then `post_processing_gw.py` generates the corresponding `downlink.txt` file which will contain the downlink entry
- `post_processing_gw.py` then remove the pending requests and generates a new `downlink-post-queued.txt` file containing all remaining queued/pending requests
- in case the LoRa gateway is restarted with an existing `downlink-post-queued.txt` file, then these requests will be read and queued for future transmission
- the `downlink-post.txt` file contains a series of lines in JSON format: `{"status":"send_request","dst":3,"data":"/@Px#"}`
- mandatory keys are `"status"`, `"dst"` and `"data"`. `"status"` must be `"send_request"`
- each line must be terminated by `\n` (0x0A). Do not leave an empty line at the end, just `\n` after the last line
- `\r` (0x0D) characters will be removed (some OS/tools use `\r\n` for next line), as well as all empty lines
- you can add other fields for your logging/information purposes but they will not be processed
- `downlink-post.txt` can be created in various ways: interactive mode (e.g. echo), MQTT, ftp, http, Python HTTP server,...
- we provide an example using a simple python HTTP server with upload feature
- the web admin interface can also be used to issue downlink requests
- a `downlink-send.txt` file be appended with the new downlink transmission, marked as `"status":"sent"` or `"status":"sent_fail"` if `lora_gateway` is compiled with `#define KEEP_DOWNLINK_SENT_FILE`
	
## Example

- `["gateway_conf"]["downlink"]=30`

Start the gateway with post-processing features:

	> sudo ./lora_gateway --mode 1 | python post_processing_gw.py
	
Output at gateway when starting

	> sudo ./lora_gateway --mode 1 | python post_processing_gw.py
	...
	2020-01-20 17:17:16.755276
	post downlink: checking for existing downlink/downlink-post-queued.txt
	post downlink: reading existing downlink/downlink-post-queued.txt
	post downlink: start with current list of pending downlink requests
	Loading lib to compute downlink MIC
	Starting thread to check for downlink requests every 30 seconds
	Starting thread to perform fast statistics tasks

	Current working directory: /home/pi/lora_gateway
	SX1276 detected, starting.
	SX1276 LF/HF calibration
	...
	**********Power ON: state 0
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
	Default sync word: 0x12
	Low-level gateway has downlink support
	2020-01-20T17:17:18.581158
	Low-level gw status ON
	...

New `downlink-post.txt` created interactively (to test). Log on the gateway in another terminal

	> cd lora_gateway/downlink
	> echo "{\"status\":\"send_request\",\"dst\":6,\"data\":\"hello from gw\"}" > downlink-post.txt
	
Gateway's output, continued

	...
	Low-level gw status ON
	post downlink: reading downlink/downlink-post.txt
	{u'status': u'send_request', u'dst': 6, u'data': u'hello from gw'}
	post downlink: writing to downlink/downlink-post-queued.txt
	...
	
See how `post_processing_gw.py` process the `downlink/downlink-post.txt` file. The request has been queued and the content of `downlink-post-queued.txt` is
	
	{"status":"send_request","dst":6,"data":"hello from gw"}
	
Then use `Arduino_LoRa_temp`, the address of the device is 6. The configuration is as follows to have a simple non-LoRaWAN device:

	#define WITH_EEPROM
	#define STRING_LIB
	#define LOW_POWER
	#define LOW_POWER_HIBERNATE
	#define SHOW_LOW_POWER_CYCLE
	#define WITH_RCVW
	
	///////////////////////////////////////////////////////////////////
	// CHANGE HERE THE NODE ADDRESS BETWEEN 2 AND 255
	uint8_t node_addr=6;
	//////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////
	// CHANGE HERE THE LORA MODE
	#define LORAMODE  1
	//////////////////////////////////////////////////////////////////	


`Arduino_LoRa_temp` illustrates an example where the device opens a receive window after every transmission (`#define WTH_RCVW`) to wait for downlink message coming from the gateway.

	LoRa temperature sensor, extended version
	Arduino Pro Mini detected
	ATmega328P detected
	SX1276 detected, starting
	SX1276 LF/HF calibration
	...
	Get back previous sx1272 config
	Using packet sequence number of 110
	Forced to use default parameters
	Using node addr of 6
	Using idle period of 10
	Setting Mode: state 0
	Setting frequency: state 0
	Setting Power: state 0
	Setting node addr: state 0
	SX1272 successfully configured
	Mean temp is 22.50
	Sending \!TC/22.50
	Real payload size is 10
	--> CS1
	--> CAD 549
	OK1
	End send: 8560
	LoRa pkt size 10
	LoRa pkt seq 113
	LoRa Sent in 1405
	LoRa Sent w/CAD in 1956
	Packet sent, state 0
	...
	
Gateway's output continued	

	...
	--- rxlora. dst=1 type=0x10 src=6 seq=110 len=10 SNR=6 RSSIpkt=-11 BW=125 CR=4/5 SF=12
	2020-01-20T17:19:59.294221
	rcv ctrl pkt info (^p): 1,16,6,110,10,6,-11
	splitted in:  [1, 16, 6, 110, 10, 6, -11]
	(dst=1 type=0x10(DATA) src=6 seq=110 len=10 SNR=6 RSSI=-11)
	post downlink: receive from 6 with pending request
	in unicast mode
	post downlink: downlink data is "hello from gw"
	post downlink: generate downlink/downlink.txt from entry
	{"status":"send_request","dst":6,"data":"hello from gw"}
	did not find device 0x00000006 in device key list
	using AppSKey and NwkSKey from default device
	post downlink: write
	{"status": "send_request", "dst": 6, "MIC3": "0x33", "MIC2": "0x27", "MIC1": "0x8e", "MIC0": "0x11", "data": "hello from gw"}
	rcv ctrl radio info (^r): 125,5,12,865199
	splitted in:  [125, 5, 12, 865199]
	(BW=125 CR=5 SF=12)
	rcv timestamp (^t): 2020-01-20T17:19:59.292992

	adding time zone info
	new rcv timestamp (^t): 2020-01-20T17:19:59.292992+01:00
	got first framing byte
	--> got LoRa data prefix
	+++ rxlora[865199]. dst=1 type=0x10 src=6 seq=110 len=10 SNR=6 RSSIpkt=-11 BW=125 CR=4/5 SF=12
	valid app key: accept data
	number of enabled clouds is 1
	--> cloud[0]
	uploading with python CloudThingSpeak.py
	python CloudThingSpeak.py "TC/22.50" "1,16,6,110,10,6,-11" "125,5,12,865199" "2020-01-20T17:19:59.292992+01:00" "0000B827EBCE23C2"
	ThingSpeak: uploading (multiple)
	rcv msg to log (\!) on ThingSpeak ( default , default ): 
	ThingSpeak: will issue curl cmd
	curl -s -k -X POST --data field1=22.50 https://api.thingspeak.com/update?key=SGSH52UGPVAUYG3S
	ThingSpeak: returned code from server is 1008058
	--> cloud end
	...
		
It means that `lora_gateway` program received the packet from device 6, and `post_processing_gw.py` will generate the `downlink.txt` file as there is one pending downlink request for device 6. After about 1.5s, `lora_gateway` will look for the `downlink/downlink.txt` file as illustrated below:
	
	...
	-----------------------------------------------------
	Check for downlink requests 2020-01-20T17:20:00.345958

	Read downlink: 126
	Downlink entry: {"status": "send_request", "dst": 6, "MIC3": "0x33", "MIC2": "0x27", "MIC1": "0x8e", "MIC0": "0x11", "data": "hello from gw"}
	Process downlink request
	dst = 6
	--> CAD duration 201
	OK1
	--> RSSI -126
	Packet sent, state 0
	-----------------------------------------------------
	...
	
Content of `downlink-backup-2016-10-10T11:41:20.txt`

	{"status":"send_request","dst":6,"data":"hello from gw"}	
		
Content of `downlink-queued.txt`

	2016-10-10T11:41:20.300 {"status":"queued","dst":6,"data":"hello from gw"}

Content of `downlink-sent.txt`

	2016-10-10T11:41:20.502 {"status":"sent","dst":6,"data":"hello from gw"}	
	
At device 6, we can see the received message

	...
	Wait for incoming packet
	^p6,16,1,0,13,7,-47
	hello from gw	

The `Arduino_LoRa_temp` template shows for instance how an '/@L2#' command from the gateway can be parsed to toggle a LED connected to digital pin 2 and also how the device can set (change) its address when receiving the '/@Ax#' command. For instance, sending '/@A10#' to a device will set its address to 10. As a device already has a hardcoded address (the default one indicated in the `Arduino_LoRa_temp example` is 6) it is possible to use the broadcast address, i.e. 0, to send to the device if you don't know its exact address. The procedure is as follows to set the device's address to 10:
	
1. start you gateway
2. create a `downlink-post.txt` with a **single** entry
	> cd lora_gateway/downlink
	> echo "{\"status\":\"send_request\",\"dst\":0,\"data\":\"/@A10#\"}" > downlink-post.txt
3. wait at least for the downlink timer declared in `gateway_conf.json`, e.g. "downlink" : 60	
4. once the gateway has queued this downlink request, you can switch on your device
5. when power on, the device will transmit a first sample, then will open a receive window
6. as the downlink request specifies a broadcast address, the gateway will broadcast the downlink message
7. as the message is broadcasted, the device will accept the incoming message
8. as the message is "/@A10#", the device will set its address to 10
9. this new address will be saved in EEPROM so that a reset of the board will keep the new address
10. if you have several devices to configure, switch off the previous device and start again at step 2 with a new device

Note that, alternatively, if you don't want to use the downlink feature from the gateway to change a device's address, you can use an interactive device dedicated for this task. In this case, switch on your device (you can check if the gateway receives from the device) and then wait for about 5s (or about 8s if you are not checking reception on the gateway) before issuing the following command "/@D0#/@A10#" to the interactive device. This command means "broadcast /@A10#". To check if the new address has be taken into account, switch off and then switch on your device and check if the gateway receive a message from the device, indicating the new address as the source address.

## Some alternative methods to upload a new `downlink-post.txt` file

### Use the web admin interface

The web admin interface has a `downlink request`tab that can be used to issue downlink requests.

### Use scp

You can use `scp` to copy new `downlink-post.txt` file into the gateway:

	> echo "{\"status\":\"send_request\",\"dst\":6,\"data\":\"reply from gw\"}" > downlink-post.txt
	> scp downlink.txt pi@<gw_ip_addr>:/home/pi/lora_gateway/downlink

### Use the simple Python-based "SimpleHTTPRequestHandler" at https://gist.github.com/UniIsland/3346170. You have to modify it this way as described at https://gist.github.com/rctay/25bed284cd4bcc1477f4/revisions. Line 73:

	- f.write("<br><a href=\"%s\">back</a>" % self.headers['referer'])
	+ if 'referer' in self.headers:
	+ 		f.write("<br><a href=\"%s\">back</a>" % self.headers['referer'])

Then also line 102-105, remove (or comment) first 2 lines. Copy this file into the `downlink` folder and run it in background:

	> python SimpleHTTPServerWithUpload.py &

When you want to upload a new transmission request, create a new `downlink-post.txt` file:

	> echo "{\"status\":\"send_request\",\"dst\":6,\"data\":\"reply from gw\"}" > downlink-post.txt	

Use your web browser on http://<gw_ip_addr>:8000 to view the directory content and upload the new `downlink-post.txt` file using the provided tab. Or with curl-based command line:

	> curl -X POST -F file=@downlink.txt http://<gw_ip_addr>:8000

`post_processing_gw.py` will then queue this request at the next `downlink-post` request check.

### Other?

You can implement many various ways with stronger authentication mechanisms. You can also use MQTT client/server with a Python piece of code to create the `donwlink-post.txt` file.


# Impact on reception reliability

The low-cost gateway program is not multi-threaded. The radio module is anyway not capable of simultaneous reception and transmission. Therefore while the radio is transmitting downlink requests, it is not doing reception. It means that the gateway may not "see" some incoming packets if they are transmitted in the same time interval. Fortunately, if the downlink request is only a few bytes long, the gateway should spend a minimum time to handle a downlink transmissions, e.g. less than 2 seconds when RX1 is used. However, if you know that you do not need the downlink features (that includes OTAA) then set `["gateway_conf"]["downlink"]` to -1 to disable the `lora_gateway` downlink checking after each LoRa packet reception.


# Acknowledgments

**The JSON parser**

The JSON parser is RapidJSON (https://github.com/miloyip/rapidjson). It works great on the Raspberry and on many other platforms. Performances of RapidJSON are reported [here](http://rapidjson.org/md_doc_performance.html).

**Simple HTTP Server With Upload**

Use the simple Python-based "SimpleHTTPRequestHandler" (https://gist.github.com/UniIsland/3346170).


	
Enjoy!
C. Pham	
	