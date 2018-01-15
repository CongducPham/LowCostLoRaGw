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
import os.path
import json
import requests

# get key definition from external file to ease
# update of cloud script in the future
import key_NoInternet

try:
	key_NoInternet.source_list
except AttributeError:
	key_NoInternet.source_list=[]
	
_internet_pending_file = "delayed_uploading/internet_pending.txt"

#------------------------------------------------------------
#check Internet connectivity
#------------------------------------------------------------

def checkNet():
    print "CloudNoInternet checks Internet connectivity with www.google.com"
    try:
        response = requests.get("http://www.google.com")
        print "response code: " + str(response.status_code)
        return True
    except requests.ConnectionError:
        print "No Internet"
        return False


#------------------------------------------------------------
#check internet pending data
#------------------------------------------------------------

def is_internet_pending():
	
	try:
		#check delayed_uploading/internet_pending.txt
		if os.path.isfile(os.path.expanduser(_internet_pending_file)):
			return True
		else:
			return False
	except FileNotFoundError:
			print "File not found"

#------------------------------------------------------------
#store internet pending data
#------------------------------------------------------------

def store_internet_pending(ldata, pdata, rdata, tdata, gwid):
	
	try:
		data = "{\"ldata\":\""+ldata.replace('\n','')+"\",\"pdata\":\""+pdata.replace('\n','')+"\",\"rdata\":\""+rdata.replace('\n','')+"\",\"tdata\":\""+tdata.replace('\n','')+"\",\"gwid\":\""+gwid.replace('\n','')+"\"}"
		print data						
		
		if is_internet_pending():
			f = open(os.path.expanduser(_internet_pending_file),'a')
		else:
			f = open(os.path.expanduser(_internet_pending_file),'w')
		
		f.write(data+'\n')
		f.close()
	except IOError:
		print "Unable to open file"

#------------------------------------------------------------
#upload internet pending data
#------------------------------------------------------------

def upload_internet_pending():
	# - use delayed_uploading/internet_pending.txt as input
	# - maintain a list of internet pending data by reading delayed_uploading/internet_pending.txt
	# - deletes delayed_uploading/internet_pending.txt after reading 
	
	print "delayed uploading: checking for "+_internet_pending_file
	
	if is_internet_pending():
		
		try:
			if os.path.getsize(os.path.expanduser(_internet_pending_file)) > 0:
				lines = []
		
				print "delayed uploading: reading "+_internet_pending_file
		
				f = open(os.path.expanduser(_internet_pending_file),"r")
				lines = f.readlines()
				f.close()

				for line in lines:
					#remove \r=0xOD from line if some are inserted by OS and various tools
					line = line.replace('\r','')
					if len(line) > 1 or line != '\n':
						line_json=json.loads(line)
						#print line_json
						try:
							cloud_script = key_NoInternet.execute
							print "uploading with "+cloud_script
							sys.stdout.flush()
							cmd_arg=cloud_script+" \""+line_json["ldata"].replace('\n','')+"\""+" \""+line_json["pdata"].replace('\n','')+"\""+" \""+line_json["rdata"].replace('\n','')+"\""+" \""+line_json["tdata"].replace('\n','')+"\""+" \""+line_json["gwid"].replace('\n','')+"\""
						except UnicodeDecodeError, ude:
							print ude
						else:
							print cmd_arg
							sys.stdout.flush()
							try:
								os.system(cmd_arg)
							except:
								print "Error when uploading data to the cloud"	
					
				print "delayed uploading: all internet pending data uploaded"
				os.remove(os.path.expanduser(_internet_pending_file))
					
			else:
				print "delayed uploading: no internet pending data"

		except IOError:
			print "Unable to open file"
	else:
		print "delayed uploading: none existing internet_pending.txt"
		
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

	if (src_str in key_NoInternet.source_list) or (len(key_NoInternet.source_list)==0):
						
		if checkNet():
			print "Detected Internet connectivity"
			#try to upload any pending messages
			upload_internet_pending()
		else:
			print "delayed uploading: storing in "+_internet_pending_file
			store_internet_pending(ldata, pdata, rdata, tdata, gwid)
	else:
		print "Source is not is source list, no processing with CloudNoInternet.py"		
		
if __name__ == "__main__":
        main(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])
