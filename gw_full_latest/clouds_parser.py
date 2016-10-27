#------------------------------------------------------------
# Copyright 2016 Congduc Pham, University of Pau, France.
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
#------------------------------------------------------------

import os
import sys
import json
import datetime

###############################################
########### GETTING ENABLED CLOUDS ############
###############################################

#name of json file containing the cloud declarations
cloud_filename = "clouds.json"

def retrieve_enabled_clouds(cloud_array="clouds"):
	#enabled cloud array
	_enabled_clouds = []

	#open json file to retrieve enabled clouds
	f = open(os.path.expanduser(cloud_filename),"r")
	string = f.read()
	f.close()
		
	#change it into a python array
	json_array = json.loads(string)
	
	hasCloudSection=1
	
	try:
		#retrieving all cloud declarations
		clouds = json_array[cloud_array]
	except KeyError:
		print "Error when looking for "+cloud_array+" section"
		hasCloudSection=0

	if hasCloudSection==1:
		print "Parsing cloud declarations"
		
		#filling _enabled_clouds
		for cloud in clouds:
			if cloud["enabled"]:
				_enabled_clouds.append(cloud["script"])
				print _enabled_clouds
		
		print "Parsed all cloud declarations"
			 	
	return _enabled_clouds		

