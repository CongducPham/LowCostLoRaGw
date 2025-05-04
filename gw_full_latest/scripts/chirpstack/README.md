Using ChirpStack as locally installed Network Server
====================================================

What is it about?
-----------------

The [ChirpStack](https://www.chirpstack.io/) open-source LoRaWAN Network Server can run locally on the RPI. With an SX1301-based concentrator hat, as described in [README.md](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/scripts/rak2245-rak831/README.md), you can have a fully autonomous LoRaWAN network. With a single-channel configuration you can also have an autonomous LoRaWAN network but limited to one uplink frequency. The local ChirpStack Network Server can then handle all uplink and downlink messages, including those for Over-The-Air-Activation (OTAA), i.e. join-request and join-accept.

Installation
------------

The ChirpStack installation procedure for the RPI using Rasbian is described [here](https://www.chirpstack.io/guides/debian-ubuntu/). However, there is an installation script proposed by RAK that can automatize the whole process:

- log into your Raspberry gateway as user `pi`
- get the entire `rak_common_for_gateway` repository from their github `https://github.com/RAKWireless/rak_common_for_gateway`

```
	> cd /home/pi
	> svn checkout https://github.com/RAKWireless/rak_common_for_gateway
```	

- install the ChirpStack component

```
	> cd rak_common_for_gateway/chirpstack
	> sudo ./install.sh
```
	
After installation you will have the ChirpStack Network Server installed locally (127.0.0.1) which can be accessed from a computer on `http://your_raspberry_ip_address:8080`, for instance `http://192.168.2.5:8080`. Login as `admin` with password `admin`.

You can create your device as described [here](https://www.chirpstack.io/application-server/use/devices/).

Incoming LoRaWAN messages will be uploaded to the local ChirpStack Network Server with the `CloudChirpStack.py` cloud script if it is enabled in `clouds.json`:

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

Starting ChirpStack
-------------------

If you are using an SX1301-based concentrator hat as explained in [README.md](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/scripts/rak2245-rak831/README.md), you can add the full LoRaWAN support using ChirpStack as follows:

- install, if it is not already done, the SX1301-based concentrator support with instructions [here](https://github.com/CongducPham/LowCostLoRaGw/blob/master/gw_full_latest/scripts/rak2245-rak831/README.md) and indicate that it should be started at boot

If you use the single-channel gateway version, you can just start with:

- go into `scripts/chirpstack` folder and run

```
	> ./enable_chirpstack.sh
```
	
then reboot. Enabling ChirpStack will start the ChirpStack services (and also configure them to be started on boot) and also set in `/opt/ttn-gateway/packet_forwarder/lora_pkt_fwd/global_conf.json` "server_address" with "127.0.0.1" to use the local ChirpStack Network Server.

To disable ChirpStack, go into `scripts/chirpstack` folder and run	

	> ./disable_chirpstack.sh
	
Disabling ChirpStack stops the ChirpStack services and also set in `/opt/ttn-gateway/packet_forwarder/lora_pkt_fwd/global_conf.json` "server_address" with "router.eu.thethings.network" to use the TTN Network Server.

The SD card image has already both the SX1301-based concentrator configuration files and ChirpStack installed, so if you are using our SD card image, you can just use the web admin interface and the `Configure for LoRaWAN` functionality to simply start/stop ChirpStack which is equivalent to:

	> cd ../chirpstack/
	> ./enable_chirpstack.sh

If you enable `ChirpStack cloud` then the Network Server address in `/opt/ttn-gateway/packet_forwarder/lora_pkt_fwd/global_conf.json` is automatically updated with the one indicated in the `Network Server` tab. 

If you have an SX1301-based concentrator multi-channel configuration, you still need to connect with ssh to your gateway and run: 

	> cd scripts/rak2245-rak831
	> ./install_lpf.sh
	
Using only the Network Server
-----------------------------

You can also use a dedicated RPI, noted RPICS, with ChirpStack Network Server enabled only to serve as a LoRaWAN Network Server. If this RPICS is accessible from the Internet with IP address 10.2.7.9 for instance, then on all other RPI that are used as gateways, activate only `CloudChirpStack` and set in `key_ChirpStack.py` :

```
lorawan_server="10.2.7.9"
lorawan_port=1700
```

Then, connect to `http://10.2.7.9:8080` to administrate your ChirpStack Network Server, add devices,... This configuration would be very similar to a TTN scenario where the TTN Network Server will be replaced by your private ChirpStack Network Server.

Output example
--------------

There are some output examples and screenshoot to show ChirpStack working with our gateway framework. Look at `output-example-web-conf-1.png`, `output-example-web-conf-2.png`, `output-example.png` for the screenshots and `output-example.txt` for a commented output example showing for both SX1301-based multi-channel and single-channel configurations how downlink and OTAA are realized. 	
	
Enjoy! C. Pham
