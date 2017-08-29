#-------------------------------------------------------------------------------
# Copyright 2015 Congduc Pham, University of Pau, France.
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
# Jul/2016 adapted by N. Bertuol under C. Pham supervision 
# 
# nicolas.bertuol@etud.univ-pau.fr

# Apr 1st, 2017. Restructured to used only gateway_conf.json file
# Congduc.Pham@univ-pau.fr

import sys
import os
import shutil
import datetime
import json
import getopt

#use "os.path.expanduser(path)" to allow use of "~"
_folder_path = "/home/pi/Dropbox/LoRa-test/"

#open json file to recover gateway_address
f = open(os.path.expanduser("gateway_conf.json"),"r")
lines = f.readlines()
f.close()
array = ""
#get all the lines in a string
for line in lines :
	array += line

#change it into a python array
json_array = json.loads(array)

#get the gateway ID
_gwaddr = json_array["gateway_conf"]["gateway_ID"]

#log files
#_parselog_filename = _folder_path+"post-processing_"+str(_gwaddr)+".log"
#_prefix_filename = _folder_path+"post-processing_"+str(_gwaddr)+"_"
_parselog_filename = _folder_path+"post-processing.log"
_prefix_filename = _folder_path+"post-processing_"


#recovering if we use weekly logs and monthly logs or just monthly logs
_use_weekly_log = json_array["gateway_conf"]["log_weekly"]

# create a folder and copy log files into
def monthChanged(last_year, last_month) :
	
	#create folder which is named "gwaddr_mm-yyyy" with chmod 744
	_new_folder_name = _folder_path+str(_gwaddr)+"_"+str(last_year)+"-"+str(last_month)
	os.system("mkdir "+_new_folder_name)
	os.system("chmod 744 "+_new_folder_name)
	
	#move the log files in the folder
	files = os.listdir(os.path.expanduser(_folder_path))
	for f in files:
		if f.startswith("post-processing_"+str(last_year)+"-"+str(last_month)) :
			shutil.move(os.path.expanduser(_folder_path+f), os.path.expanduser(_new_folder_name))

the_line=sys.stdin.readline()
	
while the_line:
	#weekly and monthly logs
	if _use_weekly_log :
		#if the log file exists and isn't empty
		if(os.path.exists(os.path.expanduser(_parselog_filename)) and os.path.getsize(os.path.expanduser(_parselog_filename)) != 0) :
	
			#get date object (now and last_modif)
			now = datetime.datetime.utcnow().date()
	
			last_modif = datetime.datetime.utcfromtimestamp(os.path.getmtime(os.path.expanduser(_parselog_filename))).date()
		
			#check if we are in a new month or not
			new_month = False
		
			#new month, check if months are different OR if years are different (in case we have the same month but not the same year)
			if last_modif.month != now.month or last_modif.year != now.year :
				os.rename(os.path.expanduser(_parselog_filename), os.path.expanduser(_prefix_filename+str(last_modif.year)+"-"+str(last_modif.month)+"-"+str(last_modif.day)+"_to_"+str(now.year)+"-"+str(now.month)+"-"+str(now.day)+".log"))
				monthChanged(last_modif.year, last_modif.month)
				new_month = True
		
		
			if not new_month :
				f=open(os.path.expanduser(_parselog_filename),"r")
				a = f.readline()
				f.close()
				last_character = 0
				date_aux = ""
				#we recover the 10 first characters which corresponds to the date : "xxxx-xx-xx"
				while(last_character < 10) :
					date_aux += a[last_character]
					last_character += 1
		
				#separate year, month, day
				date = date_aux.split('-')
			
				date_aux = now.replace(year=int(date[0]), month=int(date[1]), day=int(date[2]))
		
				difference_days = now - date_aux
				#if there is one week of logs or more, change the file name
				if(difference_days.days >= 7) :
					os.rename(_parselog_filename, _prefix_filename+str(date_aux)+"_to_"+str(now)+".log")
	#only monthly logs
	else :
		#if the log file exists and isn't empty
		if(os.path.exists(os.path.expanduser(_parselog_filename))) :
	
			#get date object (now and last_modif)
			now = datetime.datetime.utcnow().date()
	
			last_modif = datetime.datetime.utcfromtimestamp(os.path.getmtime(os.path.expanduser(_parselog_filename))).date()
			
			#new month, check if months are different OR if years are different (in case we have the same month but not the same year)
			if last_modif.month != now.month or last_modif.year != now.year :
				os.rename(os.path.expanduser(_parselog_filename), os.path.expanduser(_prefix_filename+str(last_modif.year)+"-"+str(last_modif.month)+".log"))
	
	
	f=open(os.path.expanduser(_parselog_filename),"a")	
	now = datetime.datetime.now()
	f.write(now.isoformat()+'> ')
	f.write(the_line)
	f.close()
	sys.stdout.write(the_line)
	the_line=sys.stdin.readline()	

