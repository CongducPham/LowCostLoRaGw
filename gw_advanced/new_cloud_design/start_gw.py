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

# Oct/2016. re-designed by C. Pham to use self-sufficient script per cloud
# Congduc.Pham@univ-pau.fr

import json
import os

conf_filename = "global_conf.json"

def start_config_from_json() :
	#open json file to recover values
	f = open(os.path.expanduser(conf_filename),"r")
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
		
	if json_array["log_post_processing"] :
		call_string_log_gw = " | python log_gw.py"
	
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
