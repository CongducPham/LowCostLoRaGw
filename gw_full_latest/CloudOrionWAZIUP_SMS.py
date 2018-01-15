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

# get key definition from external file to ease
# update of cloud script in the future
import key_Orion
import key_OrionWAZIUP_SMS

try:
	key_Orion.source_list
except AttributeError:
	key_Orion.source_list=[]

global sm, always_enabled, gammurc_file

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
	if "CloudOrionWAZIUP_SMS.py" in cloud["script"]:
			
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
	gammurc_file=key_OrionWAZIUP_SMS.gammurc_file
except AttributeError:
	gammurc_file="/home/pi/.gammurc"
	
#check Gammu configuration
if (not libSMS.gammuCheck()):
	print 'CloudOrionWAZIUP_SMS: Gammu is not available'
	sys.exit()
else: 
	if (not libSMS.gammurcCheck(gammurc_file)):
		print 'CloudOrionWAZIUP_SMS: gammurc file is not available'
		sys.exit()

if (libSMS.phoneConnection(gammurc_file, key_OrionWAZIUP_SMS.PIN) == None):
	print 'CloudOrionWAZIUP_SMS: Can not connect to cellular network'
	sys.exit()
else:	
	sm = libSMS.phoneConnection(gammurc_file, key_OrionWAZIUP_SMS.PIN)
		

# main
# -------------------

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

	if (src_str in key_Orion.source_list) or (len(key_Orion.source_list)==0):
					
		#this part depends on the syntax used by the end-device
		#we use: thingspeak_channel#thingspeak_field#TC/22.4/HU/85... 
		#ex: ##TC/22.4/HU/85... or TC/22.4/HU/85... or thingspeak_channel##TC/22.4/HU/85... 
		#or #thingspeak_field#TC/22.4/HU/85... to use some default value
	
		# get number of '#' separator
		nsharp = ldata.count('#')
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
				# if the length of BOTH fields is greater than 2 then we take them into account
				if len(data_array[0])<3 or len(data_array[1])<3:
					data_array[0]=''
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
	
		#sms data to be sent
		sms_data = "SensorData "+key_Orion.sensor_name+src_str
	
		#sms_data = "SRC#"+str(src)+"#RSSI#"+str(RSSI)+"#BW#"+str(bw)+"#CR#"+str(cr)+"#SF#"+str(sf)+"#GWID#"+gwid+"/"+data
		#sms_data = "SensorData  Sensor"+str(src)+" RSSI "+str(RSSI)+" BW "+str(bw)+" CR "+str(cr)+" SF "+str(sf)+" GWID "+gwid+" "
	
		#start from the first nomenclature
		i = 2
	
		while i < len(data_array)-1 :
			sms_data += " "+data_array[i]+" "+data_array[i+1]
			i += 2
	
		if data_array[0]=='':
			data_array[0]=key_Orion.project_name

		if data_array[1]=='':
			data_array[1]=key_Orion.service_path
		
		sms_data += " "+data_array[0]+" "+data_array[1]
	
		# Send data to expected contacts
		success = False
		
		print("rcv msg to send via the Orion WAZIUP SMS Service: "+sms_data)
		success = libSMS.send_sms(sm, sms_data, key_OrionWAZIUP_SMS.contacts)
	
		if (success):
				print "Sending SMS done"	
	else:
		print "Source is not is source list, not sending with CloudOrionWAZIUP_SMS.py"
		
if __name__ == "__main__":
        main(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])
