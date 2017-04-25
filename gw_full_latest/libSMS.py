#-------------------------------------------------------------------------------
# Copyright 2017 Mamour Diop, University of Pau, France.
# 
# mamour.diop@univ-pau.fr
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

#import json
import os
#import re
import socket
import ssl
#import sys
import time
import urllib2



#------------------------------------------------------------
# Check gammu availability
#------------------------------------------------------------
def gammuCheck():
	try:
		import gammu
		return True	
	except ImportError:
		print "gammu must be installed"
		return False

#------------------------------------------------------------
# Check gammurc file
#------------------------------------------------------------
def gammurcCheck(gammurc_file):
	if os.path.isfile(os.path.expanduser(gammurc_file)):
		return True	
	else:
		print "missing .gammurc file"
		return False

#------------------------------------------------------------
# Connect to phone based on your Gammu configuration
# stored in ~/.gammurc 
#------------------------------------------------------------
def phoneConnection():
	sm = None
	try:
		# Create object for talking with phone
		sm = gammu.StateMachine()
		# Read the configuration (~/.gammurc)
		sm.ReadConfig(Filename=gammurc_file)
		# Connect to the phone
		sm.Init()
	except ImportError:
		print "python-gammu must be installed"
		
	return sm
#------------------------------------------------------------
# Check Internet connection
#------------------------------------------------------------
def internet_ON():
	try:
		# 3sec timeout in case of server available but overcrowded
		response=urllib2.urlopen('http://google.com', timeout=3)
		return True
	except urllib2.URLError, e: pass
	except socket.timeout: pass
	except ssl.SSLError: pass
	
	return False
	
#------------------------------------------------------------
# Check 3G dongle detection
#------------------------------------------------------------
def is_3G_dongle_detected():
	connection = False
	iteration = 0

	# we try 4 times to connect to the phone.
	while(not connection and iteration < 4) :
		response = os.popen('gammu-detect')
		#print response.read()

		if(response.read()==''):
			print('The 3G dongle is not recognized on the USB bus, retrying to connect soon...')
			# wait before retrying
			time.sleep(1)
			iteration += 1
		else:
			#print response.read()
			connection = True

	if(iteration == 4):
		print('The 3G dongle is still not recognized on the USB bus. Might be a driver issue.')

	return connection

#------------------------------------------------------------
# Check operator availability
#------------------------------------------------------------
def is_Operator_detected(sm):	
	operator = False
	iteration = 0

	# we try 4 times to connect to the phone.
	while(not operator and iteration < 4) :
		# Reads network information from phone
		netinfo = sm.GetNetworkInfo()
		if (netinfo['GPRS'] == 'Attached'):
			operator = True
		else:
			print('Operator issue, retrying to connect soon...')
			# wait before retrying
			time.sleep(1)
			iteration += 1
    		
	if(iteration == 4):
		print('Operator issue still.')

	return operator

#------------------------------------------------------------
# Send SMS data
#------------------------------------------------------------
def send_sms(sm, pin, data, numbers):
	
	if (is_3G_dongle_detected()):
		
		if (is_Operator_detected()):
			# Enter PIN code if requested
			if (sm.GetSecurityStatus() == pin):
				sm.EnterSecurityCode('PIN', pin)
        		
			# Prepare SMS template : message data
			# We tell that we want to use first SMSC number stored in phone
			message = {
					'Text': data,
					'SMSC': {'Location': 1},
			}

			# Sending SMS to expected contacts
			for number in numbers:
				message['Number'] = number
				try:
					# Send SMS if all is OK
					sm.SendSMS(message)
					print('Sent to %s successfully!' % (number))
				except Exception, exc:
					print('Sending to %s failed: %s' % (number, exc))



