# adapted by N. Bertuol under C. Pham supervision 
#
#

import httplib
import xdrlib
import time
import math
import random
import sys
import urllib2
import ssl
import socket
import datetime

AUTH_SERVER = "sensorcloud.microstrain.com"

#info for API key authentication	
device_id="XXXXXXXXXXXXXXXX"
auth_key="XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
sensor_name="sensor6"
channel_name="temp"
server="dsx.sensorcloud.microstrain.com"
auth_token=""
#info for alternate method of authentication using username/password
#(Needs to be enabled by user under permissions for the device)
username = "YOUR_LOGIN_NAME_HERE"
password = "YOU_LOGIN_PASSWORD_HERE"

# didn't get a response from sensorcloud server?
connection_failure = False
# are we authenticated to the sensorcloud server?
authenticated = False

#samplerate types
HERTZ = 1
SECONDS = 0

def_sensor_name=""

# function to check connection availability with the server
def test_network_available():
	connection = False
	iteration = 0
	response = None
	
	# we try 4 times to connect to the server.
	while(not connection and iteration < 4) :
	    	try:
	    		# 3sec timeout in case of server available but overcrowded
			response=urllib2.urlopen('https://sensorcloud.microstrain.com/',timeout=3)
			connection = True
	    	except urllib2.URLError, e: pass
		except socket.timeout: pass
		except ssl.SSLError: pass
	    	
	    	# if sensorcloud_connection_failure == True and the connection with the server is unavailable, don't waste more time, exit directly
	    	if(connection_failure and response is None) :
	    		print('Sensorcloud: the server is still unavailable')
	    		iteration = 4
	    	# print connection failure
	    	elif(response is None) :
	    		print('Sensorcloud: server unavailable, retrying to connect soon...')
	    		# wait before retrying
	    		time.sleep(1)
	    		iteration += 1
	    		
	return connection
	
# send a data to the server
def send_data(my_data, sensor, channel):
 	
 	conn = httplib.HTTPSConnection(server)

	url="/SensorCloud/devices/%s/sensors/%s/channels/%s/streams/timeseries/data/?version=1&auth_token=%s"%(device_id, sensor, channel, auth_token)
	
	#we need to pack these strings into an xdr structure
	packer = xdrlib.Packer()
	packer.pack_int(1)  #version 1
	
	#set samplerate to 10 Hz
	packer.pack_enum(SECONDS)
	packer.pack_int(10)
	 
	#Total number of datapoints  
	POINTS = 1
	packer.pack_int(POINTS)
	
	print("Sensorcloud: generating data...")
	#now pack each datapoint, we'll use a sin wave function to generate fake data.  we'll use the current time as the starting point
	timestamp_nanoseconds = int(time.time()*1000000000)  #start time in nanoseconds

	packer.pack_hyper(timestamp_nanoseconds)
	packer.pack_float(float(my_data))  #generate value as a function of time
	
	data = packer.get_buffer()
	
	print("Sensorcloud: adding data...")
	headers = {"Content-type" : "application/xdr"}
	conn.request('POST', url=url, body=data, headers=headers)
	response =conn.getresponse()
	
	#if response is 201 created then we know the channel was added
	if response.status is httplib.CREATED: 
		print response.status , response.reason
		print("Sensorcloud: data successfully added")
		
	#if response is 401 unauthorized then we need to reauthenticate and send the data
	elif response.status == int(httplib.UNAUTHORIZED):
		print response.status , response.reason
		authenticate_key(device_id, auth_key)
		conn = httplib.HTTPSConnection(server)
		# we change the url then connect
		url="/SensorCloud/devices/%s/sensors/%s/channels/%s/streams/timeseries/data/?version=1&auth_token=%s"%(device_id, sensor, channel, auth_token)
		conn.request('POST', url=url, body=data, headers=headers)
		response =conn.getresponse()
		
		#if response is 201 created then we know the channel was added
		if response.status is httplib.CREATED: 
			print response.status , response.reason
			print("Sensorcloud: data successfully added")
		else :
			print "Sensorcloud: Error adding data.  Error:", response.read()
		
	else :
		print "Error adding data.  Error:", response.read()

def authenticate_key(device_id, key):
	"""
	authenticate with sensorcloud and get the server and auth_key for all subsequent api requests
	"""
	conn = httplib.HTTPSConnection(AUTH_SERVER)

	headers = {"Accept": "application/xdr"}
	url = "/SensorCloud/devices/%s/authenticate/?version=1&key=%s"%(device_id, key)
	
	print "Sensorcloud: authenticating..."
	conn.request('GET', url=url, headers=headers)
	response =conn.getresponse()
	print response.status, response.reason
	
	#if response is 200 ok then we can parse the response to get the auth token and server
	if response.status is httplib.OK: 
		print "Sensorcloud: Credential are correct"
			
		#read the body of the response
		data = response.read()
		
		#response will be in xdr format. Create an XDR unpacker and extract the token and server as strings 
		unpacker = xdrlib.Unpacker(data)
		
		global authenticated
		authenticated = True
		
		global auth_token
		auth_token = unpacker.unpack_string()
		global server
		server = unpacker.unpack_string()

def authenticate_alternate(device_id, username, password):
	"""
	authenticate with sensorcloud and get the server and auth_key for all subsequent api requests
	"""
	conn = httplib.HTTPSConnection(AUTH_SERVER)

	headers = {"Accept": "application/xdr"}
	url = "/SensorCloud/devices/%s/authenticate/?version=1&username=%s&password=%s"%(device_id, username, password)
	
	print "authenticating..."
	conn.request('GET', url=url, headers=headers)
	response =conn.getresponse()
	print response.status, response.reason
	
	#if response is 200 ok then we can parse the response to get the auth token and server
	if response.status is httplib.OK: 
		print "Credential are correct"
			
		#read the body of the response
		data = response.read()
		
		#response will be in xdr format. Create an XDR unpacker and extract the token and server as strings 
		unpacker = xdrlib.Unpacker(data)
		auth_token = unpacker.unpack_string()
		server = unpacker.unpack_string()
		
		global authenticated
		authenticated = True
		
		return server, auth_token

def addSensor(server, auth_token, device_id, sensor_name, sensor_type="", sensor_label="", sensor_desc=""):
	"""
	Add a sensor to the device. type, label, and description are optional.
	"""
	
	conn = httplib.HTTPSConnection(server)

	url="/SensorCloud/devices/%s/sensors/%s/?version=1&auth_token=%s"%(device_id, sensor_name, auth_token)
	
	headers = {"Content-type" : "application/xdr"}
	
	#addSensor allows you to set the sensor type label and description.  All fileds are strings.
	#we need to pack these strings into an xdr structure
	packer = xdrlib.Packer()
	packer.pack_int(1)  #version 1
	packer.pack_string(sensor_type)
	packer.pack_string(sensor_label)
	packer.pack_string(sensor_desc)
	data = packer.get_buffer()
	
	print "adding sensor..."
	conn.request('PUT', url=url, body=data, headers=headers)
	response =conn.getresponse()
	print response.status , response.reason
	
	#if response is 201 created then we know the sensor was added
	if response.status is httplib.CREATED: 
		print "Sensor added"
	else:
		print "Error adding sensor. Error:", response.read()

def addChannel(server, auth_token, device_id, sensor_name, channel_name, channel_label="", channel_desc=""):
	"""
	Add a channel to the sensor.  label and description are optional.
	"""

	conn = httplib.HTTPSConnection(server)

	url="/SensorCloud/devices/%s/sensors/%s/channels/%s/?version=1&auth_token=%s"%(device_id, sensor_name, channel_name, auth_token)
	
	headers = {"Content-type" : "application/xdr"}
	
	#addChannel allows you to set the channel label and description.  All fileds are strings.
	#we need to pack these strings into an xdr structure
	packer = xdrlib.Packer()
	packer.pack_int(1)  #version 1
	packer.pack_string(channel_label)
	packer.pack_string(channel_desc)
	data = packer.get_buffer()
	
	print "adding channel..."
	conn.request('PUT', url=url, body=data, headers=headers)
	response =conn.getresponse()
	print response.status , response.reason
	
	#if response is 201 created then we know the channel was added
	if response.status is httplib.CREATED: 
		print "Channel successfuly added"
	else:
		print "Error adding channel.  Error:", response.read()

def delChannel(server, auth_token, device_id, sensor_name, channel_name, channel_label="", channel_desc=""):
	"""
	Delete a channel of the sensor.  label and description are optional.
	"""

	conn = httplib.HTTPSConnection(server)

	url="/SensorCloud/devices/%s/sensors/%s/channels/%s/?version=1&auth_token=%s"%(device_id, sensor_name, channel_name, auth_token)
	
	headers = {"Content-type" : "application/xdr"}
	
	#delChannel allows you to delete the channel and all its associated data streams.  All fileds are strings.
	#we need to pack these strings into an xdr structure
	packer = xdrlib.Packer()
	packer.pack_int(1)  #version 1
	packer.pack_string(channel_label)
	packer.pack_string(channel_desc)
	data = packer.get_buffer()
	
	print "deleting channel..."
	conn.request('DELETE', url=url, body=data, headers=headers)
	response =conn.getresponse()
	print response.status , response.reason
	
	#if response is 201 created then we know the channel was added
	#if response.status is httplib.OK: 
	#	print "Channel successfuly deleted"
	#else:
	#	print "Error deleting channel.  Error:", response.read()	

def uploadSinWave(server, auth_token, device_id, sensor_name, channel_name):
 	"""
 	Upload 10 minutes of 10 hz sin wave data.  use time.now() as the starting timestamp
 	"""
 	
 	conn = httplib.HTTPSConnection(server)

	url="/SensorCloud/devices/%s/sensors/%s/channels/%s/streams/timeseries/data/?version=1&auth_token=%s"%(device_id, sensor_name, channel_name, auth_token)
	
	#we need to pack these strings into an xdr structure
	packer = xdrlib.Packer()
	packer.pack_int(1)  #version 1
	
	#set samplerate to 10 Hz
	packer.pack_enum(SECONDS)
	packer.pack_int(10)
	 
	
	#Total number of datapoints.  6000 points is 10 minutes of data sampled at 10 Hz    
	POINTS = 10
	packer.pack_int(POINTS)
	
	print "generating data..."
	#now pack each datapoint, we'll use a sin wave function to generate fake data.  we'll use the current time as the starting point
	timestamp_nanoseconds = int(time.time()*1000000000)  #start time in nanoseconds
	sampleInterval_nanoseconds = 100000000  #number of nanoseconds between 2 datapoints when sampling at 10 Hz
	for i in range(0,POINTS):
		packer.pack_hyper(timestamp_nanoseconds)
		packer.pack_float(random.uniform(19, 24))
		#packer.pack_float(math.sin(timestamp_nanoseconds/20000000000.0))  #generate value as a function of time
		
		#increment the timestamp for the next datapoint
		timestamp_nanoseconds += sampleInterval_nanoseconds
	
	data = packer.get_buffer()
	
	print "adding data..."
	headers = {"Content-type" : "application/xdr"}
	conn.request('POST', url=url, body=data, headers=headers)
	response =conn.getresponse()
	print response.status , response.reason
	
	#if response is 201 created then we know the channel was added
	if response.status is httplib.CREATED: 
		print "data successfuly added"
	else:
		print "Error adding data.  Error:", response.read()

def sensorcloud_uploadSingleData(my_data):
 	"""
 	Upload a single data.  use time.now() as the starting timestamp
 	"""
	connected = test_network_available()
	
	global authenticated
	global connection_failure
	
	if (connected):
		# authenticate if not already
		if(not authenticated) :
			authenticate_key(device_id, auth_key)
	
		send_data(my_data, sensor_name, channel_name)
		
		#update connection_failure
		connection_failure = not connected
	else:
		print("SensorCloud: not uploading")
		
		#update authenticated and connection_failure
		#if we lost the connection with the server, we shall reauthenticate if we recover the connection
		authenticated = False
		connection_failure = not connected
		

def downloadData(server, auth_token, device_id, sensor_name, channel_name, startTime, endTime):
	"""
	download the 10 minutes of data uploaded by uploadSinWave.
	Returns an array of tuples, where each tuple is a timestamp and a value
	"""
	conn = httplib.HTTPSConnection(server)

	url = "/SensorCloud/devices/%s/sensors/%s/channels/%s/streams/timeseries/data/?version=1&auth_token=%s&starttime=%s&endtime=%s" % (device_id, sensor_name, channel_name, auth_token, startTime, endTime)
	headers = {"Accept":"application/xdr"}
	print "Downloading data..."
	conn.request("GET", url=url, headers=headers)
	response = conn.getresponse()
	data = []
	if response.status is httplib.OK:
		print "Data retrieved"
		unpacker = xdrlib.Unpacker(response.read())
		while True:
			try:
				timestamp = unpacker.unpack_uhyper()
				value = unpacker.unpack_float()
				data.append((timestamp, value))
			except Exception as e:
				print e
				break
		return data
	else:
		print "Status: %s" % response.status
		print "Reason: %s" % response.reason
		return data

def GetSensors(server, auth_token, device_id):
	"""
	Download the Sensors and Channel information for the Device.
	Packs into a dict for easy parsing
	"""
	conn = httplib.HTTPSConnection(server)
	
	url = "/SensorCloud/devices/%s/sensors/?version=1&auth_token=%s" % (device_id, auth_token)
	headers = {"Accept":"application/xdr"}
	conn.request("GET", url=url, headers=headers)
	sensors = {}
	response = conn.getresponse()
	if response.status is httplib.OK:
		print "Data Retrieved"
		unpacker = xdrlib.Unpacker(response.read())
		#unpack version, always first
		unpacker.unpack_int()
		#sensor info is an array of sensor structs.  In XDR, first you read an int, and that's the number of items in the array.  You can then loop over the number of elements in the array
		numSensors = unpacker.unpack_int()
		for i in xrange(numSensors):
			sensorName = unpacker.unpack_string()
			sensorType = unpacker.unpack_string()
			sensorLabel = unpacker.unpack_string()
			sensorDescription = unpacker.unpack_string()
			#using sensorName as a key, add info to sensor dict
			sensors[sensorName] = {"name":sensorName, "type":sensorType, "label":sensorLabel, "description":sensorDescription, "channels":{}}
			#channels for each sensor is an array of channelInfo structs.  Read array length as int, then loop through the items
			numChannels = unpacker.unpack_int()
			for j in xrange(numChannels):
				channelName = unpacker.unpack_string()
				channelLabel = unpacker.unpack_string()
				channelDescription = unpacker.unpack_string()
				#using channel name as a key, add info to sensor's channel dict
				sensors[sensorName]["channels"][channelName] = {"name":channelName, "label":channelLabel, "description":channelDescription, "streams":{}}
				#dataStreams for each channel is an array of streamInfo structs, Read array length as int, then loop through the items
				numStreams = unpacker.unpack_int()
				for k in xrange(numStreams):
					#streamInfo is a union, where the type indicates which stream struct to use.  Currently we only support timeseries version 1, so we'll just code for that
					streamType = unpacker.unpack_string()
					if streamType == "TS_V1":
						#TS_V1 means we have a timeseriesInfo struct
						#total bytes allows us to jump ahead in our buffer if we're uninterested in the units.  For verbosity, we will parse them.
						total_bytes = unpacker.unpack_int()
						#units for each data stream is an array of unit structs.  Read array length as int, then loop through the items
						numUnits = unpacker.unpack_int()
						#add TS_V1 to streams dict
						sensors[sensorName]["channels"][channelName]["streams"]["TS_V1"] = {"units":{}}
						for l in xrange(numUnits):
							storedUnit = unpacker.unpack_string()
							preferredUnit = unpacker.unpack_string()
							unitTimestamp = unpacker.unpack_uhyper()
							slope = unpacker.unpack_float()
							offset = unpacker.unpack_float()
							#using unitTimestamp as a key, add unit info to unit dict
							sensors[sensorName]["channels"][channelName]["streams"]["TS_V1"]["units"][str(unitTimestamp)] = {"stored":storedUnit, 
							"preferred":preferredUnit, "unitTimestamp":unitTimestamp, "slope":slope, "offset":offset}
	return sensors

if __name__ == "__main__":

	#info for API key authentication	
	device_id="XXXXXXXXXXXXXXXX"
	key="XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
	#info for alternate method of authentication using username/password
	#(Needs to be enabled by user under permissions for the device)
	username = "YOUR_LOGIN_NAME_HERE"
	password = "YOU_LOGIN_PASSWORD_HERE"
	
	#first autheticate using the open api device serial and it's coresponding key
	#autheticate will return the server and an auth_token for all subsequent reguests
	authenticate_key(device_id, key)
	#alternate method, uncomment to use
	#server, auth_token = authenticate_alternate(device_id, username, password)

	while True:
		print "0- set default sensor"
		print "1- clear default sensor"		
		print "2- create sensor"
		print "3- create channel temp"
		print "4- delete channel temp"
		print "5- upload a sin wave data"
		print "6- upload a single test data"
		print "7- get sensor"
		print "8- quit"	
		
		if def_sensor_name!="":
			print "--> sensor is : %s" % def_sensor_name
		else:
			print "--> default sensor not set"	
		
		ASK = str(raw_input("Your choice: "))

		if ASK == "0":
			#add a new sensor to the device
			def_sensor_name = str(raw_input("default sensor name: "))		
		elif ASK == "1":
			def_sensor_name=""	
		elif ASK == "2":
			#add a new sensor to the device
			if def_sensor_name=="": 
				sensor_name = str(raw_input("sensor name: "))
			else:
				sensor_name = def_sensor_name
			addSensor(server, auth_token, device_id, sensor_name, sensor_desc=sensor_name)
		elif ASK == "3":
			if def_sensor_name=="": 
				sensor_name = str(raw_input("sensor name: "))
			else:
				sensor_name = def_sensor_name
			#now add a channel to the sensor
			addChannel(server, auth_token, device_id, sensor_name, channel_name="temp")
		elif ASK == "4":
			if def_sensor_name=="": 
				sensor_name = str(raw_input("sensor name: "))
			else:
				sensor_name = def_sensor_name
			#now del the channel to the sensor
			delChannel(server, auth_token, device_id, sensor_name, channel_name="temp")
		elif ASK == "5":	
			if def_sensor_name=="": 
				sensor_name = str(raw_input("sensor name: "))
			else:
				sensor_name = def_sensor_name
			#now add some data to the channel
			uploadSinWave(server, auth_token, device_id, sensor_name, channel_name="temp")
		elif ASK == "6":
			if def_sensor_name=="": 
				sensor_name = str(raw_input("sensor name: "))
			else:
				sensor_name = def_sensor_name
			my_data = str(raw_input("Float value: "))
			uploadSingleData(server, auth_token, device_id, my_data, sensor_name, channel_name="temp")
		elif ASK == "7":
			if def_sensor_name=="": 
				sensor_name = str(raw_input("sensor name: "))
			else:
				sensor_name = def_sensor_name
			#now download the channel data to an array
			#we need to specify range, roughly we can guess that the data is from a minute ago up to 20 minutes from now (it's 10 minutes worth of data in our data generator above)
			startTime = int(time.time()) - 60
			endTime = startTime + 1200 #1200 = 20 minutes in seconds
			#call download function, the time ranges are in nanoseconds
			data = downloadData(server, auth_token, device_id, sensor_name, "temp", startTime*1000000000, endTime*1000000000)
	
			print "Downloaded %s points" % len(data)
			print "First point %s" % str(data[0])
			print "Last point %s" % str(data[-1])
		
			sensors = GetSensors(server, auth_token, device_id)
		elif ASK == "8":
			print "Bye"
			sys.exit(0)
		else:
			print "not a valid choice"	
