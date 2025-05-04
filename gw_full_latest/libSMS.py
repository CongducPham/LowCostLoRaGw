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


import os
import socket
import ssl
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
		print "gammu and python-gammu must be installed"
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
def phoneConnection(gammurc_file, pin):
	sm = None
	try:
		import gammu
		
		if (gammurcCheck(gammurc_file)):
			# Create object for talking with phone
			sm = gammu.StateMachine()
			# Read the configuration (~/.gammurc)
			sm.ReadConfig(Filename=gammurc_file)
			
			# Connect to the phone
			sm.Init()
			
			if (sm.GetSecurityStatus() == "PIN"):
				sm.EnterSecurityCode("PIN", pin)
			
			# Read network information from phone 
			netInfo = sm.GetNetworkInfo()
			#print netInfo
			if (netInfo['GPRS'] != 'Attached'):
				print "Operator issue"
				sm = None
	except ImportError: 
		print "gammu and python-gammu must be installed"
	except gammu.ERR_DEVICENOTEXIST: 
		#print "Device does not exit"
		print "3G dongle is not detected"
		sm = None
	except gammu.ERR_TIMEOUT: 
		print "No response in specified timeout"
		sm = None
	except gammu.ERR_SECURITYERROR:
		print "Pin code issue: enter the correct pin code"
		print "You can also remove the pin code protection on the SIM card"
		sm = None
	except gammu.ERR_UNKNOWN:
		print "Pin code issue: enter the correct pin code"
		print "You can also remove the pin code protection on the SIM card"
		sm = None
	#except gammu.ERR_UNKNOWN:
	#	print "Gammu Unknow Error"
	#	sm = None
		
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
# Send SMS data
#------------------------------------------------------------
def send_sms(sm, data, numbers):
	
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
			success = True
		except Exception, exc:
			print('Sending to %s failed: %s' % (number, exc))
			success = False
			
	return success	


