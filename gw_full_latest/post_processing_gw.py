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
# v3.8 - image modification and need to incorporate aux_radio features
# + copy post-processing feature
#------------------------------------------------------------

# IMPORTANT NOTE
# Parts that can be modified are identified with

#////////////////////////////////////////////////////////////
# TEXT

# END
#////////////////////////////////////////////////////////////

dateutil_tz=True

import sys
import subprocess
import select
import threading
from threading import Timer
import time
from collections import deque
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
import re
import string
import base64
import requests
import libSMS

#////////////////////////////////////////////////////////////
# ADD HERE VARIABLES FOR YOUR OWN NEEDS  
#////////////////////////////////////////////////////////////



#////////////////////////////////////////////////////////////

#------------------------------------------------------------
#low-level data prefix
#------------------------------------------------------------

LL_PREFIX_1='\xFF'
LL_PREFIX_LORA='\xFE'
#add here other data prefix for other type of low-level radio gateway


#list here other radio type
LORA_RADIO=1

#will be dynamically determined according to the second data prefix
radio_type=LORA_RADIO

#------------------------------------------------------------
#LoRa header packet information
#------------------------------------------------------------

HEADER_SIZE=4
APPKEY_SIZE=4
PKT_TYPE_DATA=0x10
PKT_TYPE_ACK=0x20

PKT_FLAG_ACK_REQ=0x08
PKT_FLAG_DATA_ENCRYPTED=0x04
PKT_FLAG_DATA_WAPPKEY=0x02
PKT_FLAG_DATA_DOWNLINK=0x01

LORAWAN_HEADER_SIZE=13

#------------------------------------------------------------
#last pkt information
#------------------------------------------------------------
pdata="0,0,0,0,0,0,0,0"
rdata="0,0,0"
tdata="N/A"

dst=0
ptype=0
ptypestr="N/A"
src=0
seq=0
datalen=0
SNR=0
RSSI=0
bw=0
cr=0
sf=0

_hasRadioData=False
#------------------------------------------------------------

#to display non printable characters
replchars = re.compile(r'[\x00-\x1f]')

def replchars_to_hex(match):
	return r'\x{0:02x}'.format(ord(match.group()))
	
#------------------------------------------------------------
#will ignore lines beginning with '?'
#------------------------------------------------------------
_ignoreComment=1

#------------------------------------------------------------
#for appkey management
#------------------------------------------------------------
the_app_key = '\x00\x00\x00\x00'

#valid app key? by default we do not check for the app key
_validappkey=1

#------------------------------------------------------------
#for local AES decrypting
#------------------------------------------------------------
_hasClearData=0

#------------------------------------------------------------
#open gateway_conf.json file 
#------------------------------------------------------------

f = open(os.path.expanduser("gateway_conf.json"),"r")
lines = f.readlines()
f.close()
array = ""
#get all the lines in a string
for line in lines :
	array += line

#change it into a python array
json_array = json.loads(array)

#------------------------------------------------------------
#get gateway ID
#------------------------------------------------------------

#set the gateway_address for having different log filenames
_gwid = json_array["gateway_conf"]["gateway_ID"]

#------------------------------------------------------------
#raw format?
#------------------------------------------------------------
try:
	_rawFormat = json_array["gateway_conf"]["raw"]
except KeyError:
	_rawFormat = 0
	
if _rawFormat:
	print "raw output from low-level gateway. post_processing_gw will handle packet format"	
	
#------------------------------------------------------------
#local aes?
#------------------------------------------------------------
try:
	_local_aes = json_array["gateway_conf"]["aes"]
except KeyError:
	_local_aes = 0	
	
if _local_aes:
	print "enable local AES decryption"	
	
#------------------------------------------------------------
#with app key?
#------------------------------------------------------------
try:
	_wappkey = json_array["gateway_conf"]["wappkey"]
except KeyError:
	_wappkey = 0	
	
if _wappkey:
	print "will enforce app key"
	print "importing list of app key"
	try:
		import key_AppKey
	except ImportError:
		print "no key_AppKey.py file"
		_wappkey = 0
		
#------------------------------------------------------------
#initialize gateway DHT22 sensor
#------------------------------------------------------------
try:
	_gw_dht22 = json_array["gateway_conf"]["dht22"]
except KeyError:
	_gw_dht22 = 0

if _gw_dht22 < 0:
	_gw_dht22 = 0
		
_date_save_dht22 = None

try:
	_dht22_mongo = json_array["gateway_conf"]["dht22_mongo"]
except KeyError:
	_dht22_mongo = False

if (_dht22_mongo):
	global add_document	
	from MongoDB import add_document
	
if (_gw_dht22):
	print "Use DHT22 to get gateway temperature and humidity level"
	#read values from dht22 in the gateway box
	sys.path.insert(0, os.path.expanduser('./sensors_in_raspi/dht22'))
	from read_dht22 import get_dht22_values
	
	_temperature = 0
	_humidity = 0

# retrieve dht22 values
def save_dht22_values():
	global _temperature, _humidity, _date_save_dht22
	_humidity, _temperature = get_dht22_values()
	
	_date_save_dht22 = datetime.datetime.now()

	print "Gateway TC : "+_temperature+" C | HU : "+_humidity+" % at "+str(_date_save_dht22)
	
	#save values from the gateway box's DHT22 sensor, if _mongodb is true
	if(_dht22_mongo):
		#saving data in a JSON var
		str_json_data = "{\"th\":"+_temperature+", \"hu\":"+_humidity+"}"
	
		#creating document to add
		doc = {
			"type" : "DATA_GW_DHT22",
			"gateway_eui" : _gwid, 
			"node_eui" : "gw",
			"snr" : "", 
			"rssi" : "", 
			"cr" : "", 
			"datarate" : "", 
			"time" : _date_save_dht22,
			"data" : json.dumps(json.loads(str_json_data))
		}
	
		#adding the document
		add_document(doc)
	
def dht22_target():
	while True:
		print "Getting gateway temperature"
		save_dht22_values()
		sys.stdout.flush()	
		global _gw_dht22
		time.sleep(_gw_dht22)


#------------------------------------------------------------
#copy post-processing.log into /var/www/html/admin/log folder
#------------------------------------------------------------

#you can enable periodic copy of post-processing.log file by setting to True
#but you need to install the web admin interface in order to have the /var/www/html/admin/log/ folder
#note that this feature is obsoleted by an option in the web admin interface to copy post-processing.log file on demand
_gw_copy_post_processing=False

#TODO: integrate copy post_processing feature into periodic status/tasks?
def copy_post_processing():
	print "extract last 500 lines of post-processing.log into /var/www/html/admin/log/post-processing-500L.log"
	cmd="sudo tail -n 500 log/post-processing.log > /var/www/html/admin/log/post-processing-500L.log"
	
	try:
		os.system(cmd)
	except:
		print "Error when extracting lines from post-processing_"+_gwid+".log"
		
	cmd="sudo chown -R pi:www-data /var/www/html/admin/log"
	
	try:
		os.system(cmd)
	except:
		print "Error when setting file ownership to pi:www-data"
	

def copy_post_processing_target():
	while True:
		copy_post_processing()
		sys.stdout.flush()
		#change here if you want to change the time between 2 extraction
		#here it is 30mins	
		time.sleep(1800)
	
#------------------------------------------------------------
#for downlink features
#------------------------------------------------------------

try:
	_gw_downlink = json_array["gateway_conf"]["downlink"]
except KeyError:
	_gw_downlink = 0

if _gw_downlink < 0:
	_gw_downlink = 0
	
_post_downlink_file = "downlink/downlink-post.txt"
_post_downlink_queued_file = "downlink/downlink-post-queued.txt"
_gw_downlink_file = "downlink/downlink.txt"

pending_downlink_requests = []

#actually, periodic output for downlink may be not very convenient
#as typical value for checking downlink is 1 to 5 minutes
#so we disable it here
_verbose_downlink=False

#if we check every hour, then switch output on
#you can also disable this behavior
if _gw_downlink > 3600:
	_verbose_downlink=True

def check_downlink():

	# - post_processing_gw.py checks and uses downlink/downlink_post.txt as input
	# - post_processing_gw.py will maintain a list of downlink message requests by reading downlink/downlink_post.txt
	# - valid requests will be appended to downlink/downlink-post_queued.txt
	# - after reading downlink/downlink_post.txt, post_processing_gw.py deletes it
	# - when a packet from device i is processed by post_processing_gw.py, it will check whether there is a queued message for i
	# - if yes, then it generates a downlink/downlink.txt file with the queue message as content
	
	if _verbose_downlink:	
		print datetime.datetime.now()
		print "post downlink: checking for "+_post_downlink_file
	
	if os.path.isfile(os.path.expanduser(_post_downlink_file)):

		lines = []
		
		print "post downlink: reading "+_post_downlink_file
		
		f = open(os.path.expanduser(_post_downlink_file),"r")
		lines = f.readlines()
		f.close()
	
		for line in lines:
			#remove \r=0xOD from line if some are inserted by OS and various tools
			line = line.replace('\r','')
			if len(line) > 1 or line != '\n':
				line_json=json.loads(line)
				print line_json
				
				if line_json["status"]=="send_request":
					pending_downlink_requests.append(line)		
	
		#print pending_downlink_request
		
		print "post downlink: writing to "+_post_downlink_queued_file
		
		f = open(os.path.expanduser(_post_downlink_queued_file),"w")
		
		for downlink_request in pending_downlink_requests:
			f.write("%s" % downlink_request)
		
		os.remove(os.path.expanduser(_post_downlink_file))	
		
	else:
		if _verbose_downlink:
			print "post downlink: no downlink requests"

	if _verbose_downlink:
		print "post downlink: list of pending downlink requests"
	
		if len(pending_downlink_requests) == 0:
			print "None"
		else:	
			for downlink_request in pending_downlink_requests:
				print downlink_request.replace('\n','')		
	
def downlink_target():
	while True:
		check_downlink()
		sys.stdout.flush()	
		global _gw_downlink
		time.sleep(_gw_downlink)
		
#------------------------------------------------------------
#for doing periodic status/tasks
#------------------------------------------------------------

try:
	_gw_status = json_array["gateway_conf"]["status"]
except KeyError:
	_gw_status = 0		

if _gw_status < 0:
	_gw_status = 0 

# if _gw_status:
# 	try:
# 		_gw_lat = json_array["gateway_conf"]["ref_latitude"]
# 	except KeyError:
# 		_gw_lat = "undef"
# 	try:
# 		_gw_long = json_array["gateway_conf"]["ref_longitude"]
# 	except KeyError:
# 		_gw_long = "undef"		
					
def status_target():
	while True:
		print datetime.datetime.now()
		print 'post status: gw ON'
		if _gw_downlink:
			print 'post status: will check for downlink requests every %d seconds' % _gw_downlink
		print 'post status: executing periodic tasks'
		sys.stdout.flush()		
		try:
			os.system('python post_status_processing_gw.py')
		except:
			print "Error when executing post_status_processing_gw.py"			
		global _gw_status
		time.sleep(_gw_status)

#------------------------------------------------------------
#check Internet connectivity
#------------------------------------------------------------

def checkNet():
    print "post_processing_gw.py checks Internet connecitivity with www.google.com"
    try:
        response = requests.get("http://www.google.com")
        print "response code: " + str(response.status_code)
        return True
    except requests.ConnectionError:
        print "No Internet"
        return False
        
#------------------------------------------------------------
#check for alert_conf.json section
#------------------------------------------------------------

try:
	alert_conf=json_array["alert_conf"]
	_has_alert_conf = True
	print "post_processing_gw.py found an alert_conf section"
except KeyError:
	_has_alert_conf = False
		
#------------------------------------------------------------
#for mail alerting
#------------------------------------------------------------			
#got example from https://myhydropi.com/send-email-with-a-raspberry-pi-and-python

_use_mail_alert = False

if _has_alert_conf:
	#global _use_mail_alert
	_use_mail_alert = json_array["alert_conf"]["use_mail"]

if _use_mail_alert:
	import smtplib
	from email.mime.multipart import MIMEMultipart
	from email.mime.text import MIMEText	
	print "Alert by mail is ON. Contact mail is "+json_array["alert_conf"]["contact_mail"]
		
def send_alert_mail(m):
	fromaddr = json_array["alert_conf"]["mail_from"]
	toaddr = json_array["alert_conf"]["contact_mail"]
	#in case we have several contact mail separated by ','
	alladdr=toaddr.split(",")

	msg = MIMEMultipart()
	msg['From'] = fromaddr
	msg['To'] = toaddr
	msg['Subject'] = m 
	body = m
	msg.attach(MIMEText(body, 'plain'))

	server = smtplib.SMTP(json_array["alert_conf"]["mail_server"], 587)
	server.starttls()
	server.login(fromaddr, json_array["alert_conf"]["mail_passwd"])
	text = msg.as_string()
	server.sendmail(fromaddr, alladdr, text)
	server.quit()

if _use_mail_alert :
	print "post_processing_gw.py sends mail indicating that gateway has started post-processing stage..."
	if checkNet():
		try:
			send_alert_mail("Gateway "+_gwid+" has started post-processing stage")
			print "Sending mail done"
		except:
			print "Unexpected error when sending mail"
				
	sys.stdout.flush()		
	
#------------------------------------------------------------
#for SMS alerting
#------------------------------------------------------------			

_use_sms_alert = False

if _has_alert_conf:
	#global _use_sms_alert
	_use_sms_alert = json_array["alert_conf"]["use_sms"]

global PIN, contact_sms, gammurc_file, sm

if _use_sms_alert:
	#check Gammu configuration
	if (not libSMS.gammuCheck()):
		print "overriding use_sms to false"
		_use_sms_alert = False

if _use_sms_alert:
	PIN = json_array["alert_conf"]["pin"]
	contact_sms = json_array["alert_conf"]["contact_sms"]
	gammurc_file = json_array["alert_conf"]["gammurc_file"]

	if (libSMS.gammurcCheck(gammurc_file)):
		print "Alert by SMS is ON. Contact SMS is ",
		print contact_sms
		print "Initializing gammu for SMS"
	else:
		print "overriding use_sms to false"
		_use_sms_alert = False	

if _use_sms_alert:
	if (libSMS.phoneConnection(gammurc_file, PIN) == None):
		print "overriding use_sms to false"
		print "Sending SMS failed"
		_use_sms_alert = False
	else:	
		sm = libSMS.phoneConnection(gammurc_file, PIN)

if _use_sms_alert :
	print "post_processing_gw.py sends SMS indicating that gateway has started post-processing stage..."
	success = libSMS.send_sms(sm, "Gateway "+_gwid+" has started post-processing stage", contact_sms)
	if (success):
		print "Sending SMS done"
	sys.stdout.flush()
#------------------------------------------------------------
#for handling images
#------------------------------------------------------------
#list of active nodes
nodeL = deque([])
#association to get the file handler
fileH = {}
#association to get the image filename
imageFilenameA = {}
#association to get the image SN
imgsnA= {} 
#association to get the image quality factor
qualityA = {}
#association to get the cam id
camidA = {}
#global image seq number
imgSN=0

def image_timeout():
	#get the node which timer has expired first
	#i.e. the one that received image packet earlier
	node_id=nodeL.popleft()
	print "close image file for node %d" % node_id
	f=fileH[node_id]
	f.close()
	del fileH[node_id]
	print "decoding image "+os.path.expanduser(imageFilenameA[node_id])
	sys.stdout.flush()
	
	cmd = '/home/pi/lora_gateway/ucam-images/decode_to_bmp -received '+os.path.expanduser(imageFilenameA[node_id])+\
		' -SN '+str(imgsnA[node_id])+\
		' -src '+str(node_id)+\
		' -camid '+str(camidA[node_id])+\
		' -Q '+str(qualityA[node_id])+\
		' -vflip'+\
		' /home/pi/lora_gateway/ucam-images/128x128-test.bmp'
	
	print "decoding with command"
	print cmd
	args = cmd.split()

	out = 'error'
		
	try:
		out = subprocess.check_output(args, stderr=None, shell=False)
		
		if (out=='error'):
			print "decoding error"
		else:
        	# leave enough time for the decoding program to terminate
			time.sleep(3)
			out = out.replace('\r','')
			out = out.replace('\n','')
			print "producing file " + out
			print "creating if needed the uploads/node_"+str(node_id)+" folder"
			try:
				os.mkdir(os.path.expanduser(_web_folder_path+"images/uploads/node_"+str(node_id)))
			except OSError:
				print "folder already exist"				 	 
			print "moving decoded image file into " + os.path.expanduser(_web_folder_path+"images/uploads/node_"+str(node_id))
			os.rename(os.path.expanduser("./"+out), os.path.expanduser(_web_folder_path+"images/uploads/node_"+str(node_id)+"/"+out))
			print "done"	

	except subprocess.CalledProcessError:
		print "launching image decoding failed!"
		
	sys.stdout.flush()
	
#------------------------------------------------------------
#for managing the input data when we can have aes encryption
#------------------------------------------------------------
_linebuf="the line buffer"
_linebuf_idx=0
_has_linebuf=0

def getSingleChar():
	global _has_linebuf
	#if we have a valid _linebuf then read from _linebuf
	if _has_linebuf==1:
		global _linebuf_idx
		global _linebuf
		if _linebuf_idx < len(_linebuf):
			_linebuf_idx = _linebuf_idx + 1
			return _linebuf[_linebuf_idx-1]
		else:
			#no more character from _linebuf, so read from stdin
			_has_linebuf = 0
			return sys.stdin.read(1)
	else:
		return sys.stdin.read(1)	
	
def getAllLine():
	global _linebuf_idx
	p=_linebuf_idx
	_linebuf_idx = 0
	global _has_linebuf
	_has_linebuf = 0	
	global _linebuf
	#return the remaining of the string and clear the _linebuf
	return _linebuf[p:]	
	
def fillLinebuf(n):
	global _linebuf_idx
	_linebuf_idx = 0
	global _has_linebuf
	_has_linebuf = 1
	global _linebuf
	#fill in our _linebuf from stdin
	_linebuf=sys.stdin.read(n)

#////////////////////////////////////////////////////////////
# CHANGE HERE THE VARIOUS PATHS FOR YOUR LOG FILES
#////////////////////////////////////////////////////////////
_folder_path = "/home/pi/Dropbox/LoRa-test/"
_web_folder_path = "/var/www/html/"
_telemetrylog_filename = _folder_path+"telemetry_"+str(_gwid)+".log"
_imagelog_filename = _folder_path+"image_"+str(_gwid)+".log"

# END
#////////////////////////////////////////////////////////////


#------------------------------------------------------------
#open clouds.json file to get enabled clouds
#------------------------------------------------------------

from clouds_parser import retrieve_enabled_clouds

#get a copy of the list of enabled clouds
_enabled_clouds=retrieve_enabled_clouds()

print "post_processing_gw.py got cloud list: "
print _enabled_clouds

#------------------------------------------------------------
#open clouds.json file to get clouds for encrypted data
#------------------------------------------------------------

_cloud_for_encrypted_data=retrieve_enabled_clouds("encrypted_clouds")
print "post_processing_gw.py got encrypted cloud list: "
print _cloud_for_encrypted_data

_cloud_for_lorawan_encrypted_data=retrieve_enabled_clouds("lorawan_encrypted_clouds")
print "post_processing_gw.py got LoRaWAN encrypted cloud list: "
print _cloud_for_lorawan_encrypted_data

#------------------------------------------------------------
#start various threads
#------------------------------------------------------------

#gateway dht22
if (_gw_dht22):
	print "Starting thread to measure gateway temperature"
	sys.stdout.flush()
	t_dht22 = threading.Thread(target=dht22_target)
	t_dht22.daemon = True
	t_dht22.start()

#downlink feature
if (_gw_downlink):

	#check for an existing downlink-post-queued.txt file
	#
	print datetime.datetime.now()
	print "post downlink: checking for existing "+_post_downlink_queued_file
	
	if os.path.isfile(os.path.expanduser(_post_downlink_queued_file)):

		lines = []

		print "post downlink: reading existing "+_post_downlink_queued_file
		
		f = open(os.path.expanduser(_post_downlink_queued_file),"r")
		lines = f.readlines()
		f.close()
	
		for line in lines:
			if len(line) > 1 or line != '\n':
				
				line_json=json.loads(line)
				#print line_json
				
				if line_json["status"]=="send_request":
					pending_downlink_requests.append(line)		
	
		print "post downlink: start with current list of pending downlink requests"
	
		for downlink_request in pending_downlink_requests:
			print downlink_request.replace('\n','')
	else:
		print "post downlink: none existing downlink-post-queued.txt"			

	print "Loading lib to compute downlink MIC"
	from loraWAN import loraWAN_get_MIC

	print "Starting thread to check for downlink requests every %d seconds" % _gw_downlink
	sys.stdout.flush()
	t_downlink = threading.Thread(target=downlink_target)
	t_downlink.daemon = True
	t_downlink.start()
	time.sleep(1)

#status feature
if (_gw_status):
	print "Starting thread to perform periodic gw status/tasks"
	sys.stdout.flush()
	t_status = threading.Thread(target=status_target)
	t_status.daemon = True
	t_status.start()
	time.sleep(1)	
	
#copy post_processing feature
#TODO: integrate copy post_processing feature into periodic status/tasks?
	
if (_gw_copy_post_processing):
	print "Starting thread to copy post_processing.log"
	sys.stdout.flush()
	t_status = threading.Thread(target=copy_post_processing_target)
	t_status.daemon = True
	t_status.start()
	time.sleep(1)

print ''	
print "Current working directory: "+os.getcwd()

#------------------------------------------------------------
#main loop
#------------------------------------------------------------

while True:

	sys.stdout.flush()
	
  	ch = getSingleChar()

	#expected prefixes
	#	^p 	indicates a ctrl pkt info ^pdst(%d),ptype(%d),src(%d),seq(%d),len(%d),SNR(%d),RSSI=(%d) for the last received packet
	#		example: ^p1,16,3,0,234,8,-45
	#
	#	^r	indicate a ctrl radio info ^rbw,cr,sf for the last received packet
	#		example: ^r500,5,12
	#
	#	^$	indicates an output (debug or log purposes) from the gateway that should be logged in the (Dropbox) gateway.log file 
	#		example: ^$Set LoRa mode 4
	#
	#	^l	indicates a ctrl LAS info ^lsrc(%d),type(%d)
	#		type is 1 for DSP_REG, 2 for DSP_INIT, 3 for DSP_UPDT, 4 for DSP_DATA 
	#		example: ^l3,4
	#
	#	\$	indicates a message that should be logged in the (Dropbox) telemetry.log file
	#		example: \$hello -> 	hello will be logged in the following format
	#					(src=3 seq=0 len=6 SNR=8 RSSI=-54) 2015-10-16T14:47:44.072230> hello    
	#
	#
	#	\!	indicates a message that should be logged on a cloud, see clouds.json
	#
	#		example for a ThingSpeak channel as implemented in CloudThinkSpeak.py
	#				\!SGSH52UGPVAUYG3S#9.4		-> 9.4 will be logged in the SGSH52UGPVAUYG3S ThingSpeak channel at default field, i.e. field 1
	#				\!2#9.4						-> 9.4 will be logged in the default channel at field 2
	#				\!SGSH52UGPVAUYG3S#2#9.4	-> 9.4 will be logged in the SGSH52UGPVAUYG3S ThingSpeak channel at field 2
	#				\!##9.4 or \!9.4			-> will be logged in default channel and field
	#
	#		you can add nomemclature codes:
	#				\!##TC/9.4/HU/85/DO/7		-> with ThingSpeak you can either upload only the first value or all values on several fields
	#											-> with an IoT cloud such as Grovestreams you will be able to store both nomenclatures and values 
	#
	#		you can log other information such as src, seq, len, SNR and RSSI on specific fields
	#
	#	\xFF\xFE		indicates radio data prefix
	#
	#	\xFF\x50-\x54 	indicates an image packet. Next fields are src_addr(2B), seq(1B), Q(1B), size(1B)
	#					cam id is coded with the second framing byte: i.e. \x50 means cam id = 0
	#
	
	#------------------------------------------------------------
	# '^' is reserved for control information from the gateway
	#------------------------------------------------------------

	if (ch=='^'):
		now = datetime.datetime.now()
		ch=sys.stdin.read(1)
		
		if (ch=='p'):		
			pdata = sys.stdin.readline()
			print now.isoformat()
			print "rcv ctrl pkt info (^p): "+pdata,
			arr = map(int,pdata.split(','))
			print "splitted in: ",
			print arr
			dst=arr[0]
			ptype=arr[1]
			ptypestr="N/A"
			if ((ptype & 0xF0)==PKT_TYPE_DATA):
				ptypestr="DATA"
				if (ptype & PKT_FLAG_DATA_DOWNLINK)==PKT_FLAG_DATA_DOWNLINK:
					ptypestr = ptypestr + " DOWNLINK"
				if (ptype & PKT_FLAG_DATA_WAPPKEY)==PKT_FLAG_DATA_WAPPKEY:
					ptypestr = ptypestr + " WAPPKEY"
				if (ptype & PKT_FLAG_DATA_ENCRYPTED)==PKT_FLAG_DATA_ENCRYPTED:
					ptypestr = ptypestr + " ENCRYPTED"
				if (ptype & PKT_FLAG_ACK_REQ)==PKT_FLAG_ACK_REQ:
					ptypestr = ptypestr + " ACK_REQ"														
			if ((ptype & 0xF0)==PKT_TYPE_ACK):
				ptypestr="ACK"
			src=arr[2]
			seq=arr[3]
			datalen=arr[4]
			SNR=arr[5]
			RSSI=arr[6]
			if (_rawFormat==0):	
				info_str="(dst=%d type=0x%.2X(%s) src=%d seq=%d len=%d SNR=%d RSSI=%d)" % (dst,ptype,ptypestr,src,seq,datalen,SNR,RSSI)
			else:
				info_str="rawFormat(len=%d SNR=%d RSSI=%d)" % (datalen,SNR,RSSI)	
			print info_str
			#TODO: maintain statistics from received messages and periodically add these informations in the gateway.log file
			
			#here we check for pending downlink message that need to be sent back to the end-device
			#
			for downlink_request in pending_downlink_requests:
				request_json=json.loads(downlink_request)
				if (request_json["dst"]==0) or (src == request_json["dst"]):
					print "post downlink: receive from %d with pending request" % src
					if (request_json["dst"]==0):
						print "in broadcast mode"
					else:
						print "in unicast mode"	
					print "post downlink: downlink data is \"%s\"" % request_json["data"]
					print "post downlink: generate "+_gw_downlink_file+" from entry"
					print downlink_request.replace('\n','')
					
					#generate the MIC corresponding to the clear data and the destination device address
					#it is possible to have a broadcast address but since the only device that is listening 
					#is the one that has sent a packet, there is little interest in doing so
					#so currently, we use the sending device's address to compute the MIC
					MIC=loraWAN_get_MIC(src,request_json["data"])
					#add the 4 byte MIC information into the json line
					request_json['MIC0']=hex(MIC[0])
					request_json['MIC1']=hex(MIC[1])
					request_json['MIC2']=hex(MIC[2])
					request_json['MIC3']=hex(MIC[3])
					
					downlink_json=[]
					downlink_json.append(request_json)
					
					f = open(os.path.expanduser(_gw_downlink_file),"a")
					
					print "post downlink: write"
					for downlink_json_line in downlink_json:
						#print downlink_json_line
						print json.dumps(downlink_json_line)
						f.write(json.dumps(downlink_json_line)+'\n')
					
					f.close()
					
					pending_downlink_requests.remove(downlink_request)
					
					#update downlink-post-queued.txt
					f = open(os.path.expanduser(_post_downlink_queued_file),"w")
					for downlink_request in pending_downlink_requests:
						f.write("%s" % downlink_request)
					#TODO: should we write all pending request for this node
					#or only the first one that we found?
					#currently, we do only the first one					
					break;
	
		if (ch=='r'):		
			rdata = sys.stdin.readline()
			print "rcv ctrl radio info (^r): "+rdata,
			arr = map(int,rdata.split(','))
			print "splitted in: ",
			print arr
			bw=arr[0]
			cr=arr[1]
			sf=arr[2]
			info_str="(BW=%d CR=%d SF=%d)" % (bw,cr,sf)
			print info_str

		if (ch=='t'):
			tdata = sys.stdin.readline()
			print "rcv timestamp (^t): "+tdata
									
		if (ch=='l'):
			#TODO: LAS service	
			print "not implemented yet"
			
		if (ch=='$'):
			
			data = sys.stdin.readline()
			print data,
			
			#when the low-level gateway program reset the radio module then it is will send "^$Resetting the radio module"
			if 'Resetting' in data:
			
				if _use_mail_alert:
					print "post_processing_gw.py sends mail indicating that gateway has reset radio module..."
					if checkNet():
						send_alert_mail("Gateway "+_gwid+" has reset its radio module")
						print "Sending mail done"
				
				if _use_sms_alert:
					print "post_processing_gw.py sends SMS indicating that gateway has reset radio module..."
					success = libSMS.send_sms(sm, "Gateway "+_gwid+" has reset its radio module", contact_sms)
					if (success):
						print "Sending SMS done"
						
		continue


#------------------------------------------------------------
# '\' is reserved for message logging service
#------------------------------------------------------------

	if (ch=='\\' and _hasRadioData==True):
	
		_hasRadioData=False
		
		now = datetime.datetime.now()
		
		if _validappkey==1:

			print "valid app key: accept data"
					
			ch=getSingleChar()			
					
			if (ch=='$'): #log in a file
				
				data = getAllLine()
				
				print "rcv msg to log (\$) in log file: "+data,
				f=open(os.path.expanduser(_telemetrylog_filename),"a")
				f.write(info_str+' ')	
				f.write(now.isoformat()+'> ')
				f.write(data)
				f.close()	

			#/////////////////////////////////////////////////////////////
			# YOU CAN MODIFY HERE HOW YOU WANT DATA TO BE PUSHED TO CLOUDS
			# WE PROVIDE EXAMPLES FOR THINGSPEAK, GROVESTREAM
			# IT IS ADVISED TO USE A SEPERATE PYTHON SCRIPT PER CLOUD
			#////////////////////////////////////////////////////////////
											
			#log on clouds: thingspeak, grovestreams, sensorcloud,...
			#or even on MongoDB as it is declared as a regular cloud
			#enabled clouds must be declared in clouds.json	
			elif (ch=='!'): 

				ldata = getAllLine()
				
				if (dateutil_tz==True):
					#replacing tdata to get time zone information when uploading to clouds
					localdt = datetime.datetime.now(dateutil.tz.tzlocal())
					tdata = localdt.replace(microsecond=0).isoformat()
				
				print "number of enabled clouds is %d" % len(_enabled_clouds)	
				
				#loop over all enabled clouds to upload data
				#once again, it is up to the corresponding cloud script to handle the data format
				#
				for cloud_index in range(0,len(_enabled_clouds)):
					
					try:
						print "--> cloud[%d]" % cloud_index
						cloud_script=_enabled_clouds[cloud_index]
						print "uploading with "+cloud_script
						sys.stdout.flush()
						cmd_arg=cloud_script+" \""+ldata.replace('\n','').replace('\0','')+"\""+" \""+pdata.replace('\n','')+"\""+" \""+rdata.replace('\n','')+"\""+" \""+tdata.replace('\n','')+"\""+" \""+_gwid.replace('\n','')+"\""
					except UnicodeDecodeError, ude:
						print ude
					else:
						print cmd_arg
						sys.stdout.flush()
						try:
							os.system(cmd_arg)
						except:
							print "Error when uploading data to the cloud"	

				print "--> cloud end"
				
			#END
			#////////////////////////////////////////////////////////////				
															
			else: 
				#not a known data logging prefix
				#you may want to upload to a default service
				#so just implement it here
				print "unrecognized data logging prefix: discard data"
				getAllLine() 
					
		else:
			print "invalid app key: discard data"
			getAllLine()

		continue
	
	#handle low-level gateway data
	if (ch == LL_PREFIX_1):
	
		print "got first framing byte"
		ch=getSingleChar()	
		
		#data from low-level LoRa gateway?		
		if (ch == LL_PREFIX_LORA):
			#the data prefix is inserted by the gateway
			#do not modify, unless you know what you are doing and that you modify lora_gateway (comment WITH_DATA_PREFIX)
			print "--> got LoRa data prefix"
			radio_type=LORA_RADIO
			
			#if SNR < -20:
			#	print "--> SNR too low, discarding data"
			#	sys.stdin.readline()
			#	continue	
							
			_hasRadioData=True
			
			#we actually need to use DATA_PREFIX in order to differentiate data from radio coming to the post-processing stage
			#if _wappkey is set then we have to first indicate that _validappkey=0
			if (_wappkey==1):
				_validappkey=0
			else:
				_validappkey=1	

			#if we have raw output from gw, then try to determine which kind of packet it is
			#
			if (_rawFormat==1):
				print "raw format from LoRa gateway"
				ch=getSingleChar()
				
				#probably our modified Libelium header where the destination (i.e. 1) is the gateway
				#dissect our modified Libelium format
				if ord(ch)==1:			
					dst=ord(ch)
					ptype=ord(getSingleChar())
					src=ord(getSingleChar())
					seq=ord(getSingleChar())
					print "Header[dst=%d ptype=0x%.2X src=%d seq=%d]" % (dst,ptype,src,seq)
					#now we read datalen-4 (the header length) bytes in our line buffer
					fillLinebuf(datalen-HEADER_SIZE)
					datalen=datalen-HEADER_SIZE
					pdata="%d,%d,%d,%d,%d,%d,%d" % (dst,ptype,src,seq,datalen,SNR,RSSI)
					print "update ctrl pkt info (^p): "+pdata				
				
				#LoRaWAN uses the MHDR(1B)
				#----------------------------
				#| 7  6  5 | 4  3  2 | 1  0 |
				#----------------------------
				#  0  1  0   0  0  0   0  0		unconfirmed data up
				#  1  0  0   0  0  0   0  0		confirmed data up 
				#   MType      RFU     major
				#
				#the main MType is unconfirmed data up b010 or confirmed data up b100
				#and packet format is as follows, payload starts at byte 9
				#MHDR[1] | DevAddr[4] | FCtrl[1] | FCnt[2] | FPort[1] | EncryptedPayload | MIC[4]
				if ord(ch) & 0x40 == 0x40 or ord(ch) & 0x80 == 0x80:
					#Do the LoRaWAN decoding
					print "LoRaWAN?"
					
					fillLinebuf(datalen-1)
					lorapktstr=ch+getAllLine()
					
					lorapkt=[]
					for i in range (0,len(lorapktstr)):
						lorapkt.append(ord(lorapktstr[i]))
						
					#you can uncomment/comment this display if you want
					print [hex(x) for x in lorapkt]
					
					datalen=datalen-LORAWAN_HEADER_SIZE
					
					src = lorapkt[4]*256*256*256
					src += lorapkt[3]*256*256
					src += lorapkt[2]*256
					src += lorapkt[1]
					
					seq=lorapkt[7]*256+lorapkt[6]
					
					#just to print the src in 0x01020304 form
					pdata="%d,%d,%s,%d,%d,%d,%d" % (256,ord(ch),"0x%0.8X" % src,seq,datalen,SNR,RSSI)
					print "update ctrl pkt info (^p): "+pdata
					#internally, we convert in int
					pdata="%d,%d,%d,%d,%d,%d,%d" % (256,ord(ch),src,seq,datalen,SNR,RSSI)					
					
					if _local_aes==1:							
						
						from loraWAN import loraWAN_process_pkt
						try:
							plain_payload=loraWAN_process_pkt(lorapkt)
						except:
							print "### unexpected decryption error ###"
							plain_payload="###BADMIC###"
						
						if plain_payload=="###BADMIC###":
							print plain_payload
						else:	
							print "plain payload is : ",
							print(replchars.sub(replchars_to_hex, plain_payload))
							#if ((ptype & PKT_FLAG_DATA_WAPPKEY)==PKT_FLAG_DATA_WAPPKEY):
							#	the_app_key = plain_payload[0]
							#	the_app_key = the_app_key + plain_payload[1]
							#	the_app_key = the_app_key + plain_payload[2]
							#	the_app_key = the_app_key + plain_payload[3]
							#	print " ".join("0x{:02x}".format(ord(c)) for c in the_app_key),
							#	print plain_payload[APPKEY_SIZE:] 
							#else:
							#	print plain_payload
							_linebuf = plain_payload
							_has_linebuf=1
							_hasClearData=1							
							
					else:
						print "--> DATA encrypted: local aes not activated"
						lorapktstr_b64=base64.b64encode(lorapktstr)
						print "--> FYI base64 of LoRaWAN frame w/MIC: "+lorapktstr_b64
						
						print "--> number of enabled clouds is %d" % len(_cloud_for_lorawan_encrypted_data)
						
						if len(_cloud_for_lorawan_encrypted_data)==0:
							print "--> discard encrypted data"
						else:					
							#loop over all enabled clouds to upload data
							#once again, it is up to the corresponding cloud script to handle the data format
							#
							for cloud_index in range(0,len(_cloud_for_lorawan_encrypted_data)):
								
								try:
									print "--> LoRaWAN encrypted cloud[%d]" % cloud_index
									cloud_script=_cloud_for_lorawan_encrypted_data[cloud_index]
									print "uploading with "+cloud_script
									sys.stdout.flush()
									cmd_arg=cloud_script+" \""+lorapktstr_b64.replace('\n','')+"\""+" \""+pdata.replace('\n','')+"\""+" \""+rdata.replace('\n','')+"\""+" \""+tdata.replace('\n','')+"\""+" \""+_gwid.replace('\n','')+"\""
								except UnicodeDecodeError, ude:
									print ude
								else:
									print cmd_arg
									sys.stdout.flush()
									try:
										os.system(cmd_arg)
									except:
										print "Error when uploading data to LoRaWAN encrypted cloud"								
								
							print "--> LoRaWAN encrypted cloud end"		
							
					continue	
					
			else:								
				#now we read datalen bytes in our line buffer
				fillLinebuf(datalen)				
				
			#encrypted data payload?
			if ((ptype & PKT_FLAG_DATA_ENCRYPTED)==PKT_FLAG_DATA_ENCRYPTED):
				print "--> DATA encrypted: encrypted payload size is %d" % datalen
				
				_hasClearData=0
				lorapktstr=getAllLine()
				
				if _local_aes==1:
						print "--> decrypting in AES-CTR mode (LoRaWAN version)"
						
						lorapkt=[]
						for i in range (0,len(lorapktstr)):
							lorapkt.append(ord(lorapktstr[i]))
						
						from loraWAN import loraWAN_process_pkt
						try:
							plain_payload=loraWAN_process_pkt(lorapkt)
						except:
							print "### unexpected decryption error ###"
							plain_payload="###BADMIC###"
						
						if plain_payload=="###BADMIC###":
							print plain_payload
						else:	
							print "plain payload is : ",
							print(replchars.sub(replchars_to_hex, plain_payload))
							
							#if ((ptype & PKT_FLAG_DATA_WAPPKEY)==PKT_FLAG_DATA_WAPPKEY):
							#	the_app_key = plain_payload[0]
							#	the_app_key = the_app_key + plain_payload[1]
							#	the_app_key = the_app_key + plain_payload[2]
							#	the_app_key = the_app_key + plain_payload[3]
							#	print " ".join("0x{:02x}".format(ord(c)) for c in the_app_key),
							#	print plain_payload[APPKEY_SIZE:] 
							#else:
							#	print plain_payload
							_linebuf = plain_payload
							_has_linebuf=1
							_hasClearData=1
							
							#remove the data encrypted flag
							ptype = ptype & (~PKT_FLAG_DATA_ENCRYPTED)
							pdata="%d,%d,%d,%d,%d,%d,%d" % (dst,ptype,src,seq,datalen,SNR,RSSI)	
							print '--> changed packet type to clear data' 					
						
				else:
						print "--> DATA encrypted: local aes not activated"
						lorapktstr_b64=base64.b64encode(lorapktstr)
						print "--> FYI base64 of LoRaWAN frame w/MIC: "+lorapktstr_b64
						
						print "--> number of enabled clouds is %d" % len(_cloud_for_encrypted_data)
						
						if len(_cloud_for_encrypted_data)==0:
							print "--> discard encrypted data"
						else:					
							#update pdata with new data length
							pdata="%d,%d,%d,%d,%d,%d,%d" % (dst,ptype,src,seq,datalen,SNR,RSSI)
							
							#loop over all enabled clouds to upload data
							#once again, it is up to the corresponding cloud script to handle the data format
							#							
							for cloud_index in range(0,len(_cloud_for_encrypted_data)):
								
								try:
									print "--> encrypted cloud[%d]" % cloud_index
									cloud_script=_cloud_for_encrypted_data[cloud_index]
									print "uploading with "+cloud_script
									sys.stdout.flush()
									cmd_arg=cloud_script+" \""+lorapktstr_b64.replace('\n','')+"\""+" \""+pdata.replace('\n','')+"\""+" \""+rdata.replace('\n','')+"\""+" \""+tdata.replace('\n','')+"\""+" \""+_gwid.replace('\n','')+"\""
								except UnicodeDecodeError, ude:
									print ude
								else:
									print cmd_arg
									sys.stdout.flush()
									try:
										os.system(cmd_arg)
									except:
										print "Error when uploading data to encrypted cloud"
											
							print "--> encrypted cloud end"	
			else:
				_hasClearData=1
										
			#with_appkey?
			if ((ptype & PKT_FLAG_DATA_WAPPKEY)==PKT_FLAG_DATA_WAPPKEY and _hasClearData==1): 
				print "--> DATA with_appkey: read app key sequence"
				
				the_app_key = getSingleChar()
				the_app_key = the_app_key + getSingleChar()
				the_app_key = the_app_key + getSingleChar()
				the_app_key = the_app_key + getSingleChar()
				
				print "app key is ",
				print " ".join("0x{:02x}".format(ord(c)) for c in the_app_key)

				if _wappkey==1:
					if the_app_key in key_AppKey.app_key_list:
						print "in app key list"				
						_validappkey=1
					else:
						print "not in app key list"
						_validappkey=0
				else:
					print "app key disabled"
					_validappkey=1				
				
			continue	
					
					
		if (ch >= '\x50' and ch <= '\x54'):
			print "--> got image packet"
			
			cam_id=ord(ch)-0x50;
			src_addr_msb = ord(getSingleChar())
			src_addr_lsb = ord(getSingleChar())
			src_addr = src_addr_msb*256+src_addr_lsb
					
			seq_num = ord(getSingleChar())
	
			Q = ord(getSingleChar())
	
			data_len = ord(getSingleChar())
	
			if (src_addr in nodeL):
				#already in list
				#get the file handler
				theFile=fileH[src_addr]
				#TODO
				#start some timer to remove the node from nodeL
			else:
				#new image packet from this node
				nodeL.append(src_addr)
				filename =(_folder_path+"images/ucam_%d-node_%.4d-cam_%d-Q%d.dat" % (imgSN,src_addr,cam_id,Q))
				print "first pkt from node %d" % src_addr
				print "creating file %s" % filename
				theFile=open(os.path.expanduser(filename),"w")
				#associates the file handler to this node
				fileH.update({src_addr:theFile})
				#and associates imageFilename, imagSN,Q and cam_id
				imageFilenameA.update({src_addr:filename})
				imgsnA.update({src_addr:imgSN})
				qualityA.update({src_addr:Q})
				camidA.update({src_addr:cam_id})
				imgSN=imgSN+1
				t = Timer(90, image_timeout)
				t.start()
				#log only the first packet and the filename
				f=open(os.path.expanduser(_imagelog_filename),"a")
				f.write(info_str+' ')	
				now = datetime.datetime.now()
				f.write(now.isoformat()+'> ')
				f.write(filename+'\n')
				f.close()				
							
			print "pkt %d from node %d data size is %d" % (seq_num,src_addr,data_len)
			print "write to file"
			
			theFile.write(format(data_len, '04X')+' ')
	
			for i in range(1, data_len):
				ch=getSingleChar()
				#sys.stdout.write(hex(ord(ch)))
				#sys.stdout.buffer.write(ch)
				print (hex(ord(ch))),
				theFile.write(format(ord(ch), '02X')+' ')
				
			print "End"
			sys.stdout.flush()
			theFile.flush()
			continue
			
	if (ch == '?' and _ignoreComment==1):
		sys.stdin.readline()
		continue
	
	sys.stdout.write(ch)
