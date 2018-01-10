#------------------------------------------------------------
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
#
# v3.7 - image modification and need to incorporate aux_radio features
# + copy post-processing feature
#------------------------------------------------------------

dateutil_tz=True

import sys
import subprocess
import select
import threading
from threading import Timer
import time
import datetime
try:
	import dateutil.tz
except ImportError:
	print "no timezone support, time will be expressed only in local time"
	dateutil_tz=False
import getopt
import os
import os.path
import json

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
#add you own functions here 
#------------------------------------------------------------

#------------------------------------------------------------
#show GPS coordinates stored in gateway_conf.json

def show_gps():

	#re-open in case dynamic GPS is enabled
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
		
	print 'post status show GPS: current GPS coordinate: gw lat '+_gw_lat+' long '+_gw_long	

#------------------------------------------------------------
#get GPS coordinates using sensors_in_raspi/get_gps.py with a USB GPS module
#update the gateway's position in gateway_conf.json

def get_gps():

	try:
		gps_port = gw_json_array["status_conf"]["gps_port"]
	except KeyError:
		print 'post status get GPS: no GPS port in gateway_conf.json, use default'
		gps_port = ''
			
	gw_gps_file='/home/pi/lora_gateway/gateway_gps.txt'
	
	print 'post status get GPS: use sensors_in_raspi/get_gps.py to get GPS position'
	try:
		os.system('python sensors_in_raspi/get_gps.py '+gps_port)
	except:
		print 'post status get GPS: Error when executing sensors_in_raspi/get_gps.py'
		
	if os.path.isfile(os.path.expanduser(gw_gps_file)):
	
		with open(os.path.expanduser(gw_gps_file), 'r') as gps_file:
			gps_data=gps_file.read().replace('\n', '')
			gps_data=gps_data.split(',')
    	
		lat_str=gps_data[0]
		lgt_str=gps_data[1]
    	
		cmd="""sed -i -- 's/"ref_latitude.*,/"ref_latitude" : """+'"'+lat_str+'"'+""",/g' /home/pi/lora_gateway/gateway_conf.json"""
    	
		try:
			print 'post status get GPS: replacing GPS latitude in gateway_conf.json'
			os.system(cmd)
		except:
			print 'post status get GPS: Error when replacing GPS latitude in gateway_conf.json'
			return
	
		cmd="""sed -i -- 's/"ref_longitude.*,/"ref_longitude" : """+'"'+lgt_str+'"'+""",/g' /home/pi/lora_gateway/gateway_conf.json"""
    	
		try:
			print 'post status get GPS: replacing GPS longitude in gateway_conf.json'
			os.system(cmd)
		except:
			print 'post status get GPS: Error when replacing GPS longitude in gateway_conf.json'
			return
							
	else:
		print 'post status get GPS: no GPS coordinates'	

#------------------------------------------------------------
#add whatever you want in the main function 
#------------------------------------------------------------

def main():

	print 'post status: start running'
	sys.stdout.flush()
	
	#------------------------------------------------------------
	#HERE ADD what ever you want
	#------------------------------------------------------------

	#------------------------------------------------------------
	#update GPS coordinates if a GPS is connected to the Raspberry?
	try:
		dynamic_gps = gw_json_array["status_conf"]["dynamic_gps"]
	except KeyError:
		dynamic_gps = False
		
	if dynamic_gps:
		print 'post status: dynamic GPS is requested'
		get_gps()		

	#------------------------------------------------------------		
	#print current GPS
	print 'post status: show current GPS position'
	show_gps()

	#------------------------------------------------------------
	#notify for WAZIUP gw status?
	
	#------------------------------------------------------------
	#notify for TTN gw status?

	#------------------------------------------------------------	
	print 'post status: exiting'
	sys.stdout.flush()
	
	
if __name__ == "__main__":
	main()