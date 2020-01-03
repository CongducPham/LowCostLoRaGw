Using ChirpStack as locally installed Network Server
====================================================

What is it about?
-----------------

The [ChirpStack](https://www.chirpstack.io/) open-source LoRaWAN Network Server can be used locally on the RPI to have a fully autonomous gateway supporting LoRaWAN specifications. This is mainly intended when using an SX1301-based concentrator with our gateway framework as described in [README.md](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/scripts/rak2245-rak831/README.md).

Installation
------------

The ChirpStack installation procedure for the RPI using Rasbian is described [here](https://www.chirpstack.io/guides/debian-ubuntu/). However, there is an installation script proposed by RAK that can automatize the whole process:

- log into your Raspberry gateway as user `pi
- get the entire `rak_common_for_gateway` repository from their github `https://github.com/RAKWireless/rak_common_for_gateway`

	> cd /home/pi
	> svn checkout https://github.com/RAKWireless/rak_common_for_gateway
	
- install the ChirpStack component

	> cd rak_common_for_gateway/chirpstack
	> sudo ./install.sh
	
After installation you will have the ChirpStack Network Server installed locally (127.0.0.1) which can be accessed from a computer on `http://your_raspberry_ip_address:8080`, for instance `http://192.168.2.5:8080`. Login as `admin` with password `admin`.

You can create your device as described [here](https://www.chirpstack.io/application-server/use/devices/).

Incoming LoRaWAN Messages will be uploaded to the local ChirpStack Network Server with the `CloudChirpStack.py` cloud script if it is enabled in `clouds.json`:

```
	"lorawan_encrypted_clouds" : [
		{	
			"name":"TheThingsNetwork cloud",
			"script":"python CloudTTN.py",
			"type":"lorawan",			
			"enabled":true			
		},
		{	
			"name":"ChirpStack network server",
			"script":"python CloudChirpStack.py",
			"type":"lorawan",			
			"enabled":true			
		},
```

`key_ChirpStack.py` simply defines:

```
lorawan_server="127.0.0.1"
lorawan_port=1700
```

Using an SX1301-based concentrator hat
--------------------------------------

If you are using an SX1301-based concentrator hat as explained in [README.md](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/scripts/rak2245-rak831/README.md), you can add the full LoRaWAN support using ChirpStack as follows:

- install, if it is not already done, the SX1301-based concentrator support with instructions [here](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/scripts/rak2245-rak831/README.md) and indicate that it should be started at boot

- go into `scripts/chirpstack` folder and run

	> ./enable_chirpstack.sh
	
then reboot. Enabling ChirpStack will start the ChirpStack services (and also configure them to be started on boot) and also set in `/opt/ttn-gateway/packet_forwarder/lora_pkt_fwd/global_conf.json` "server_address" with "127.0.0.1" to use the local ChirpStack Network Server.

To disable ChirpStack, go into `scripts/chirpstack` folder and run	

	> ./disable_chirpstack.sh
	
Disabling ChirpStack stops the ChirpStack services also set in `/opt/ttn-gateway/packet_forwarder/lora_pkt_fwd/global_conf.json` "server_address" with "router.eu.thethings.network" to use the TTN Network Server.
	
Note
----

The SD card image has already both the SX1301-based concentrator and ChirpStack installed. If you are using our SD card image, just run:

	> cd scripts/rak2245-rak831
	> ./install_lpf.sh
	> cd ../chirpstack/
	> ./enable_chirpstack.sh
	
	
Enjoy! C. Pham
