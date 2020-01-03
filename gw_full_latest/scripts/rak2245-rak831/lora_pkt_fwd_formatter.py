#-------------------------------------------------------------------------------
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
#-------------------------------------------------------------------------------
#
# May 27th, 2019. First draft
# Last update: June 5th, 2019 
# Congduc.Pham@univ-pau.fr

#this script will translate the output from lora_pkt_fwd into the format of post_processing_gw.py
#
# EX:
# JSON up: {"rxpk":[{"tmst":29641444,"time":"2019-05-27T12:42:40.117758Z","tmms":1242996179117,"chan":0,"rfch":1,"freq":868.100000,"stat":1,"modu":"LORA","datr":"SF12BW125","codr":"4/5","lsnr":10.0,"rssi":-61,"size":23,"data":"QCQfBCaAAAABJD5j3dEiB1B9aI//04Y="}]}
#
#cat lora_pkt_logger_output.txt | python lora_pkt_fwd_formatter.py
#
#^p0,0,0,0,23,10,-61
#^r125,5,12,868100
#^t2019-05-27T12:42:40.117758
#FFFE then payload (40241f042680000001243e63ddd12207507d688fffd386)

import sys
import binascii
import json
import os

#------------------------------------------------------------
#low-level data prefix
#------------------------------------------------------------

LL_PREFIX_1='\xFF'
LL_PREFIX_LORA='\xFE'

#just to keep track: local seq number, will get override by post_processing_gw.py
seq=0

while 1:

	sys.stdout.flush()
	l=sys.stdin.readline()
	
	if l:
	
		if l!='\n':
			sys.stdout.flush()
			l=l.replace('\n','')
			ls=l.split(':')
			#print ls
			
			#['JSON up', ' {"rxpk"', '[{"tmst"', '29641444,"time"', '"2019-05-27T12', '42', '40.117758Z","tmms"', '1242996179117,"chan"', '0,"rfch"', '1,"freq"', '868.100000,"stat"', '1,"modu"', '"LORA","datr"', '"SF12BW125","codr"', '"4/5","lsnr"', '10.0,"rssi"', '-61,"size"', '23,"data"', '"QCQfBCaAAAABJD5j3dEiB1B9aI//04Y="}]}']


			if ls[0]=='INFO' and "concentrator started" in ls[1]:
				print l

			if ls[0]=='INFO' and "concentrator stopped" in ls[1]:
				print l			
				
			if ls[0]=='INFO' and "PULL_RESP received" in ls[1]:
				print l
				
			if ls[0]=='INFO' and "tx_start_delay" in ls[1]:
				print l				
								
			if ls[0]=='ERROR':
				print l				

			if ls[0]=='JSON down':
				print l

			sys.stdout.flush()
			
			#here we parse the rxpk json line
			if ls[0]=='JSON up' and "rxpk" in ls[1]:
				print l
				
				try:
					l_json=json.loads(l[9:])
										
					#print "LORA PACKET ->> ",
					#print l_json
				
					rxpk_array=l_json['rxpk']
				
					for rxpk in rxpk_array: 
			
						print "^p%d,%d,%d,%d,%d,%d,%d" % (0,0,0,seq,int(rxpk['size']),int(float(rxpk['lsnr'])),int(rxpk['rssi']))
				
						seq=seq+1
			
						bw=rxpk['datr'].split('W')[1]
						cr=rxpk['codr'].split('/')[1]
						sf=rxpk['datr'].split('B')[0].split('F')[1]
					
						print "^r%d,%d,%d,%d" % (int(bw), int(cr), int(sf), int(rxpk['freq']*1000))
	
						print "^t%s*%s" % (rxpk['time'].replace('Z',''), rxpk['tmst'])
				
						#print ls[payload]
					
						#convert the hex string into bytes array
						lsbytes=bytearray.fromhex(binascii.b2a_hex(binascii.a2b_base64(rxpk['data'])))

						#print prefix
						sys.stdout.write(LL_PREFIX_1)
						sys.stdout.write(LL_PREFIX_LORA)
				
						#print each character
						#for i in range(0, len(lsbytes)):
						#	sys.stdout.write(lsbytes[i])
				
						sys.stdout.write(lsbytes)
						sys.stdout.write('\n')
						sys.stdout.flush()

				except ValueError:
					pass
					
			#here we parse the stat json line to get the GPS position given by lora_pkt_fwd
			#we use the GPS coordinates and replace them in our gateway_conf.json file			
			if ls[0]=='JSON up' and "stat" in ls[1]:
				#print l
				
				try:
					l_json=json.loads(l[9:])
				
					try:
						lati=float(l_json['stat']['lati'])
					except KeyError:
						lati='undef'
					
					try:		
						long=float(l_json['stat']['long'])
					except KeyError:
						long='undef'
				
					if lati!='undef' and long!='undef':
									
						gps_pos='lora_pkt_fwd_formatter: GPS(%.5f,%.5f)' % (lati,long)
						print gps_pos
		
						cmd="""sed -i -- 's/"ref_latitude.*,/"ref_latitude" : """+'"'+str(lati)+'"'+""",/g' /home/pi/lora_gateway/gateway_conf.json"""

						try:
							#print 'lora_pkt_fwd_formatter: replacing GPS latitude in gateway_conf.json'
							os.system(cmd)
						except:
							#print 'lora_pkt_fwd_formatter: Error when replacing GPS latitude in gateway_conf.json'
							pass
					
						cmd="""sed -i -- 's/"ref_longitude.*,/"ref_longitude" : """+'"'+str(long)+'"'+""",/g' /home/pi/lora_gateway/gateway_conf.json"""

						try:
							#print 'lora_pkt_fwd_formatter: replacing GPS longitude in gateway_conf.json'
							os.system(cmd)
						except:
							#print 'lora_pkt_fwd_formatter: Error when replacing GPS longitude in gateway_conf.json'			
							pass
							
				except ValueError:
					pass
