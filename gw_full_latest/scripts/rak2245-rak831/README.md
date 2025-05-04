Using an RPI-based LoRaWAN gateway (SX1301)
===========================================

What is it about?
-----------------

The whole low-cost LoRa IoT framework initially developed in the [EU H2020 WAZIUP project](http://www.waziup.eu) proposes a low-cost LoRa solution with a single-channel LoRa gateway approach. On top of the single-channel (`lora_gateway.cpp`) low-level radio bridge program we developed an open source and modular framework (post-processing stage) to build a versatile IoT gateway with many configuration (multi-cloud support, local processing & storage, simple web admin interface, 3G/4G USB Dongle, OLED screen, ...) and customization possibilities (new clouds, new upload mechanisms, new periodic tasks, new hardware,...). The initial purpose of the H2020 WAZIUP project was to empower sub-saharian Africa countries by enabling deployment of low-cost IoT technologies for rural applications, taking into account constraints such as lack of operators and sometimes no Internet connectivity at all.

While radio concentrators such as the Semtech SX1301 used to build multi-channel LoRaWAN gateways were very expensive, thus motivating the single-channel approach in WAZIUP, their price has gone down dramatically in the last years with the maturation of the LoRa ecosystem. Therefore, there are a number of low-cost multi-channel gateways available on the market that embedding the Semtech SX1301 radio concentrator for less than 150€. The early IMST iC880A radio concentrator board is still available but the recent RAK831, or even more recent RAK2245 LoRaWAN front-end boards in a much more compact format, are now very interesting alternatives. There is also the LoRaGo PORT board and there will probably be more choices in the future. 

The purpose of this add-on is to take benefit from a multi-channel LoRaWAN gateway and keep the versatility and flexibility offered by the higher layer of our post-processing stage. Normally, a LoRaWAN gateway uses the Semtech's `lora_pkt_fwd` program which drives that SX1301 module and uses a UDP connection to push received LoRa packet to a LoRaWAN Network Server (such as TTN). Here, we slightly modified `lora_pkt_fwd` to not send UDP packets to the Network Server and use the output to pass information to our post-processing stage. This is the purpose of this add-on and the general approach is as follows:

- a slightly modified version of `lora_pkt_fwd` is run which does not send uplink packets to Network Server
- a `lora_pkt_fwd_formatter.py` python script will translate the output of `lora_pkt_fwd` into the format accepted by our post-processing stage. See [Section 4 and 5](http://cpham.perso.univ-pau.fr/LORA/RPIgateway.html) of our gateway documentation, [README Section "Adding LoRa gateway's post-processing features"](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README.md#adding-lora-gateways-post-processing-features) and the [Low-cost-LoRa-GW-step-by-step](https://github.com/CongducPham/tutorials/blob/master/Low-cost-LoRa-GW-step-by-step.pdf) tutorial for more details.
- the post-processing stage and all the features offered by our low-cost IoT framework are then available unchanged. As the post-processing stage also provide TTN support (see [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-TTN.md)), you can then use the post-processing stage to upload encrypted LoRaWAN packet to TTN but as well as using other clouds (REST, MQTT, FTP, SMS,...) which is the main advantage of our post-processing stage.

For those who dug into more details of our gateway architecture, the original gateway was simply run with:

	> sudo ./lora_gateway | python ./post_processing_gw.py
	
Here, it simply becomes:

	> sudo /opt/ttn_gateway/packet_forwarder/lora_pkt_fwd/lora_pkt_fwd | python ./lora_pkt_fwd_formatter.py | python ./post_processing_gw.py 	

Here is a picture of a RAK831 gateway mounted in an outdoor case running our framework to push data to TTN but also to other clouds such as WAZIUP cloud, ThingSpeak and MQTT channels.

![](https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/rak831-outdoor.jpg)


Installation and test
---------------------

**Use our** [SD card image](https://github.com/CongducPham/LowCostLoRaGw#get-our-sd-card-image) (not the one provided by the RAK gateway if any) because it has everything you need for all the added features. At time of writing, the latest RAK software for RAK2245/RAK831 RPI-based gateway can be obtained from https://github.com/RAKWireless/rak_common_for_gateway. We provide an `install_rak_lora.sh` script that performs a light installation that it is recommended to use with our gateway software instead of the RAK's installer which is more adapted to install a regular LoRaWAN packet forwarder to TTN.

	> sudo ./install_rak_lora.sh
	
It will install the Semtech's `lora_gateway` & `packet_forwarder` packages from github and will also copy some RAK's specific files. Once Semtech's `lora_gateway` and `packet_forwarder` have been installed (in `/opt/ttn-gateway` as for the RAK's installation). Go into `/home/pi/lora_gateway/scripts/rak2245-rak831` and type:

	> ./install_lpf.sh
	
then follow instructions. The script checks for the existence of `/opt/ttn_gateway`.

When prompted for `configure gateway for TTN Y/N ` answer `Y` if you want to use TTN cloud and interface. 

It is **recommended** to first answer `N` when prompted for `run gateway at boot Y/N` (and then `N` at `reboot Y/N`). Then after installation, manually test your RAK gateway with:

	> cd /home/pi/lora_gateway
	> sudo ./start_lpf_pprocessing_gw.sh
	
You should see something like:
	
	Copying configuration file *.json from /opt/ttn-gateway/packet_forwarder/lora_pkt_fwd/
	Writing 0000B827EBDA09D7
	Done
	Creating /home/pi/lora_gateway/gateway_id.md5 file
	Done
	Replacing gw id in /home/pi/lora_gateway/gateway_conf.json
	Done
	Launching lora_pkt_fwd: retry = 1
	ERROR: [main] failed to start the concentrator
	lora_pkt_fwd not running for some reason
	trying to restart
	retry=1. Will retry in 15 seconds
	Launching lora_pkt_fwd: retry = 2
	INFO: [main] concentrator started, packet can now be received
	...

`/opt/ttn-gateway/packet_forwarder/lora_pkt_fwd/global_conf.json` is always the configuration file that will be copied into `/home/pi/lora_gateway`. Then `/home/pi/lora_gateway/global_conf.json` will modified according to local settings, e.g. gateway id, network server address,... As you can see, the radio concentrator might not start at the first attempt therefore there can several retries until the concentrator starts. There are by default a maximum of 10 retries and each retry will be separated by an increasing amount of time until a maximum is reached. We observed that 2 to 6 retries can be necessary.		

Once the concentrator has started, take your favorite Arduino LoRaWAN device using either Arduino LMIC library (see [our code example](https://github.com/CongducPham/LMIC_low_power/tree/master/Arduino_LoRa_LMIC_ABP_temp)) or our LoRa communication library (see our [`Arduino_LoRa_temp` code template](https://github.com/CongducPham/LowCostLoRaGw/tree/master/Arduino/Arduino_LoRa_temp)) with LoRaWAN mode to test the gateway. You should see packets received by the gateway and data uploaded to your TTN console as well (using the `CloudTTN.py` script).

	2019-05-22T17:09:59.626963
	rcv ctrl pkt info (^p): 0,0,0,0,23,10,-27
	splitted in:  [0, 0, 0, 0, 23, 10, -27]
	rawFormat(len=23 SNR=10 RSSI=-27)
	rcv ctrl radio info (^r): 125,5,12,868100
	splitted in:  [125, 5, 12, 868100]
	(BW=125 CR=5 SF=12)
	rcv timestamp (^t): 2019-05-22T15:09:59.625

	adding time zone info
	new rcv timestamp (^t): 2019-05-22T15:09:59.625+02:00
	got first framing byte
	--> got LoRa data prefix
	raw format from LoRa gateway
	LoRaWAN?
	['0x40', '0x24', '0x1f', '0x4', '0x26', '0x80', '0x0', '0x0', '0x1', '0x24', '0x3e', '0x63', '0xdd', '0xd1', '0x23', '0x1', '0x50', '0x7e', '0x6b', '0x43', '0xbe', '0xa7', '0x3b']
	update ctrl pkt info (^p): 256,64,0x26041F24,0,10,10,-27
	--> DATA encrypted: local AES not activated
	--> FYI base64 of LoRaWAN frame w/MIC: QCQfBCaAAAABJD5j3dEjAVB+a0O+pzs=
	--> number of enabled clouds is 1
	--> LoRaWAN encrypted cloud[0]
	uploading with python CloudTTN.py
	python CloudTTN.py "QCQfBCaAAAABJD5j3dEjAVB+a0O+pzs=" "256,64,637804324,0,10,10,-27" "125,5,12,868100" "2019-05-22T15:09:59.625+02:00" "0000B827EBDA09D7"
	CloudTTN: gw id: B827EBFFFFDA09D7
	CloudTTN: Opening UDP socket to router.eu.thethings.network (52.169.76.203) port 1700...
	CloudTTN: Try to forward packet: ^A<CC>:^@<B8>'<EB><FF><FF><DA>     <D7>{"rxpk": [{"stat": 1, "chan": 0, "datr": "SF12BW125", "tmst": 1558537799, "codr": "4/5", "lsnr": 10, "freq": 868.1, "data": "QCQfBCaAAAABJD5j3dEjAVB+a0O+pzs=", "size": 10, "modu": "LORA", "rfch": 0, "time": "2019-05-22T15:09:59.625+02:00", "rssi": -27}]}
	CloudTTN: Received packet: {"rxpk": [{"stat": 1, "chan": 0, "datr": "SF12BW125", "tmst": 1558537799, "codr": "4/5", "lsnr": 10, "freq": 868.1, "data": "QCQfBCaAAAABJD5j3dEjAVB+a0O+pzs=", "size": 10, "modu": "LORA", "rfch": 0, "time": "2019-05-22T15:09:59.625+02:00", "rssi": -27}]}
	--> LoRaWAN encrypted cloud end
	
Once everything seems to work, run again the installation script and answer `Y` to `run gateway at boot Y/N`. You can then choose to reboot or do it later yourself. To start an operational gateway, it is better to reboot and let the system starting from a clean boot. **Note** that when `start_lpf_pprocessing_gw.sh` is started at boot time, it will wait for 60s before trying to start the radio concentrator. So wait at least 1min before checking whether your gateway is visible on the TTN console. 

![](https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/ttn-example.png)

How is it working?
------------------ 

The gateway using the modified version of `lora_pkt_fwd` on top of our post-processing stage can be started at boot time: `/etc/rc.local` is configured during installation with 2 lines at the end:

	/home/pi/lora_gateway/scripts/start_gw.sh startup_only
	/home/pi/lora_gateway/start_lpf_pprocessing_gw.sh wait 2> /home/pi/lora_gateway/start_lpf.log
	
Note that we are not launching the gateway with a system service (using `systemctl enable|start` for instance) because we use pipe redirection to pass information from one stage to another, therefore several processes linked together need to be launched.

The `start_gw.sh` script run with the first line starts all side modules (for instance launching Node-Red if needed, setting up the 3G connection if needed,...) needed by the low-cost LoRa IoT framework except for the single-channel gateway low-level program. In this way, every new functionality added to the low-cost LoRa IoT framework is also available. The second line actually launch the `start_lpf_pprocessing_gw.sh` shell script to start the modified Semtech's `lora_pkt_fwd` with our post-processing stage to benefit from the multi-channel radio support, i.e:

	> sudo /opt/ttn_gateway/packet_forwarder/lora_pkt_fwd/lora_pkt_fwd | python ./lora_pkt_fwd_formatter.py | python ./post_processing_gw.py

However, as the radio concentrator may not start at the first attempt (`retry=1`), `start_lpf_pprocessing_gw.sh` implements a control loop to perform several retries. If you look at the script, there are 5 variables controlling how the attempts are realized:
 
	MAX_RETRY=10
	BACKOFF_RETRY_TIME=15
	MAX_SLEEP_TIME=60
	PID_CHECK_PERIOD=30	
	MIN_RUN_TIME=120
		
`MAX_RETRY` defines the number of retries before giving up, the script starts with `retry=1`. `start_lpf_pprocessing_gw.sh` tries to launch `lora_pkt_fwd` and then checks whether the process is running or not. If the concentrator failed to start, then there is no PID associated to `lora_pkt_fwd`. If a retry is necessary, `start_lpf_pprocessing_gw.sh` will wait `BACKOFF_RETRY_TIME*retry`, e.g. 15s for the first retry, 30s for the second,... until `MAX_SLEEP_TIME` (in seconds) is reached and the time between 2 retries will be set to `MAX_SLEEP_TIME`. 

Once the radio concentrator has been successfully started `start_lpf_pprocessing_gw.sh` keeps monitoring whether `lora_pkt_fwd` is running or not. This is done every `PID_CHECK_PERIOD`. If for some reason `lora_pkt_fwd` has exited, then `start_lpf_pprocessing_gw.sh` will perform the retry procedure. If the execution time of `lora_pkt_fwd` was less than `MIN_RUN_TIME` (in seconds) then `start_lpf_pprocessing_gw.sh` continues with the previous retry number otherwise `retry` will be reset to 1. Again, if the maximum number of retries, `MAX_RETRY`, has been performed unsuccessfully, then the script exits. 	

If you run:

	> ps aux
	
you will typically see these processes: 	

	root      1838  0.0  0.2   4668  2236 pts/0    S    10:55   0:00 /bin/bash ./start_lpf_pprocessing_gw.sh
	root      1910  2.6  0.1   2628  1576 pts/0    S    10:56   2:33 /opt/ttn-gateway/packet_forwarder/lora_pkt_fwd/lora_pkt_fwd
	root      1911  0.0  0.5   8996  5188 pts/0    S    10:56   0:00 python lora_pkt_fwd_formatter.py
	root      1912  0.0  1.3  34800 12604 pts/0    Sl   10:56   0:01 python post_processing_gw.py
	root      1913  0.0  0.6   9256  5928 pts/0    S    10:56   0:00 python log_gw.py

where `start_lpf_pprocessing_gw.sh` is the main script that launches all the other processes as described.

How it works: `lora_pkt_fwd_formatter.py` python script will intercept lines such as:

```
JSON up: {"rxpk":[{"tmst":29641444,"time":"2019-05-27T12:42:40.117758Z","tmms":1242996179117,"chan":0,"rfch":1,"freq":868.100000,"stat":1,"modu":"LORA","datr":"SF12BW125","codr":"4/5","lsnr":10.0,"rssi":-61,"size":23,"data":"QCQfBCaAAAABJD5j3dEiB1B9aI//04Y="}]}
```

and will translate them into the format accepted by our post-processing stage.

Downlink messages
-----------------

As only UDP uplink packet (including join-request) and uplink stats are disabled in the modified version of `lora_pkt_fwd` and replaced by LoRaWAN cloud scripts, all the other features, and especially the downlink (including join-accept for OTAA) feature and GPS position, are preserved.

GPS position
------------

`lora_pkt_fwd_formatter.py` python script will also intercept lines such as:

```
JSON up: {"stat":{"time":"2019-05-27 12:42:44 GMT","lati":43.31416,"long":-0.36415,"alti":191,"rxnb":1,"rxok":1,"rxfw":1,"ackr":0.0,"dwnb":0,"txnb":0}}
```

to update the GPS coordinates in `/home/pi/lora_gateway/gateway_conf.json`.


Running the simpler `util_pkt_logger` in no Internet scenarios
==============================================================

Instead of using `lora_pkt_fwd` which requires Internet connectivity for various purposes, we can use the Semtech's `util_pkt_logger` which simply receives LoRa packets and log them in a CSV file instead. This is particularly useful when deploying a LoRa network for private usage in rural and remote areas with local processing on the gateway. Therefore, we also propose a slightly modified version of `util_pkt_logger` which write to the Unix standard output instead of a file. That allows us to easily re-inject the received LoRa packets into our post-processing stage. The general approach is very similar to what have been explained:

- a slightly modified version of `util_pkt_logger` is run to receive and output received LoRa packet to Unix `stdout`
- a `util_pkt_logger_formatter.py` python script will translate the output of `util_pkt_logger` into the format accepted by our post-processing stage

Installation and test
---------------------

Once Semtech's `lora_gateway` and `packet_forwarder` have been installed (assuming in `/opt/ttn-gateway` similar to the RAK2245/831's installation procedure). Go into `/home/pi/lora_gateway/scripts/rak2245-rak831` and type:

	> ./install_upl.sh
	
then follow instructions. The script checks for the existence of `/opt/ttn_gateway`.

When prompted for `configure gateway for TTN Y/N ` answer `Y` if you want to use TTN cloud and interface. 

It is **recommended** to answer `N` when prompted for `run gateway at boot Y/N` (and then `N` at `reboot Y/N`). Then after installation, manually test your RAK gateway with:

	> cd /home/pi/lora_gateway
	> sudo ./start_upl_pprocessing_gw.sh
	
You should see something like:

	Copying configuration file *.json from /opt/ttn-gateway/packet_forwarder/lora_pkt_fwd/
	Writing 0000B827EBDA09D7
	Done
	Creating /home/pi/lora_gateway/gateway_id.md5 file
	Done
	Replacing gw id in /home/pi/lora_gateway/gateway_conf.json
	Done
	Launching util_pkt_logger: retry = 1
	loragw_pkt_logger: INFO: found global configuration file global_conf.json, trying to parse it
	loragw_pkt_logger: INFO: global_conf.json does contain a JSON object named SX1301_conf, parsing SX1301 parameters
	loragw_pkt_logger: INFO: lorawan_public 1, clksrc 1
	loragw_pkt_logger: INFO: radio 0 enabled (type SX1257), center frequency 867500000, RSSI offset -166.000000, tx enabled 1, tx_notch_freq 0
	loragw_pkt_logger: INFO: radio 1 enabled (type SX1257), center frequency 868500000, RSSI offset -166.000000, tx enabled 0, tx_notch_freq 0
	loragw_pkt_logger: INFO: LoRa multi-SF channel 0 enabled, radio 1 selected, IF -400000 Hz, 125 kHz bandwidth, SF 7 to 12
	loragw_pkt_logger: INFO: LoRa multi-SF channel 1 enabled, radio 1 selected, IF -200000 Hz, 125 kHz bandwidth, SF 7 to 12
	loragw_pkt_logger: INFO: LoRa multi-SF channel 2 enabled, radio 1 selected, IF 0 Hz, 125 kHz bandwidth, SF 7 to 12
	loragw_pkt_logger: INFO: LoRa multi-SF channel 3 enabled, radio 0 selected, IF -400000 Hz, 125 kHz bandwidth, SF 7 to 12
	loragw_pkt_logger: INFO: LoRa multi-SF channel 4 enabled, radio 0 selected, IF -200000 Hz, 125 kHz bandwidth, SF 7 to 12
	loragw_pkt_logger: INFO: LoRa multi-SF channel 5 enabled, radio 0 selected, IF 0 Hz, 125 kHz bandwidth, SF 7 to 12
	loragw_pkt_logger: INFO: LoRa multi-SF channel 6 enabled, radio 0 selected, IF 200000 Hz, 125 kHz bandwidth, SF 7 to 12
	loragw_pkt_logger: INFO: LoRa multi-SF channel 7 enabled, radio 0 selected, IF 400000 Hz, 125 kHz bandwidth, SF 7 to 12
	loragw_pkt_logger: INFO: LoRa standard channel enabled, radio 1 selected, IF -200000 Hz, 250000 Hz bandwidth, SF 7
	loragw_pkt_logger: INFO: FSK channel enabled, radio 1 selected, IF 300000 Hz, 125000 Hz bandwidth, 50000 bps datarate
	loragw_pkt_logger: INFO: global_conf.json does contain a JSON object named gateway_conf, parsing gateway parameters
	loragw_pkt_logger: INFO: gateway MAC address is configured to 0000000000000000
	loragw_pkt_logger: INFO: found local configuration file local_conf.json, trying to parse it
	loragw_pkt_logger: INFO: local_conf.json does not contain a JSON object named SX1301_conf
	loragw_pkt_logger: INFO: local_conf.json does contain a JSON object named gateway_conf, parsing gateway parameters
	loragw_pkt_logger: INFO: gateway MAC address is configured to B827EBFFFEDA09D7
	loragw_pkt_logger: ERROR: failed to start the concentrator
	util_pkt_logger not running for some reason
	trying to restart
	retry=1. Will retry in 15 seconds
	**Launching util_pkt_logger: retry = 2**
	loragw_pkt_logger: INFO: found global configuration file global_conf.json, trying to parse it
	loragw_pkt_logger: INFO: global_conf.json does contain a JSON object named SX1301_conf, parsing SX1301 parameters
	loragw_pkt_logger: INFO: lorawan_public 1, clksrc 1
	loragw_pkt_logger: INFO: radio 0 enabled (type SX1257), center frequency 867500000, RSSI offset -166.000000, tx enabled 1, tx_notch_freq 0
	loragw_pkt_logger: INFO: radio 1 enabled (type SX1257), center frequency 868500000, RSSI offset -166.000000, tx enabled 0, tx_notch_freq 0
	loragw_pkt_logger: INFO: LoRa multi-SF channel 0 enabled, radio 1 selected, IF -400000 Hz, 125 kHz bandwidth, SF 7 to 12
	loragw_pkt_logger: INFO: LoRa multi-SF channel 1 enabled, radio 1 selected, IF -200000 Hz, 125 kHz bandwidth, SF 7 to 12
	loragw_pkt_logger: INFO: LoRa multi-SF channel 2 enabled, radio 1 selected, IF 0 Hz, 125 kHz bandwidth, SF 7 to 12
	loragw_pkt_logger: INFO: LoRa multi-SF channel 3 enabled, radio 0 selected, IF -400000 Hz, 125 kHz bandwidth, SF 7 to 12
	loragw_pkt_logger: INFO: LoRa multi-SF channel 4 enabled, radio 0 selected, IF -200000 Hz, 125 kHz bandwidth, SF 7 to 12
	loragw_pkt_logger: INFO: LoRa multi-SF channel 5 enabled, radio 0 selected, IF 0 Hz, 125 kHz bandwidth, SF 7 to 12
	loragw_pkt_logger: INFO: LoRa multi-SF channel 6 enabled, radio 0 selected, IF 200000 Hz, 125 kHz bandwidth, SF 7 to 12
	loragw_pkt_logger: INFO: LoRa multi-SF channel 7 enabled, radio 0 selected, IF 400000 Hz, 125 kHz bandwidth, SF 7 to 12
	loragw_pkt_logger: INFO: LoRa standard channel enabled, radio 1 selected, IF -200000 Hz, 250000 Hz bandwidth, SF 7
	loragw_pkt_logger: INFO: FSK channel enabled, radio 1 selected, IF 300000 Hz, 125000 Hz bandwidth, 50000 bps datarate
	loragw_pkt_logger: INFO: global_conf.json does contain a JSON object named gateway_conf, parsing gateway parameters
	loragw_pkt_logger: INFO: gateway MAC address is configured to 0000000000000000
	loragw_pkt_logger: INFO: found local configuration file local_conf.json, trying to parse it
	loragw_pkt_logger: INFO: local_conf.json does not contain a JSON object named SX1301_conf
	loragw_pkt_logger: INFO: local_conf.json does contain a JSON object named gateway_conf, parsing gateway parameters
	loragw_pkt_logger: INFO: gateway MAC address is configured to B827EBFFFEDA09D7	
	loragw_pkt_logger: INFO: concentrator started, packet can now be received
	...

`/opt/ttn-gateway/packet_forwarder/lora_pkt_fwd/global_conf.json` is always the configuration file that will be copied into `/home/pi/lora_gateway`. Then `/home/pi/lora_gateway/global_conf.json` will modified according to local settings, e.g. gateway id. We have the same mechanism than previously where `start_upl_pprocessing_gw.sh` tries to launch `util_pkt_logger` and then checks whether the process is running or not. If you run:

	> ps aux
	
you will typically see these processes: 	

	root      1838  0.0  0.2   4668  2236 pts/0    S    10:55   0:00 /bin/bash ./start_upl_pprocessing_gw.sh
	root      1910  2.6  0.1   2628  1576 pts/0    S    10:56   2:33 /opt/ttn-gateway/lora_gateway/util_pkt_logger/util_pkt_logger
	root      1911  0.0  0.5   8996  5188 pts/0    S    10:56   0:00 python util_pkt_logger_formatter.py
	root      1912  0.0  1.3  34800 12604 pts/0    Sl   10:56   0:01 python post_processing_gw.py
	root      1913  0.0  0.6   9256  5928 pts/0    S    10:56   0:00 python log_gw.py

where `start_upl_pprocessing_gw.sh` is the main script that launches all the other processes as described.

How it works: `util_pkt_logger_formatter.py` python script will intercept lines such as:

```
"B827EBFFFEDA09D7","","2019-04-08 14:09:33.445Z",  17482172, 868300000,1, 1,"CRC_OK",20,"LORA",125000,"SF12"  ,"4/5",-109,-10.8,"0110090A-2A2A2A2A-2A2A2A2A-2A2A2A2A-2A2A2A2A"
```

and will translate them into the format accepted by our post-processing stage.

Important notice
----------------

`piShutdown.py` script uses pin 37 and 40 to respectively shutdown and reboot the RPI if these pins are connected to ground (see ChangeLog.md, Nov. 21st, 2018). On the RAK831, pin 40 is used. If you run the image on an RPI with the RAK831 already plugged, your RPI will constantly reboot. Either remove the RAK831 board from the RPI in order to configure your RPI with the aforementioned instructions, or create a file named `rak831.txt` on the `/boot` partition of the SD card (you can create such file from a Windows/MacOS/Linux computer). Once configured, the `piShutdown.py` script is disabled when running with RAK831 or RAK2245 concentrator boards.
 
Enjoy! C. Pham
