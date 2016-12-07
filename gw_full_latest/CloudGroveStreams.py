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

# 2015. Adapted by C. Pham from GroveStreams template
# Congduc.Pham@univ-pau.fr
# University of Pau, France
#
# Jul/2016. adapted by N. Bertuol under C. Pham supervision 
# nicolas.bertuol@etud.univ-pau.fr
#
# Oct/2016. re-designed by C. Pham to use self-sufficient script per cloud
# Congduc.Pham@univ-pau.fr

import time
import datetime
import httplib
import random
import urllib
import urllib2
import ssl
import socket
import sys
import re

# get key definition from external file to ease
# update of cloud script in the future
import key_GroveStreams

base_url = '/api/feed?'

# didn't get a response from grovestreams server?
connection_failure=False
    
# function to check connection availability with the server
def test_network_available():
	connection = False
	iteration = 0
	response = None
	
	# we try 4 times to connect to the server.
	while(not connection and iteration < 4) :
	    	try:
	    		# 3sec timeout in case of server available but overcrowded
			response=urllib2.urlopen('https://www.grovestreams.com/', timeout=3)
			connection = True
	    	except urllib2.URLError, e: pass
		except socket.timeout: pass
		except ssl.SSLError: pass
	    	
	    	# if connection_failure == True and the connection with the server is unavailable, don't waste more time, exit directly
	    	if(connection_failure and response is None) :
	    		print('GroveStreams: the server is still unavailable')
	    		iteration = 4
	    	# print connection failure
	    	elif(response is None) :
	    		print('GroveStreams: server unavailable, retrying to connect soon...')
	    		# wait before retrying
	    		time.sleep(1)
	    		iteration += 1
	    		
	return connection

# send a data to the server
def send_data(data, src, nomenclatures):
	#10seconds timeout in case we lose connection right here, then do not waste time
	conn = httplib.HTTPConnection('www.grovestreams.com', timeout=10)
	
	#Upload the feed
	try:

		url = base_url + "compId="+src
		
		#add in the url the channels and the data
		i = 0
		while i < len(data) :
			url += "&"+nomenclatures[i]+"="+data[i]
			i += 1
			                       
		headers = {"Connection" : "close", "Content-type": "application/json", "Cookie" : "api_key="+key_GroveStreams.api_key}

		print('Grovestreams: Uploading feed to: ' + url)

		conn.request("PUT", url, "", headers)

		#Check for errors
		response = conn.getresponse()
		status = response.status

		if status != 200 and status != 201:
			try:
				if (response.reason != None):
					print('Grovestreams: HTTP Failure Reason: ' + response.reason + ' body: ' + response.read())
				else:
					print('Grovestreams: HTTP Failure Body: ' + response.read())
			
			except Exception:
				print('Grovestreams: HTTP Failure Status: %d' % (status) )

	except Exception as e:
		print('Grovestreams: HTTP Failure: ' + str(e))

	finally:
		if conn != None:
			conn.close()	
	
	
def grovestreams_uploadData(nomenclatures, data, src):
	
	connected = test_network_available()
	
	# if we got a response from the server, send the data to it
	if(connected):
		#len(nomenclatures) == len(data)
		print("GroveStreams: uploading")
		send_data(data, "node_"+src, nomenclatures)
	else:
		print("Grovestreams: not uploading")
		
	#update grovestreams_connection_failure value
	global connection_failure
	connection_failure = not connected

# main
# -------------------

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
	
	# this part depends on the syntax used by the end-device
	# we use: TC/22.4/HU/85...
	#
	# but we accept also a_str#b_str#TC/22.4/HU/85... for compatibility with ThingSpeak
	# or simply 22.4 in which case, the nomemclature will be DEF

	# get number of '#' separator
	nsharp = ldata.count('#')		 		
	nslash=0

	#will field delimited by '#' in the MongoDB
	data=['','']
				
	#no separator
	if nsharp==0:
		# get number of '/' separator on ldata
		nslash = ldata.count('/')
				
		#contains ['', '', "s1", s1value, "s2", s2value, ...]
		data_array = data + re.split("/", ldata)		
	else:				
		data_array = re.split("#", ldata)
		
		# only 1 separator
		if nsharp==1:
			# get number of '/' separator on data_array[1]
			nslash = data_array[1].count('/')
		
			# then reconstruct data_array
			data_array=data + re.split("/", data_array[1])	

		# we have 2 separators
		if nsharp==2:
			# get number of '/' separator on data_array[2]
			nslash = data_array[2].count('/')
		
			# then reconstruct data_array
			data_array=data + re.split("/", data_array[2])
		
	#just in case we have an ending CR or 0
	data_array[len(data_array)-1] = data_array[len(data_array)-1].replace('\n', '')
	data_array[len(data_array)-1] = data_array[len(data_array)-1].replace('\0', '')	
	
	#test if there are characters at the end of each value, then delete these characters
	i = 3
	while i < len(data_array) :
		while not data_array[i][len(data_array[i])-1].isdigit() :
			data_array[i] = data_array[i][:-1]
		i += 2
																		
	nomenclatures = []
	# data to send	
	data = []
	
	if nslash==0:
		# old syntax without nomenclature key, so insert only one key
		# we use DEF
		nomenclatures.append("DEF")
		data.append(data_array[2])
	else:
		#completing nomenclatures and data
		i=2
		while i < len(data_array)-1 :
			nomenclatures.append(data_array[i])
			data.append(data_array[i+1])
			i += 2
	
	#upload data to grovestreams
	grovestreams_uploadData(nomenclatures, data, str(src))		

if __name__ == "__main__":
	main(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])
