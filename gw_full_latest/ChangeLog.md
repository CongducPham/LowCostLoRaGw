Change logs
===========

March 4th, 2018
---------------
- post-processing_gw.py.
	* fix a bug when locally decrypting received data. Now the packet type is updated to remove the `PKT_FLAG_DATA_ENCRYPTED` flag
	* this bug mainly affected `CloudMQTT.py` that can both publish clear and encrypted data
	
February 28th, 2018
-------------------
- v3.8 post-processing_gw.py.
	* `post_status_processing_gw.py` can now compute a LoRaWAN-like MIC (Message Integrity Code) for downlink request so that the low-level gateway program can add the 4-byte MIC in downlink packet. Downlink data are still sent in clear but the end-device can now check the received MIC and compared it with a valid MIC by performing encryption on the clear data.
- v1.8 lora_gateway.cpp.
	* add a 4-byte MIC when sending downlink packet
	* packet type is then set to `PKT_TYPE_DATA | PKT_FLAG_DATA_ENCRYPTED | PKT_FLAG_DATA_DOWNLINK`
- there is no longer `is_binary` flag in SX1272.h, replaced by `is_downlink` flag
- the flags are then from left to right: `ack_requested|encrypted|with_appkey|is_downlink`
 
December 29th, 2017
-------------------

- It is recommended to only store in `clouds.json` minimum information, such as `"name"`, `"script"`, `"type"` and `"enabled"`. Then store in the associated `key*` file additional information as needed. For instance, we removed the `"gammurc_file"` in `clouds.json` to put them in `key_SMS.py`.

December 12th, 2017
-------------------

- v3.7 post-processing_gw.py. 
	* List of app keys has been move to `key_AppKey.py` file. 
	* `post_status_processing_gw.py` will be called periodically by `post_processing_gw.py` (defined by `"status":600` for instance in `gateway_conf.json`, 600 means every 600s). Periodic tasks can then be added into `post_status_processing_gw.py`. For the moment, the gateway will just indicate periodically that it is up and will show its GPS coordinates. In `gateway_conf.json`, a new section called `"status_conf"` can be used to store dedicated information/variable to be used by custom tasks defined in `post_status_processing_gw.py`.
	* `post_processing_gw.py` has been slightly modified to be able to support other low-level gateway bridge (e.g. 802.14.5). Still experimental.
	
November 22th, 2017
-------------------

- v3.6 post-processing_gw.py. `post_processing_gw.py` can now handle the 4-byte LoRaWAN devAddr and pass this address to cloud scripts. All scripts have been modified to handle both 1-byte address and 4-byte address. 

- Add Node-Red support. `CloudNodeRed.py` can be enabled in `clouds.json` to generate json entries into `nodered/nodered.txt` that will be followed by a Node-Red flow. Look at README-NodeRed.md for more details.

- Add MQTT suport. `CloudMQTT.py` can be enabled in `clouds.json` and `key_MQTT.py` defines `test.mosquitto.org` as the MQTT broker for testing.

November 2nd, 2017
------------------

- `create_gwid.sh` will be called by `start_gw.sh` (on boot) to create the gateway id. This is done in order to automatically have a valid gateway id when installing a new gateway with the provided SD card image.

- The gateway id SHOULD NOT BE changed from the web admin interface anymore.

- Fix the MD5 digest computation. `cmd.sh`, `scripts/basic_gw_config.sh`, `scripts/config_gw.sh` and `scripts/create_gwid.sh are changed.

October 17th, 2017
------------------

- v3.5 post-processing_gw.py. Timestamps are in ISO format with timezone information if `dateutil.tz` is installed (`pip install python-dateutil`), otherwise it is the "naïve" version without timezone information.

October 1st, 2017
-----------------

- Add `CloudNoInternet.py` script that stores received messages into a temporary file when Internet connectivity is down. When Internet is up again, it is possible to upload all stored messages. `key_NoInternet.py` indicates a script that can be executed with the correct parameters to upload stored data. You can add/enable `CloudNoInternet.py` cloud in `clouds.json`.

October 1st, 2017
-----------------

- Start maintaining a `ChangeLog.md` file





