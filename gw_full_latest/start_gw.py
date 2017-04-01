#-------------------------------------------------------------------------------
# Copyright 2016 Nicolas Bertuol, University of Pau, France.
# 
# nicolas.bertuol@etud.univ-pau.fr
#
# This file is part of the low-cost LoRa gateway developped at University of Pau
#
# Contact: Congduc.Pham@univ-pau.fr
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

# Apr 1st, 2017. Restructured to used only gateway_conf.json file
# Congduc.Pham@univ-pau.fr

# Oct/2016. re-designed by C. Pham to use self-sufficient script per cloud
# Congduc.Pham@univ-pau.fr

import json
import os

gateway_conf_filename = "gateway_conf.json"

gateway_json_array={}

#recover each field different than 0 and call post_processing_gw.py with parameters
call_string_cpp = "sudo ./lora_gateway"
call_string_python = " | python post_processing_gw.py"
call_string_log_gw = ""
	
def start_config_from_json() :
	#open json file to recover values
	f = open(os.path.expanduser(gateway_conf_filename),"r")
	lines = f.readlines()
	f.close()
	array = ""
	#get all the lines in a string
	for line in lines :
		array += line
	
	global gateway_json_array
	#change it into a python array
	gateway_json_array = json.loads(array)
	
	global call_string_cpp
	global call_string_python
	global call_string_log_gw		
	
	try:		
		if gateway_json_array["gateway_conf"]["raw"] :
			call_string_cpp += " --raw"
	except KeyError:
		pass
	
	try:			
		if gateway_json_array["gateway_conf"]["log_post_processing"] :
			call_string_log_gw = " | python log_gw.py"
	except KeyError:
		pass
	
	try:		
		if gateway_json_array["radio_conf"]["mode"] != -1 :
			call_string_cpp += " --mode %s" % str(gateway_json_array["radio_conf"]["mode"])
		else :
			call_string_cpp += " --bw %s --cr %s --sf %s" % (str(gateway_json_array["radio_conf"]["bw"]),str(gateway_json_array["radio_conf"]["cr"]),str(gateway_json_array["radio_conf"]["sf"]))
	except KeyError:
		call_string_cpp += " --mode 1"
	
	try:		
		if gateway_json_array["radio_conf"]["ch"] != -1 :
			call_string_cpp += " --ch %s" % str(gateway_json_array["radio_conf"]["ch"])
		elif gateway_json_array["radio_conf"]["freq"] != -1:
			call_string_cpp += " --freq %s" % str(gateway_json_array["radio_conf"]["freq"])
	except KeyError:
		pass
	
	try:			
		if gateway_json_array["gateway_conf"]["downlink"]==0 :
			call_string_cpp += " --ndl"	
	except KeyError:
		pass
			
	print call_string_cpp+call_string_python+call_string_log_gw
	#launch the commands
	os.system(call_string_cpp + call_string_python + call_string_log_gw)
	
if __name__ == "__main__":
	start_config_from_json()
