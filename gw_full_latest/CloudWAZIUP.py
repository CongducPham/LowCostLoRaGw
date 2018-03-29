#-------------------------------------------------------------------------------
# Copyright 2016 Congduc Pham, University of Pau, France.
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
import requests
import subprocess
import time
import ssl
import socket
import datetime
import sys
import os
import json
import re
import md5
#import shlex

# get key definition from external file to ease
# update of cloud script in the future
import key_WAZIUP

try:
	key_WAZIUP.source_list
except AttributeError:
	key_WAZIUP.source_list=[]

####################################################
# To create a new entitiy
# curl -X POST "http://dev.waziup.io/api/v1/domains/waziup-UPPA-TESTS/sensors" -H "accept:application/json" -H "content-type:application/json" -d @- <<EOF 
# {
#  "id": "UPPA_Sensor2",
#  "gateway_id": "4b13a223f24d3dba5403c2727fa92e62",
#  "measurements": [ 
#    { 
#      "id": "TC",
#      "values": [
#           {
#             "value": "25.6",
#             "timestamp": "2016-06-08T18:20:27.873Z"
#           }
#		]
#    }
#   ]
# }
# EOF

# Note that if organization_name is 'ORG' (default value), then 'Id' will be appended with the gw's md5 hash: "Id": "ORG_Sensor2_4b13a223f24d3dba5403c2727fa92e62"

# curl -X POST "http://dev.waziup.io:80/api/v1/domains/waziup-UPPA-TESTS/sensors" -H "accept:application/json" -H "content-type:application/json" -d '{"id":"UPPA_Sensor2","gateway_id":"4b13a223f24d3dba5403c2727fa92e62","measurements":[{"id":"TC","values":[{"value": "25.6","timestamp":"2016-06-08T18:20:27.873Z"}]}]}'

# Further updates of the values are like that:
# curl -X POST "http://dev.waziup.io/api/v1/domains/waziup-UPPA-TESTS/sensors/UPPA_Sensor2/measurements/TC/values" -H  "accept:application/json" -H  "content-type:application/json" -d @- <<EOF
# {  
#    "value": "25.6",
#    "timestamp": "2016-06-08T18:20:27.873Z"
# }

# curl -X POST "http://dev.waziup.io/api/v1/domains/waziup-UPPA-TESTS/sensors/UPPA_Sensor2/measurements/TC/values" -H  "accept:application/json" -H  "content-type:application/json" -d '{"value":"25.6","timestamp":"2016-06-08T18:20:27.873Z"}'

#To retrieve the last data point inserted:
#curl -X GET "http://dev.waziup.io/api/v1/domains/waziup-UPPA-TESTS/sensors/UPPA_Sensor2/measurements/TC" -H  "accept:application/json" -H  "content-type:application/json"

####################################################

# didn't get a response from server?
connection_failure = False

gw_id_md5=''

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
			response=urllib2.urlopen(key_WAZIUP.waziup_server+'/domains/waziup/sensors', timeout=3)
			connection = True
		except urllib2.URLError, e: pass
		except socket.timeout: pass
		except ssl.SSLError: pass
	    	
		# if connection_failure == True and the connection with the server is unavailable, don't waste more time, exit directly
		if(connection_failure and response is None) :
			print('WAZIUP: the server is still unavailable')
			iteration = 4
			# print connection failure
		elif(response is None) :
			print('WAZIUP: server unavailable, retrying to connect soon...')
			# wait before retrying
			time.sleep(1)
			iteration += 1
	    		
	return connection

def create_new_entity(data, src, nomenclatures, tdata):

	global connection_failure
	
	print "WAZIUP: create new entity"
	
	append_gw_str=''
	
	if key_WAZIUP.organization_name=='':
		key_WAZIUP.organization_name='ORG'
		
	#default organization name?
	if key_WAZIUP.organization_name=='ORG':
		#we append the md5 hash to the sensor name: ORG_Sensor2_4b13a223f24d3dba5403c2727fa92e62
		append_gw_str='_'+gw_id_md5		

	WAZIUP_headers = {'accept':'application/json','content-type':'application/json'}
	
	WAZIUP_url=key_WAZIUP.waziup_server+'/domains/'+key_WAZIUP.project_name+key_WAZIUP.service_path+'/sensors' 
	
	WAZIUP_data = '{"id":"'+key_WAZIUP.organization_name+'_'+src+append_gw_str+'","gateway_id":"'+gw_id_md5+'","measurements":['
	
	i=0
	
	while i < len(data)-2:
		
		WAZIUP_data = WAZIUP_data+'{"id":"'+nomenclatures[i]+'","values":[{"value":'
		
		isnumber = re.match(num_format,data[i+2])
		isnumber = False
		
		if isnumber:
			WAZIUP_data = WAZIUP_data+data[i+2]+',"timestamp":"'+tdata+'"}]}'
		else:	
			WAZIUP_data = WAZIUP_data+'"'+data[i+2]+'","timestamp":"'+tdata+'"}]}'
		i += 1
		if i < len(data)-2:
			WAZIUP_data = WAZIUP_data+','
			
	WAZIUP_data = WAZIUP_data+']'
	WAZIUP_data = WAZIUP_data+'}'
	
	print "CloudWAZIUP: will issue requests with"
		
	print 'url: '+WAZIUP_url
	#print 'headers: '+json.dumps(WAZIUP_headers)
	print 'data: '+WAZIUP_data
	
	try:
		response = requests.post(WAZIUP_url, headers=WAZIUP_headers, data=WAZIUP_data, timeout=30)

		print 'WAZIUP: returned msg from server is ',
		print response.status_code	
						
		if response.ok:
			print 'WAZIUP: entity creation success'
	except ConnectTimeout:
		print 'WAZIUP: requests command failed (maybe a disconnection)'
		connection_failure = True 	
		
		
# send a data to the server
def send_data(data, src, nomenclatures, tdata):

	global connection_failure
	
	entity_need_to_be_created=False
	
	append_gw_str=''
	
	i=0
	
	if data[0]=='':
		data[0]=key_WAZIUP.project_name

	if data[1]=='':
		data[1]=key_WAZIUP.service_path
			
	while i < len(data)-2  and not entity_need_to_be_created:

		if key_WAZIUP.organization_name=='':
			key_WAZIUP.organization_name='ORG'
		
		#default organization name?
		if key_WAZIUP.organization_name=='ORG':
			#we append the md5 hash to the sensor name: ORG_Sensor2_4b13a223f24d3dba5403c2727fa92e62
			append_gw_str='_'+gw_id_md5
		
		#with WAZIUP new API, it is always a string	
		isnumber = re.match(num_format,data[i+2])
		isnumber = False
		
		WAZIUP_headers = {'accept':'application/json','content-type':'application/json'}
		
		WAZIUP_url=key_WAZIUP.waziup_server + '/domains/'+key_WAZIUP.project_name+key_WAZIUP.service_path+'/sensors/'+key_WAZIUP.organization_name+"_"+src+append_gw_str+'/measurements/'+nomenclatures[i]+'/values' 
		
		WAZIUP_data = '{"value":'
		
		if isnumber:
			WAZIUP_data = WAZIUP_data+data[i+2]+',"timestamp":"'+tdata+'"}'
		else:
			WAZIUP_data = WAZIUP_data+'"'+data[i+2]+'","timestamp":"'+tdata+'"}'
					
		print "CloudWAZIUP: will issue requests with"
		print 'url: '+WAZIUP_url
		#print 'headers: '+json.dumps(WAZIUP_headers)
		print 'data: '+WAZIUP_data
		
		try:
			response = requests.post(WAZIUP_url, headers=WAZIUP_headers, data=WAZIUP_data, timeout=30)

			print 'WAZIUP: returned msg from server is ',
			print response.status_code

			if response.ok:
				print 'WAZIUP: upload success'
				i += 1
			elif response.status_code==403 or response.status_code==404:
				#we try to (re-)create the measurement first
				WAZIUP_url=key_WAZIUP.waziup_server + '/domains/'+key_WAZIUP.project_name+key_WAZIUP.service_path+'/sensors/'+key_WAZIUP.organization_name+"_"+src+append_gw_str+'/measurements'
				WAZIUP_data = '{"id":"'+nomenclatures[i]+'"}'
				print "CloudWAZIUP: will try to create measurement and issue requests with"
				print 'url: '+WAZIUP_url
				print 'data: '+WAZIUP_data	
				
				try:
					response = requests.post(WAZIUP_url, headers=WAZIUP_headers, data=WAZIUP_data, timeout=30)
								
					print 'WAZIUP: returned msg from server is ',
					print response.status_code		
					
					if response.ok:
						print 'WAZIUP: measurement creation success'
						print 'WAZIUP: retry upload'
					else:	
						entity_need_to_be_created=True

				except ConnectTimeout:
					print 'WAZIUP: requests command failed (maybe a disconnection)'
					connection_failure = True						
				
			if entity_need_to_be_created:
				create_new_entity(data, src, nomenclatures, tdata)
				
		except ConnectTimeout:
			print 'WAZIUP: requests command failed (maybe a disconnection)'
			connection_failure = True			

# main
# -------------------
#
# ldata can be formatted to indicate a specific project_name and service_path. Options are:
# 	TC/22.4/HU/85 -> use default project_name and service_path
#	-UPPA-TESTS#TC/22.4/HU/85 -> use default project_name and service_path=-UPPA-TESTS
#	waziup#-UPPA-TEST#TC/22.4/HU/85 -> project_name=waziup and service_path=-UPPA-TESTS
#
#	project_name and service_path must BOTH have more than 2 characters
#
#   the final domain will be project_name+service_path, e.g waziup-UPPA-TESTS
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
	
	#compute the MD5 digest (hash) from the clear gw id provided by post_processing_gw.py
	global gw_id_md5
	gw_id_md5=md5.new(gwid).hexdigest()
	
	#LoRaWAN packet
	if dst==256:
		src_str="%0.8X" % src
	else:
		src_str=str(src)	

	if (src_str in key_WAZIUP.source_list) or (len(key_WAZIUP.source_list)==0):

		#remove any space in the message as we use '/' as the delimiter
		#any space characters will introduce error in the json structure and then the curl command will fail
		ldata=ldata.replace(' ', '')
			
		# this part depends on the syntax used by the end-device
		# we use: TC/22.4/HU/85...
		#
		# but we accept also a_str#b_str#TC/22.4/HU/85... to indicate a project_name and a service_path
		# or simply 22.4 in which case, the nomemclature will be DEF
		 		
		# get number of '#' separator
		nsharp=ldata.count('#')
		nslash=0
				
		# no separator
		if nsharp==0:
			# will use default project_name and service_path
			data=['','']

			# get number of '/' separator on ldata
			nslash = ldata.count('/')
				
			# contains ['', '', "s1", s1value, "s2", s2value, ...]
			data_array = data + re.split("/", ldata)		
		else:
			data_array = re.split("#", ldata)
		
			# only 1 separator
			if nsharp==1:
				# insert '' to indicate default project name
				# as we assume that the only parameter indicate the service_path
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
			# because ldata may contain '/' in service_path name
			nslash = data_array[2].count('/')
	
			# then reconstruct data_array
			data_array=[data_array[0],data_array[1]]+re.split("/", data_array[2])
				
			# at the end data_array contains
			# ["project_name", "service_path", "s1", s1value, "s2", s2value, ...]
		
		# just in case we have an ending CR or 0
		data_array[len(data_array)-1] = data_array[len(data_array)-1].replace('\n', '')
		data_array[len(data_array)-1] = data_array[len(data_array)-1].replace('\0', '')	
																		
		nomenclatures = []
		# data to send
		data = []
		data.append(data_array[0]) #project_name (if '' default)
		data.append(data_array[1]) #service_path (if '' default)
		
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
			print("WAZIUP: uploading")
			#here we append the device's address to get for instance Sensor2
			#if packet come from a LoRaWAN device with 4-byte devAddr then we will have for instance Sensor01020304
			#where the devAddr is expressed in hex format
			send_data(data, key_WAZIUP.sensor_name+src_str, nomenclatures, tdata)
		else:
			print("WAZIUP: not uploading")
			
			if (CloudNoInternet_enabled):
				print("Using CloudNoInternet")
				from CloudNoInternet import store_internet_pending
				# we call store_internet_pending to store the message for future upload
				store_internet_pending(ldata, pdata, rdata, tdata, gwid)
			
		# update connection_failure value
		global connection_failure
		connection_failure = not connected
			
	else:
		print "Source is not is source list, not sending with CloudWAZIUP.py"				

if __name__ == "__main__":
	main(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])