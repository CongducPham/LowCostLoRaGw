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

import errno
import binascii
import json
import os
import socket
import time
import datetime
import sys
import thread
from collections import OrderedDict

#------------------------------------------------------------
#open gateway_conf.json file 
#------------------------------------------------------------

gwconf_filename = "/home/pi/lora_gateway/gateway_conf.json"

f = open(os.path.expanduser(gwconf_filename),"r")
lines = f.readlines()
f.close()
array = ""
#get all the lines in a string
for line in lines :
	array += line

#change it into a python array
gw_json_array = json.loads(array)

#you can use the gw id in case you need it
_gwid = gw_json_array["gateway_conf"]["gateway_ID"]

try:
	lorawan_server = gw_json_array["gateway_conf"]["downlink_network_server"]
except KeyError:
	lorawan_server="127.0.0.1"
	#lorawan_server = "eu1.cloud.thethings.network"

lorawan_port=1700

_downlink_file = "/home/pi/lora_gateway/downlink/downlink.txt"

PROTOCOL_VERSION = 2

PUSH_DATA = 0
PUSH_ACK = 1
PULL_DATA = 2
PULL_ACK = 4
PULL_RESP = 3

TX_ERR_NONE = 'NONE'
TX_ERR_TOO_LATE = 'TOO_LATE'
TX_ERR_TOO_EARLY = 'TOO_EARLY'
TX_ERR_COLLISION_PACKET = 'COLLISION_PACKET'
TX_ERR_COLLISION_BEACON = 'COLLISION_BEACON'
TX_ERR_TX_FREQ = 'TX_FREQ'
TX_ERR_TX_POWER = 'TX_POWER'
TX_ERR_GPS_UNLOCKED = 'GPS_UNLOCKED'

UDP_THREAD_CYCLE_MS = 100/1000.0

TX_ACK_PK = {
    'txpk_ack': {
        'error': ''
    }
}

TX_PK = {
    'status': 'send_request',
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
}
	
class lorawan_pull:

    def __init__(self, id, server, port):
        self.id = id
        self.server = server
        self.port = port
        self.server_ip = None

        self.dwnb = 0
        self.sock = None
        self.udp_stop = False
        self.udp_lock = thread.allocate_lock()

    def start(self):

        self._log('lorawan_pull: gw id {}', self.id)

        # get the server IP and create an UDP socket
        self.server_ip = socket.getaddrinfo(self.server, self.port)[0][-1]
        self._log('lorawan_pull: Opening UDP socket to {} ({}) port {}...', self.server, self.server_ip[0], self.server_ip[1])
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.setblocking(False)

        # send pull_data the first time immediatelly
        self.pull_data()

        # start the UDP receive thread
        self.udp_stop = False
         
        thread.start_new_thread(self._udp_thread, ())

    def stop(self):

        self._log('lorawan_pull: Stopping...')

        # signal the UDP thread to stop
        self.udp_stop = True
        while self.udp_stop:
            #50ms
        	time.sleep(50/1000.0)       

    def pull_data(self):
        self._log('lorawan_pull: PULL_DATA')
        token = os.urandom(2)
        packet = bytearray([PROTOCOL_VERSION]) + token + bytearray([PULL_DATA]) + binascii.unhexlify(self.id)
        with self.udp_lock:
            try:
                self.sock.sendto(packet, self.server_ip)
            except Exception as ex:
                self._log('lorawan_pull: Failed to pull downlink packets from server: {}', ex)
        self._log('lorawan_pull: PULL_DATA done')

    def _ack_pull_rsp(self, token, error):
        TX_ACK_PK["txpk_ack"]["error"] = error
        resp = json.dumps(TX_ACK_PK)
        packet = bytearray([PROTOCOL_VERSION]) + token + bytearray([PULL_ACK]) + binascii.unhexlify(self.id) + resp
        with self.udp_lock:
            try:
                self.sock.sendto(packet, self.server_ip)
            except Exception as ex:
                self._log('lorawan_pull: PULL RSP ACK exception: {}', ex)
                
    def _create_down_link(self, pk):
        
        #{txpk:{"imme":false,"rfch":0,"powe":14,"ant":0,"brd":0,"tmst":1580262735,"freq":868.1,"modu":"LORA",
        #	"datr":"SF12BW125","codr":"4/5","ipol":true,"size":19,"data":"YCEXASYHHAAFANKthAgBd02sPg=="}}
        
        
        #we want to keep the same order
        #tx_pk = json.loads(pk,object_pairs_hook=OrderedDict)        
        #insert "status":"send_request" at front
        #c=OrderedDict({"status":"send_request"})
        #both=OrderedDict(list(c.items()) + list(tx_pk['txpk'].items()))
        
        f = open(os.path.expanduser(_downlink_file),"w")
        #write to downlink.txt file
        #json.dump(both,f)
        f.write(pk+'\n')
        f.close()
        #self._log(
        #    'lorawan_pull: Write {} to {} {}', json.dumps(both), _downlink_file, datetime.datetime.now().isoformat())
        self._log(
            'lorawan_pull: Write {} to {} {}', pk, _downlink_file, datetime.datetime.now().isoformat())             
               
    def _udp_thread(self):

        while not self.udp_stop:
            try:
                data = self.sock.recv(1024)
                _token = data[1:3]
                _type = ord(data[3])
                if _type == PUSH_ACK:
                    self._log("lorawan_pull: PUSH ACK")
                elif _type == PULL_ACK:
                    self._log("lorawan_pull: PULL ACK")
                elif _type == PULL_RESP:
                    self._log("lorawan_pull: PULL RESP")
                    self.dwnb += 1
                    ack_error = TX_ERR_NONE
                    tx_pk_str=data[4:]
                    #print tx_pk_str
                    if "tmst" in data:
                            self._create_down_link(tx_pk_str) 
                    self._ack_pull_rsp(_token, ack_error)
                else:
                    self._log("lorawan_pull, udp_thread: type: {}", _type)
            except socket.timeout:
                pass
            except socket.error, e:
                err = e.args[0]
                if err == errno.EAGAIN or err == errno.EWOULDBLOCK:
                    pass
            except OSError as ex:
                if ex.errno != errno.EAGAIN:
                    self._log('lorawan_pull: UDP recv OSError Exception: {}', ex)
            except Exception as ex:
                #pass
                self._log('lorawan_pull: UDP recv Exception: {}', ex)

            # wait before trying to receive again
            time.sleep(UDP_THREAD_CYCLE_MS)

        # we are to close the socket
        self.sock.close()
        self.udp_stop = False
        self._log('UDP thread stopped')

    def _log(self, message, *args):
        print('{}'.format(str(message).format(*args)))

def main(gwid):     
		    
    lp = lorawan_pull(
        id=gwid,
        server=lorawan_server,
        port=lorawan_port)

    lp.start()
    
    while True:
        time.sleep(15)
        lp.pull_data()

if __name__ == "__main__":

	argc=len(sys.argv)
	
	if argc>1:
		#a gateway id is given in parameter
		main(sys.argv[1])
	else:
		#use the default gateway id procedure
		main(_gwid[4:10]+"FFFF"+_gwid[10:])
