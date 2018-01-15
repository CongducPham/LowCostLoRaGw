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

import os
import sys
import os.path
import re
import libSMS
from math import radians, cos, sin, asin, sqrt, atan2, degrees
import json
import codecs
#import code
import datetime
from dateutil import parser

_gps_jsonfile = "gps/gps.json"
_active_gps_jsonfile = "gps/active_gps.json"
_gps_file = "gps/gps.txt"

# get key definition from external file to ease
# update of cloud script in the future
import key_GpsFile

try:
	key_GpsFile.source_list
except AttributeError:
	key_GpsFile.source_list=[]

#------------------------------------------------------------
#get from https://stackoverflow.com/questions/4913349/haversine-formula-in-python-bearing-and-distance-between-two-gps-points
#------------------------------------------------------------

def haversine(pointA, pointB):

    if (type(pointA) != tuple) or (type(pointB) != tuple):
        raise TypeError("GPS file: Only tuples are supported as arguments")

    lat1 = pointA[0]
    lon1 = pointA[1]

    lat2 = pointB[0]
    lon2 = pointB[1]

    # convert decimal degrees to radians 
    lat1, lon1, lat2, lon2 = map(radians, [lat1, lon1, lat2, lon2]) 

    # haversine formula 
    dlon = lon2 - lon1 
    dlat = lat2 - lat1 
    a = sin(dlat/2)**2 + cos(lat1) * cos(lat2) * sin(dlon/2)**2
    c = 2 * asin(sqrt(a)) 
    r = 6371 # Radius of earth in kilometers. Use 3956 for miles
    return c * r


def initial_bearing(pointA, pointB):

    if (type(pointA) != tuple) or (type(pointB) != tuple):
        raise TypeError("GPS file: Only tuples are supported as arguments")

    lat1 = radians(pointA[0])
    lat2 = radians(pointB[0])

    diffLong = radians(pointB[1] - pointA[1])

    x = sin(diffLong) * cos(lat2)
    y = cos(lat1) * sin(lat2) - (sin(lat1)
            * cos(lat2) * cos(diffLong))

    initial_bearing = atan2(x, y)

    # Now we have the initial bearing but math.atan2 return values
    # from -180 to +180 which is not what we want for a compass bearing
    # The solution is to normalize the initial bearing as shown below
    initial_bearing = degrees(initial_bearing)
    compass_bearing = (initial_bearing + 360) % 360

    return compass_bearing
    
#------------------------------------------------------------
#end
#------------------------------------------------------------

#------------------------------------------------------------
#update list of gps remote devices 
#------------------------------------------------------------

def update_gps_device(src, SNR, RSSI, seq, bc, lat, lgt, fxt, distance, tdata, gwid):

	#------------------------------------------------------------
	# Open gps.json file 
	#------------------------------------------------------------

	#open json file to retrieve active devices
	try:
		f = open(os.path.expanduser(_gps_jsonfile),"r")
		string = f.read()
		f.close()
		
		#change it into a python array
		gps_json_array = json.loads(string)

		#retrieving all gps devices declarations
		gps_devices = gps_json_array["devices"]		
	except IOError:	
		#create a new array	
		gps_json_array = json.loads('{ "devices":[]}')
		#in "devices" section
		gps_devices = gps_json_array["devices"]		

	found_device=False

	#search in current list
	for device in gps_devices:
		#found it
		if src==device["src"]:
			found_device=True
			device["snr"]=SNR
			device["rssi"]=RSSI
			device["seq"]=seq
			device["bc"]=bc			
			device["time"]=tdata
			device["gw"]=gwid
			device["fxt"]=int(fxt)
			device["lat"]=float(lat)
			device["lgt"]=float(lgt)
			device["distance"]=float(distance)
			device["active"]='yes'
	
	#new device
	if found_device==False:
		new_device={}
		new_device["src"]=src
		new_device["snr"]=SNR
		new_device["rssi"]=RSSI
		new_device["seq"]=seq
		new_device["bc"]=bc
		new_device["time"]=tdata
		new_device["gw"]=gwid
		new_device["fxt"]=int(fxt)
		new_device["lat"]=float(lat)
		new_device["lgt"]=float(lgt)
		new_device["distance"]=float(distance)
		new_device["active"]='yes'
		#append in list
		gps_devices.append(new_device)			
    
	#create a new array	
	active_gps_json_array = json.loads('{ "devices":[]}')
	#in "devices" section
	active_gps_devices = active_gps_json_array["devices"]


	localdt = datetime.datetime.now()		
	tnow = localdt.replace(microsecond=0).isoformat()
	now = parser.parse(tnow)

	#search for device that were active in the time window
	for device in gps_devices:
		t1=parser.parse(device['time'])
		diff=now-t1
	
		if diff.total_seconds() < key_GpsFile.active_interval_minutes*60:
			device['active']=u'yes'
			active_gps_devices.append(device)
		else:
			device['active']=u'no'	

	#update the list of remote devices on file	
	with open(os.path.expanduser(_gps_jsonfile), 'w') as f:
		json.dump(gps_json_array, codecs.getwriter('utf-8')(f), ensure_ascii=False, indent=4)
	
	#update the list of active remote devices on file		
	with open(os.path.expanduser(_active_gps_jsonfile), 'w') as f:
		json.dump(active_gps_json_array, codecs.getwriter('utf-8')(f), ensure_ascii=False, indent=4)    						
			
#------------------------------------------------------------
#store gps coordinate from remote devices 
#------------------------------------------------------------

def store_gps_coordinate(src, SNR, RSSI, seq, bc, lat, lgt, fxt, tdata, gwid):

	#------------------------------------------------------------
	#open gateway_conf.json file 
	#------------------------------------------------------------

	gwconf_filename = "gateway_conf.json"

	f = open(os.path.expanduser(gwconf_filename),"r")
	lines = f.readlines()
	f.close()
	array = ""
	#get all the lines in a string
	for line in lines :
		array += line

	#change it into a python array
	gw_json_array = json.loads(array)
	
	try:
		_gw_lat = gw_json_array["gateway_conf"]["ref_latitude"]
	except KeyError:
		_gw_lat = "undef"
	try:
		_gw_long = gw_json_array["gateway_conf"]["ref_longitude"]
	except KeyError:
		_gw_long = "undef"
		
	try:
		gw_lat = float(_gw_lat)
	except ValueError:
		_gw_lat = "undef"
		
	try:
		gw_long = float(_gw_long)
	except ValueError:
		_gw_long = "undef"		
		
	if (_gw_lat=="undef") or (_gw_long=="undef"):
		print 'GPS file: gw has no valid GPS coordinate'
		distance=-1
	else:		
		# gw
		pA = (gw_lat, gw_long)
		pB = (float(lat), float(lgt))
		distance = haversine(pA, pB)
	
	#open gps file to store the GPS coordinate and the calculated distance
	f = open(os.path.expanduser(_gps_file),"a")
	
	data = 'src '+src+' seq '+str(seq)+' bc '+str(bc)+' snr '+str(SNR)+' rssi '+str(RSSI)+' time '+tdata+' gw '+gwid+' fxt '+fxt+' lat '+lat+' lgt '+lgt+' distance '+str("%.4f" % distance)
	f.write(data+'\n')	
	f.close()
	
	print 'GPS file: lat='+lat
	print 'GPS file: lgt='+lgt
	print 'GPS file: d='+"%.4f" % distance
	
	#update the list of remote GPS device
	#information from device older than key_GpsFile.active_interval_minutes will be marked not active
	update_gps_device(src, SNR, RSSI, seq, bc, lat, lgt, fxt, str("%.4f" % distance), tdata, gwid)

	if (key_GpsFile.SMS==True):

		try:
			gammurc_file=key_GpsFile.gammurc_file
		except AttributeError:
			gammurc_file="/home/pi/.gammurc"
		
		print 'GPS file: SMS is requested'
		#check Gammu configuration
		if (not libSMS.gammuCheck()):
			print 'GPS file: Gammu is not available'
			sys.exit()
		else: 
			if (not libSMS.gammurcCheck(gammurc_file)):
				print 'GPS file: gammurc file is not available'
				sys.exit()

		if (libSMS.phoneConnection(gammurc_file, key_GpsFile.PIN) == None):
			print 'GPS file: Can not connect to cellular network'
			sys.exit()
		else:	
			sm = libSMS.phoneConnection(gammurc_file, key_GpsFile.PIN)
			
		print "GPS file: Sending SMS"
		success = libSMS.send_sms(sm, data, key_GpsFile.contacts)
		
		if (success):
			print "GPS file: Sending SMS done"
							
#------------------------------------------------------------
# main
#------------------------------------------------------------

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

	if (src_str in key_GpsFile.source_list) or (len(key_GpsFile.source_list)==0):	

		#remove any space in the message as we use '/' as the delimiter
		#any space characters will introduce error in the json structure and then the curl command will fail
		ldata=ldata.replace(' ', '')
				
		ldata_array = re.split("/", ldata)
		
		try:
			lat_index=ldata_array.index("LAT")
		except ValueError:
			lat_index=-1
		
		try:			
			lgt_index=ldata_array.index("LGT")
		except ValueError:
			lgt_index=-1
			
		try:			
			fxt_index=ldata_array.index("FXT")
		except ValueError:
			fxt_index=-1			
			
		try:			
			bc_index=ldata_array.index("BC")
		except ValueError:
			bc_index=-1
			
		if (bc_index == -1):
			bc=-1	
		else:
			bc=int(ldata_array[bc_index+1]) 
						
		if (lat_index == -1) or (lgt_index == -1) or (fxt_index == -1):
			print "GPS file: No GPS fields"
		else:
			if (ldata_array[fxt_index+1] == '-1'):
				print "GPS file: No GPS fix"
			else:
				lat=ldata_array[lat_index+1]
				lgt=ldata_array[lgt_index+1]
				fxt=ldata_array[fxt_index+1]
				
				the_time=parser.parse(tdata)
				#replace in naive format because we don't know if we have time zone info or not
				tdata = the_time.replace(microsecond=0,tzinfo=None).isoformat()
				
				store_gps_coordinate(key_GpsFile.project_name+'_'+key_GpsFile.organization_name+'_'+key_GpsFile.sensor_name+src_str, SNR, RSSI, seq, bc, lat, lgt, fxt, tdata, gwid)				

# you can test CloudGpsFile.py in standalone mode with
# python CloudGpsFile.py "BC/9/LAT/43.31402/LGT/-0.36370/FXT/4180" "1,16,6,0,9,8,-45" "125,5,12" "2017-11-20T14:18:54+01:00" "00000027EBBEDA21"
		
if __name__ == "__main__":
        main(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])
