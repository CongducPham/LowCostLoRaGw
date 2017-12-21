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
#add you own function here 
#------------------------------------------------------------

def show_gps():

	try:
		_gw_lat = gw_json_array["gateway_conf"]["ref_latitude"]
	except KeyError:
		_gw_lat = "undef"
	try:
		_gw_long = gw_json_array["gateway_conf"]["ref_longitude"]
	except KeyError:
		_gw_long = "undef"
		
	print 'show GPS: current GPS coordinate: gw lat '+_gw_lat+' long '+_gw_long	

def get_gps():

	print 'get GPS: not implemented yet' 

#------------------------------------------------------------
#add whatever you want in the main function 
#------------------------------------------------------------

def main():

	print 'status: start running'

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
		print 'status: dynamic GPS is requested'
		get_gps()		

	#------------------------------------------------------------		
	#print current GPS
	print 'status: show current GPS position'
	show_gps()

	#------------------------------------------------------------
	#notify for WAZIUP gw status?
	
	#------------------------------------------------------------
	#notify for TTN gw status?

	#------------------------------------------------------------	
	print 'status: exiting'
	
	
if __name__ == "__main__":
	main()