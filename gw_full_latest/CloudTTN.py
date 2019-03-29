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
import calendar
import sys

import key_TTN

try:
	key_TTN.source_list
except AttributeError:
	key_TTN.source_list=[]
	
SERVER = 'router.eu.thethings.network'
PORT = 1700

PROTOCOL_VERSION = 1

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


class TTN:

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

        self._log('CloudTTN: gw id: {}', self.id)

        # get the server IP and create an UDP socket
        self.server_ip = socket.getaddrinfo(self.server, self.port)[0][-1]
        self._log('CloudTTN: Opening UDP socket to {} ({}) port {}...', self.server, self.server_ip[0], self.server_ip[1])
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.setblocking(False)

    def _sf_bw_to_dr(self, sf, bw):
        dr = 'SF' + str(sf)
        if bw == 125:
            return dr + 'BW125'
        elif bw == 250:
            return dr + 'BW250'
        else:
            return dr + 'BW500'

    def rx_packet(self, ldata, datalen, tdata, rssi, snr):

        RX_PK["rxpk"][0]["time"] = tdata
        RX_PK["rxpk"][0]["tmst"] = calendar.timegm(time.gmtime())
        RX_PK["rxpk"][0]["freq"] = self.frequency
        RX_PK["rxpk"][0]["datr"] = self._sf_bw_to_dr(self.sf, self.bw)
        RX_PK["rxpk"][0]["rssi"] = rssi
        RX_PK["rxpk"][0]["lsnr"] = snr
        RX_PK["rxpk"][0]["data"] = ldata
        RX_PK["rxpk"][0]["size"] = datalen
        
        #packet = self._make_node_packet(rx_data, tdata, 0, self.sf, self.bw, rssi, snr)
        packet=json.dumps(RX_PK)
        self._push_data(packet)
        self._log('CloudTTN: Received packet: {}', packet)

    def _push_data(self, data):
        token = os.urandom(2)
        packet = bytearray([PROTOCOL_VERSION]) + token + bytearray([PUSH_DATA]) + binascii.unhexlify(self.id) + data
        print ''.join('{:02x}'.format(x) for x in packet)
        self._log('CloudTTN: Try to forward packet: {}', packet)
        try:
	        self.sock.sendto(packet, self.server_ip)
	        self.sock.close()
        except Exception as ex:
	        self._log('CloudTTN: Failed to push uplink packet to server: {}', ex)

    def _log(self, message, *args):
		print('{}'.format(str(message).format(*args)))

# Testing with pau_lorawan_testing/Pau_testing_device 0x26011721
#
# python CloudTTN.py "QCEXASYAAAABhCGE1L87NCDMk0jLa6hYXm0e+g==" "256,64,637605665,0,28,8,-45" "125,5,12,868100" "2019-03-25T18:46:00.528+01:00" "0000B827EBD1B236"
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
	
	arr = map(int,rdata.split(','))
	rbw=arr[0]
	rcr=arr[1]				
	rsf=arr[2]
	rfq=arr[3]/1000.0
	
	#LoRaWAN packet
	if dst==256:
		src_str="%0.8X" % src
	else:
		src_str=str(src)	

	if (src_str in key_TTN.source_list) or (len(key_TTN.source_list)==0):

		#build the ttn_gwid which is defined to be gwid[4:10]+"FFFF"+gwid[10:]
		#gwid is normally defined as eth0 MAC address filled by 0 in front: 0000B827EBD1B236
		ttn_gwid=gwid[4:10]+"FFFF"+gwid[10:]
		
		ttn = TTN(
			id=ttn_gwid,
			frequency=rfq,
			bw=rbw,
			sf=rsf,
			server=SERVER,
			port=PORT)

		ttn.start()
		
		ttn.rx_packet(ldata, datalen, tdata, RSSI, SNR)
			
	else:
		print "Source is not is source list, not sending to TTN"	
		            
if __name__ == "__main__":
	main(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])
	            
