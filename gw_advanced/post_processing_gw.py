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

import sys
import select
import threading
from threading import Timer
import time
from collections import deque
import datetime
import getopt
import os
import json
import re

#------------------------------------------------------------
#header packet information
#------------------------------------------------------------

HEADER_SIZE=4
APPKEY_SIZE=4
PKT_TYPE_DATA=0x10
PKT_TYPE_ACK=0x20

PKT_FLAG_ACK_REQ=0x08
PKT_FLAG_DATA_ENCRYPTED=0x04
PKT_FLAG_DATA_WAPPKEY=0x02
PKT_FLAG_DATA_ISBINARY=0x01

#------------------------------------------------------------
#last pkt information
#------------------------------------------------------------
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
#------------------------------------------------------------


#------------------------------------------------------------
#will ignore lines beginning with '?'
#------------------------------------------------------------
_ignoreComment=1

#------------------------------------------------------------
#with firebase support?
#------------------------------------------------------------
_firebase=False

#------------------------------------------------------------
#with thingspeak support?
#------------------------------------------------------------
_thingspeak=False
#plot snr instead of seq
_thingspeaksnr=False

#------------------------------------------------------------
#with sensorcloud support?
#------------------------------------------------------------
_sensorcloud=False

#------------------------------------------------------------
#with grovestreams support?
#------------------------------------------------------------
_grovestreams=False

#------------------------------------------------------------
#with fiware support?
#------------------------------------------------------------
_fiware=False

#------------------------------------------------------------
#with mongoDB support?
#------------------------------------------------------------
_mongodb = False

#------------------------------------------------------------
#log gateway message?
#------------------------------------------------------------
_logGateway=0
#path of json file containing the gateway address
_filename_path = "local_conf.json"


#open json file to recover gateway_address
f = open(os.path.expanduser(_filename_path),"r")
lines = f.readlines()
f.close()
array = ""
#get all the lines in a string
for line in lines :
	array += line

#change it into a python array
json_array = json.loads(array)

#set the gateway_address for having different log filenames
_gwaddr = json_array["gateway_conf"]["gateway_ID"]


#------------------------------------------------------------
#change here the various path for your log file on Dropbox
_gwlog_filename = "~/Dropbox/LoRa-test/gateway_"+str(_gwaddr)+".log"
_telemetrylog_filename = "~/Dropbox/LoRa-test/telemetry_"+str(_gwaddr)+".log"
_imagelog_filename = "~/Dropbox/LoRa-test/image_"+str(_gwaddr)+".log"
#------------------------------------------------------------

#------------------------------------------------------------
#raw output from gateway?
#------------------------------------------------------------
_rawFormat=0
#------------------------------------------------------------
_ourcustomFormat=0;
_lorawanFormat=0
#------------------------------------------------------------

#------------------------------------------------------------
#check for app key?
#------------------------------------------------------------
_wappkey=0
#------------------------------------------------------------
the_app_key = '\x00\x00\x00\x00'

#valid app key? by default we do not check for the app key
_validappkey=1

#------------------------------------------------------------
#initialize gateway DHT22 sensor
#------------------------------------------------------------
_gw_dht22 = json_array["gateway_conf"]["dht22"]
_date_save_dht22 = None

if(_gw_dht22):
	print "Use DHT22 to get gateway temperature and humidity level"
	#read values from dht22 in the gateway box
	sys.path.insert(0, os.path.expanduser('./sensors_in_raspi/dht22'))
	from read_dht22 import get_dht22_values
	
	_temperature = 0
	_humidity = 0

#------------------------------------------------------------
#add here app keys that you want to allow for your gateway
#
#NOTE:	the format of the application key list has changed from a list of list, to a list of string that will be
#		process as a byte array. Doing so wil allow for dictionary construction using the appkey to retrieve information
#		such as an encryption key,...
#		
app_key_list = [
	#for testing
	'****',
	#change here your application key
	'\x01\x02\x03\x04',
	'\x05\x06\x07\x08' 
]
#------------------------------------------------------------


#------------------------------------------------------------
#for handling images
#------------------------------------------------------------
#list of active nodes
nodeL = deque([])
#association to get the file handler
fileH = {}
#global image seq number
rcvImg=0
#------------------------------------------------------------

#------------------------------------------------------------
#for managing the input data when we can have aes encryption
#------------------------------------------------------------
_linebuf="the line buffer"
_linebuf_idx=0
_has_linebuf=0

def getSingleChar():
	global _has_linebuf
	# if we have a valid _linebuf then read from _linebuf
	if _has_linebuf==1:
		global _linebuf_idx
		global _linebuf
		if _linebuf_idx < len(_linebuf):
			_linebuf_idx = _linebuf_idx + 1
			return _linebuf[_linebuf_idx-1]
		else:
			# no more character from _linebuf, so read from stdin
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
	# return the remaining of the string and clear the _linebuf
	return _linebuf[p:]	
	
def fillLinebuf(n):
	global _linebuf_idx
	_linebuf_idx = 0
	global _has_linebuf
	_has_linebuf = 1
	global _linebuf
	# fill in our _linebuf from stdin
	_linebuf=sys.stdin.read(n)
	
#------------------------------------------------------------
	
#------------------------------------------------------------
#for local AES decrypting
#------------------------------------------------------------	
_aes=0
_hasClearData=0
#put your key here, should match the end-device's key
aes_key="0123456789010123"
#put your initialisation vector here, should match the end-device's initialisation vector
aes_iv="\x9a\xd0\x30\x02\x00\x00\x00\x00\x9a\xd0\x30\x02\x00\x00\x00\x00"
#aes_iv="\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00

#association between appkey and aes_key
appkey_aeskey = {
	'\x01\x02\x03\x04':"0123456789010123",
	'\x05\x06\x07\x08':"0123456789010123"
}

#association between appkey and aes_iv
appkey_aesiv = {
	'\x01\x02\x03\x04':"\x9a\xd0\x30\x02\x00\x00\x00\x00\x9a\xd0\x30\x02\x00\x00\x00\x00",
	'\x05\x06\x07\x08':"\x9a\xd0\x30\x02\x00\x00\x00\x00\x9a\xd0\x30\x02\x00\x00\x00\x00"
}
#------------------------------------------------------------

def image_timeout():
	#get the node which timer has expired first
	#i.e. the one that received image packet earlier
	node_id=nodeL.popleft()
	print "close file for node %d" % node_id
	f=fileH[node_id]
	f.close()
	del fileH[node_id]
	
# retrieve dht22 values
def save_dht22_values():
	global _temperature, _humidity, _date_save_dht22
	_humidity, _temperature = get_dht22_values()
	
	_date_save_dht22 = datetime.datetime.utcnow()

	print "Gateway TC : "+_temperature+" C | HU : "+_humidity+" % at "+str(_date_save_dht22)
	
	#save values from the gateway box's DHT22 sensor, if _mongodb is true
	if(_mongodb):
		#saving data in a JSON var
		str_json_data = "{\"th\":"+_temperature+", \"hu\":"+_humidity+"}"
	
		#creating document to add
		doc = {
			"type" : "DATA_GW_DHT22",
			"gateway_eui" : _gwaddr, 
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

def main(argv):
	try:
		opts, args = getopt.getopt(argv,'iftLam:',['ignorecomment','firebase','thingspeak','retrythsk','thingspeaksnr','fiware','sensorcloud','grovestreams','loggw','addr', 'wappkey', 'raw', 'aes', 'mongodb'])
	except getopt.GetoptError:
		print 'parseLoRaStdin -i -f/--firebase -t/--thingspeak --retrythsk --thingspeaksnr --fiware --sensorcloud --grovestreams -L/--loggw -a/--addr --wappkey --raw --aes -m/--mongodb'
		sys.exit(2)
	
	for opt, arg in opts:
		if opt in ("-i", "--ignorecomment"):
			print("will ignore commented lines")
			global _ignoreComment
			_ignoreComment = 1
			
		elif opt in ("-f", "--firebase"):
			print("will enable firebase support")
			global _firebase
			_firebase = True
			global firebase_uploadSingleData
			from FireBase import firebase_uploadSingleData

		elif opt in ("-t", "--thingspeak"):
			print("will enable thingspeak support")
			global _thingspeak
			_thingspeak = True
			global thingspeak_uploadSingleData, thingspeak_uploadMultipleData
			from ThingSpeak import thingspeak_uploadSingleData, thingspeak_uploadMultipleData
			
		elif opt in ("--retrythsk"):
			print("will enable thingspeak retry")
			global thingspeak_setRetry
			from ThingSpeak import thingspeak_setRetry
			#set retry to True
			thingspeak_setRetry(True)

		elif opt in ("--thingspeaksnr"):
			print("will plot snr instead of seq")
			global _thingspeaksnr
			_thingspeaksnr = True
			
		elif opt in ("--fiware"):
			print("will enable fiware support")
			global _fiware
			_fiware = True

		elif opt in ("--sensorcloud"):
			print("will enable sensorcloud support")
			global _sensorcloud
			_sensorcloud = True
			global sensorcloud_uploadSingleData
			from SensorCloud import sensorcloud_uploadSingleData

		elif opt in ("--grovestreams"):
			print("will enable grovestreams support")
			global _grovestreams
			_grovestreams = True
			global grovestreams_uploadSingleData
			from GroveStreams import grovestreams_uploadSingleData
															
		elif opt in ("-L", "--loggw"):
			print("will log gateway message prefixed by ^$")
			global _logGateway
			_logGateway = 1	

		elif opt in ("-a", "--addr"):
			global _gwaddr
			_gwaddr = arg
			print("overwrite: will use _"+str(_gwaddr)+" for gateway and telemetry log files")
			
		elif opt in ("--wappkey"):
			global _wappkey
			_wappkey = 1
			global _validappkey
			_validappkey=0
			print("will check for correct app key")

		elif opt in ("--raw"):
			global _rawFormat
			_rawFormat = 1
			print("raw output from gateway. post_processing_gw will handle packet format")
			
		elif opt in ("--aes"):
			global _aes
			_aes = 1
			global AES
			from Crypto.Cipher import AES
			print("enable AES encrypted data")
			
		elif opt in ("-m", "--mongodb"):
			print("will enable local MongoDB support, max months to store is "+arg)
			global _mongodb
			_mongodb = True
			
			global add_document, remove_if_new_month, mongodb_set_max_months
			from MongoDB import add_document, remove_if_new_month, mongodb_set_max_months
			#setting max months
			mongodb_set_max_months(int(arg))
			
					
if __name__ == "__main__":
	main(sys.argv[1:])
      
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
#	\&	indicates a message that should be logged in the firebase cloud database
#		example: \&hello ->	hello will be logged in json format
#
#	\!	indicates a message that should be logged on a thingspeak channel
#		example: \!SGSH52UGPVAUYG3S#9.4 ->	9.4 will be logged in the SGSH52UGPVAUYG3S ThingSpeak channel at default field, i.e. field 1
#				 \!2#9.4 -> 9.4 will be logged in the default channel at field 2
#				 \!SGSH52UGPVAUYG3S#2#9.4 -> 9.4 will be logged in the SGSH52UGPVAUYG3S ThingSpeak channel at field 2
#
#		you can log other information such as src, seq, len, SNR and RSSI on specific fields
#
#	\xFF\xFE		indicates radio data prefix
#
#	\xFF\x50-\x54 	indicates an image packet. Next fields are src_adr(2B), seq(1B), Q(1B), size(1B)
#					cam id is coded with the second framing byte: i.e. \x50 means cam id = 0
#

#gateway dht22
if (_gw_dht22):
	print "Starting thread to measure gateway temperature"
	t = threading.Thread(target=dht22_target)
	t.daemon = True
	t.start()

while True:

	sys.stdout.flush()
  	ch = getSingleChar()

#
# '^' is reserved for control information from the gateway
#
	if (ch=='^'):
		now = datetime.datetime.utcnow()
		ch=sys.stdin.read(1)
		
		if (ch=='p'):		
			data = sys.stdin.readline()
			print now.isoformat()
			print "rcv ctrl pkt info (^p): "+data,
			arr = map(int,data.split(','))
			print "splitted in: ",
			print arr
			dst=arr[0]
			ptype=arr[1]
			ptypestr="N/A"
			if ((ptype & 0xF0)==PKT_TYPE_DATA):
				ptypestr="DATA"
				if (ptype & PKT_FLAG_DATA_ISBINARY)==PKT_FLAG_DATA_ISBINARY:
					ptypestr = ptypestr + " IS_BINARY"
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
			# TODO: maintain statistics from received messages and periodically add these informations in the gateway.log file

		if (ch=='r'):		
			data = sys.stdin.readline()
			print "rcv ctrl radio info (^r): "+data,
			arr = map(int,data.split(','))
			print "splitted in: ",
			print arr
			bw=arr[0]
			cr=arr[1]
			sf=arr[2]
			info_str="(BW=%d CR=%d SF=%d)" % (bw,cr,sf)
			print info_str

		if (ch=='t'):
			rcv_timestamp = sys.stdin.readline()
			print "rcv timestamp (^t): "+rcv_timestamp
									
		if (ch=='l'):
			# TODO: LAS service	
			print 'not implemented yet'
			
		if (ch=='$' and _logGateway==1):
			data = sys.stdin.readline()
			print "rcv gw output to log (^$): "+data,
			f=open(os.path.expanduser(_gwlog_filename),"a")
			f.write(now.isoformat()+'> ')
			f.write(data)
			f.close()					
		continue


#
# '\' is reserved for message logging service
#

	if (ch=='\\'):
		now = datetime.datetime.utcnow()
		
		if _validappkey==1:

			print 'valid app key: accept data'
					
			ch=getSingleChar()			
					
			if (ch=='$'): #log on Dropbox
				
				data = getAllLine()
				
				print "rcv msg to log (\$) on dropbox: "+data,
				f=open(os.path.expanduser(_telemetrylog_filename),"a")
				f.write(info_str+' ')	
				f.write(now.isoformat()+'> ')
				f.write(data)
				f.close()	
								
			elif (ch=='&' and _firebase): #log on Firebase
				
				ldata = getAllLine()
				
				print 'rcv msg to log (\&) on firebase: '+data
				firebase_msg = {
					'dst':dst,
					'type':ptypestr,
					'gateway_eui' : _gwaddr,					
					'node_eui':src,
					'seq':seq,
					'len':datalen,
					'snr':SNR,
					'rssi':RSSI,
					'cr' : cr, 
					'datarate' : "SF"+str(sf)+"BW"+str(bw),
					'time':now.isoformat(),
					'info_str':info_str+' '+now.isoformat()+'> '+ldata,					
					'data':ldata
				}
				
				if _mongodb :
					#------------------
					#saving in MongoDB
					#------------------
					
					#get the data
					data = ldata.split('/')
				
					#change data in two arrays : nomenclature_array and value_array
					iteration = 0
					nomenclature_array = []
					value_array = []
					while iteration<len(data) :
						if (iteration == 0 or iteration%2 == 0) :
						   	nomenclature_array.append(data[iteration])
						else :
						   	value_array.append(data[iteration])
						   	
						iteration += 1
				
					#check if new month
					remove_if_new_month(now)
				
					print("MongoDB: saving the document in the collection...")
				
					#saving data in a JSON var
					str_json_data = "{"
					iteration = 0
					while iteration < len(nomenclature_array) :
						#last iteration, do not add "," at the end
						if iteration == len(nomenclature_array)-1 :
							str_json_data += "\""+nomenclature_array[iteration]+"\" : "+value_array[iteration]
						else :
							str_json_data += "\""+nomenclature_array[iteration]+"\" : "+value_array[iteration]+", "
						iteration += 1
					str_json_data += "}"
				
					#creating document to add
					doc = {
						"type" : ptypestr,
						"gateway_eui" : _gwaddr, 
						"node_eui" : src,
						"snr" : SNR, 
						"rssi" : RSSI, 
						"cr" : cr, 
						"datarate" : "SF"+str(sf)+"BW"+str(bw), 
						"time" : now,
						"data" : json.dumps(json.loads(str_json_data))
					}
				
					#adding the document
					add_document(doc)
				
					print("MongoDB: saving done")
				
				sensor_entry='sensor%d'% (src)
				msg_entry='msg%d' % (seq)	
				
				#upload data to firebase
				firebase_uploadSingleData(firebase_msg, sensor_entry, msg_entry, now)
				
			elif (ch=='!'): #log on thingspeak, grovestreams, sensorcloud,...
	
				ldata = getAllLine()
				
				# get number of '#' separator
				nsharp = ldata.count('#')			
				#no separator
				if nsharp==0:
					#will use default channel and field
					data=['','']
					
					#contains ['', '', "s1", s1value, "s2", s2value, ...]
					data_array = data + re.split("/", ldata)		
				elif nsharp==1:
					#only 1 separator
					
					data_array = re.split("#|/", ldata)
					
					#if the first item has length > 1 then we assume that it is a channel write key
					if len(data_array[0])>1:
						#insert '' to indicate default field
						data_array.insert(1,'');		
					else:
						#insert '' to indicate default channel
						data_array.insert(0,'');		
				else:
					#contains [channel, field, "s1", s1value, "s2", s2value, ...]
					data_array = re.split("#|/", ldata)	
					
				#just in case we have an ending CR or 0
				data_array[len(data_array)-1] = data_array[len(data_array)-1].replace('\n', '')
				data_array[len(data_array)-1] = data_array[len(data_array)-1].replace('\0', '')	
				
				#test if there are characters at the end of each value, then delete these characters
				i = 3
				while i < len(data_array) :
					while not data_array[i][len(data_array[i])-1].isdigit() :
						data_array[i] = data_array[i][:-1]
					i += 2
				
				if _mongodb :	
					#------------------
					#saving in MongoDB
					#------------------
				
					#check if new month
					remove_if_new_month(now)
				
					print("MongoDB: saving the document in the collection...")
					
					#saving data in a JSON var
					str_json_data = "{"
					#start from the first nomenclature
					iteration = 2
					while iteration < len(data_array)-1 :
						#last iteration, do not add "," at the end
						if iteration == len(data_array)-2 :
							str_json_data += "\""+data_array[iteration]+"\" : "+data_array[iteration+1]
						else :
							str_json_data += "\""+data_array[iteration]+"\" : "+data_array[iteration+1]+", "
						iteration += 2
					str_json_data += "}"
				
					#creating document to add
					doc = {
						"type" : ptypestr,
						"gateway_eui" : _gwaddr, 
						"node_eui" : src,
						"snr" : SNR, 
						"rssi" : RSSI, 
						"cr" : cr, 
						"datarate" : "SF"+str(sf)+"BW"+str(bw),
						"time" : now,
						"data" : json.dumps(json.loads(str_json_data))
					}
				
					#adding the document
					add_document(doc)
				
					print("MongoDB: saving done")

				# get number of '/' separator
				nslash = ldata.count('/')
				
				index_first_data = 2
				
				if nslash==0:
					# old syntax without nomenclature key
					index_first_data=2
				else:
					# new syntax with nomenclature key				
					index_first_data=3
																				
				#------------------
				#test for thingspeak
				#------------------				 
				if (_thingspeak):
						
					second_data=str(seq)
				
					if (_thingspeaksnr):
						second_data=str(SNR)
					
					#data to send to thingspeak
					data = []
					data.append(data_array[0]) #channel (if '' default)
					data.append(data_array[1]) #field (if '' default)		
					
					data.append(data_array[index_first_data]) #value to add (the first sensor value in data_array)
					
					#upload data to thingspeak
					#JUST FOR UPLOAD A SINGLE DATA IN A SPECIFIC FIELD AND SECOND DATA					
					thingspeak_uploadSingleData(data, second_data)   

					# if you want to upload all data starting at field 1, uncomment next line, and comment previous line
					#thingspeak_uploadMultipleData(data_array) # upload all data in the fields
			
				#------------------
				#test for FIWARE 
				#need FIWARE access
				#------------------				 
				if (_fiware):
					print("FIWARE: upload")
					#entity_id = 'test_item_'+now.isoformat()
					entity_id = 'sensor%d'% (src)
					#send the first sensor value in data_array
					cmd = 'python ./fiware_UpdateEntityAttribute.py '+entity_id+' test temperature float '+data_array[index_first_data]
				
					print("FiWare: will issue python script")
					print(cmd)
					args = cmd.split()
					try:
						out = subprocess.check_output(args, shell=False)
					except subprocess.CalledProcessError:
						print("FiWare: python script failed")
	    			
					if out.find('"reasonPhrase" : "OK"') > 0:
						print("FiWare: Entity updated with ENTITY_ID "+entity_id)
					else:
						print("FiWare: Entity update failed")

				#------------------
				#test for sensorcloud 
				#------------------
				if (_sensorcloud) :
					#send the first sensor value in data_array
					sensorcloud_uploadSingleData(data_array[index_first_data])

				#------------------
				#test for grovestreams 
				#------------------				 
				if (_grovestreams):
				
					nomenclatures = []
					data = []
					
					if nslash==0:
						# old syntax without nomemclature key, so insert only one key
						nomenclatures.append("temp")
						data.append(data_array[index_first_data])
					else:
						#completing nomenclatures and data
						i=2
						while i < len(data_array)-1 :
							nomenclatures.append(data_array[i])
							data.append(data_array[i+1])
							i += 2
					
					#upload data to grovestreams
					grovestreams_uploadSingleData(nomenclatures, data, str(src))
									
															
			else: # not a known data logging prefix
				#you may want to upload to a default service
				#so just implement it here
				print('unrecognized data logging prefix: discard data')
				getAllLine() 
					
		else:
			print('invalid app key: discard data')
			getAllLine()

		continue
	
	# handle binary prefixes
	if (ch == '\xFF' or ch == '+'):
	#if (ch == '\xFF'):
	
		print("got first framing byte")
		ch=getSingleChar()	
		
		# data prefix for non-encrypted data
		if (ch == '\xFE' or ch == '+'):			
		#if (ch == '\xFE'):
			#the data prefix is inserted by the gateway
			#do not modify, unless you know what you are doing and that you modify lora_gateway (comment WITH_DATA_PREFIX)
			print("--> got data prefix")
			
			#we actually need to use DATA_PREFIX in order to differentiate data from radio coming to the post-processing stage
			#if _wappkey is set then we have to first indicate that _validappkey=0
			if (_wappkey==1):
				_validappkey=0
			else:
				_validappkey=1	

			# if we have raw output from gw, then try to determine which kind of packet it is
			if (_rawFormat==1):
				ch=getSingleChar()
				
				# probably our modified Libelium header where the destination is the gateway
				# dissect our modified Libelium format
				if ch==1:			
					dst=ord(ch)
					ptype=ord(getSingleChar())
					src=ord(getSingleChar())
					seq=ord(getSingleChar())
					print("Libelium[dst=%d ptype=0x%.2X src=%d seq=%d]" % (dst,ptype,src,seq))
					# now we read datalen-4 (the header length) bytes in our line buffer
					fillLinebuf(datalen-HEADER_SIZE)				
				
				# TODO: dissect LoRaWAN
				# you can implement LoRaWAN decoding if this is necessary for your system
				# look at the LoRaWAN packet format specification to dissect the packet in detail
				# 
				# LoRaWAN uses the MHDR(1B)
				# ----------------------------
				# | 7  6  5 | 4  3  2 | 1  0 |
				# ----------------------------
				#    MType      RFU     major
				#
				# the main MType is unconfirmed data up which value is 010
				if (ch & 0x40)==0x40:
					# Do the LoRaWAN decoding
					print("LoRaWAN?")
					# for the moment just discard the data
					fillLinebuf(datalen-1)
					getAllLine()
			else:								
				# now we read datalen bytes in our line buffer
				fillLinebuf(datalen)				
			
				
			# encrypted data payload?
			if ((ptype & PKT_FLAG_DATA_ENCRYPTED)==PKT_FLAG_DATA_ENCRYPTED):
				print("--> DATA encrypted: encrypted payload size is %d" % datalen)
				
				_hasClearData=0
				
				if _aes==1:
					print("--> decrypting")
					
					decrypt_handler = AES.new(aes_key, AES.MODE_CBC, aes_iv)
					# decrypt 
					s = decrypt_handler.decrypt(_linebuf)
					
					for i in range(0, len(s)):
						print "%.2X " % ord(s[i]),
					
					print "\nEnd"
						
					# get the real (decrypted) payload size
					rsize = ord(s[APPKEY_SIZE])
					
					print("--> real payload size is %d" % rsize)
					
					# then add the appkey + the appkey framing bytes
					rsize = rsize+APPKEY_SIZE+1
					
					_linebuf = s[:APPKEY_SIZE] + s[APPKEY_SIZE+1:rsize]
					
					for i in range(0, len(_linebuf)):
						print "%.2X " % ord(_linebuf[i]),
					
					print "\nEnd"
						
					# normally next read from input will get data from the decrypted _linebuf
					print "--> decrypted payload is: ",
					print _linebuf[APPKEY_SIZE:]
					
					_hasClearData=1
				else:
					print("--> DATA encrypted: aes not activated")
					# drain stdin of all the encrypted data
					enc_data=getAllLine()
					print("--> discard encrypted data")
			else:
				_hasClearData=1
										
			# with_appkey?
			if ((ptype & PKT_FLAG_DATA_WAPPKEY)==PKT_FLAG_DATA_WAPPKEY and _hasClearData==1): 
				print("--> DATA with_appkey: read app key sequence")
				
				the_app_key = getSingleChar()
				the_app_key = the_app_key + getSingleChar()
				the_app_key = the_app_key + getSingleChar()
				the_app_key = the_app_key + getSingleChar()
				
				print "app key is ",
				print " ".join("0x{:02x}".format(ord(c)) for c in the_app_key)
				
				if the_app_key in app_key_list:
					print("in app key list")
					if _wappkey==1:
						_validappkey=1
				else:		
					print("not in app key list")
					if _wappkey==1:
						_validappkey=0
					else:	
						#we do not check for app key
						_validappkey=1
						print("but app key disabled")				
				
			continue	
					
					
		if (ch >= '\x50' and ch <= '\x54'):
			print("--> got image packet")
			
			cam_id=ord(ch)-0x50;
			src_adr_msb = ord(getSingleChar())
			src_adr_lsb = ord(getSingleChar())
			src_adr = src_adr_msb*256+src_adr_lsb
					
			seq_num = ord(getSingleChar())
	
			Q = ord(getSingleChar())
	
			data_len = ord(getSingleChar())
	
			if (src_adr in nodeL):
				#already in list
				#get the file handler
				theFile=fileH[src_adr]
				#TODO
				#start some timer to remove the node from nodeL
			else:
				#new image packet from this node
				nodeL.append(src_adr)
				filename =("~/Dropbox/LoRa-test/images/tmp_%d-node#%.4d-cam#%d-Q%d.dat" % (rcvImg,src_adr,cam_id,Q))
				print("first pkt from node %d" % src_adr)
				print("creating file %s" % filename)
				open(os.path.expanduser(filename),"w")
				# associates the file handler to this node
				fileH.update({src_adr:theFile})
				rcvImg=rcvImg+1
				t = Timer(35, image_timeout)
				t.start()
				#log only the first packet and the filename
				f=open(os.path.expanduser(_imagelog_filename),"a")
				f.write(info_str+' ')	
				now = datetime.datetime.now()
				f.write(now.isoformat()+'> ')
				f.write(filename+'\n')
				f.close()				
							
			print("pkt %d from node %d data size is %d" % (seq_num,src_adr,data_len))
			print("write to file")
			
			theFile.write(format(data_len, '04X')+' ')
	
			for i in range(1, data_len):
				ch=getSingleChar()
				# sys.stdout.write(hex(ord(ch)))
				# sys.stdout.buffer.write(ch)
				print (hex(ord(ch))),
				theFile.write(format(ord(ch), '02X')+' ')
				
			print("End")
			sys.stdout.flush()
			theFile.flush()
			continue
			
	if (ch == '?' and _ignoreComment==1):
		sys.stdin.readline()
		continue
	
	sys.stdout.write(ch)
