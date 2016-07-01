#-------------------------------------------------------------------------------
# Copyright 2015 Congduc Pham, University of Pau, France.
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
import socket
import time
import datetime
import ssl

#change here for your own firebase url when you have one
firebase_database='https://XXXXXXXXXXX.firebaseio.com'

# didn't get a response from firebase server?
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
			response=urllib2.urlopen('https://www.firebase.com/', timeout=3)
			connection = True
	    	except urllib2.URLError, e: pass
		except socket.timeout: pass
		except ssl.SSLError: pass
			
    		# if connection_failure == True and the connection with the server is unavailable, don't waste more time, exit directly
	    	if(connection_failure and response is None) :
	    		print('FireBase: the server is still unavailable')
	    		iteration = 4
	    	# print connection failure
	    	elif(response is None) :
	    		print('FireBase: server unavailable, retrying to connect soon...')
	    		# wait before retrying
	    		time.sleep(1)
	    		iteration += 1
		    		
	    		
	return connection
	
# msg is already in JSON format
def send_data(msg, path):
	from firebase import Firebase
	try:
		fbase = Firebase(path)
		fbase.push(msg)
	except Exception as e:
		print("FireBase: can not send the data to the server:" + str(e))
		print("FireBase: not uploading")
		return "exception"
		
	
def firebase_uploadSingleData(msg, sensor, msg_entry, date):
	global connection_failure
	
	# test if network is available before sending to the server
	connected = test_network_available()
	
	# if we got a response from the server, send the data to it	
	if(connected) :
		print("FireBase: uploading data")
		send_to = firebase_database+date.date().isoformat()+'/'+sensor+'/'+msg_entry
		time.sleep(10)
		str_result = send_data(msg, send_to)
		
		#exception while sending data, probably a disconnection error
		if(str_result == "exception"):
			connection_failure = True
		else:
			connection_failure = False
			
	else:
		print("FireBase: not uploading")
		connection_failure = True
	
