TTN and LoRaWAN
===============

The low-cost gateway can be configured to push data to TheThingsNetwork platform (or any other LoRaWAN Network Server such as ChirpStack for instance). Pushing to TTN/ChirpStack is realized with the legacy UDP-based protocol between the gateway and the network server as initially defined by Semtech. `CloudTTN.py` or `CloudChirpStack.py` is a Python script based on the Pycom NanoGateway Python script to interact with a LoRaWAN Network Server. Note that it is possible to only use encapsulated encrypted mode packet and still be able to push data to TTN/ChirpStack as the encapsulated encrypted payload is formatted like a LoRaWAN packet. It is of course also possible to use the native (encrypted) LoRaWAN mode to both push to TTN/ChirpStack and advertise the gateway as a single-channel LoRaWAN-limited gateway on TTN/ChirpStack (meaning that the low-cost gateway can receive and push data from other LoRaWAN devices - sending on the same LoRa setting BW, SF and frequency - that are registered on TTN/ChirpStack).

The regular gateway id normally contains the 6 bytes of the eth0 MAC address, i.e. 0000B827EBD1B236. The gateway id for TTN/ChirpStack will then be defined as B827EBFFFFD1B236. Then, to configure the TTN/ChirpStack support, you need to proceed as follows:

- for TTN, you need to create an account on TTN. Follow tutorials on Internet. A nice tutorial can be found on [https://medium.com/kkbankol-events/tutorial-build-a-open-source-smart-city-based-on-lora-7ca76b9a098](https://medium.com/kkbankol-events/tutorial-build-a-open-source-smart-city-based-on-lora-7ca76b9a098) but there are others

- then you need to register your gateway on TTN/ChirpStack (using the gateway TTN/ChirpStack id, B827EBFFFFD1B236 for instance). Follow tutorials on Internet
	- it is good to mention that it is a low-cost single channel gateway and also indicating the listening frequency
	- see for instance our test gateway: https://console.thethingsnetwork.org/gateways/eui-b827ebffffd1b236
	
- you need to create an application on TTN/ChirpStack. Follow tutorials on Internet

- you need to create a device on TTN/ChirpStack in ABP mode. Follow tutorials on Internet, then

	- get the DevAddr, the Network Session Key and the App Session Key from the TTN/ChirpStack console to fill-in the Arduino example `Arduino_LoRa_temp` to create a device capable of forging encrypted packet.

	- if you want to enable the native LORAWAN mode where the device can communicate to a LoRaWAN gateway (and the low-cost gateway), you have to use native LoRaWAN by uncommenting in `Arduino_LoRa_temp` both `#define WITH_AES` and `#define LORAWAN`. See this [README](https://github.com/CongducPham/LowCostLoRaGw/tree/master/Arduino#lorawan-example-and-support) in the Arduino folder. By default, the device will use SF12BW125 on 868.1MHz.

	- `Arduino_LoRa_LMIC_ABP_BASIC` is an example that uses the Arduino LMIC port and is provided as an example of a full LoRaWAN stack that can communicate with our low-cost gateway as well.

- For the gateway to handle native (encrypted) LoRaWAN packet and push it to TTN/ChirpStack, proceed as follows:
	- set `["radio_conf"]["mode"]` to 11 in `gateway_conf.json` to configure for native (encrypted) LoRaWAN reception on single channel. Gateway will be configured with BW=125MHz, CR=4/5, SF=12 by default. The frequency is set by default to 868.1MHz for BAND868, 923.2MHz for BAND900 and 433.175 for BAND433
	- set `["gateway_conf"]["raw"]` to true in `gateway_conf.json` 
	- set `["gateway_conf"]["aes_lorawan"]` to false as encrypted data will be pushed to TTN/ChirpStack
	- set `["gateway_conf"]["downlink_lorawan"]` to true to enable periodic PULL_DATA and get PULL_RESP (downlink messages) from Network Server
	- indicate in `["gateway_conf"]["downlink_network_server"]` the address of the Network Server for downlink messages 
	- `python CloudTTN.py` and `python CloudChirpStack.py` have been be added in the LoRaWAN encrypted cloud section of `clouds.json`. Enable one of them by setting `"enabled"` to `true`

	```
		"lorawan_encrypted_clouds" : [
			{	
				"name":"TheThingsNetwork cloud",
				"script":"python CloudTTN.py",
				"type":"TTN",			
				"enabled":true			
			},
			{	
				"name":"ChirpStack network server",
				"script":"python CloudChirpStack.py",
				"type":"lorawan",			
				"enabled":false
			},				
			...	
		]
	```	

- if you also want to be able to locally decrypt and handle the packet with other clouds, then you can set `["gateway_conf"]["aes_lorawan"]` to true and provide in `key_LoRaWAN.py` both the Network Session Key and the App Session Key from the TTN/ChirpStack console. 

- if you want to use the encapsulated encrypted mode with TTN/ChirpStack, uncomment in `Arduino_LoRa_temp` both `#define WITH_AES` and `#define EXTDEVADDR` (and leave `#define LORAWAN` commented) to enable only encapsulated encrypted mode. Communication between the device and the gateway uses our header where the addressing still use the 1-byte address (the LoRaWAN-like packet is encapsulated in our packet). In the encapsulated mode, the device can only communicate to our gateway, not to a LoRaWAN gateway. However, you have to indicate the 4-byte DevAddr to make the link with the device declared on the TTN/ChirpStack console. 

```
	///////////////////////////////////////////////////////////////////
	// LORAWAN OR EXTENDED DEVICE ADDRESS FOR LORAWAN CLOUD
	#if defined LORAWAN || defined EXTDEVADDR
	///////////////////////////////////////////////////////////////////
	//ENTER HERE your Device Address from the TTN device info (same order, i.e. msb). Example for 0x12345678
	unsigned char DevAddr[4] = { 0x12, 0x34, 0x56, 0x78 };
	///////////////////////////////////////////////////////////////////
```

- For the gateway to handle encapsulated encrypted packet and push it to TTN/ChirpStack, proceed as follows:
	- set `["gateway_conf"]["raw"]` to true in `gateway_conf.json` 
	- set `["gateway_conf"]["aes"]` to false to push the encrypted data on TTN/ChirpStack	
	- `python CloudTTN.py` and `python CloudChirpStack.py` have been be added in the LoRaWAN encrypted cloud section of `clouds.json`. Enable one of them by setting `"enabled"` to `true`

	```
		"encrypted_clouds" : [
			{	
				"name":"TheThingsNetwork cloud",
				"script":"python CloudTTN.py",
				"type":"TTN",			
				"enabled":true			
			},
			{	
				"name":"ChirpStack network server",
				"script":"python CloudChirpStack.py",
				"type":"lorawan",			
				"enabled":false
			},				
			...	
		]
	```	

- if `["radio_conf"]["mode"]` is 11 **AND** `["status_conf"]["ttn_status"]`/`["status_conf"]["cs_status"]` is true then `scripts/lorawan_stats/ttn_stats.py`/`scripts/lorawan_stats/ttn_stats.py` will be called periodically by the `post_status_processing_gw.py` status loop (which is called by `post-processing_gw.py`) to report the gateway status on TTN or ChirpStack respectively. You can therefore always leave `["status_conf"]["ttn_status"]`/`["status_conf"]["cs_status"]` to true and status will be reported to TTN/ChirpStack only when you are using LoRaWAN mode, i.e. `["radio_conf"]["mode"]` is 11.

In both cases, i.e. native (encrypted) LoRaWAN and encapsulated encrypted, once everything is set up, go to your TTN/ChirpStack console and search for your device. Go to the `Data` tab to see your packets arriving on TTN/ChirpStack.

Enjoy!
C. Pham 
