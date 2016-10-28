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
import base64
import LoRaWAN
from LoRaWAN.MHDR import MHDR

AppSKey = '2B7E151628AED2A6ABF7158809CF4F3C'
NwkSKey = '2B7E151628AED2A6ABF7158809CF4F3C'

PKT_TYPE_DATA=0x10

def loraWAN_process_pkt(lorapkt):

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
		print "?loraWAN: plain payload is "+plain_payload
		return plain_payload
	else:
		return "###BADMIC###"	
		
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

		
	
			