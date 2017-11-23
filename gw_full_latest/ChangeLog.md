Change logs
===========

November 22th, 2017
------------------

- v3.6 post-processing_gw.py. post_processing_gw.py can now handle the 4-byte LoRaWAN devAddr and pass this address to cloud scripts. The latter has been modified to handle both 1-byte address and 4-byte address. 

- Add Node-Red support. CloudNodeRed.py can be enabled in clouds.json to generate json entries into nodered/nodered.txt that will be followed by a Node-Red flow. Look at README-NodeRed.md for more details.

- Add MQTT suport. CloudMQTT.py can be enabled in clouds.json and key_MQTT.py defines test.mosquitto.org as the MQTT broker for testing.

November 2nd, 2017
------------------

- create_gwid.sh will be called by start_gw.sh (on boot) to create the gateway id. This is done in order to automatically have a valid gateway id when installing a new gateway with the provided SD card image.

- The gateway id SHOULD NOT BE changed from the web admin interface anymore.

- Fix the MD5 digest computation. cmd.sh, scripts/basic_gw_config.sh, scripts/config_gw.sh and scripts/create_gwid.sh are changed.

October 17th, 2017
------------------

- v3.5 post-processing_gw.py. Timestamps are in ISO format with timezone information if dateutil.tz is installed (pip install python-dateutil), otherwise it is the "naïve" version without timezone information.

October 1st, 2017
-----------------

- Add CloudNoInternet.py script that stores received messages into a temporary file when Internet connectivity is down. When Internet is up again, it is possible to upload all stored messages. key_NoInternet.py indicates a script that can be executed with the correct parameters to upload stored data. You can add/enable CloudNoInternet.py cloud in clouds.json.

October 1st, 2017
-----------------

- Start maintaining a ChangeLog.md file





