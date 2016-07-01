#-------------------------------------------------------------------------------
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
#-------------------------------------------------------------------------------

# written by N. Bertuol under C. Pham supervision 
#
#

import json
import os

_filename_path = "global_conf.json"

def start_config_from_json() :
	#open json file to recover values
	f = open(os.path.expanduser(_filename_path),"r")
	lines = f.readlines()
	f.close()
	array = ""
	#get all the lines in a string
	for line in lines :
		array += line
	
	#change it into a python array
	json_array = json.loads(array)
	
	#recover each field different than 0 and call post_processing_gw.py with parameters
	call_string_cpp = "sudo ./lora_gateway"
	call_string_python = " | python post_processing_gw.py"
	call_string_log_gw = ""
	
	if json_array["ignorecomment"] :
		call_string_python += " --ignorecomment"
		
	if json_array["loggw"] :
		call_string_python += " --loggw"
		
	if json_array["wappkey"] :
		call_string_python += " --wappkey"
		
	if json_array["raw"] :
		call_string_python += " --raw"
		
	if json_array["aes"] :
		call_string_python += " --aes"
		
	if json_array["mongodb"]["is_used"] :
		call_string_python += " -m %s" % str(json_array["mongodb"]["max_months_to_store"])
		
	if json_array["log_post_processing"] :
		call_string_log_gw = " | python log_gw.py"
		
	if json_array["cloud_0"]["cloud_type"] == "thingspeak" and json_array["cloud_0"]["enable"] :
		call_string_python += " --thingspeak"
		#retry activated for thingspeak
		if json_array["cloud_0"]["retry"] :
			call_string_python += " --retrythsk"
		
	if json_array["cloud_1"]["cloud_type"] == "grovestreams" and json_array["cloud_1"]["enable"] :
		call_string_python += " --grovestreams"
		
	if json_array["cloud_2"]["cloud_type"] == "sensorcloud" and json_array["cloud_2"]["enable"] :
		call_string_python += " --sensorcloud"
		
	if json_array["cloud_3"]["cloud_type"] == "firebase" and json_array["cloud_3"]["enable"] :
		call_string_python += " --firebase"
	
	if json_array["mode"] != -1 :
		call_string_cpp += " --mode %s" % str(json_array["mode"])
	else :
		call_string_cpp += " --bw %s --cr %s --sf %s" % (str(json_array["bw"]),str(json_array["cr"]),str(json_array["sf"]))
	
	if json_array["ch"] != -1 :
		call_string_cpp += " --ch %s" % str(json_array["ch"])
	elif json_array["freq"] != -1:
		call_string_cpp += " --freq %s" % str(json_array["freq"])
		
	print call_string_cpp+call_string_python+call_string_log_gw
	#launch the commands
	os.system(call_string_cpp + call_string_python + call_string_log_gw)
	
	
	
	
if __name__ == "__main__":
	start_config_from_json()
