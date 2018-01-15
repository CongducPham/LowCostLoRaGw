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
import re
import sys
import json
import libSMS

import key_SMS

global sm, always_enabled, gammurc_file

try:
	key_SMS.source_list
except AttributeError:
	key_SMS.source_list=[]	

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
	if "CloudSMS.py" in cloud["script"]:
			
		try:
			always_enabled = cloud["always_enabled"]
		except KeyError:
			print "always_enabled undefined"
			always_enabled = False
		
		try:
			gammurc_file = cloud["gammurc_file"]
		except KeyError:
			gammurc_file="/home/pi/.gammurc"			

if not always_enabled:
	if (libSMS.internet_ON()):
		print('Internet is available, no need to use the SMS Service')
		sys.exit()

try:
	gammurc_file=key_SMS.gammurc_file
except AttributeError:
	gammurc_file="/home/pi/.gammurc"
			
#check Gammu configuration
if (not libSMS.gammuCheck()):
	print 'CloudSMS: Gammu is not available'
	sys.exit()
else: 
	if (not libSMS.gammurcCheck(gammurc_file)):
		print 'CloudSMS: gammurc file is not available'
		sys.exit()

if (libSMS.phoneConnection(gammurc_file, key_SMS.PIN) == None):
	print 'CloudSMS: Can not connect to cellular network'
	sys.exit()
else:	
	sm = libSMS.phoneConnection(gammurc_file, key_SMS.PIN)
		
#------------------------------------------------------------
# main
#------------------------------------------------------------

def main(ldata, pdata, rdata, tdata, gwid):
	
	#this is common code to process packet information provided by the main gateway script (i.e. post_processing_gw.py)
	#these information are provided in case you need them   
	arr = map(int,pdata.split(','))
	dst=arr[0]
	ptype=arr[1]
	src=arr[2]
	seq=arr[3]
	datalen=arr[4]
	SNR=arr[5]
	RSSI=arr[6]
	
	arr = map(int,rdata.split(','))
	bw=arr[0]
	cr=arr[1]
	sf=arr[2]

	#LoRaWAN packet
	if dst==256:
		src_str="%0.8X" % src
	else:
		src_str=str(src)	

	if (src_str in key_SMS.source_list) or (len(key_SMS.source_list)==0): 
				
		#this part depends on the syntax used by the end-device
		#we use: thingspeak_channel#thingspeak_field#TC/22.4/HU/85... 
		#ex: ##TC/22.4/HU/85... or TC/22.4/HU/85... or thingspeak_channel##TC/22.4/HU/85... 
		#or #thingspeak_field#TC/22.4/HU/85... to use some default value
	
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
	
		#sms data to be sent
		#here we append the device's address to get for instance Sensor2
		#if packet come from a LoRaWAN device with 4-byte devAddr then we will have for instance Sensor01020304
		#where the devAddr is expressed in hex format		
		sms_data = "SensorData Sensor"+src_str+" SNR "+str(SNR)+" RSSI "+str(RSSI)+" SN "+str(seq)+" BW "+str(bw)+" CR "+str(cr)+" SF "+str(sf)+" GWID "+gwid
		
		nomenclatures = []
		# data to send
		data = []
		data.append(data_array[0]) 
		data.append(data_array[1])
		
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
		
		#add the nomemclatures and sensed values
		i = 0
		while i < len(data)-2:
			sms_data += " "+nomenclatures[i]+" "+data[i+2]
			i += 1
	
		# Send data to expected contacts
		success = False

		print("rcv msg to send via the SMS Service: "+sms_data)
		success = libSMS.send_sms(sm, sms_data, key_SMS.contacts)
		
		if (success):
			print "Sending SMS done"	
	else:
		print "Source is not is source list, not sending with CloudSMS.py"
		
if __name__ == "__main__":
        main(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])
