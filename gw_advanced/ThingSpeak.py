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

# adapted by N. Bertuol under C. Pham supervision 
#
#

import urllib2
import subprocess
import time
import ssl
import socket
import datetime


# WAZIUP LoRa demo channel
#_def_thingspeak_channel_key='ORE0DIFZIIPT61DO'

# LoRa test channel
_def_thingspeak_channel_key='SGSH52UGPVAUYG3S'

# didn't get a response from thingspeak server?
connection_failure = False

# retry if return from server is 0?
retry = False

# function to check connection availability with the server
def test_network_available():
	connection = False
	iteration = 0
	response = None
	
	# we try 4 times to connect to the server.
	while(not connection and iteration < 4) :
	    	try:
	    		# 3sec timeout in case of server available but overcrowded
			response=urllib2.urlopen('https://api.thingspeak.com/', timeout=3)
			connection = True
	    	except urllib2.URLError, e: pass
		except socket.timeout: pass
		except ssl.SSLError: pass
	    	
	    	# if connection_failure == True and the connection with the server is unavailable, don't waste more time, exit directly
	    	if(connection_failure and response is None) :
	    		print('Thingspeak: the server is still unavailable')
	    		iteration = 4
	    	# print connection failure
	    	elif(response is None) :
	    		print('Thingspeak: server unavailable, retrying to connect soon...')
	    		# wait before retrying
	    		time.sleep(1)
	    		iteration += 1
	    		
	return connection
	
# send a data to the server
def send_data(data, second_data):
	global connection_failure

	print 'rcv msg to log (\!) on ThingSpeak (',
				
	#use default channel?
	if data[0]=='':
		data[0]=_def_thingspeak_channel_key
		print 'default',
	else:
		print data[0],
	
	print ',',
		
	#use default field?
	if data[1]=='':
		data[1]='1'
		print 'default',
	else:
		print data[1],				
	
	print '): '+data[2]
		
	#in the current example, log the data on the specified field x, then log the sequence number on field x+4
	#assuming that the first 4 fields are used for up to 4 sensors and the next 4 fields are used for their sequence number
	cmd = 'curl -s -k -X POST --data '+\
		'field'+data[1]+'='+data[2]+\
		'&field'+str(int(data[1])+4)+'='+second_data+\
		' https://api.thingspeak.com/update?key='+data[0]
	
	print("ThingSpeak: will issue curl cmd")
	print(cmd)
	args = cmd.split()
	
	#retry enabled
	if (retry) :
		out = '0'
		iteration = 0
		
		while(out == '0' and iteration < 6 and not connection_failure) :
			try:
				out = subprocess.check_output(args, shell=False)

				#if server return 0, we didn't wait 15sec, wait then
				if(out == '0'):
					print('ThingSpeak: less than 15sec between posts, retrying in 3sec')
					iteration += 1
					time.sleep( 3 )
				else:
					print('ThingSpeak: returned code from server is %s' % out)
			except subprocess.CalledProcessError:
				print("ThingSpeak: curl command failed (maybe a disconnection)")
				
				#update connection_failure
				connection_failure = True
				
	#retry disabled
	else :
		try:
			out = subprocess.check_output(args, shell=False)
			if (out == '0'):
				print('ThingSpeak: returned code from server is %s, do not retry' % out)
			else :
				print('ThingSpeak: returned code from server is %s' % out)
				
		except subprocess.CalledProcessError:
			print("ThingSpeak: curl command failed (maybe a disconnection)")
			connection_failure = True
			
	
def thingspeak_uploadSingleData(data, second_data):
	global connection_failure

	connected = test_network_available()
	
	# if we got a response from the server, send the data to it
	if(connected):
		connection_failure = False
		
		print("ThingSpeak: uploading")
		send_data(data, second_data)
	else:
		connection_failure = True
		
	if(connection_failure):
		print("ThingSpeak: not uploading")
	
# upload multiple data
def thingspeak_uploadMultipleData(data_array):
	global connection_failure
	
	connected = test_network_available()
	
	# if we got a response from the server, send the data to it
	if(connected):
		connection_failure = False
	
		#use default channel?
		if data_array[0]=='':
			data_array[0]=_def_thingspeak_channel_key
			print 'default',
		else:
			print data_array[0],
			
		# we ignore the field and the channel for the moment
		iteration = 3
		fieldNumber = 1
		cmd = 'curl -s -k -X POST --data '
		while(iteration<len(data_array)):
			if(iteration == 3):
				cmd += 'field'+str(fieldNumber)+'='+data_array[iteration]
			else:
				cmd += '&field'+str(fieldNumber)+'='+data_array[iteration]
				
			# don't get the nomenclature so += 2
			iteration += 2
			fieldNumber += 1
			
		cmd += ' https://api.thingspeak.com/update?key='+data_array[0]
		
	
		print("ThingSpeak: will issue curl cmd")
		print(cmd)
		args = cmd.split()
	
		#retry enabled
		if (retry) :
			out = '0'
			iteration = 0
		
			while(out == '0' and iteration < 6 and not connection_failure) :
				try:
					out = subprocess.check_output(args, shell=False)

					#if server return 0, we didn't wait 15sec, wait then
					if(out == '0'):
						print('ThingSpeak: less than 15sec between posts, retrying in 3sec')
						iteration += 1
						time.sleep( 3 )
					else:
						print('ThingSpeak: returned code from server is %s' % out)
				except subprocess.CalledProcessError:
					print("ThingSpeak: curl command failed (maybe a disconnection)")
					
					#update connection_failure
					connection_failure = True
				
		#retry disabled
		else :
			try:
				out = subprocess.check_output(args, shell=False)
				if (out == '0'):
					print('ThingSpeak: returned code from server is %s, do not retry' % out)
				else :
					print('ThingSpeak: returned code from server is %s' % out)
				
			except subprocess.CalledProcessError:
				print("ThingSpeak: curl command failed (maybe a disconnection)")
				connection_failure = True
	else:
		connection_failure = True
		
	if(connection_failure):
		print("ThingSpeak: not uploading")
		
	
	
def thingspeak_setRetry(retry_bool):

	global retry
	retry = retry_bool
	
	
	
	

