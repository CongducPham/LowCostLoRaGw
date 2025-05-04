#------------------------------------------------------------
# Copyright 2019 Congduc Pham, University of Pau, France.
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
import re
import string
import base64
import os
import os.path

try:
	import LoRaWAN
	from LoRaWAN.MHDR import MHDR
except ImportError:
	print "LoRaWAN python lib must be installed"

#don't generate pyc (no compilation of imported module) so change in key_* file can be done dynamically
sys.dont_write_bytecode = True

#contains the list device's encryption keys used by LoRaWAN: AppSKey and NwkSKey
import key_LoRaWAN as key
		
def import_LoRaWAN_lib():

	try:
		import LoRaWAN
		from LoRaWAN.MHDR import MHDR
	except ImportError:
		print "LoRaWAN python lib must be installed"
		print "loraWAN.py: checking for LoRaWAN lib"
	
		if os.path.isfile(os.path.expanduser("aes-python-lib/LoRaWAN/MHDR.py")):
			cmd_arg="sudo cp -r aes-python-lib/LoRaWAN /usr/lib/python2.7/dist-packages"
			print "copying LoRaWAN lib in python distribution"
			try:
				os.system(cmd_arg)
			except:
				print "Error when copying LoRaWAN lib"
				return False
			
			print "try importing LoRaWAN lib again"
			
			try:	
				import LoRaWAN
				from LoRaWAN.MHDR import MHDR
				print "import now ok"
				return True
			except ImportError:
				print "sorry, error."
				return False							
		else:	
			return False
	
	print "import ok"		
	return True	

PKT_TYPE_DATA=0x10
PKT_FLAG_DATA_ENCRYPTED=0x04

#to display non printable characters
replchars = re.compile(r'[\x00-\x1f]')

def replchars_to_hex(match):
	return r'\x{0:02x}'.format(ord(match.group()))
	
def loraWAN_process_pkt(lorapkt):

	#print "entering loraWAN_process_pkt"

	if (lorapkt[0] == 0x40) or (lorapkt[0] == 0x80):
	
		src = lorapkt[4]*256*256*256
		src += lorapkt[3]*256*256
		src += lorapkt[2]*256
		src += lorapkt[1]
		
		src_str="%0.8X" % src
		
		if src_str in key.device_key:		
			print "found device 0x%s in device key list" % src_str
		else:
			print "did not find device 0x%s in device key list" % src_str
			print "using AppSKey and NwkSKey from default device"	
			src_str = 'default'

		AppSKey = key.device_key[src_str]['AppSKey']
		NwkSKey = key.device_key[src_str]['NwkSKey']
					
		#print "start decryption"
	
		appskey=bytearray.fromhex(AppSKey)
		appskeylist=[]
		for i in range (0,len(appskey)):
			appskeylist.append(appskey[i])

		nwkskey=bytearray.fromhex(NwkSKey)
		nwkskeylist=[]
		for i in range (0,len(nwkskey)):
			nwkskeylist.append(nwkskey[i])

		lorawan = LoRaWAN.new(nwkskeylist)
		lorawan.read(lorapkt)
		lorawan.compute_mic()
		
		if lorawan.valid_mic():
			print "?loraWAN: valid MIC"
			lorawan = LoRaWAN.new(appskeylist)
			lorawan.read(lorapkt)	
			plain_payload = ''.join(chr(x) for x in lorawan.get_payload())
			print "?loraWAN: plain payload is "+replchars.sub(replchars_to_hex, plain_payload)
			return plain_payload
		else:
			return "###BADMIC###"
				
	else:
		return "###BADMIC###"	
		
def loraWAN_get_MIC(src, lorapktstr):

	src_str=str(src)

	if src_str in key.device_key:		
		print "found device %0.8X in device key list" % src_str
	else:
		print "did not find device 0x%s in device key list" % src_str
		print "using AppSKey and NwkSKey from default device"	
		src_str = 'default'

	AppSKey = key.device_key[src_str]['AppSKey']
	NwkSKey = key.device_key[src_str]['NwkSKey']
	
	appskey=bytearray.fromhex(key.AppSKey)
	appskeylist=[]
	for i in range (0,len(appskey)):
		appskeylist.append(appskey[i])

	nwkskey=bytearray.fromhex(key.NwkSKey)
	nwkskeylist=[]
	for i in range (0,len(nwkskey)):
		nwkskeylist.append(nwkskey[i])
	
	srcHex = "%0.8X" % src
	srcArray=bytearray.fromhex(srcHex)
	srcaddr=[]

	for i in range (0,len(srcArray)):
		srcaddr.append(srcArray[len(srcArray)-1-i])

	#print srcaddr
	#print '[{}]'.format(', '.join(hex(x) for x in srcaddr))
	
	lorawan = LoRaWAN.new(appskey)
	
	lorawan.create(MHDR.UNCONF_DATA_UP, {'devaddr': srcaddr, 'data': list(map(ord, lorapktstr)) })
	
	lorawan.__init__(nwkskey)
	MIC=lorawan.compute_mic()	
	
	#print '[{}]'.format(', '.join(hex(x) for x in MIC))
	
	return MIC			
		
if __name__ == "__main__":
	
	argc=len(sys.argv)
	
	if argc>1:
		#we assume that the input frame is given in base64 format
		lorapktstr_b64=sys.argv[1]
	else:
		sys.exit("loraWAN.py needs at least a base64 encoded string argument")
	
	if argc>2:	
		pdata=sys.argv[2]
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
		
	if argc>3:	
		rdata=sys.argv[3]
		
	plain_payload="###BADMIC###"
	
	try:
		lorapktstr=base64.b64decode(lorapktstr_b64)
		lorapkt=[]
	
		for i in range (0,len(lorapktstr)):
			lorapkt.append(ord(lorapktstr[i]))
	
		plain_payload=loraWAN_process_pkt(lorapkt)		
		
	except TypeError:
		plain_payload="###BADMIC###"

	if plain_payload=="###BADMIC###":
		print '?'+plain_payload
	else:
		#the output is clear data		
		print "?plain payload is: "+plain_payload
		if argc>2:
			ptype = ptype & (~PKT_FLAG_DATA_ENCRYPTED)		
			print "^p%d,%d,%d,%d,%d,%d,%d" % (dst,ptype,src,seq,len(plain_payload),SNR,RSSI)
		if argc>3:
			print "^r"+rdata
		print "\xFF\xFE"+plain_payload

