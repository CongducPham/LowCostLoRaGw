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
# Apr 9th, 2019. First draft
# Congduc.Pham@univ-pau.fr

#this script will translate the output from util_pkt_logger into the format of post_processing_gw.py
#
# EX:
# "gateway ID","node MAC","UTC timestamp","us count","frequency","RF chain","RX chain","status","size","modulation","bandwidth","datarate","coderate","RSSI","SNR","payload"
# "B827EBFFFEDA09D7","","2019-04-08 14:09:33.445Z",  17482172, 868300000,1, 1,"CRC_OK",20,"LORA",125000,"SF12"  ,"4/5",-109,-10.8,"011009B0-2A2A2A2A-2A2A2A2A-202A2A2A-2A2A2A2A"
#
#cat util_pkt_logger_output.txt | python util_pkt_logger_formatter.py
#
#^p0,0,0,0,20,-10,-109
#^r125,5,12,868300
#^t2019-04-08T14:09:33.445
#FFFE then payload (011009B02A2A2A2A2A2A2A2A202A2A2A2A2A2A2A)

import sys

#------------------------------------------------------------
#low-level data prefix
#------------------------------------------------------------

LL_PREFIX_1='\xFF'
LL_PREFIX_LORA='\xFE'

#field index in csv line
gwid=0
nodemac=1
utc=2
uscount=3
freq=4
rfchain=5
rxchain=6
status=7
size=8
modulation=9
bw=10
dr=11
cr=12
rssi=13
snr=14
payload=15

#just to keep track: local seq number, will get override by post_processing_gw.py
seq=0

while 1:

	l=sys.stdin.readline()
	
	if l:
	
		if l!='\n':
			l=l.replace('\n','')
			ls=l.split(',')
			#print ls
			
			#['"B827EBFFFEDA09D7"', '""', '"2019-04-15 12:28:23.749Z"', '  16165684', ' 868100000', '1', ' 0', '"CRC_OK" ', ' 23', '"LORA"', '125000', '"SF12"  ', '"4/5"', '-31', '+11.2', '"40211701-26000000-019065BC-FBFF2963-7C94CE0F-DED9AA"']
			
			#remove unwanted character from util_pkt_logger output
			ls[gwid]=ls[gwid].replace('"','')
			ls[utc]=ls[utc].replace('"','')
			ls[utc]=ls[utc].replace(' ','T')
			ls[utc]=ls[utc].replace('Z','')
			ls[freq]=ls[freq].replace('"','')
			ls[freq]=ls[freq].replace(' ','')			
			ls[status]=ls[status].replace('"','')
			ls[status]=ls[status].replace(' ','')
			ls[modulation]=ls[modulation].replace('"','')
			ls[modulation]=ls[modulation].replace(' ','')
			ls[size]=ls[size].replace(' ','')		
			ls[dr]=ls[dr].replace('"','')
			ls[dr]=ls[dr].replace(' ','')
			ls[cr]=ls[cr].replace('"','')
			ls[cr]=ls[cr].replace(' ','')
			ls[rssi]=ls[rssi].replace(' ','')
			ls[snr]=ls[snr].replace(' ','')
			ls[payload]=ls[payload].replace('"','')
			ls[payload]=ls[payload].replace('-','')

			if ls[gwid]!='gateway ID' and ls[modulation]=='LORA' and ls[status]=='CRC_OK':	
				#print "LORA PACKET ->> ",
				#print l
			
				print "^p%d,%d,%d,%d,%d,%d,%d" % (0,0,0,seq,int(ls[size]),int(float(ls[snr])),int(ls[rssi]))
				
				seq=seq+1
			
				print "^r%d,%s,%s,%d" % (int(ls[bw])/1000, ls[cr][2], ls[dr][2:], int(ls[freq])/1000)
	
				print "^t%s" % ls[utc]
				
				#print ls[payload]
				
				#convert the hex string into bytes array				
				lsbytes=bytearray.fromhex(ls[payload])

				#print prefix
				sys.stdout.write(LL_PREFIX_1)
				sys.stdout.write(LL_PREFIX_LORA)
				
				#print each character
				#for i in range(0, len(lsbytes)):
				#	sys.stdout.write(lsbytes[i])
				
				sys.stdout.write(lsbytes)
				sys.stdout.write('\n')
				sys.stdout.flush()
	
