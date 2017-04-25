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
import sys
import json
import libSMS

# get key definition from external file to ease
# update of cloud script in the future
import key_WAZIUP

global sm, PIN, contacts, always_enabled, gammurc_file

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
	if "CloudWAZIUP-SMS.py" in cloud["script"]:
		try:
			PIN = cloud["pin"]
		except KeyError:
			print "pin undefined"
		
		try:
			contacts = cloud["contacts"]
		except KeyError:
			print "contacts undefined"
			
		try:
			always_enabled = cloud["always_enabled"]
		except KeyError:
			print "always_enabled undefined"
		
		try:
			gammurc_file = cloud["gammurc_file"]
		except KeyError:
			print "gammurc_file undefined"


#check Gammu configuration
if (not libSMS.gammuCheck()):
	sys.exit()
else: 
	if (not libSMS.gammurcCheck(gammurc_file)):
		sys.exit()

if (libSMS.phoneConnection() == None):
	sys.exit()
else:	
	sm = libSMS.phoneConnection()
		

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
	sms_data = "SensorData Sensor"+str(src)
	
	#sms_data = "SRC#"+str(src)+"#RSSI#"+str(RSSI)+"#BW#"+str(bw)+"#CR#"+str(cr)+"#SF#"+str(sf)+"#GWID#"+gwid+"/"+data
	#sms_data = "SensorData  Sensor"+str(src)+" RSSI "+str(RSSI)+" BW "+str(bw)+" CR "+str(cr)+" SF "+str(sf)+" GWID "+gwid+" "
	
	#start from the first nomenclature
	i = 2
	
	while i < len(data_array)-1 :
		sms_data += " "+data_array[i]+" "+data_array[i+1]
		i += 2
	
	if data_array[0]=='':
		data_array[0]=key_WAZIUP.project_name

	if data_array[1]=='':
		data_array[1]=key_WAZIUP.service_path
		
	sms_data += " "+data_array[0]+" "+data_array[1]
	
	# Send data to expected contacts
	if(not always_enabled):
		if (libSMS.internet_ON()):
			print('Internet is available, no need to use the SMS Service')
			sys.exit()
		else:
			print("rcv msg to send via the WAZIUP SMS Service: "+sms_data)
			libSMS.send_sms(sm, PIN, sms_data, contacts)
	else:
		print("rcv msg to send via the WAZIUP SMS Service: "+sms_data)
		libSMS.send_sms(sm, PIN, sms_data, contacts)

if __name__ == "__main__":
        main(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])
