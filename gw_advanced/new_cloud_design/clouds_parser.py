import os
import sys
import json
import datetime

###############################################
########### GETTING ENABLED CLOUDS ############
###############################################

#name of json file containing the cloud declarations
cloud_filename = "clouds.json"

def retrieve_enabled_clouds():
	#enabled cloud array
	_enabled_clouds = []

	#open json file to retrieve enabled clouds
	f = open(os.path.expanduser(cloud_filename),"r")
	string = f.read()
	f.close()
		
	#change it into a python array
	json_array = json.loads(string)

	#retrieving all cloud declarations
	clouds = json_array["clouds"]

	print "Parsing cloud declarations"
	
	#filling _enabled_clouds
	for cloud in clouds:
		if cloud["enabled"]:
			_enabled_clouds.append(cloud["script"])
			print _enabled_clouds
	
	print "Parsed all cloud declarations"
			 	
	return _enabled_clouds		

	