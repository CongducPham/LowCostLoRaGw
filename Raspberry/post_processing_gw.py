# Copyright 2015 Congduc Pham, University of Pau, France.
# 
# Conduc.Pham@univ-pau.fr
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
from threading import Timer
import time
from collections import deque
import datetime
import getopt
import os
import subprocess

#------------------------------------------------------------
#header packet information
#------------------------------------------------------------

HEADER_SIZE=4
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


#------------------------------------------------------------
#with firebase support?
#------------------------------------------------------------
_firebase=0
#------------------------------------------------------------
#change here the entry path for your firebase sensor data
_dbpath='/LoRa/LIUPPA/RPIgateway/'
#change here for your own firebase url when you have one
_defdatabase='https://XXXXXXXXXXX.firebaseio.com'
#------------------------------------------------------------


#------------------------------------------------------------
#with thingspeak support?
#------------------------------------------------------------
_thingspeak=0
#plot snr instead of seq
_thingspeaksnr=0
#------------------------------------------------------------
#we have a public channel for your test
#https://thingspeak.com/channels/66794
#the write key is SGSH52UGPVAUYG3S
#the read key is 5OMG8DESB3PTUX20
#change here for your own channel write key when you have one
_defchannel='SGSH52UGPVAUYG3S'
#------------------------------------------------------------

#------------------------------------------------------------
#with fiware support?
#------------------------------------------------------------
_fiware=0
#------------------------------------------------------------

#------------------------------------------------------------
#log gateway message?
#------------------------------------------------------------
_logGateway=0
#gateway id/addr, just for giving a different name to various log files
_gwaddr=1
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


#------------------------------------------------------------
#check for app key?
#------------------------------------------------------------
_wappkey=0
#------------------------------------------------------------
the_app_key = [0,0,0,0]

#valid app key? by default we do not check for the app key
_validappkey=1
#------------------------------------------------------------
#add here app keys that you want to allow for your gateway
app_key_list = [
	#change here your application key
	[1,2,3,4],
	[5,6,7,8] 
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


def image_timeout():
	#get the node which timer has expired first
	#i.e. the one that received image packet earlier
	node_id=nodeL.popleft()
	print "close file for node %d" % node_id
	f=fileH[node_id]
	f.close()
	del fileH[node_id]

def main(argv):
	try:
		opts, args = getopt.getopt(argv,'iftLa:',['ignorecomment','firebase','thingspeak','fiware','loggw','addr', 'wappkey', 'raw'])
	except getopt.GetoptError:
		print 'parseLoRaStdin -i -f -t --fiware -L -a --wappkey --raw'
		sys.exit(2)
	
	for opt, arg in opts:
		if opt in ("-i", "--ignorecomment"):
			print "will ignore commented lines"
			global _ignoreComment
			_ignoreComment = 1
			
		elif opt in ("-f", "--firebase"):
			print "will enable firebase support"
			global _firebase
			_firebase = 1
			from firebase import firebase
			global _db
			_db = firebase.FirebaseApplication(_defdatabase, None)

		elif opt in ("-t", "--thingspeak"):
			print "will enable thingspeak support"
			global _thingspeak
			_thingspeak = 1

		elif opt in ("--fiware"):
			print "will enable fiware support"
			global _fiware
			_fiware = 1
									
		if opt in ("-L", "--loggw"):
			print "will log gateway message prefixed by ^$"
			global _logGateway
			_logGateway = 1			
			
		elif opt in ("-a", "--addr"):
			global _gwaddr
			_gwaddr = arg
			print "will use _"+str(_gwaddr)+" for gateway and telemetry log files"			

		elif opt in ("--wappkey"):
			global _wappkey
			_wappkey = 1
			global _validappkey
			_validappkey=0
			print "will check for correct app key"
			
		elif opt in ("--raw"):
			global _rawFormat
			_rawFormat = 1
			print "raw output from gw, will handle packet format"			
		
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
#	\$	indicates a message that should be logged in the (Dropbox) telemetry.log file
#		example: \$hello -> 	hello will be logged in the following format
#					(src=3 seq=0 len=6 SNR=8 RSSI=-54) 2015-10-16T14:47:44.072230> hello    
#
#	\&	indicates a message that should be logged in the firebase cloud database
#		example: \&hello ->	hello will be logged in the following json format
#					2015-11-04
#						sensor3
#							src:3
#							seq:0
#							len:6
#							SNR:8
#							RSSI:-54
#							date:"2015-11-04T11:16:11.216227"
#							text:"hello"
#							info_str:"(src=3 seq=0 len=6 SNR=8 RSSI=-54) 2015-11-04T11:16:11.216227> hello"
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

#----------------------------------------------------------------------------
#change here the various path for your log file on Dropbox
_gwlog_filename = "~/Dropbox/LoRa-test/gateway_"+str(_gwaddr)+".log"
_telemetrylog_filename = "~/Dropbox/LoRa-test/telemetry_"+str(_gwaddr)+".log"
_imagelog_filename = "~/Dropbox/LoRa-test/image_"+str(_gwaddr)+".log"
#----------------------------------------------------------------------------

while True:
	sys.stdout.flush()
	ch=sys.stdin.read(1)

#
# '^' is reserved for control information from the gateway
#
	if (ch=='^'):
		now = datetime.datetime.now()
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
						
		if (ch=='l'):
			# TODO: LAS service	
			print 'not implemented yet'
			
		if (ch=='$' and _logGateway==1):
			data = sys.stdin.readline()
			print "rcv gw output to log (^$): "+data,
			f=open(os.path.expanduser(_gwlog_filename),"a")
			now = datetime.datetime.now()
			f.write(now.isoformat()+'> ')
			f.write(data)
			f.close()					
		continue


#
# '\' is reserved for message logging service
#

	if (ch=='\\'):
		now = datetime.datetime.now()
		
		if _validappkey==1:

			print 'valid app key: accept data'
					
			ch=sys.stdin.read(1)
			
					
			if (ch=='$'): #log on Dropbox
				
				data = sys.stdin.readline()
				print "rcv msg to log (\$) on dropbox: "+data,
				f=open(os.path.expanduser(_telemetrylog_filename),"a")
				f.write(info_str+' ')	
				now = datetime.datetime.now()
				f.write(now.isoformat()+'> ')
				f.write(data)
				f.close()	
								
			elif (ch=='&' and _firebase==1): #log on Firebase
				
				data = sys.stdin.readline()
				print 'rcv msg to log (\&) on firebase: '+data,
				now = datetime.datetime.now()
				firebase_msg = {
					'dst':dst,
					'type':ptypestr,					
					'src':src,
					'seq':seq,
					'len':datalen,
					'SNR':SNR,
					'RSSI':RSSI,
					'date':now.isoformat(),
					'text':data,
					'info_str':info_str+' '+now.isoformat()+'> '+data	 
				}			
				sensor_entry='sensor%d'% (src)
				msg_entry='msg%d' % (seq)	
				date_entry=now.date()
				db_entry = _dbpath+date_entry.isoformat()+'/'+sensor_entry
				result = _db.put(db_entry, msg_entry, firebase_msg)		
			
			elif (ch=='!' and _thingspeak==1): #log on ThingSpeak
	
				ldata = sys.stdin.readline()
	
				# get number of '#' separator
				nsharp = ldata.count('#')			
				#no separator
				if nsharp==0:
					#will use default channel and field
					data=['','','1']
					data[2]=ldata
				elif nsharp==1:
					#only 1 separator
					data = ldata.split('#')
					#if the first item has length > 1 then we assume that it is a channel write key
					if len(data[0])>1:
						#insert '' to indicate default field
						data.insert(1,'');		
					else:
						#insert '' to indicate default channel
						data.insert(0,'');								
				else:
					#get the ThingSpeak channel and the data
					data = ldata.split('#')
					#we then have split in channel,field,value
					
				#just in case we have an ending CR or 0
				data[2] = data[2].replace('\n', '')
				data[2] = data[2].replace('\0', '')
											
				print 'rcv msg to log (\!) on ThingSpeak (',
				
				#use default channel?
				if data[0]=='':
					data[0]=_defchannel
					print 'default',
				else:
					print data[0],
				
				print ',',
					
				#use default field?
				if data[1]=='':
					data[1]='1'
					print 'default',
				else:
					print data[1],				
				
				print '): '+data[2]	
				
				#in the current example, log the data on the specified field x, then log the sequence number on field x+4
				#assuming that the first 4 fields are used for up to 4 sensors and the next 4 fields are used for their sequence number
				cmd = 'curl -s -k -X POST --data '+\
					'field'+data[1]+'='+data[2]+\
					'&field'+str(int(data[1])+4)+'='+str(seq)+\
					' https://api.thingspeak.com/update?key='+data[0]
				
				print "ThingSpeak: will issue curl cmd"
				print cmd
				args = cmd.split()						
	
				# ThingSpeak accepts a POST every 15s
				# so message should not arrive too fast
				try:
					out = subprocess.check_output(args, shell=False)
					#p = subprocess.check_output(args, shell=False, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
					#out, err = p.communicate()
				except subprocess.CalledProcessError:
					print("ThingSpeak: curl command failed")
	    			
				print 'ThingSpeak: returned code from server is %s' % out
				
				#test for FIWARE 
				#need FIWARE access 
				if (_fiware==1):
					now = datetime.datetime.now()
					#entity_id = 'test_item_'+now.isoformat()
					entity_id = 'sensor%d'% (src)
					cmd = 'python ./fiware_UpdateEntityAttribute.py '+entity_id+' test temperature float '+data[2]
					
					print "FiWare: will issue python script"
					print cmd
					args = cmd.split()
					try:
						out = subprocess.check_output(args, shell=False)
					except subprocess.CalledProcessError:
						print("FiWare: python script failed")
	    			
					if out.find('"reasonPhrase" : "OK"') > 0:
						print("FiWare: Entity updated with ENTITY_ID "+entity_id)
					else:
						print("FiWare: Entity update failed")
						
			else: # not a known data logging prefix
				print 'unrecognized data logging prefix'
				sys.stdin.readline() 
					
		else:
			print 'invalid app key: discard data'
			sys.stdin.readline()

		continue
	
	# handle binary prefixes
	#if (ch == '\xFF' or ch == '+'):
	if (ch == '\xFF'):
	
		print "got first framing byte"
		ch=sys.stdin.read(1)				
		#if (ch == '\xFE' or ch == '('):
		if (ch == '\xFE'):
			#the data prefix is inserted by the gateway
			#do not modify, unless you know what you are doing and that you modify lora_gateway (comment WITH_DATA_PREFIX)
			print "--> got data prefix"
			
			#we actually need to use DATA_PREFIX in order to differentiate data from radio coming to the post-processing stage
			#if _wappkey is set then we have to first indicate that _validappkey=0
			if (_wappkey==1):
				_validappkey=0
			else:
				_validappkey=1	
				
			# if we have raw output from gw, then try to determine which kind of packet it is
			if (_rawFormat==1):
				ch=sys.stdin.read(1)
				
				# probably our modified Libelium header where the destination is the gateway and is the first byte
				#
				# format is dst(1B) ptype(1B) src(1B) seq(1B) [payload]
				if ch==1:					
					# dissect our modified Libelium format
					dst=ord(ch)
					ptype=ord(sys.stdin.read(1))
					src=ord(sys.stdin.read(1))
					seq=ord(sys.stdin.read(1))
					print "Libelium[dst=%d ptype=0x%.2X src=%d seq=%d]" % (dst,ptype,src,seq)
					
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
					print "LoRaWAN?"
					# for the moment just discard the data
					sys.stdin.read(datalen-1)
					
			# encrypted data payload?
			if ((ptype & PKT_FLAG_DATA_ENCRYPTED)==PKT_FLAG_DATA_ENCRYPTED):
				print "--> DATA encrypted: sorry can not handle encrypted data"
				# drain stdin of all the encrypted data
				sys.stdin.read(datalen)
				print "--> discard encrypted data"
				#
				# REMARK
				# you can alternatively push the encrypted data into a cloud by encoding the encryption data in
				# base64 format before
				continue
									
			# with_appkey?
			if ((ptype & PKT_FLAG_DATA_WAPPKEY)==PKT_FLAG_DATA_WAPPKEY): 
				print "--> DATA with_appkey: read app key sequence"
				
				the_app_key[0] = ord(sys.stdin.read(1))
				the_app_key[1] = ord(sys.stdin.read(1)) 
				the_app_key[2] = ord(sys.stdin.read(1))
				the_app_key[3] = ord(sys.stdin.read(1))
				
				print "app key is ",
				print the_app_key
				
				if the_app_key in app_key_list:
					print "in app key list"
					if _wappkey==1:
						_validappkey=1
				else:		
					print "not in app key list"
					if _wappkey==1:
						_validappkey=0
					else:	
						#we do not check for app key
						_validappkey=1
						print "but app key disabled"			
															
			continue				
					
		if (ch >= '\x50' and ch <= '\x54'):
			print "--> got image packet"
			
			cam_id=ord(ch)-0x50;
			src_adr_msb = ord(sys.stdin.read(1))
			src_adr_lsb = ord(sys.stdin.read(1))
			src_adr = src_adr_msb*256+src_adr_lsb
					
			seq_num = ord(sys.stdin.read(1))
	
			Q = ord(sys.stdin.read(1))
	
			data_len = ord(sys.stdin.read(1))
	
			if (src_adr in nodeL):
				#already in list
				#get the file handler
				theFile=fileH[src_adr]
				#TODO
				#start some timer to remove the node from nodeL
			else:
				#new image packet from this node
				nodeL.append(src_adr)
				filename = "tmp_%d-node#%.4d-cam#%d-Q%d.dat" % (rcvImg,src_adr,cam_id,Q)
				print "first pkt from node %d" % src_adr
				print "creating file %s" % filename
				theFile=open(filename,"w")
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
							
			print "pkt %d from node %d data size is %d" % (seq_num,src_adr,data_len)
			print "write to file"
			
			theFile.write(format(data_len, '04X')+' ')
	
			for i in range(1, data_len):
				ch=sys.stdin.read(1)
				print (hex(ord(ch))),
				theFile.write(format(ord(ch), '02X')+' ')
				
			print "\nEnd"
			sys.stdout.flush()
			theFile.flush()
			continue
			
	if (ch == '?' and _ignoreComment==1):
		sys.stdin.readline()
		continue
	
	sys.stdout.write(ch)
