#GroveStreams.com Python version 2.7 Feed Example
#Demonstrates uploading two stream feeds within a URL and URL
# encoding
 
#This example uploads two stream feeds, random temperature and humidity
# samples every 10 seconds.
#A full "how to" guide for this example can be found at:
# https://www.grovestreams.com/developers/getting_started_helloworld_python.html
#It relies and the GroveStreams API which can be found here:
# https://www.grovestreams.com/developers/api.html#1

# License:
# Copyright 2014 GroveStreams LLC.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at: http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#GroveStreams Setup:
 
#* Sign Up for Free User Account - https://www.grovestreams.com
#* Create a GroveStreams organization
#* Enter the GroveStreams org uid under "GroveStreams Settings" below
#*    (Can be retrieved from a GroveStreams organization: 
#*     Tools toolbar button - View Organization UID)
#* Enter the GroveStreams api key under "GroveStreams Settings" below  
#*    (Can be retrieved from a GroveStreams organization: 
#*     click the Api Keys toolbar button,
#*     select your Api Key, and click View Secret Key)

# 2015. Adapted by C. Pham and N. Bertuol 
# Congduc.Pham@univ-pau.fr
# University of Pau, France
#

import time
import datetime
import httplib
import random
import urllib
import urllib2
import ssl
import socket

#GroveStreams Settings

### Change This!!!
api_key = "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX"
### 
	
base_url = '/api/feed?'
channel_name="temperature"

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
			                       
		headers = {"Connection" : "close", "Content-type": "application/json", "Cookie" : "api_key="+api_key}

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
	
	
def grovestreams_uploadSingleData(nomenclatures, data, src):
	
	connected = test_network_available()
	
	# if we got a response from the server, send the data to it
	if(connected):
		#len(nomenclatures) == len(data)
		print("GroveStreams: upload")
		send_data(data, "node_"+src, nomenclatures)
	else:
		print("Grovestreams: not uploading")
		
	#update grovestreams_connection_failure value
	global connection_failure
	connection_failure = not connected
		

if __name__ == '__main__':

	component_id = "sensor-test"

	conn = httplib.HTTPConnection('www.grovestreams.com')
	
	while True:
	
		temperature_val = random.randrange(-10, 40)
		humidity_val = random.randrange(0, 100)
		
		#Upload the feed
		try:    
			#Let the GS servers set the sample times. Encode parameters
			url = base_url + urllib.urlencode({'compId' : component_id, 'temperature' : temperature_val, 'humidity' : humidity_val})
			
			#Alternative URL that includes the sample time
			#now = datetime.datetime.now()
			#sample_time = int(time.mktime(now.timetuple())) * 1000
			#url = base_url + urllib.urlencode({'compId' : component_id, 'time' : sample_time, 'temperature' : temperature_val, 'humidity' : humidity_val})
			
			#Alternative URL that uses the stream order to determine where
			# to insert the samples
			#url = base_url + urllib.urlencode({'compId' : component_id, 'data' : [temperature_val, humidity_val]}, True)
			
			#The api_key token can be passed as URL parameters or as a cookie.
			# We've chosen to pass it as a cookie to keep the URL length small as
			# some devices have a URL size limit
			headers = {"Connection" : "close", "Content-type": "application/json", "Cookie" : "api_key="+api_key}
			
			#GS limits feed calls to one per 10 seconds per outward facing router IP address
			#Use the ip_addr and headers assignment below to work around this 
			# limit by setting the below to this device's IP address
			#ip_addr = "192.168.1.72"
			#headers = {"Connection" : "close", "Content-type": "application/json", "X-Forwarded-For": ip_addr, "Cookie" : "org="+org+";api_key="+api_key}
			
			print('Uploading feed to: ' + url)
			
			conn.request("PUT", url, "", headers)
		
			#Check for errors
			response = conn.getresponse()
			status = response.status
		
			if status != 200 and status != 201:
				try:
					if (response.reason != None):
						print('HTTP Failure Reason: ' + response.reason + ' body: ' + response.read())
					else:
						print('HTTP Failure Body: ' + response.read())
					
				except Exception:
					print('HTTP Failure Status: %d' % (status) )
		
		except Exception as e:
			print('HTTP Failure: ' + str(e))
		
		finally:
			if conn != None:
				conn.close()
        
		#Pause for ten seconds
		time.sleep(10)
		
	# quit
	exit(0)
