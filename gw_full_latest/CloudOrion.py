#-------------------------------------------------------------------------------
# Copyright 2017 Congduc Pham, University of Pau, France.
# 
# Congduc.Pham@univ-pau.fr
#
# This file is part of the low-cost LoRa gateway developped at University of Pau
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with the program.  If not, see <http://www.gnu.org/licenses/>.
#-------------------------------------------------------------------------------

import urllib2
import subprocess
import time
import ssl
import socket
import datetime
import sys
import os
import json
import re
import shlex

# get key definition from external file to ease
# update of cloud script in the future
import key_Orion

try:
	key_Orion.source_list
except AttributeError:
	key_Orion.source_list=[]

####################################################
#To create a new entitiy
# curl http://broker.waziup.io/v2/entities -s -S --header 'Content-Type: application/json' --header 'Fiware-Service:waziup' --header 'Fiware-ServicePath:/UPPA' -X POST -d '{ "id": "UPPA_Sensor2", "type": "SensingDevice", "TC": { "value": 23, "type": "Number" }, "PR": { "value": 720, "type": "Number" } }'

#Further updates of the values are like that:
# curl http://broker.waziup.io/v2/entities/UPPASensor2/attrs/TC/value -s -S --header 'Content-Type: text/plain' --header 'Fiware-Service:waziup' --header 'Fiware-ServicePath:/UPPA' -X POST -d 27
# curl http://broker.waziup.io/v2/entities/UPPASensor2/attrs/PR/value -s -S --header 'Content-Type: text/plain' --header 'Fiware-Service:waziup' --header 'Fiware-ServicePath:/UPPA' -X POST -d 722

#To retrieve the last data point inserted:
# curl http://broker.waziup.io/v2/entities/UPPA_Sensor2/attrs/TC/value --header 'Fiware-Service:waziup' --header 'Fiware-ServicePath:/UPPA' -X GET
####################################################

#error messages from server
Orion_entity_not_created="The requested entity has not been found"

# didn't get a response from server?
connection_failure = False

# retry if return from server is 0?
retry = False

global CloudNoInternet_enabled

num_format = re.compile("^[\-]?[1-9][0-9]*\.?[0-9]+$")

#------------------------------------------------------------
# Open clouds.json file 
#------------------------------------------------------------

#name of json file containing the cloud declarations
cloud_filename = "clouds.json"

#open json file to retrieve enabled clouds
f = open(os.path.expanduser(cloud_filename),"r")
string = f.read()
f.close()
	
#change it into a python array
json_array = json.loads(string)

#retrieving all cloud declarations
clouds = json_array["clouds"]

#here we check for our own script entry
for cloud in clouds:
	if "CloudNoInternet.py" in cloud["script"]:
			
		try:
			CloudNoInternet_enabled = cloud["enabled"]
		except KeyError:
			print "enabled undefined"
			CloudNoInternet_enabled = False

# function to check connection availability with the server
def test_network_available():
	connection = False
	iteration = 0
	response = None
	
	# we try 4 times to connect to the server.
	while(not connection and iteration < 4) :
		try:
			# 3sec timeout in case of server available but overcrowded
			response=urllib2.urlopen(key_Orion.orion_server, timeout=3)
			connection = True
		except urllib2.URLError, e: pass
		except socket.timeout: pass
		except ssl.SSLError: pass
	    	
		# if connection_failure == True and the connection with the server is unavailable, don't waste more time, exit directly
		if(connection_failure and response is None) :
			print('Orion: the server is still unavailable')
			iteration = 4
			# print connection failure
		elif(response is None) :
			print('Orion: server unavailable, retrying to connect soon...')
			# wait before retrying
			time.sleep(1)
			iteration += 1
	    		
	return connection

def create_new_entity(data, src, nomenclatures, tdata):

	global connection_failure
	
	print "Orion: create new entity"
	
	cmd = 'curl '+key_Orion.orion_server+'/entities -s -S --header Content-Type:application/json --header Fiware-Service:'+data[0]+' --header Fiware-ServicePath:'+data[1]+' -X POST -d \'{\"id\":\"'+key_Orion.organization_name+"_"+src+'\",\"type\":\"SensingDevice\",'
						
	i=0
	while i < len(data)-2 :
		cmd = cmd+'\"'+nomenclatures[i]+'\":{\"value\":'
		
		isnumber = re.match(num_format,data[i+2])
		
		if isnumber:
			cmd = cmd+data[i+2]+',\"type\":\"Number\"'
		else:
			cmd = cmd+'\"'+data[i+2]+'\",\"type\":\"String\"'
		
		cmd=cmd+',\"metadata\":{\"timestamp\":{\"type\":\"DateTime\",\"value\":\"'+tdata+'\"}}}'
		i += 1
		if i < len(data)-2:
			cmd = cmd+","
	cmd = cmd+"}\'" 	
	
	print "CloudOldOrion: will issue curl cmd"
	print(cmd)
	args = shlex.split(cmd)
	print args	

	try:
		out = subprocess.check_output(args, shell=False)
		
		if out != '':
			print 'Orion: returned msg from server is'
			print out					
		else :
			print 'Orion: entity creation success'
			
	except subprocess.CalledProcessError:
		print "Orion: curl command failed (maybe a disconnection)"
		connection_failure = True	
	
	
# send a data to the server
def send_data(data, src, nomenclatures, tdata):

	global connection_failure
	
	entity_need_to_be_created=False
	
	i=0
	
	if data[0]=='':
		data[0]=key_Orion.project_name

	if data[1]=='':
		data[1]=key_Orion.service_path
			
	while i < len(data)-2  and not entity_need_to_be_created:
	
		#cmd = 'curl '+key_Orion.orion_server+'/entities/'+src+'/attrs/'+nomenclatures[i]+'/value -s -S --header Content-Type:text/plain --header Fiware-Service:'+data[0]+' --header Fiware-ServicePath:'+data[1]+' -X POST -d '+data[i+2]

		#we now push data with a timestamp value
		cmd = 'curl '+key_Orion.orion_server+'/entities/'+key_Orion.organization_name+"_"+src+'/attrs/ -s -S --header Content-Type:application/json --header Fiware-Service:'+data[0]+' --header Fiware-ServicePath:'+data[1]+' -X POST -d \'{\"'+nomenclatures[i]+'\":{\"value\":'
		
		isnumber = re.match(num_format,data[i+2])
		
		if isnumber:
			cmd = cmd+data[i+2]+',\"type\":\"Number\"'
		else:
			cmd = cmd+'\"'+data[i+2]+'\",\"type\":\"String\"'
		
		cmd = cmd+',\"metadata\":{\"timestamp\":{\"type\":\"DateTime\",\"value\":\"'+tdata+'\"}}}}\''

		i += 1
						
		print "CloudOldOrion: will issue curl cmd"
		
		print(cmd)
		args = shlex.split(cmd)
		print args
		
		# retry enabled
		if (retry) :
			out = 'something'
			iteration = 0
			
			while(out != '' and iteration < 6 and not connection_failure) :
				try:
					out = subprocess.check_output(args, shell=False)
	
					# if server return 0, we didn't wait 15sec, wait then
					if out != '':
						print 'Orion: returned msg from server is'
						print out
						print('Orion: retrying in 3sec')
						iteration += 1
						time.sleep( 3 )
					else:
						'Orion: upload success'
						
				except subprocess.CalledProcessError:
					print "Orion: curl command failed (maybe a disconnection)"
					connection_failure = True
					
		# retry disabled
		else :
			try:
				out = subprocess.check_output(args, shell=False)
				
				if out != '':
					print 'Orion: returned msg from server is'
					print out
					
					# the entity has not been created before
					if Orion_entity_not_created in out:
						entity_need_to_be_created=True
						create_new_entity(data, src, nomenclatures, tdata)						
				else :
					print 'Orion: upload success'
					
			except subprocess.CalledProcessError:
				print "Orion: curl command failed (maybe a disconnection)"
				connection_failure = True

#no used in this new version, we call directly send_data(data, src, nomenclatures, tdata)	
def Orion_uploadData(nomenclatures, data, src, tdata):
	
	connected = test_network_available()
	
	#if we got a response from the server, send the data to it	
	if(connected):
		print("Orion: uploading")
		send_data(data, src, nomenclatures, tdata)
	else:
		print("Orion: not uploading")
		
	# update connection_failure value
	global connection_failure
	connection_failure = not connected

# main
# -------------------
#
# ldata can be formatted to indicate a specifc Fiware-Service and Fiware-ServicePath. Options are:
# 	TC/22.4/HU/85 -> use default Fiware-Service and Fiware-ServicePath
#	/UPPA/test#TC/22.4/HU/85 -> use default Fiware-Service and Fiware-ServicePath=/UPPA/test
#	mywaziup#/UPPA/test#TC/22.4/HU/85 -> Fiware-Service=mywaziup and Fiware-ServicePath=/UPPA/test
#
#	Fiware-Service and Fiware-ServicePath must BOTH have more than 2 characters
#
def main(ldata, pdata, rdata, tdata, gwid):

	# this is common code to process packet information provided by the main gateway script (i.e. post_processing_gw.py)
	# these information are provided in case you need them
	arr = map(int,pdata.split(','))
	dst=arr[0]
	ptype=arr[1]				
	src=arr[2]
	seq=arr[3]
	datalen=arr[4]
	SNR=arr[5]
	RSSI=arr[6]
	
	#LoRaWAN packet
	if dst==256:
		src_str="%0.8X" % src
	else:
		src_str=str(src)	

	if (src_str in key_Orion.source_list) or (len(key_Orion.source_list)==0):
			
		# this part depends on the syntax used by the end-device
		# we use: TC/22.4/HU/85...
		#
		# but we accept also a_str#b_str#TC/22.4/HU/85... to indicate a Fiware-Service and Fiware-ServicePath
		# or simply 22.4 in which case, the nomemclature will be DEF
		 		
		# get number of '#' separator
		nsharp=ldata.count('#')
		nslash=0
				
		# no separator
		if nsharp==0:
			# will use default Fiware-Service and Fiware-ServicePath
			data=['','']

			# get number of '/' separator on ldata
			nslash = ldata.count('/')
				
			# contains ['', '', "s1", s1value, "s2", s2value, ...]
			data_array = data + re.split("/", ldata)		
		else:
			data_array = re.split("#", ldata)
		
			# only 1 separator
			if nsharp==1:
				# insert '' to indicate default Fiware-Service
				# as we assume that the only parameter indicate the Fiware-ServicePath
				data_array.insert(0,'');
				# if the length is greater than 2
				if len(data_array[1])<3:
					data_array[1]=''	

			# we have 2 separators
			if nsharp==2:
				# if the length of a fields is greater than 2 then we take it into account
				if len(data_array[0])<3: 
					data_array[0]=''				
				if len(data_array[1])<3:
					data_array[1]=''
									
			# get number of '/' separator on data_array[2]
			# because ldata may contain '/' as Fiware-ServicePath name
			nslash = data_array[2].count('/')
	
			# then reconstruct data_array
			data_array=[data_array[0],data_array[1]]+re.split("/", data_array[2])
				
			# at the end data_array contains
			# ["Fiware-Service", "Fiware-ServicePath", "s1", s1value, "s2", s2value, ...]
		
		# just in case we have an ending CR or 0
		data_array[len(data_array)-1] = data_array[len(data_array)-1].replace('\n', '')
		data_array[len(data_array)-1] = data_array[len(data_array)-1].replace('\0', '')	
																		
		nomenclatures = []
		# data to send
		data = []
		data.append(data_array[0]) #Fiware-service (if '' default)
		data.append(data_array[1]) #Fiware-servicePath (if '' default)
		
		if nslash==0:
			# old syntax without nomenclature key, so insert only one key
			# we use DEF
			nomenclatures.append("DEF")
			data.append(data_array[2])
		else:
			# completing nomenclatures and data
			i=2
			while i < len(data_array)-1 :
				nomenclatures.append(data_array[i])
				data.append(data_array[i+1])
				i += 2

		connected = test_network_available()
	
		#if we got a response from the server, send the data to it	
		if (connected):
			print("Orion: uploading")
			#here we append the device's address to get for instance Sensor2
			#if packet come from a LoRaWAN device with 4-byte devAddr then we will have for instance Sensor01020304
			#where the devAddr is expressed in hex format
			send_data(data, key_Orion.sensor_name+src_str, nomenclatures, tdata)
		else:
			print("Orion: not uploading")
			
			if (CloudNoInternet_enabled):
				print("Using CloudNoInternet")
				from CloudNoInternet import store_internet_pending
				# we call store_internet_pending to store the message for future upload
				store_internet_pending(ldata, pdata, rdata, tdata, gwid)
			
		# update connection_failure value
		global connection_failure
		connection_failure = not connected
		
		#upload data to Orion
		#here we append the device's address to get for instance Sensor2
		#if packet come from a LoRaWAN device with 4-byte devAddr then we will have for instance Sensor01020304
		#where the devAddr is expressed in hex format			
		#Orion_uploadData(nomenclatures, data, key_Orion.sensor_name+src_str, tdata)
	else:
		print "Source is not is source list, not sending with CloudOrion.py"				

if __name__ == "__main__":
	main(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])