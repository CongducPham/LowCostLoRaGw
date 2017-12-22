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
import json
import re
import libSMS
from math import radians, cos, sin, asin, sqrt, atan2, degrees

_gps_jsonfile = "gps/gps.json"
_gps_file = "gps/gps.txt"

# get key definition from external file to ease
# update of cloud script in the future
import key_GpsFile

try:
	key_GpsFile.source_list
except AttributeError:
	key_GpsFile.source_list=[]
		
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
#store gps coordinate from remote devices 
#------------------------------------------------------------

def store_gps_coordinate(src, SNR, RSSI, seq, lat, lgt, fxt, tdata, gwid):

	#print "Reading GPS file"

	#open json file to retrieve enabled clouds
	#f = open(os.path.expanduser(_gps_file),"r")
	#json_string = f.read()
	#f.close()

	#change it into a python array
	#json_array = json.loads(json_string)
	
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
	
	data = 'src '+src+' snr '+str(SNR)+' rssi '+str(RSSI)+' time '+tdata+' gw '+gwid+' fxt '+fxt+' lat '+lat+' lgt '+lgt+' distance '+str("%.4f" % distance)
	f.write(data+'\n')	
	f.close()
	
	print 'GPS file: lat='+lat
	print 'GPS file: lgt='+lgt
	print 'GPS file: d='+"%.4f" % distance

	if (key_GpsFile.SMS==True):
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
		cloud_json_array = json.loads(string)

		#retrieving all cloud declarations
		clouds = cloud_json_array["clouds"]

		#here we check for our own script entry
		for cloud in clouds:
			if "CloudSMS.py" in cloud["script"]:
				try:
					gammurc_file = cloud["gammurc_file"]
				except KeyError:
					print "GPS file: gammurc_file in clouds.json undefined"
					
		can_send_sms=True
		
		print 'GPS file: SMS is requested'
		#check Gammu configuration
		if (not libSMS.gammuCheck()):
			print 'GPS file: Gammu is not available'
			can_send_sms=False
		else: 
			if (not libSMS.gammurcCheck(gammurc_file)):
				print 'GPS file: gammurc file is not available'
				can_send_sms=False

		if (libSMS.phoneConnection(gammurc_file, key_GpsFile.PIN) == None):
			print 'GPS file: Can not connect to cellular network'
			can_send_sms=False
		else:	
			sm = libSMS.phoneConnection(gammurc_file, key_GpsFile.PIN)
			print 'GPS file: SMS can be sent'
			
		if (can_send_sms==True):
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
			
		if (lat_index == -1) or (lgt_index == -1) or (fxt_index == -1):
			print "GPS file: No GPS fields"
		else:
			if (ldata_array[fxt_index+1] == '-1'):
				print "GPS file: No GPS fix"
			else:
				lat=ldata_array[lat_index+1]
				lgt=ldata_array[lgt_index+1]
				fxt=ldata_array[fxt_index+1]
				store_gps_coordinate(key_GpsFile.project_name+'_'+key_GpsFile.organization_name+'_'+key_GpsFile.sensor_name+src_str, SNR, RSSI, seq, lat, lgt, fxt, tdata, gwid)				

# you can call CloudGpsFile.py in standalone mode with
# python CloudGpsFile.py "BC/9/LAT/43.31402/LGT/-0.36370/FXT/4180" "1,16,6,0,9,8,-45" "125,5,12" "2017-11-20T14:18:54+01:00" "00000027EBBEDA21"
# for testing
		
if __name__ == "__main__":
        main(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])
