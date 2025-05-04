#-------------------------------------------------------------------------------
# Part of this Python script is taken from the Pycom NanoGateway
# https://github.com/pycom/pycom-libraries/tree/master/examples/lorawan-nano-gateway
#
# Adapted by Congduc.Pham@univ-pau.fr
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

import binascii
import json
import os
import socket
import time
import datetime
from dateutil import parser
import calendar
import sys

#don't generate pyc (no compilation of imported module) so change in key_* file can be done dynamically
sys.dont_write_bytecode = True

import key_ChirpStack as key_LoRaWAN
netserv='ChirpStack'

try:
	key_LoRaWAN.source_list
except AttributeError:
	key_LoRaWAN.source_list=[]

try:
	key_LoRaWAN.lorawan_server
except AttributeError:
	#do not use localhost as it may be converted into ::1 for in IPv6 notation
	key_LoRaWAN.lorawan_server="127.0.0.1"
	
try:
	key_LoRaWAN.lorawan_port
except AttributeError:
	key_LoRaWAN.lorawan_port=1700
			
PROTOCOL_VERSION = 2

PUSH_DATA = 0
PUSH_ACK = 1
PULL_DATA = 2
PULL_ACK = 4
PULL_RESP = 3

RX_PK = {
    'rxpk': [{
        'time': '',
        'tmst': 0,
        'chan': 0,
        'rfch': 0,
        'freq': 0,
        'stat': 1,
        'modu': 'LORA',
        'datr': '',
        'codr': '4/5',
        'rssi': 0,
        'lsnr': 0,
        'size': 0,
        'data': ''
    }]
}

TX_ACK_PK = {
    'txpk_ack': {
        'error': ''
    }
}


class LoRaWAN:

    def __init__(self, id, frequency, bw, sf, server, port):
        self.id = id
        self.frequency = frequency  
        self.sf = sf
        self.bw = bw     
        self.server = server
        self.port = port
        self.server_ip = None
        self.sock = None

    def start(self):

		self._log('Cloud%s: gw id: {}' % netserv, self.id)

        # get the server IP and create an UDP socket
		try:
			self.server_ip = socket.getaddrinfo(self.server, self.port)[0][-1]	
			self._log('Cloud%s: Opening UDP socket to {} ({}) port {}...' % netserv, self.server, self.server_ip[0], self.server_ip[1])
			self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
			self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
			self.sock.setblocking(False)
		except Exception as ex:
			self._log('Cloud%s: Failed to connect to server: {}' % netserv, ex)

    def _sf_bw_to_dr(self, sf, bw):
        dr = 'SF' + str(sf)
        if bw == 125:
            return dr + 'BW125'
        elif bw == 250:
            return dr + 'BW250'
        else:
            return dr + 'BW500'

    def rx_packet(self, ldata, datalen, tdata, tmst, rssi, snr):

        RX_PK["rxpk"][0]["time"] = tdata
        if tmst=='':
        	#in seconds, maybe we should put it in microsecond?
        	RX_PK["rxpk"][0]["tmst"] = calendar.timegm(time.gmtime())
        else:
        	RX_PK["rxpk"][0]["tmst"] = int(tmst)
        RX_PK["rxpk"][0]["freq"] = self.frequency
        RX_PK["rxpk"][0]["datr"] = self._sf_bw_to_dr(self.sf, self.bw)
        RX_PK["rxpk"][0]["rssi"] = rssi
        RX_PK["rxpk"][0]["lsnr"] = snr
        RX_PK["rxpk"][0]["data"] = ldata
        RX_PK["rxpk"][0]["size"] = datalen
        
        #packet = self._make_node_packet(rx_data, tdata, 0, self.sf, self.bw, rssi, snr)
        packet=json.dumps(RX_PK)
        self._push_data(packet)
        #self._log('Cloud%s: Received packet: {}' % netserv, packet)

    def _push_data(self, data):
        token = os.urandom(2)
        packet = bytearray([PROTOCOL_VERSION]) + token + bytearray([PUSH_DATA]) + binascii.unhexlify(self.id) + data
        #print ''.join('{:02x}'.format(x) for x in packet)
        #self._log('Cloud%s: Try to forward packet: {}' % netserv, packet)
        try:
	        self.sock.sendto(packet, self.server_ip)
	        self.sock.close()
        except Exception as ex:
	        self._log('Cloud%s: Failed to push uplink packet to server: {}' % netserv, ex)

    def _log(self, message, *args):
		print('{}'.format(str(message).format(*args)))


# Testing with pau_lorawan_testing/Pau_testing_device 0x26011721
#
# python CloudChirpStack.py "QCEXASYAAAABhCGE1L87NCDMk0jLa6hYXm0e+g==" "256,64,637605665,0,28,8,-45" "125,5,12,868100" "2019-03-25T18:46:00.528+01:00" "0000B827EBD1B236"
# or
# python CloudChirpStack.py "QCEXASYAAAABhCGE1L87NCDMk0jLa6hYXm0e+g==" "256,64,637605665,0,28,8,-45" "125,5,12,868100" "`date +%FT%T%z`" "0000B827EBD1B236"
#
# get the base64 encrypted data from `Arduino_LoRa_temp` sending "Hello from UPPA"
# 
# Hello from UPPA
# plain payload hex 
# 48 65 6C 6C 6F 20 66 72 6F 6D 20 55 50 50 41 
# Encrypting
# encrypted payload
# 84 21 84 D4 BF 3B 34 20 CC 93 48 CB 6B A8 58 
# calculate MIC with NwkSKey
# transmitted LoRaWAN-like packet:
# MHDR[1] | DevAddr[4] | FCtrl[1] | FCnt[2] | FPort[1] | EncryptedPayload | MIC[4]
# 40 21 17 01 26 00 00 00 01 84 21 84 D4 BF 3B 34 20 CC 93 48 CB 6B A8 58 5E 6D 1E FA 
# [base64 LoRaWAN HEADER+CIPHER+MIC]:QCEXASYAAAABhCGE1L87NCDMk0jLa6hYXm0e+g==

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

	tmst=tdata.count('*')
	
	if (tmst != 0):
		tdata_tmp=tdata.split('*')[0]
		tmst=tdata.split('*')[1]
		tdata=tdata_tmp
	else:
		tmst=''
					
	#from 2019-05-14T14:53:10.241191+02:00 (similar to command date +%FT%T.%6N%z)
	#to 2019-05-14T14:53:10.241191Z (similar to command date +%FT%T.%6NZ)
	dt = parser.parse(tdata)
	#in case you want to remove microsecond 
	#tdata = dt.replace(microsecond=0,tzinfo=None).isoformat()+"Z"	
	tdata = dt.replace(tzinfo=None).isoformat()+"Z"
	
	arr = map(int,rdata.split(','))
	rbw=arr[0]
	rcr=arr[1]				
	rsf=arr[2]
	rfq=arr[3]/1000.0
	
	#LoRaWAN packet
	if dst==256:
		src_str="0x%0.8X" % src
		#we force to BW125 as Chirpstack is can not handle other bandwidth right now, for instance those of Lora 2.4GHz
		#TODO: change when ChirpStack will support LoRa 2.4GHz
		rbw=125
	else:
		src_str=str(src)	

	if (src_str in key_LoRaWAN.source_list) or (len(key_LoRaWAN.source_list)==0):

		#build the cs_gwid which is defined to be gwid[4:10]+"FFFF"+gwid[10:]
		#gwid is normally defined as eth0 MAC address filled by 0 in front: 0000B827EBD1B236
		cs_gwid=gwid[4:10]+"FFFF"+gwid[10:]
		
		cs = LoRaWAN(
			id=cs_gwid,
			frequency=rfq,
			bw=rbw,
			sf=rsf,
			server=key_LoRaWAN.lorawan_server,
			port=key_LoRaWAN.lorawan_port)

		cs.start()
		
		cs.rx_packet(ldata, datalen, tdata, tmst, RSSI, SNR)
			
	else:
		print "Source is not is source list, not sending to %s"  % netserv	
		            
if __name__ == "__main__":
	main(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])
	            
