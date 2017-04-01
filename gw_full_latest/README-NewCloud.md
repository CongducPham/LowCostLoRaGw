Cloud support module for the low-cost LoRa gateway
==================================================

Description 
-----------

clouds.json contains a list of clouds where you want your data to be uploaded. Here is an example with 3 clouds: local MongoDB, ThingSpeak and Grovestreams.

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
				"write_key":"",
				"enabled":true
			},
			{	
				"name":"GroveStreams cloud",
				"script":"python CloudGroveStreams.py",
				"type":"iotcloud",			
				"write_key":"",
				"enabled":true
			}
	}

Note that storage on the local MongoDB is declared as a cloud, among others that you can declare. You should not remove this cloud declaration and leave it in first position even if position has no matter. clouds.json is parsed by post_processing_gw.py using clouds_parser.py. For each cloud declaration, there are only 2 relevant fields: "script" and "enabled". "script" is used for you to provide the name of a script. You have also to indicate which launcher will be used. In this way, you can use several script languages (including shell scripts or executables provided that they read parameters that are passed by their command line). For instance, if the script is a python script, enter "python my_script_filename". "enabled" set to true indicates that you want this cloud to be active so that post_processing_gw.py will call the associated script to perform upload of the received data. All the other fields are not relevant for post_processing_gw.py but can be used by the associated script to get additional information that you may want to provide through the clouds.json file. Otherwise, you can always provide these additional information statically in the script.

When your script is launched, post_processing_gw.py provides 5 parameters. 

- ldata: the received data 
	- e.g. #4#TC/21.5 as 1st argument (sys.argv[1] in python)
- pdata: packet information
	- e.g. "1,16,3,0,10,8,-45" as 2nd argument (sys.argv[2] in python)
	- interpreted as dst,ptype,src,seq,len,SNR,RSSI for the last received packet
- rdata: the LoRa radio information
	- e.g. "500,5,12" as 3rd argument (sys.argv[3] in python)
	- interpreted as bw,cr,sf for the last received packet
- tdata: the timestamp information
	- e.g. "2016-10-04T02:03:28.783385" as 4th argument (sys.argv[4] in python)
- gwid: the gateway id
	- e.g. 00000027EBBEDA21 as 5th argument (sys.argv[5] in python)	
	
These parameters are passed to the script. It is up to the cloud script to use these parameters or not. The main structure of a python cloud script to handle a particular cloud can therefore be as follows:

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

- a very generic post_processing_gw.py script that handles the interface with the low-level lora_gateway program
- the end-user to have the entire responsability (through a cloud script) to decode the raw data provided by the end-device

Assuming that _enabled_clouds contains:

	['python CloudThingSpeak.py', 'python CloudGroveStreams.py']

The main data upload processing task in post_processing_gw.py is very simple and looks like:

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

Good practice for storing keys
------------------------------

Most of cloud platforms use some kind of keys for write access. These keys should be stored in a separate Python file so that updates on the cloud script can be realized independently from the existing keys (for instance if you already customized from a previous install). In the provided examples, key_FireBase.py, key_GroveStreams.py and key_ThingSpeak.py contain keys for their corresponding clouds. For instance, CloudThingSpeak.py starts with an "import key_ThingSpeak" statement. Only key_ThingSpeak.py has a usable key, which is our LoRa demo channel write key: SGSH52UGPVAUYG3S. For the other clouds, you have to create your own account in order to get your private key before being able to upload data to FireBase or GroveStreams.

Some words about data format
----------------------------
	
If several cloud systems is used, each with some specific features, then the raw data format can be complex and the decoding tasks of these data by the various scripts may need to be also more complex. This is exactely the case with our example templates where we want to be able to specify a particular write key and field when uploading to ThingSpeak and still be able to use the dynamic field creation feature of Grovestreams platform. For example, we want to be able to send various message formats such as:

- SGSH52UGPVAUYG3S##22.5 : specified a ThingSpeak write key with a the default field, value is 22.5
- SGSH52UGPVAUYG3S#4#22.5 : specified both a ThingSpeak write key and a field, value is 22.5
- #4#22.5 : specified a field when using the default ThingSpeak write key, value is 22.5
- ##22.5 : use default value for both ThingSpeak write key and field
- 22.5 : use default value for both ThingSpeak write key and field; or use default nomenclature (i.e. DEF) for Grovestreams and MongoDB
- ##TC/22.5 : use the possibility to define a particular nomenclature with Grovestreams and MongoDB (e.g. TC for temperature in Celsius)
- TC/22.5 : use the possibility to define a particular nomenclature with Grovestreams and MongoDB (e.g. TC for temperature in Celsius)
- TC/22.5/HU/85/LU/78 : use the possibility to define multiple nomenclatures and values with Grovestreams and MongoDB
	
If you look at the provided example for ThingSpeak, Grovestreams and MongoDB clouds, you can see how we handle/decode/process these various data formats (by looking for delimiters) so that data from an end-device can sequentially be uploaded on various clouds platforms. For instance, '#' delimiters are not relevant for both Grovestreams and MongoDB (these cloud scripts will simply ignore them) while they are for ThingSpeak. If you only use one cloud platform then the data format and the cloud script can be much more simple.

Output examples
---------------
	
The LoRa gateway is usually started as follows:
	
	> sudo ./lora_gateway --mode 1 | python post_processing_gw.py &
	
The post_processing_gw_py script will start by parsing the clouds declarations in clouds.json and adds a new cloud script for each enabled cloud.

	Parsing cloud declarations
	[u'python CloudThingSpeak.py']
	[u'python CloudThingSpeak.py', u'python CloudGroveStreams.py']
	Parsed all cloud declarations
	post_processing_gw.py got cloud list: 
	[u'python CloudThingSpeak.py', u'python CloudGroveStreams.py']	
	
When incoming data is processed, post_processing_gw.py will loop over all enabled clouds to call their respective script.

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
	
Support of MongoDB as a cloud declaration
-----------------------------------------	

As indicated previously, local storage of incoming data in the local MongoDB database is now viewed as a cloud upload. In the previous cloud declarations of clouds.json, if you enable the MongoDB cloud here is a typical output:

	--> cloud[0]
	uploading with python CloudMongoDB.py
	MongoDB with max months to store is 2
	MongoDB: removing obsolete entries
	MongoDB: deleting data older than 2 month(s)...
	MongoDB: 0 documents deleted
	MongoDB: saving the document in the collection...
	MongoDB: saving done
	
Enjoy!
C. Pham	
	
	