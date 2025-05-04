Change logs
===========

Jan 24th, 2022
--------------
- `nodogsplash` captive WiFi portal software can be used to automatically display the gateway web page when connecting to the gateway's WiFi access point
	* `splash` folder contains the captive portal files as well as quick documentation on the feature

Oct 2021
--------
- a beta version of an embedded AI framework is available. Download the 64-bit RaspiOS SD image to be flashed on a 32GB class 10 SD card.
- the `AI` folder contains some regression models and a notebook with a regression example application on agricultural data
- the web admin interface has a link to the `Jupyter lab` notebook

Jan 20th, 2021
--------------
- the low-level SPI library on the Raspberry has moved from Libelium's `arduPi` lib to Gordon's `wiringPi` lib
	* the main reason is to prepare the low-level radio bridge to 64-bit architecture as `arduPi` relies heavily on direct memory accesses that are difficult to port
	* on 32-bit OS, `arduPi`can still be used
	* on 32-bit OS, the original wiringPi distribution, although not maintained anymore by the author, can be installed (http://wiringpi.com/)
	* on 64-bit OS, the "UNOFICIAL WiringPi library for RPi 64bit OS" (https://github.com/TheNextLVL/wiringPi) must be installed
	* however, for RPI4, you must take this updated version where RPI4 has been added as detected platform
		- https://github.com/CongducPham/LowCostLoRaGw/blob/master/zip/wiringPi-64bit.tgz
	* for more details see https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-WiringPi-64bit-OS

Oct 28th, 2020
--------------
- v2.2 `lora_gateway.cpp`
	* The low-level LoRa communication lib has moved from Libelium-based lib to Stuart Robinson's SX12XX lib.
		- https://github.com/StuartsProjects/SX12XX-LoRa
		- SX126X, SX127X, and SX128X related files have been ported to run on both Arduino & RaspberryPI environment
		- support for SX1261, SX1262, SX1268, SX1272, SX1276, SX1277, SX1278, SX1279, SX1280, SX1281
		- for more details see https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-SX12XX.md
	* `SX1272_lora_gateway.cpp` is the Libelium-based lib version only for SX1272/76
	* `SX12XX_lora_gateway.cpp` is the Stuart Robinson's SX12XX lib version	

May 19th, 2020
--------------
- v2.1a `lora_gateway.cpp`
	*	uplink and downlink feequency
		- define LORAWAN_UPFQ, LORAWAN_D2FQ and LORAWAN_D2SF to better adapt the custom frequency plan
		- change these parameters for each band definition (BAND868, BAND900 and BAND433)
 
Jan 29th, 2020
---------------
- v2.1 `lora_gateway.cpp`
	* `lora_gateway` in raw mode now checks 4 times for `downlink.txt` after a packet reception at t_r (to work with Network Server sending PULL_RESP in just-in-time mode)
		- first, after t_r+DELAY_DNWFILE (delay of 900ms, no reception possible) to target RX1
		- second, after t_r+DELAY_DNWFILE+DELAY_EXTDNW2 (additional delay of 1000ms, no reception possible) to target RX2
		- third, after t_r+DELAY_JACC1-DELAY_DNWFILE (radio in reception mode) to target RX1 for join-accept only
			* if a new packet is received during the last wait period then it has priority
			* lora_gateway then starts a new cycle of downlink attempts
		- last, after t_r+DELAY_JACC1+DELAY_EXTDNW2-DELAY_DNWFILE (additional delay of 1000ms, no reception possible) to target RX2 for join-accept only
 
Jan 27th, 2020
--------------
- v2.0 `lora_gateway.cpp`
	* add LoRaWAN data downlink transmission from downlink messages from LoRaWAN Network Server
- `scripts/start_gw.sh`	
	* `gateway_conf.json:"gateway_conf":"downlink_lorawan" : true` enables periodic PULL_DATA on Network Server to get downlink messages
		- `scripts/lorawan_stats/downlink_lorawan.py` will be launched by `scripts/start_gw.sh` to issue PULL_DATA and handle PULL_RESP
		- `gateway_conf.json:"gateway_conf":"downlink_network_server" indicates a LoRaWAN Network Server for `downlink_lorawan.py` 
		- more details in the dedicated [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-downlink.md) that has been updated	
	* remove old and unused code and only targets RPI-based gateway
	
Dec 24th, 2019
--------------
- when using a RAK831/2245 SX1301-based hat with our `lora_pkt_fwd_formatter.py` formatter script to provide multi-channel support using a modified version of Semtech's `lora_pkt_fwd`, we tested that a LoRaWAN device (commercial or LMIC-based) is fully supported (uplink, downlink, in both ABP and OTAA mode).
- `scripts/ttn` has been renamed to `scripts/lorawan_stats`
- the gateway can also interact with the ChirpStack open-source LoRaWAN Network Server. Tests have been realized with a locally installed ChirpStack NS (127.0.0.1).
	* `CloudChirpStack.py`, `key_ChirpStack.py` and `scripts/lorawan_stats/cs_stats.py` are added. They are very similar to the TTN scripts but we choose to duplicate them so that both TTN and ChirpStack can be used at the same time if this is desirable.
	* `scripts/chirpstack` has been added and contains shell scripts to start/stop ChirpStack
	* see `scripts/chirpstack/README.md` for more details
- the gateway web admin interface has been improved and also includes support of ChirpStack	


Dec 12th, 2019
--------------
- if there is a `lora_gateway` folder in the boot partition then its whole content is copied into `/home/pi/lora_gateway`. This is usefull when you want to configure the gateway by writing on the SD card from a computer. This is done only once as the folder is renamed after its content have been copied. 
- change network configuration style to use `dhcpcd.conf`
- add `scripts/compile_lora_gateway.sh` script to mutualize the compilation procedure
- `scripts/start_gw.sh` will automatically recompile the low-level gateway program if the RPI model has changed from the one it has been compiled for
- `post_processing_gw.py`
	* when receiving packet from the low-level LoRa bridge (either from single `lora_gateway` or Semtech `lora_pkt_fwd`), `post_processing_gw.py` will print a summary line beginning with `+++ rxlora`. This pattern is used by the web admin packet logger page.
		- `+++ rxlora[865199]. dst=1 type=0x12 src=3 seq=140 len=77 SNR=5 RSSIpkt=-43 BW=500 CR=4/5 SF=12`
		- `+++ rxlora[868100]. lorawan type=0x40 src=0x26041F24 seq=59 len=10 SNR=10 RSSIpkt=-42 BW=125 CR=4/5 SF=12`
			
Dec 4th, 2019
--------------
- web admin interface
	* use toggle switch for boolean configuration items (`bootstrap-toggle`)
	* add `Cloud TTN` tab to handle `CloudTTN.py` which is the script to upload to TTN platform
	* if an image file called `my_logo_1.png` is present in the `pages` folder then the image will be displayed as logo brand for instance. Replace the existing image with yours or simply delete this image.

Nov 30th, 2019
--------------
- web admin interface
	* add Internet online/offline indicator
	* add simple packet logger (display last 20 LoRa packet receptions with a refresh rate of 5s)
	* add indication of last low-level status and last rxlora in radio tab
	* add cloud server online/offline indicator in cloud tabs as well as last upload time per cloud
- `CloudNoInternet.py`
	* a maximum number of pending messages can be defined (set to 10 by default) to limit the number of uploads per received message
- `post_status_processing_gw.py`
	* `gateway_conf.json:"status_conf":"check_internet_pending": true` can be used to trigger upload of Internet pending messages in a periodic manner (check will be performed every `gateway_conf.json:"gateway_conf":"status` 	
	
Nov 20th, 2019
--------------
- ported to Raspbian Buster
- v1.9a `lora_gateway.cpp` 
	* handle getopt issue on newer Linux distrib, compile with `-DGETOPT_ISSUE`

Nov 11th, 2019
--------------
- rename `loraWAN.py` in `decrypt_LoRaWAN.py` and `loraWAN_config.py` in `key_LoRaWAN.py`. Allow LoRaWAN packet to be decoded locally with `aes_lorawan` option in `gateway_conf.json`. This is now separated from the encapsulated encrypted mode which can be enabled with `aes` option in `gateway_conf.json`. For the encapsulated encrypted mode, `decrypt_AES.py` and `key_AES.py` are defined.
- `key_LoRaWAN.py` defines a Python dictionary where you can add your device id (4-byte device address) and the associated NwkSKey and AppSKey. 
- similarly, and to maintain consistency in naming convention, `LSC_decrypt.py` has been renamed in `decrypt_LSC.py` and the key is stored in `key_LSC.py`
- in most of `Cloud*` scripts, the `sys.dont_write_bytecode = True` is added to disable byte code compilation of imported modules, thus enabling dynamic changes in `key_*` files.
- v3.9b `post_processing_gw.py`
	* now, it is possible to upload a LoRaWAN packet to TTN for instance, and still be able to decrypt locally the packet to inject the clear data for upload to normal clouds (such as ThingSpeak or WAZIUP cloud for instance).
	* raw mode is forced to true at post-processing level when LoRa mode is set to 11 (LoRaWAN mode)
- `MQTT_server` field in `key_MQTT.py` can now specify a port number, i.e. `test.mosquitto.org:1234`, and `CloudMQTT.py` can add the `-p` parameter when running the `mosquitto_pub` command.	

Oct 30th, 2019
--------------
- Update web admin gateway interface for view and update `global_conf.json` configuration file for SX1301-based gateway 
- Update `CloudTTN.py` to use 2019-10-28T10:28:02.0585678Z date format 

May 22nd, 2019
--------------
- add support of RPI-based LoRaWAN gateway (SX1301+`lora_gateway`+`packet_forwarder`) such as RAK831/2245
	- benefit from the open, versatile, multi-cloud Low-cost LoRa IoT framework with the flexibility and scalability of a multi-channel LoRaWAN gateway
	- you need to install the Semtech's `lora_gateway` and `packet_forwarder` packages
	- for RAK831: see https://github.com/RAKWireless/RAK2245-RAK831-LoRaGateway-RPi-Raspbian-OS
	- then run `scripts/rak2245-rak831/install_lpf.sh`
		- `cd /home/pi/lora_gateway/scripts/rak2245-rak831`
		- `sudo ./install_lpf.sh`
	- a slightly modified version of `lora_pkt_fwd` is used to inject LoRa packets into the post-processing stage		
	- there is also a slightly modified version of `util_pkt_logger` for no Internet connection deployment scenarios
	- more details in the dedicated [README](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/scripts/rak2245-rak831/README.md)	

May 15th, 2019
--------------
- fast statistics is controlled by `gateway_conf.json:"status_conf":"fast_stats: 15`
	- 0 will disable
	- a value greater than 0, expressed in seconds, will trigger periodic fast statistics tasks
	- for fast statistic details refer to change logs of December 12th, 2018
- when using the web admin interface to download a .zip file, it is possible to erase the current install first
	- this feature is added to allow installing a specific gateway version from a .zip archive in `/home/pi/lora_gateway`
	- when building the archive, include a file named `VERSION.txt` which content will be displayed as the current installed version
	- when performing a new install or a full update from the official github, then `/home/pi/lora_gateway/VERSION.txt` will be deleted	

April 25th, 2019
----------------
- move DHT22 and copy log file task from `post_processing_gw.py` into `post_status_processing_gw.py`
	- `gateway_conf.json:"gateway_conf"` has no more "dht22" nor "dht22_mongo" fields. These are moved to `gateway_conf.json:"status_conf"` section

March 26rd, 2019
----------------
- add support of TheThingsNetwork (TTN): `CloudTTN.py`, `key_TTN.py` and `scripts/ttn/ttn_stats.py` are added
- the regular gateway id has been changed from 00000027EBD1B236 (for instance) to 0000B827EBD1B236 to take the 6 bytes of the eth0 MAC address
- the gateway id for TTN will then be defined as B827EBFFFFD1B236
- there is a dedicated README file for TTN support and configuration
	- https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/README-TTN.md
- you need to create an account on TTN. Follow tutorials on Internet
- you need to register your gateway on TTN first (using the gw TTN id). Follow tutorials on Internet
	- it is good to mention that it is a low-cost single channel gateway and also indicating the listening frequency
	- see for instance our test gateway: https://console.thethingsnetwork.org/gateways/eui-b827ebffffd1b236
- you need to create an application on TTN. Follow tutorials on Internet
- you need to create a device on TTN in ABP mode. Follow tutorials on Internet
- get the DevAddr, the Network Session Key and the App Session Key from the TTN console to fill-in the Arduino examples (`Arduino_LoRa_LMIC_ABP_BASIC` or `Arduino_LoRa_temp`) to create a device capable sending LoRaWAN packets
- read change logs of March 23rd, 2019 for additional details 

March 23rd, 2019
----------------
- improved LoRaWAN support
- v1.9 `lora_gateway.cpp`
	* fix bug for LoRaWAN reception by low-cost gateway
- set mode to 11 in `gateway_conf.json` to configure for LoRaWAN reception on single channel
- when mode is set to 11, specific bw, cr and sf in `gateway_conf.json` can be taken into account, so make sure they are set correctly. LoRaWAN mostly uses BW125. You can change SF
- set "raw" to true in `gateway_conf.json` 
- if you want to push to TTN or other LoRaWAN network server, then set "aes" to false
- if you REALLY know what you are doing, you can set "aes" to true if you want local AES decryption
	- but then, indicate both NwkSKey and AppSKey in `loraWAN_config.py`
- BW=125MHz, CR=4/5, SF=12. 868.1MHz for BAND868, 923.2MHz for BAND900 and 433.175 for BAND433 by default
- you can specify another customized frequency if you want. That should match the one of the end-device of course
- special thanks to Fabien Ferrero from U. Nice as finding these bugs was make possible during the test campaign in Danang, Vietnam, where he installed a LoRaWAN gateway on top of the Danang Software Park (DSP) building.
		
January 15th, 2019
-------------------
- v3.9a `post_processing_gw.py`
	* add support for Lightweight Stream Cipher (LSC)
	* add `LSC_decrypt.py` library file that will be called by `post_processing_gw.py`	
	* in `gateway_conf.json` file, `gateway_conf` section, `"lsc" : true` enables local LSC decryption at gateway
	* `post_processing_gw.py` will try to decrypt an encrypted packet with all enabled encryption modules, e.g. AES, LSC, ...
		* this process is done sequentially, module by module
		* as it is most likely that decryption will succeed only when both encryption and decryption algorithms do match, there is normally only one decryption method that will succeed
		* therefore `post_processing_gw.py` will stop when a decryption module succeeds
		* actually, it is mandatory for an encryption module to implement a Message Integrity Check (MIC) procedure at both the sender and receiver side. A correct MIC means that encryption and decryption algorithms match
	* `Arduino_LoRa_temp` has also been updated to be able to send an LSC-encrypted packet
		* comment `WITH_AES` and uncomment `WITH_LSC`
		* the encryption key is defined by `uint8_t LSC_Nonce[16]`
	
December 12th, 2018
-------------------
- v3.8a `post_processing_gw.py`
	* add a fast rate statistic thread that will be launched if _gw_statistics is set to True
	* the thread will periodically (every 5s, currently hard-coded) call `python sensors_in_raspi/stats.py`
	* `stats.py` is mainly based from Adafruit example (https://learn.adafruit.com/adafruit-pioled-128x32-mini-oled-for-raspberry-pi/usage)
	* print some RPI statistics and last packet info, as well as the number of received packet since last restart
	* Note that you can hot-(un)plug the OLED screen at any time. This feature has been added as an additional convenient way for quick debugging
	* If you get the last SD card image, support for I2C and OLED screen has been added.
	* If you update the gateway software, you need before enabling this feature to install the required libs
		* `sudo apt-get install python-imaging python-smbus`
		* `git clone https://github.com/adafruit/Adafruit_Python_SSD1306.git`
		* `cd Adafruit_Python_SSD1306`
		* `sudo python setup.py install`
		* then don't forget to enable i2c interface by launching `sudo raspi-config` (interfacing options)

November 21st, 2018
-------------------
- `scripts/start_gw.sh`
	* run `piShutdown.py` script at startup (use the RPI2 long header by default)
		- to shutdown the gateway properly by connecting GPIO26 (pin 37) to ground which can be pin 39 that is next to pin 37
		- to reboot the gateway by connecting GPIO21 (pin 40) to ground which can be pin 39 that is next to pin 40
		- if you have other usage for GPIO26 or GPIO21, edit `scripts/piShutdown.py` to use other pins for shutdown/reboot

```
          SD card slot side
          
                    +----++----+
                    |  1 ||  2 |
                    | .. || .. |
                    | .. || .. |
shutdown        +-> | 37 || 38 |
                +-> | 39 || 40 |
                    +----++----+
                      ^	    ^
                      +-----+ reboot
                      
            USB and Ethernet port side          
```	

November 13th, 2018
-------------------
- `scripts/start_gw.sh`
	* run `mongo_repair.sh` at startup to repair a damaged database when gateway has been shutdown abruptly.

June 15th, 2018
---------------
- when downloading and installing a .zip file with the web admin interface, the zip file will be uncompressed and its content will overwrite existing files in `/home/pi/lora_gateway`. It is a convenient way to update the gateway by providing a .zip archive.

- `scripts/start_gw.sh` and `scripts/update_gw.sh`
	* manage gateway software version from github
	* `/home/pi/git-VERSION.txt` contains the github version
	* `/home/pi/VERSION.txt` contains the installed version 
	* both versions are shown in the web admin interface
	* if there is `"auto_update":true` in `gateway_conf.json:"gateway_conf"` then on boot (reboot) `scripts/start_gw.sh` will update the gateway to the latest version from github (assuming that the gateway has Internet connectivity).

May 28th, 2018
--------------
- `scripts/start_gw.sh`
	* adds routing rules to share the wired Internet connection (eth0) through the WiFi access point
	* now, a device connected to the gateway's WiFi has Internet connection assuming the gateway has Internet connection with the wired interface
	* these rules are added if the gateway runs in access point mode 
	
April 25th, 2018
----------------
- `scripts/start_gw.sh`
	* run `node-red-start` if `CloudNodeRed.py` is enabled in `clouds.json`

March 4th, 2018
---------------
- `post_processing_gw.py`
	* fix a bug when locally decrypting received data. Now the packet type is updated to remove the `PKT_FLAG_DATA_ENCRYPTED` flag
	* this bug mainly affected `CloudMQTT.py` that can both publish clear and encrypted data
	
February 28th, 2018
-------------------
- v3.8 `post_processing_gw.py`
	* `post_processing_gw.py` can now compute a LoRaWAN-like MIC (Message Integrity Code) for downlink request so that the low-level gateway program can add the 4-byte MIC in downlink packet. Downlink data are still sent in clear but the end-device can now check the received MIC and compared it with a valid MIC by performing encryption on the clear data.
- v1.8 `lora_gateway.cpp`
	* add a 4-byte MIC when sending downlink packet
	* packet type is then set to `PKT_TYPE_DATA | PKT_FLAG_DATA_ENCRYPTED | PKT_FLAG_DATA_DOWNLINK`
- there is no longer `is_binary` flag in SX1272.h, replaced by `is_downlink` flag
- the flags are then from left to right: `ack_requested|encrypted|with_appkey|is_downlink`
 
December 29th, 2017
-------------------
- It is recommended to only store in `clouds.json` minimum information, such as `"name"`, `"script"`, `"type"` and `"enabled"`. Then store in the associated `key*` file additional information as needed. For instance, we removed the `"gammurc_file"` in `clouds.json` to put them in `key_SMS.py`.

December 12th, 2017
-------------------
- v3.7 `post_processing_gw.py`
	* List of app keys has been move to `key_AppKey.py` file. 
	* `post_status_processing_gw.py` will be called periodically by `post_processing_gw.py` (defined by `"status":600` for instance in `gateway_conf.json`, 600 means every 600s). Periodic tasks can then be added into `post_status_processing_gw.py`. For the moment, the gateway will just indicate periodically that it is up and will show its GPS coordinates. In `gateway_conf.json`, a new section called `"status_conf"` can be used to store dedicated information/variable to be used by custom tasks defined in `post_status_processing_gw.py`.
	* `post_processing_gw.py` has been slightly modified to be able to support other low-level gateway bridge (e.g. 802.14.5). Still experimental.
	
November 22th, 2017
-------------------
- v3.6 `post_processing_gw.py`
	* `post_processing_gw.py` can now handle the 4-byte LoRaWAN devAddr and pass this address to cloud scripts. All scripts have been modified to handle both 1-byte address and 4-byte address. 

- Add Node-Red support
	* `CloudNodeRed.py` can be enabled in `clouds.json` to generate json entries into `nodered/nodered.txt` that will be followed by a Node-Red flow. Look at README-NodeRed.md for more details.

- Add MQTT suport
	* `CloudMQTT.py` can be enabled in `clouds.json` and `key_MQTT.py` defines `test.mosquitto.org` as the MQTT broker for testing.

November 2nd, 2017
------------------
- `create_gwid.sh` will be called by `start_gw.sh` (on boot) to create the gateway id. This is done in order to automatically have a valid gateway id when installing a new gateway with the provided SD card image.

- The gateway id SHOULD NOT BE changed from the web admin interface anymore.

- Fix the MD5 digest computation. `cmd.sh`, `scripts/basic_gw_config.sh`, `scripts/config_gw.sh` and `scripts/create_gwid.sh are changed.

October 17th, 2017
------------------
- v3.5 `post_processing_gw.py`
	* Timestamps are in ISO format with timezone information if `dateutil.tz` is installed (`pip install python-dateutil`), otherwise it is the "naïve" version without timezone information.

October 1st, 2017
-----------------
- Add `CloudNoInternet.py` script that stores received messages into a temporary file when Internet connectivity is down. When Internet is up again, it is possible to upload all stored messages. `key_NoInternet.py` indicates a script that can be executed with the correct parameters to upload stored data. You can add/enable `CloudNoInternet.py` cloud in `clouds.json`.

October 1st, 2017
-----------------
- Start maintaining a `ChangeLog.md` file





