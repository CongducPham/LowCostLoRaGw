Change logs
===========

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
- v1.9 lora_gateway.cpp
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
- v3.9 post-processing_gw.py
	* add support for Lightweight Stream Cipher (LSC)
	* add `LSC_decrypt.py` library file that will be called by `post-processing_gw.py`	
	* in `gateway_conf.json` file, `gateway_conf` section, `"lsc" : true` enables local LSC decryption at gateway
	* `post-processing_gw.py` will try to decrypt an encrypted packet with all enabled encryption modules, e.g. AES, LSC, ...
		* this process is done sequentially, module by module
		* as it is most likely that decryption will succeed only when both encryption and decryption algorithms do match, there is normally only one decryption method that will succeed
		* therefore `post-processing_gw.py` will stop when a decryption module succeeds
		* actually, it is mandatory for an encryption module to implement a Message Integrity Check (MIC) procedure at both the sender and receiver side. A correct MIC means that encryption and decryption algorithms match
	* `Arduino_LoRa_temp` has also been updated to be able to send an LSC-encrypted packet
		* comment `WITH_AES` and uncomment `WITH_LSC`
		* the encryption key is defined by `uint8_t LSC_Nonce[16]`
	
December 12th, 2018
-------------------
- v3.8a post-processing_gw.py
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
- scripts/start_gw.sh
	* run `piShutdown.py` script at startup (use the RPI2 long header by default)
		- to shutdown the gateway properly by connecting GPIO26 (pin 37) to ground which can be pin 39 that is next to pin 37
			+-> | 37 || 38 |
			+-> | 39 || 40 |
				+----++----+
				   ^	 ^
				   +-----+ reboot
				   
		- to reboot the gateway by connecting GPIO21 (pin 40) to ground which can be pin 39 that is next to pin 40
		- if you have other usage for GPIO26 or GPIO21, edit `scripts/piShutdown.py` to use other pins for shutdown/reboot
	
November 13th, 2018
-------------------
- scripts/start_gw.sh
	* run `mongo_repair.sh` at startup to repair a damaged database when gateway has been shutdown abruptly.

June 15th, 2018
---------------
- when downloading and installing a .zip file with the web admin interface, the zip file will be uncompressed and its content will overwrite existing files in `/home/pi/lora_gateway`. It is a convenient way to update the gateway by providing a .zip archive.

- scripts/start_gw.sh and scripts/update_gw.sh
	* manage gateway software version from github
	* `/home/pi/git-VERSION.txt` contains the github version
	* `/home/pi/VERSION.txt` contains the installed version 
	* both versions are shown in the web admin interface
	* if there is `"auto_update":true` in `gateway_conf.json:"gateway_conf"` then on boot (reboot) `scripts/start_gw.sh` will update the gateway to the latest version from github (assuming that the gateway has Internet connectivity).

May 28th, 2018
--------------
- scripts/start_gw.sh
	* adds routing rules to share the wired Internet connection (eth0) through the WiFi access point
	* now, a device connected to the gateway's WiFi has Internet connection assuming the gateway has Internet connection with the wired interface
	* these rules are added if the gateway runs in access point mode 
	
April 25th, 2018
----------------
- scripts/start_gw.sh
	* run `node-red-start` if `CloudNodeRed.py` is enabled in `clouds.json`

March 4th, 2018
---------------
- post-processing_gw.py
	* fix a bug when locally decrypting received data. Now the packet type is updated to remove the `PKT_FLAG_DATA_ENCRYPTED` flag
	* this bug mainly affected `CloudMQTT.py` that can both publish clear and encrypted data
	
February 28th, 2018
-------------------
- v3.8 post-processing_gw.py
	* `post_status_processing_gw.py` can now compute a LoRaWAN-like MIC (Message Integrity Code) for downlink request so that the low-level gateway program can add the 4-byte MIC in downlink packet. Downlink data are still sent in clear but the end-device can now check the received MIC and compared it with a valid MIC by performing encryption on the clear data.
- v1.8 lora_gateway.cpp
	* add a 4-byte MIC when sending downlink packet
	* packet type is then set to `PKT_TYPE_DATA | PKT_FLAG_DATA_ENCRYPTED | PKT_FLAG_DATA_DOWNLINK`
- there is no longer `is_binary` flag in SX1272.h, replaced by `is_downlink` flag
- the flags are then from left to right: `ack_requested|encrypted|with_appkey|is_downlink`
 
December 29th, 2017
-------------------
- It is recommended to only store in `clouds.json` minimum information, such as `"name"`, `"script"`, `"type"` and `"enabled"`. Then store in the associated `key*` file additional information as needed. For instance, we removed the `"gammurc_file"` in `clouds.json` to put them in `key_SMS.py`.

December 12th, 2017
-------------------
- v3.7 post-processing_gw.py
	* List of app keys has been move to `key_AppKey.py` file. 
	* `post_status_processing_gw.py` will be called periodically by `post_processing_gw.py` (defined by `"status":600` for instance in `gateway_conf.json`, 600 means every 600s). Periodic tasks can then be added into `post_status_processing_gw.py`. For the moment, the gateway will just indicate periodically that it is up and will show its GPS coordinates. In `gateway_conf.json`, a new section called `"status_conf"` can be used to store dedicated information/variable to be used by custom tasks defined in `post_status_processing_gw.py`.
	* `post_processing_gw.py` has been slightly modified to be able to support other low-level gateway bridge (e.g. 802.14.5). Still experimental.
	
November 22th, 2017
-------------------
- v3.6 post-processing_gw.py
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
- v3.5 post-processing_gw.py
	* Timestamps are in ISO format with timezone information if `dateutil.tz` is installed (`pip install python-dateutil`), otherwise it is the "naïve" version without timezone information.

October 1st, 2017
-----------------
- Add `CloudNoInternet.py` script that stores received messages into a temporary file when Internet connectivity is down. When Internet is up again, it is possible to upload all stored messages. `key_NoInternet.py` indicates a script that can be executed with the correct parameters to upload stored data. You can add/enable `CloudNoInternet.py` cloud in `clouds.json`.

October 1st, 2017
-----------------
- Start maintaining a `ChangeLog.md` file





