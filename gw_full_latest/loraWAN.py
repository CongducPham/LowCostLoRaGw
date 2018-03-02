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

import loraWAN_config
		
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

#to display non printable characters
replchars = re.compile(r'[\x00-\x1f]')

def replchars_to_hex(match):
	return r'\x{0:02x}'.format(ord(match.group()))
	
def loraWAN_process_pkt(lorapkt):

	#print "entering loraWAN_process_pkt"
	
	if True:
	#if import_LoRaWAN_lib()==True:

		#print "start decryption"
		
		appskey=bytearray.fromhex(loraWAN_config.AppSKey)
		appskeylist=[]

		for i in range (0,len(appskey)):
			appskeylist.append(appskey[i])

		nwkskey=bytearray.fromhex(loraWAN_config.NwkSKey)
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
		
def loraWAN_get_MIC(device, lorapktstr):

	appskey=bytearray.fromhex(loraWAN_config.AppSKey)
	appskeylist=[]
	for i in range (0,len(appskey)):
		appskeylist.append(appskey[i])

	nwkskey=bytearray.fromhex(loraWAN_config.NwkSKey)
	nwkskeylist=[]
	for i in range (0,len(nwkskey)):
		nwkskeylist.append(nwkskey[i])
	
	deviceHex = "%0.8X" % device
	deviceArray=bytearray.fromhex(deviceHex)
	devaddr=[]

	for i in range (0,len(deviceArray)):
		devaddr.append(deviceArray[len(deviceArray)-1-i])

	#print devaddr
	#print '[{}]'.format(', '.join(hex(x) for x in devaddr))
	
	lorawan = LoRaWAN.new(appskey)
	
	lorawan.create(MHDR.UNCONF_DATA_UP, {'devaddr': devaddr, 'data': list(map(ord, lorapktstr)) })
	
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
		ptype=PKT_TYPE_DATA			
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
	
	lorapktstr=base64.b64decode(lorapktstr_b64)
	
	lorapkt=[]
	
	for i in range (0,len(lorapktstr)):
		lorapkt.append(ord(lorapktstr[i]))
	
	plain_payload=loraWAN_process_pkt(lorapkt)

	if plain_payload=="###BADMIC###":
		print '?'+plain_payload
	else:	
		print "?plain payload is : "+plain_payload
		if argc>2:
			print "^p%d,%d,%d,%d,%d,%d,%d" % (dst,ptype,src,seq,len(plain_payload),SNR,RSSI)
		if argc>3:
			print "^r"+rdata
		print "\xFF\xFE"+plain_payload

		
	
			