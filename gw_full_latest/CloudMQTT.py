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
import key_MQTT

try:
	key_MQTT.source_list
except AttributeError:
	key_MQTT.source_list=[]

# didn't get a response from server?
connection_failure = False

# function to check connection availability with the server
def test_network_available():
	connection = False
	iteration = 0
	response = None
	
	# we try 4 times to connect to the server.
	while(not connection and iteration < 4) :
		try:
			# 3sec timeout in case of server available but overcrowded
			response=urllib2.urlopen('http://'+key_MQTT.MQTT_server, timeout=3)
			connection = True
		except urllib2.URLError, e: pass
		except socket.timeout: pass
		except ssl.SSLError: pass
	    	
		# if connection_failure == True and the connection with the server is unavailable, don't waste more time, exit directly
		if(connection_failure and response is None) :
			print('MQTT: the server is still unavailable')
			iteration = 4
			# print connection failure
		elif(response is None) :
			print('MQTT: server unavailable, retrying to connect soon...')
			# wait before retrying
			time.sleep(1)
			iteration += 1
	    		
	return connection
	
# send a data to the server
def send_data(data, src, nomenclatures, tdata):

	global connection_failure
	
	i=0
	
	if data[0]=='':
		data[0]=key_MQTT.project_name

	if data[1]=='':
		data[1]=key_MQTT.organization_name
			
	while i < len(data)-2:

		#we use mosquitto client
		#be sure to have run sudo apt-get install mosquitto-clients
		#the topic will be for instance waziup_UPPA_Sensor2/TC
		if nomenclatures=='':
			cmd = 'mosquitto_pub -h '+key_MQTT.MQTT_server+' -t '+data[0]+'/'+data[1]+'/'+src+' -m \"'+data[i+2]+'\"'			
		else:		
			cmd = 'mosquitto_pub -h '+key_MQTT.MQTT_server+' -t '+data[0]+'/'+data[1]+'/'+src+'/'+nomenclatures[i]+' -m \"'+data[i+2]+'\"'

		i += 1
						
		print "CloudMQTT: will issue cmd"
		
		print(cmd)
		args = shlex.split(cmd)
		print args
		
		try:
			out = subprocess.check_output(args, shell=False)
			
			if out != '':
				print 'MQTT: returned msg from server is'
				print out					
			else :
				print 'MQTT: publish success'
				
		except subprocess.CalledProcessError:
			print "MQTT: publish command failed (maybe a disconnection)"
			connection_failure = True
	
def MQTT_uploadData(nomenclatures, data, src, tdata):
	
	connected = test_network_available()
	
	#if we got a response from the server, send the data to it	
	if(connected):
		print("MQTT: publishing")
		send_data(data, src, nomenclatures, tdata)
	else:
		print("MQTT: not publishing")
		
	# update connection_failure value
	global connection_failure
	connection_failure = not connected

# main
# -------------------
#
# ldata can be formatted to indicate a specifc project and organization name. Options are:
# 	TC/22.4/HU/85 -> use default project and organization name
#	UPPA#TC/22.4/HU/85 -> use default project and organization name=UPPA
#	waziup#UPPA#TC/22.4/HU/85 -> project=waziup and organization name=UPPA
#
#	project and organization name must BOTH have more than 2 characters
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

	if (src_str in key_MQTT.source_list) or (len(key_MQTT.source_list)==0):

		#LoRaWAN (so encrypted packet)
		#or encapsulated encrypted
		if ptype & 0x40 == 0x40 or ptype & 0x80 == 0x80 or ptype & 0x04 == 0x04:
			nomenclatures = ''
			data=['','']
			data.append(ldata)		
			MQTT_uploadData(nomenclatures, data, key_MQTT.sensor_name+src_str, tdata)
		else:
						
			# this part depends on the syntax used by the end-device
			# we use: TC/22.4/HU/85...
			#
			# but we accept also a_str#b_str#TC/22.4/HU/85... to indicate a project and organization
			# or simply 22.4 in which case, the nomemclature will be DEF
				
			# get number of '#' separator
			nsharp=ldata.count('#')
			nslash=0
				
			# no separator
			if nsharp==0:
				# will use default project and organisation name
				data=['','']

				# get number of '/' separator on ldata
				nslash = ldata.count('/')
				
				# contains ['', '', "s1", s1value, "s2", s2value, ...]
				data_array = data + re.split("/", ldata)		
			else:
				data_array = re.split("#", ldata)
		
				# only 1 separator
				if nsharp==1:
					# insert '' to indicate default project
					# as we assume that the only parameter indicate the organisation name
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
				# because ldata may contain '/' 
				nslash = data_array[2].count('/')
	
				# then reconstruct data_array
				data_array=[data_array[0],data_array[1]]+re.split("/", data_array[2])
				
				# at the end data_array contains
				# ["project", "organisation_name", "s1", s1value, "s2", s2value, ...]
		
			# just in case we have an ending CR or 0
			data_array[len(data_array)-1] = data_array[len(data_array)-1].replace('\n', '')
			data_array[len(data_array)-1] = data_array[len(data_array)-1].replace('\0', '')	
																		
			nomenclatures = []
			# data to send
			data = []
			data.append(data_array[0]) #project (if '' default)
			data.append(data_array[1]) #organization name (if '' default)
		
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

			#here we append the device's address to get for instance UPPA_Sensor2
			#if packet come from a LoRaWAN device with 4-byte devAddr then we will have for instance UPPA_Sensor01020304
			#where the devAddr is expressed in hex format		
			MQTT_uploadData(nomenclatures, data, key_MQTT.sensor_name+src_str, tdata)
	else:
		print "Source is not is source list, not sending with CloudMQTT.py"				

if __name__ == "__main__":
	main(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])