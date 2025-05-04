Cloud support module for the low-cost LoRa gateway
==================================================

Description 
-----------

`clouds.json` contains a list of clouds where you want your data to be uploaded. Here is an example with 5 clouds: local MongoDB, WAZIUP (based on FiWare), `ThingSpeak`, MQTT (with `test.mosquitto.org` broker) and `GroveStreams`.

	{
		"clouds" : [
			{	
				"name":"Local gateway MongoDB",			
				"notice":"do not remove the MongoDB cloud declaration, just change enabled and max_months_to_store if needed"
				"script":"python CloudMongoDB.py",
				"type":"database",
				"max_months_to_store":2,
				"enabled":false
			},
				"name":"WAZIUP Orion cloud new API",
				"script":"python CloudWAZIUP.py",
				"type":"iotcloud",
				"enabled":true
			},				
			{	
				"name":"ThingSpeak cloud",
				"script":"python CloudThingSpeak.py",
				"type":"iotcloud",			
				"enabled":true
			},
			{	
				"name":"MQTT cloud",
				"script":"python CloudMQTT.py",
				"type":"MQTT",			
				"enabled":false
			},				
			{	
				"name":"GroveStreams cloud",
				"script":"python CloudGroveStreams.py",
				"type":"iotcloud",			
				"enabled":true
			}
	}

Note that storage on the local MongoDB is declared as a cloud, among others that you can declare. You should not remove this cloud declaration and leave it in first position even if position has no real matter. `clouds.json` is parsed by `post_processing_gw.py` using `clouds_parser.py`. For each cloud declaration, there are only 2 relevant fields: `script` and `enabled`. `script` is used for you to provide the name of a script. You have also to indicate which launcher will be used. In this way, you can use several script languages (including shell scripts or executables provided that they read parameters that are passed by their command line). For instance, if the script is a python script, enter `python my_script_filename`. `enabled` set to true indicates that you want this cloud to be active so that` post_processing_gw.py` will call the associated script to perform upload of the received data. All the other fields are not relevant for `post_processing_gw.py` but can be used by the associated script to get additional information that you may want to provide through the `clouds.json` file. Otherwise, you can always provide these additional information statically in the script.

Recall that a message will be upload to cloud only if it is prefixed with `\!`. So assuming that you are sending `\!#4#TC/21.5` (the data format will be explained in following paragraphs) `post_processing_gw.py` provides 5 parameters to your script when it is launched. 

- `ldata`: the received data (without the prefix)
	- e.g. `#4#TC/21.5` as 1t argument (sys.argv[1] in python)
- `pdata`: packet information
	- e.g. `1,16,3,0,10,8,-45` as 2nd argument (sys.argv[2] in python)
	- interpreted as `dst`, `ptype`, `src`, `seq`, `len`, `SNR`, `RSSI` for the last received packet
- `rdata`: the LoRa radio information
	- e.g. `500,5,12` as 3rd argument (sys.argv[3] in python)
	- interpreted as `bw`, `cr`, `sf` for the last received packet
- `tdata`: the timestamp information
	- e.g. `2016-10-04T02:03:28.783385` as 4th argument (sys.argv[4] in python)
- `gwid`: the gateway id
	- e.g. `00000027EBBEDA21` as 5th argument (sys.argv[5] in python)	
	
These parameters are passed to the script and **it is up to the cloud script to use these parameters or not**. The main structure of a Python cloud script to handle a particular cloud can therefore be summarized as follows:

	IMPORT-AS-MANY-PACKAGES-AS-YOU-NEED
	DEFINE-AS-MANY-FUNCTIONS-AS-YOU-NEED
	
	def main(ldata, pdata, rdata, tdata, gwid):
		arr = map(int,pdata.split(','))
		dst=arr[0]
		ptype=arr[1]				
		src=arr[2]
		seq=arr[3]
		datalen=arr[4]
		SNR=arr[5]
		RSSI=arr[6]
		
		arr = map(int,rdata.split(','))
		bw=arr[0]
		cr=arr[1]
		sf=arr[2]
		
		DO-WHATEVER-YOU-NEED-TO-DO-FOR-DATA-UPLOADING
		
		USE-PARAMETERS-AS-YOU-NEED
			
	if __name__ == "__main__":
		main(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])

This cloud design approach allows for:

- a very generic `post_processing_gw.py` script that handles the interface with the low-level `lora_gateway` program
- the end-user to have the entire responsability (through a cloud script) to decode the raw data provided by the end-device

Assuming that `_enabled_clouds` contains: 

	['python CloudThingSpeak.py', 'python CloudGroveStreams.py']

The main data upload processing task in `post_processing_gw.py` is very simple and looks like:

	ldata = getAllLine()
	print "number of enabled clouds is %d" % len(_enabled_clouds)	
	
	#loop over all enabled clouds to upload data
	#once again, it is up to the corresponding cloud script to handle the data format
	#
	for cloud_index in range(0,len(_enabled_clouds)):
		print "--> cloud[%d]" % cloud_index
		cloud_script=_enabled_clouds[cloud_index]
		print "uploading with "+cloud_script
		cmd_arg=cloud_script+" \""+ldata+"\""+" \""+pdata+"\""+" \""+rdata+"\""+" \""+tdata+"\""+" \""+_gwid+"\""
		os.system(cmd_arg) 
	print "--> cloud end"

Good practice for storing keys or identification information
------------------------------------------------------------

Most of cloud platforms use some kind of keys for write access. These keys should be stored in a separate Python file so that updates on the cloud script can be realized independently from the existing keys (for instance if you already customized from a previous install). In the provided examples, `key_FireBase.py`, `key_GroveStreams.py` and `key_ThingSpeak.py` contain keys for their corresponding clouds. For instance, `CloudThingSpeak.py` starts with an `import key_ThingSpeak` statement. Only `key_ThingSpeak.py` has a usable key, which is our LoRa demo channel write key: `SGSH52UGPVAUYG3S`. 

	> cat key_ThingSpeak.py
	# LoRa demo channel
	_def_thingspeak_channel_key='SGSH52UGPVAUYG3S'

For the other clouds, you have to create your own account in order to get your private key before being able to upload data to `FireBase` or `GroveStreams`.

Indicate a list of allowed source addresses and other advanced features
-----------------------------------------------------------------------

We also define in the key file a list of allowed device source address:

	> cat key_ThingSpeak.py
	# LoRa demo channel
	_def_thingspeak_channel_key='SGSH52UGPVAUYG3S'
	#Note how we can indicate a device source addr that are allowed to use the script
	#Use decimal between 2-255 and use 4-byte hex format for LoRaWAN devAddr
	#leave empty to allow all devices
	#source_list=["3", "255", "01020304"]	
	source_list=["3","10"]
 
When `source_list` is set to `[]` then all device are accepted. If it specifies a list of device address, then `CloudThingSpeak.py` will only upload data from these devices.

The code of `CloudThingSpeak.py` scripts looks like:

	if (src_str in key_ThingSpeak.source_list) or (len(key_ThingSpeak.source_list)==0):
	
		DO-WHATEVER-YOU-NEED-TO-DO-FOR-DATA-UPLOADING
		
		USE-PARAMETERS-AS-YOU-NEED
	else:
		print "Source is not is source list, not sending with CloudThingSpeak.py"
		
This feature is quite useful when you want to upload data to various different clouds, depending on the device. Suppose that you have 10 devices whose addresses are from 1 to 10. You want to upload data from sensors 1..5 to a ThingSpeak channel and data from sensors 6..10 to GroveStreams. Then you need to activate in `clouds.json` both clouds and specify in `key_ThingSpeak.py` `source_list=["1","2","3","4","5"]` and in `key_GroveStreams.py` `source_list=["6","7","8","9","10"]`. 

If you want to upload to 2 different ThingSpeak channels, you can do so by duplicating the `CloudThingSpeak.py` script into `CloudThingSpeak_1.py`, creating a new `key_ThingSpeak_1.py` file to store both the other ThingSpeak write key and `source_list`, changing in `CloudThingSpeak_1.py` the `import key_ThingSpeak` into `import key_ThingSpeak_1 as key_ThingSpeak` and then activating both `CloudThingSpeak.py` and `CloudThingSpeak_1.py` in `clouds.json` as follows:

	{
		"clouds" : [
			{	
				"notice":"do not remove the MongoDB cloud declaration, just change enabled and max_months_to_store if needed"
				"name":"Local gateway MongoDB",
				"script":"python CloudMongoDB.py",
				"type":"database",
				"max_months_to_store":2,
				"enabled":false
			},	
			{	
				"name":"ThingSpeak cloud",
				"script":"python CloudThingSpeak.py",
				"type":"iotcloud",			
				"enabled":true
			},
				"name":"ThingSpeak cloud",
				"script":"python CloudThingSpeak_1.py",
				"type":"iotcloud",			
				"enabled":true
			}
	}
		
However, as `ThingSpeak` is a simple and very popular IoT cloud, we enhanced `CloudThingSpeak.py` with many useful features such as: be able to assign a specific channel write key, a specific chart (field index) and a specific field offset (depending a nomenclature code) according to the sensor source address. In addition to `_def_thingspeak_channel_key` and `source_list`, `key_ThingSpeak.py` defines: 

- `key_association`
- `field_association`
- `nomenclature_association`

for instance:

- `key_association=[('AAAAAAAAAAAAAAAA', 9), ('BBBBBBBBBBBBBBBB', 10, 11)]`
	- node 9 will use channel AAAAAAAAAAAAAAAA
	- node 10 and 11 will use channel BBBBBBBBBBBBBBBB
	- other nodes will use default channel
	- note that priority is given to key association defined on gateway	
	- with key association you can go beyond the limitation of 8 charts per channel: if you have many sensors, you can assign specific channel's write key to specific sensors
- `field_association=[(6,1),(7,5)]`
	- [(6,1),(7,5)] means data from respectively sensor 6/7 will use starting field index of 1/5 
- `nomenclature_association=[("TC",0),("HU",1),("LU",2),("CO2",3)]`
	- ("TC",0) means that if nomemclature is "TC" then the offset for field index will be 0

More details are given below.

Some words about data format
----------------------------
	
If several cloud systems is used, each with some specific features, then the raw data format can be complex and the decoding tasks of these data by the various scripts may need to be also more complex. This is exactely the case with our example templates where we want to be able to specify a particular write key and field when uploading to `ThingSpeak` and still be able to use the dynamic field creation feature of `Grovestreams` or WAZIUP Orion platform. For example, we want to be able to send various message formats such as:

- `SGSH52UGPVAUYG3S##22.5` or `SGSH52UGPVAUYG3S#22.5` : specified a ThingSpeak write key with a the default field, value is 22.5
- `#4#22.5` or `4#22.5` : specified a field when using the default ThingSpeak write key, value is 22.5
- `SGSH52UGPVAUYG3S#4#22.5` : specified both a ThingSpeak write key and a field, value is 22.5
- `##22.5` : use default value for both ThingSpeak write key and field
- `22.5` : use default value for both ThingSpeak write key and field; or use default nomenclature (i.e. DEF) for Grovestreams and MongoDB
- `##TC/22.5` : use the possibility to define a particular nomenclature with Grovestreams and MongoDB (e.g. TC for temperature in Celsius)
- `TC/22.5` : use the possibility to define a particular nomenclature with Grovestreams and MongoDB (e.g. TC for temperature in Celsius)
- `TC/22.5/HU/85/LU/78` : use the possibility to define multiple nomenclatures and values with Grovestreams and MongoDB

Even for ThingSpeak, we recommend now to use the following format example: `TC/22.5/HU/24/LU/345/CO2/456...`. Heere is a detailed example with  `CloudThingSpeak.py`. Assuming that `key_ThingSpeak.py` contains:

	_def_thingspeak_channel_key='SGSH52UGPVAUYG3S'
	source_list=["6", "7", "8", "9"]
	key_association=[('AAAAAAAAAAAAAAAA', 9), ('BBBBBBBBBBBBBBBB', 10, 11)]
	field_association=[(6,1),(7,5)]
	nomenclature_association=[("TC",0),("HU",1),("LU",2),("CO2",3)]

and assuming that each sensor node has 4 physical sensors: TC, HU, LU, CO2

- if we receive "TC/22.5/HU/24/LU/345/CO2/456" from sensor 6
	- data will be accepted
	- starting field index for the channel will be 1
	- TC will be uploaded on field 1, HU on field 2, LU on field 3 and CO2 on field 4
- if we receive only "HU/24/LU/345" from sensor 6 because these physical measures are sent at higher frequency
	- HU and LU will still be respectively uploaded on field 2 and 3
- if we receive "TC/22.5/HU/24/LU/345/CO2/456" from sensor 7
	- data will be accepted
	- starting field index for the channel will be 5
	- TC will be uploaded on field 5, HU on field 6, LU on field 7 and CO2 on field 8
	- in this example, with 4 physical sensors per node, then a ThingSpeak channel can handle 2 nodes	
- if we receive "CCCCCCCCCCCCCCCC#TC/22.5/HU/24/LU/345/CO2/456" from sensor 8
	- data will be accepted
	- data will be uploaded on the channel which write key is "CCCCCCCCCCCCCCCC" as there is no key association defined
	- starting field index will be the default value, i.e. 1
	- TC will be uploaded on field 1, HU on field 2, LU on field 3 and CO2 on field 4
- if we receive "CCCCCCCCCCCCCCCC#5#TC/22.5/HU/24/LU/345/CO2/456" from sensor 8
	- data will be accepted
	- data will be uploaded on the channel which write key is "CCCCCCCCCCCCCCCC"
	- starting field index will be 5
	- TC will be uploaded on field 5, HU on field 6, LU on field 7 and CO2 on field 8
- if we receive "CCCCCCCCCCCCCCCC#TC/22.5/HU/24/LU/345/CO2/456" from sensor 9
	- data will be accepted
	- data will be uploaded on the channel which write key is "AAAAAAAAAAAAAAAA" as there **is** a key association that has priority
	- starting field index will be the default value, i.e. 1
	- TC will be uploaded on field 1, HU on field 2, LU on field 3 and CO2 on field 4	
- if we receive "TC/22.5/HU/24/AA/345/BB/456" from sensor 6
	- data will be accepted
	- starting field index for the channel will be 1
	- TC will be uploaded on field 1, HU on field 2, AA on field 3 and BB on field 4 even if AA and BB have no nomenclature association
- if we receive "YY/30/TC/22.5/AA/345/BB/456" from sensor 6
	- data will be accepted
	- starting field index for the channel will be 1
	- YY will be uploaded on field 1, TC on field 1, AA on field 3 and BB on field 4. Here YY has no nomenclature association and you can see that TC overwrite YY on field 1
- if we receive "TC/22.5/HU/24/LU/345/CO2/456" from sensor 10
	- data will be discarded by CloudThingSpeak.py
	
If you look at the provided example for `ThingSpeak`, `Grovestreams` and `MongoDB` clouds, you can see how we handle/decode/process these various data formats (by looking for delimiters) so that data from an end-device can sequentially be uploaded on various clouds platforms. For instance, `#` delimiters are not relevant for both Grovestreams and MongoDB (these cloud scripts will simply ignore them) while they are for `ThingSpeak`. If you only use one cloud platform then the data format and the cloud script can be much more simple.

Output examples
---------------
	
The LoRa gateway is usually started as follows:
	
	> sudo ./lora_gateway --mode 1 | python post_processing_gw.py &
	
The `post_processing_gw_py` script will start by parsing the clouds declarations in `clouds.json` and adds a new cloud script for each enabled cloud.

	Parsing cloud declarations
	[u'python CloudThingSpeak.py']
	[u'python CloudThingSpeak.py', u'python CloudGroveStreams.py']
	Parsed all cloud declarations
	post_processing_gw.py got cloud list: 
	[u'python CloudThingSpeak.py', u'python CloudGroveStreams.py']	
	
When incoming data with `\!` prefix is processed, `post_processing_gw.py` will loop over all enabled clouds to call their respective script.

	number of enabled clouds is 2
	--> cloud[0]
	uploading with python CloudThingSpeak.py
	ThingSpeak: uploading
	rcv msg to log (\!) on ThingSpeak ( default , 4 ): 21.5
	ThingSpeak: will issue curl cmd
	curl -s -k -X POST --data field4=21.5&field8=0 https://api.thingspeak.com/update?key=SGSH52UGPVAUYG3S
	ThingSpeak: returned code from server is 156
	--> cloud[1]
	uploading with python CloudGroveStreams.py
	GroveStreams: uploading
	Grovestreams: Uploading feed to: /api/feed?compId=node_6&TC=21.5
	--> cloud end
	
Remotely editing the `clouds.json` file
-------------------------------------

The gateway has a simple web admin interface that allows you to configure the ThingSpeak cloud and the WAZIUP Orion cloud (used by the WAZIUP european project). For all advanced editing of `clouds.json` file there are 2 possibilities. The first one is to use `ssh` to log into the gateway and then use the `nano` editor to edit the clouds.json file. Use `CTRL-O+RETURN` to save the file and then `CTRL-X` to quit.

	> ssh pi@192.168.200.1
	> cd lora_gateway
	> nano clouds.json
	
Normally, if you have a recent SD card image, on ssh to the gateway, you will enter the simple command text interface. Then use option `D` to edit the `clouds.json` file.

	=======================================* Gateway 00000027EBD4F300 *===
	0- sudo python start_gw.py                                           +
	1- sudo ./lora_gateway --mode 1                                      +
	2- sudo ./lora_gateway --mode 1 | python post_processing_gw.py       +
	3- ps aux | grep -e start_gw -e lora_gateway -e post_proc -e log_gw  +
	4- tail --line=25 ../Dropbox/LoRa-test/post-processing.log           +
	5- tail --line=25 -f ../Dropbox/LoRa-test/post-processing.log        +
	6- less ../Dropbox/LoRa-test/post-processing.log                     +
	------------------------------------------------------* Bluetooth *--+
	a- run: sudo hciconfig hci0 piscan                                   +
	b- run: sudo python rfcomm-server.py                                 +
	c- run: nohup sudo python rfcomm-server.py -bg > rfcomm.log &        +
	d- run: ps aux | grep rfcomm                                         +
	e- run: tail -f rfcomm.log                                           +
	---------------------------------------------------* Connectivity *--+
	f- test: ping www.univ-pau.fr                                        +
	--------------------------------------------------* Filtering msg *--+
	l- List LoRa reception indications                                   +
	m- List radio module reset indications                               +
	n- List boot indications                                             +
	o- List post-processing status                                       +
	p- List low-level gateway status                                     +
	--------------------------------------------------* Configuration *--+
	A- show gateway_conf.json                                            +
	B- edit gateway_conf.json                                            +
	C- show clouds.json                                                  +
	D- edit clouds.json                                                  +
	---------------------------------------------------------* Update *--+
	U- update to latest version on repository                            +
	V- download and install a file                                       +
	W- run a command                                                     +
	-----------------------------------------------------------* kill *--+
	K- kill all gateway related processes                                +
	k- kill rfcomm-server process                                        +
	R- reboot gateway                                                    +
	S- shutdown gateway                                                  +
	---------------------------------------------------------------------+
	Q- quit                                                              +
	======================================================================
	Enter your choice: 
	D
	----------------------------------------------------------------------	
	
A second solution is to use a text editor on your computer/laptop that can directly edit and save a file on the Raspberry gateway. `Nodepad++` on Windows and `TextWrangler` on MacOs do that quite nicely and they usually use FTP/SFTP feature. You can follow this tutorial [http://trevorappleton.blogspot.fr/2014/03/remotely-modify-text-file-on-your.html](http://trevorappleton.blogspot.fr/2014/03/remotely-modify-text-file-on-your.html) or this one [https://www.dirtyoptics.com/edit-files-on-your-raspberry-pi-the-easy-way/](https://www.dirtyoptics.com/edit-files-on-your-raspberry-pi-the-easy-way/).
	
Support of MongoDB as a cloud declaration
-----------------------------------------	

As indicated previously, local storage of incoming data in the local MongoDB database is now viewed as a cloud upload. In the previous cloud declarations of `clouds.json`, if you enable the MongoDB cloud here is a typical output:

	--> cloud[0]
	uploading with python CloudMongoDB.py
	MongoDB with max months to store is 2
	MongoDB: removing obsolete entries
	MongoDB: deleting data older than 2 month(s)...
	MongoDB: 0 documents deleted
	MongoDB: saving the document in the collection...
	MongoDB: saving done
	
The local MongoDB database is structured as follows: the database is `messages` and the collection is `ReceivedData`. You can look at MongoDB.py for more details. `CloudMongoDB.py` is then the script that `post_processing_gw.py` will call to insert received data into the `ReceivedData` collection of `messages` database. 

It is assumed that your data is formatted as follows, with a nomenclature code followed by the associated value:

	TC/22.5/HU/85...
	
`CloudMongoDB.py` will create a json array with your received data as follows:

	{ "TC":22.5, "HU":85 }

but will also add the SNR and the RSSI of the received radio messages to have the following json array:

	{ "SNR":8, "RSSI": -56, "TC":22.5, "HU":85 } 

Then, the json document that will be inserted into the `ReceivedData` collection is as follows:

	"type" : ptype,
	"gateway_eui" : gwid, 
	"node_eui" : src,
	"snr" : SNR, 
	"rssi" : RSSI, 
	"cr" : cr, 
	"datarate" : "SF"+str(sf)+"BW"+str(bw),
	"time" : now,
	"data" : json.dumps(json.loads(str_json_data))
	
A typical document would then be:

	"type" : 18,
	"gateway_eui" : "00000027EBBEDA21", 
	"node_eui" : 10,
	"snr" : 8, 
	"rssi" : -56, 
	"cr" : 5, 
	"datarate" : "SF12BW125",
	"time" : "2017-03-24T20:28:55.446Z",
	"data" : { "SNR":8, "RSSI": -56, "TC":22.5, "HU":85 }

In the collection, the entry would look like:	
		
	{ "_id" : ObjectId("58d5818774fece07013e4f39"), "node_eui" : 10, "snr" : 8, "datarate" : "SF12BW125", "gateway_eui" : "00000027EBBEDA21", "rssi" : -56, "time" : ISODate("2017-03-24T20:28:55.446Z"), "type" : 18, "cr" : 5, "data" : "{\"SNR\": 8, \"RSSI\": -36, \"TC\": 22.5, \"HU\": 85}" }	
	
The local MongoDB database is directly linked with the embedded web server. Use a web browser to connect to the gateway	(e.g. http://192.168.200.1) to graphically visualize the received data. The web server will show all available nomenclature codes (e.g. TC, HU, ...) as well as the SNR and RSSI as these values have been inserted into the data json array. You will be able to select which nomenclature you want to visualize as well as selecting a set of sensors.

You can also interact with the MongoDB database using the command line as follows:

	pi@raspberrypi:~/lora_gateway $ mongo
	MongoDB shell version: 2.4.10
	connecting to: test
	Server has startup warnings: 
	Tue Jul 25 08:07:49.513 [initandlisten] 
	Tue Jul 25 08:07:49.513 [initandlisten] ** NOTE: This is a 32 bit MongoDB binary.
	Tue Jul 25 08:07:49.513 [initandlisten] **       32 bit builds are limited to less than 2GB of data (or less with --journal).
	Tue Jul 25 08:07:49.513 [initandlisten] **       See http://dochub.mongodb.org/core/32bit
	Tue Jul 25 08:07:49.513 [initandlisten] 
	> show dbs
	local	0.03125GB
	messages	0.0625GB
	> use messages
	switched to db messages
	> show collections
	ReceivedData
	system.indexes
	> db.ReceivedData.find().pretty()
	{
		"_id" : ObjectId("58d5718274fece0472bf60e5"),
		"node_eui" : 10,
		"snr" : 8,
		"datarate" : "SF12BW125",
		"gateway_eui" : "00000027EBBEDA21",
		"rssi" : -65,
		"time" : ISODate("2017-03-24T19:20:34.715Z"),
		"type" : 18,
		"cr" : 5,
		"data" : "{\"SNR\": 8, \"RSSI\": -65, \"SHU\": 1004}"
	}
	{
		"_id" : ObjectId("58d57fef74fece068e221277"),
		"node_eui" : 10,
		"snr" : 8,
		"datarate" : "SF12BW125",
		"gateway_eui" : "00000027EBBEDA21",
		"rssi" : -61,
		"time" : ISODate("2017-03-24T20:22:07.690Z"),
		"type" : 18,
		"cr" : 5,
		"data" : "{\"SNR\": 8, \"RSSI\": -61, \"SHU\": 991}"
	}
	{
		"_id" : ObjectId("58d580ae74fece069a795276"),
		"node_eui" : 10,
		"snr" : 8,
		"datarate" : "SF12BW125",
		"gateway_eui" : "00000027EBBEDA21",
		"rssi" : -66,
		"time" : ISODate("2017-03-24T20:25:18.439Z"),
		"type" : 18,
		"cr" : 5,
		"data" : "{\"SNR\": 8, \"RSSI\": -66, \"SHU\": 993}"
	}
	...
	
You can list all data from sensor 10 with:

	> db.ReceivedData.find({node_eui:10})
	{ "_id" : ObjectId("58d5718274fece0472bf60e5"), "node_eui" : 10, "snr" : 8, "datarate" : "SF12BW125", "gateway_eui" : "00000027EBBEDA21", "rssi" : -65, "time" : ISODate("2017-03-24T19:20:34.715Z"), "type" : 18, "cr" : 5, "data" : "{\"SNR\": 8, \"RSSI\": -65, \"SHU\": 1004}" }
	{ "_id" : ObjectId("58d57fef74fece068e221277"), "node_eui" : 10, "snr" : 8, "datarate" : "SF12BW125", "gateway_eui" : "00000027EBBEDA21", "rssi" : -61, "time" : ISODate("2017-03-24T20:22:07.690Z"), "type" : 18, "cr" : 5, "data" : "{\"SNR\": 8, \"RSSI\": -61, \"SHU\": 991}" }
	{ "_id" : ObjectId("58d580ae74fece069a795276"), "node_eui" : 10, "snr" : 8, "datarate" : "SF12BW125", "gateway_eui" : "00000027EBBEDA21", "rssi" : -66, "time" : ISODate("2017-03-24T20:25:18.439Z"), "type" : 18, "cr" : 5, "data" : "{\"SNR\": 8, \"RSSI\": -66, \"SHU\": 993}" }
	
If you want to delete all entries of the `ReceivedData` collection, you can issue the following command:

	> db.ReceivedData.remove({})

	
Enjoy!
C. Pham	
	
	