Using Node-Red
==============

This README describes a simple Node-Red support for our low-cost LoRa gateway. It can be improved in many ways, according to your needs. The current implementation comes with a Node-Red flow that takes the received messages from the gateway, through a file created by a Node-Red dedicated cloud script, to publish data on a topic.

Starting Node-Red on you Raspberry gateway
------------------------------------------

The SD card image has Node-Red installed. Start Node-Red with

	> node-red-start
	
Use your browser on `http://192.168.2.8:1880` for instance if this is your gateway's IP address. Make sure that your gateway has internet connection.

Importing the Node-Red flow
---------------------------

This is the flow that you can import (copy and then Import/Clipboard):

[{"id":"603b4928.cc6418","type":"tail","z":"a4273e8d.ebc9f","name":"nodered.txt","filetype":"text","split":true,"filename":"/home/pi/lora_gateway/nodered/nodered.txt","x":104.70001220703125,"y":257.0500183105469,"wires":[["79bdb931.346a6"]]},{"id":"79bdb931.346a6","type":"json","z":"a4273e8d.ebc9f","name":"","x":258.70001220703125,"y":332.75,"wires":[["64923ae1.40f7ac","86c4a66a.456fd"]]},{"id":"74c63b2c.980bb4","type":"mqtt out","z":"a4273e8d.ebc9f","name":"","topic":"","qos":"","retain":"","broker":"10144d51.2d96db","x":622.7000122070312,"y":256.3000183105469,"wires":[]},{"id":"64923ae1.40f7ac","type":"debug","z":"a4273e8d.ebc9f","name":"","active":true,"console":"false","complete":"payload","x":429.6999816894531,"y":420.3000183105469,"wires":[]},{"id":"86c4a66a.456fd","type":"function","z":"a4273e8d.ebc9f","name":"set topic and payload","func":"msg.topic=msg.payload.source+'/'+msg.payload.measure\nmsg.payload=msg.payload.value\nreturn msg;","outputs":1,"noerr":0,"x":437.7000427246094,"y":277.8500061035156,"wires":[["74c63b2c.980bb4","514a5140.c56c88"]]},{"id":"514a5140.c56c88","type":"debug","z":"a4273e8d.ebc9f","name":"","active":true,"console":"false","complete":"true","x":618.699951171875,"y":354.70001220703125,"wires":[]},{"id":"10144d51.2d96db","type":"mqtt-broker","z":"","broker":"test.mosquitto.org","port":"1883","clientid":"","usetls":false,"compatmode":true,"keepalive":"60","cleansession":true,"willTopic":"","willQos":"0","willPayload":"","birthTopic":"","birthQos":"0","birthPayload":""}]

![](https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/nodered-flow.png)

Principles
----------

`CloudNodeRed.py` will be called by `post-processing-gw.py` if a Node-Red cloud is enabled in `clouds.json`:

	{	
		"name":"NodeRed flow",
		"script":"python CloudNodeRed.py",
		"type":"nodered",			
		"enabled":true
	}
		
Additionally, `key_NodeRed.py` will define `project_name`, `organization_name` and `sensor_name` that will be used to build the full sensor name. For instance, with:

	project_name="waziup"
	organization_name="UPPA"
	sensor_name="Sensor"

then, when device 2 sends "TC/22.5/HU/85" to the gateway, `CloudNodeRed.py` will generate the following json entries in `nodered/nodered.txt` file:

	{"source":"waziup_UPPA_Sensor2","measure":"TC","value":22.5}
	{"source":"waziup_UPPA_Sensor2","measure":"HU","value":85}	
	
The Node-Red flow is composed of a tail node that follows the `nodered/nodered.txt` file for new entries. Each entry will be converted into a json object with a json node. A function node will use the json entry to build a message as follows:

	msg.topic=msg.payload.source+'/'+msg.payload.measure
	msg.payload=msg.payload.value
	return msg; 		
	
Finally, an MQTT node using the `test.mosquitto.org` broker will receive the messages with the topic defined as `waziup_UPPA_Sensor2/TC` and `waziup_UPPA_Sensor2/HU`, and will then respectively publish 22.5 and 85 under these topics. The default topic is then project\_name+"\_"+organization\_name+"\_"+sensor\_name+sensor\_address+"/"+measure\_name. You can change the function if you want to build a different topic.	

Testing with test.mosquitto.org
===============================

First, install the mosquitto software:

	sudo apt-get install mosquitto mosquitto-clients python-mosquitto
	
Subscribing to a sensor
-----------------------

In a terminal, run:

	mosquitto_sub -v -h test.mosquitto.org -t 'waziup-UPPA_Sensor2/#'
	
You can of course subscribe to only one topic if you want:

	mosquitto_sub -v -h test.mosquitto.org -t 'waziup-UPPA_Sensor2/TC'		
	

Publishing with mosquitto_pub
-----------------------------

In a terminal, run:

	mosquitto_pub -h test.mosquitto.org -t 'waziup-UPPA_Sensor2/TC' -m 22.5	
	
Emulating sensor data
---------------------

With Node-Red started and the flow loaded and deployed with the web browser, use `echo` or `printf` in a terminal to write into the `nodered/nodered.txt`file:

	printf '{"source":"waziup_UPPA_Sensor2","measure":"TC","value":22.5}\n{"source":"waziup_UPPA_Sensor2","measure":"HU","value":85}\n' >> nodered/nodered.txt
	
You should see in the web browser debug information indicating that new data has been processed by the flow and, most importantly, in the terminal where you subscribed to the `waziup-UPPA_Sensor2/# topics, you should see:

	waziup_UPPA_Sensor2/TC 22.5
	waziup_UPPA_Sensor2/HU 85
	
Running the gateway with Node-Red cloud enabled
-----------------------------------------------

- enable `CloudNodeRed.py` in `clouds.json`
- start Node-Red on your gateway: `node-red-start`	
- import the flow and deploy it
- use the simple temperature sensor to send data to the gateway: `TC/22.5` for instance
- use the gateway or another computer to subscribe to the topic: `waziup_UPPA_Sensor2/TC` for instance, depending on the address of your sensor node and your `key_NodeRed.py` configuration

Simply publish to an MQTT topic with the gateway
------------------------------------------------

Of course, it is very simple to simply publish to an MQTT topic directly with the gateway. Look at the `CloudMQTT.py` cloud script that again uses the `test.mosquitto.org` broker. `key_MQTT.py` defines variables that will be used by `CloudMQTT.py` to build the MQTT topic.
	
Enjoy!
C. Pham		