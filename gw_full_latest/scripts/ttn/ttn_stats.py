import errno
import binascii
import json
import os
import socket
import time
import datetime
import sys
	
SERVER = 'router.eu.thethings.network'
PORT = 1700

PROTOCOL_VERSION = 1

PUSH_DATA = 0
PUSH_ACK = 1
PULL_DATA = 2
PULL_ACK = 4
PULL_RESP = 3

STAT_PK = {
    'stat': {
        'time': '',
        'lati': 0,
        'long': 0,
        'alti': 0,
        'rxnb': 0,
        'rxok': 0,
        'rxfw': 0,
        'ackr': 100.0,
        'dwnb': 0,
        'txnb': 0
    }
}

class TTN_stats:

    def __init__(self, id, lati, long, rxnb, rxok, rxfw, ackr, dwnb, txnb, server, port):
        self.id = id
        self.lati = lati  
        self.long = long        
        self.rxnb = rxnb  
        self.rxok = rxok
        self.rxfw = rxfw
        self.ackr = ackr
        self.dwnb = dwnb
        self.txnb = txnb           
        self.server = server
        self.port = port
        self.server_ip = None

        self.sock = None

    def start(self):

        self._log('ttn_stats: gw id {}', self.id)

        # get the server IP and create an UDP socket
        self.server_ip = socket.getaddrinfo(self.server, self.port)[0][-1]
        self._log('ttn_stats: Opening UDP socket to {} ({}) port {}...', self.server, self.server_ip[0], self.server_ip[1])
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.setblocking(False)

        # push the first time immediatelly
        self._push_data(self._make_stat_packet())

    def _make_stat_packet(self):
        now = time.strftime('%Y-%m-%d %H:%M:%S GMT')
        STAT_PK["stat"]["time"] = now
        if self.lati!="undef":        
        	STAT_PK["stat"]["lati"] = self.lati
        if self.long!="undef":
        	STAT_PK["stat"]["long"] = self.long     	   
        STAT_PK["stat"]["rxnb"] = self.rxnb
        STAT_PK["stat"]["rxok"] = self.rxok
        STAT_PK["stat"]["rxfw"] = self.rxfw
        STAT_PK["stat"]["ackr"] = self.ackr
        STAT_PK["stat"]["dwnb"] = self.dwnb
        STAT_PK["stat"]["txnb"] = self.txnb
        return json.dumps(STAT_PK)

    def _push_data(self, data):
        token = os.urandom(2)
        packet = bytearray([PROTOCOL_VERSION]) + token + bytearray([PUSH_DATA]) + binascii.unhexlify(self.id) + data
        print ''.join('{:02x}'.format(x) for x in packet)
        self._log('ttn_stats: Try to send UDP packet: {}', packet)
        try:
	        self.sock.sendto(packet, self.server_ip)
	        self.sock.close()
        except Exception as ex:
	        self._log('ttn_stats: Failed to push uplink packet to server: {}', ex)

    def _log(self, message, *args):
		print('{}'.format(str(message).format(*args)))

# python ttn-stats.py "0,0,0,0,0,0" "0.0,0.0" "B827EBFFFFD1B236"
# replace "0,0,0,0,0,0" by "rxnb,rxok,rxfw,ackr,dwnb,txnb"
# replace "0.0,0.0" by "lati,long"

def main(stats_str, gps_str, gwid):

	arr = map(int,stats_str.split(','))
	rxnb=arr[0]
	rxok=arr[1]				
	rxfw=arr[2]
	ackr=arr[3]
	dwnb=arr[4]
	txnb=arr[5]
	
	try:
		arr = map(float,gps_str.split(','))
		lati=arr[0]
		long=arr[1]
	except ValueError:
		lati="undef"
		long="undef"		
	
	ttn = TTN_stats(
		id=gwid,
		lati=lati,
		long=long,		
		rxnb=rxnb,
		rxok=rxok,
		rxfw=rxfw,
		ackr=ackr,
		dwnb=dwnb,
		txnb=txnb,
		server=SERVER,
		port=PORT)

	ttn.start()
	            
if __name__ == "__main__":
	main(sys.argv[1], sys.argv[2], sys.argv[3])
	            
