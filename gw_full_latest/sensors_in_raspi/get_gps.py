#-------------------------------------------------------------------------------
# Copyright 2018 Congduc Pham, University of Pau, France.
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

import os
import sys
import serial

if len(sys.argv) < 2:
    thePort = '/dev/ttyACM0'
else:
    thePort = sys.argv[1]

if len(sys.argv) < 3:
    theBaud = 9600
else:
    theBaud = sys.argv[2]

try:    
	ser = serial.Serial(thePort, theBaud, timeout=0)
except SerialException:
	print 'get_gps.py: invalid serial port'
	sys.exit() 

# flush everything that may have been received on the port to make sure that we start with a clean serial input
ser.flushInput()

nofix=True
ntry=0
nfixtry=0

gw_gps_file='/home/pi/lora_gateway/gateway_gps.txt'

try:
	print 'get_gps.py: removing '+gw_gps_file+' file'
	os.remove(os.path.expanduser(gw_gps_file))
except OSError:
	print 'get_gps.py: no '+gw_gps_file+' file'

while nofix:

	GPGGA=ser.readline()
	gpgga=GPGGA.split(',')
	
	if gpgga[0]=='$GPGGA' or gpgga[0]=='$GNGGA':
	
		ntry=ntry+1
		
		lat_str=gpgga[2]
		ns=gpgga[3]
		lgt_str=gpgga[4]
		ew=gpgga[5]
		
		if lat_str!='' or lgt_str!='':
			nfixtry=nfixtry+1
			lat=float(lat_str)
			lgt=float(lgt_str)
			
			lat_d=int(lat/100)
			lgt_d=int(lgt/100)
			
			lat_m=(lat-lat_d*100)/60
			lgt_m=(lgt-lgt_d*100)/60
			
			lat=lat_d+lat_m
			lgt=lgt_d+lgt_m
			
			if ns=='S':
				lat=lat * -1.0
			
			if ew=='W':
				lgt=lgt * -1.0	
			
			gps_pos='%.5f,%.5f' % (lat,lgt)
			print gps_pos
			
			if nfixtry==3:
				nofix=False
				print 'get_gps.py: saving gps coordinates in '+gw_gps_file+' file'
				f=open(os.path.expanduser(gw_gps_file),'w')
				f.write(gps_pos+'\n')
				f.close()
				sys.stdout.flush()
		else:
			print 'get_gps.py: no fix yet'
			
			if ntry==10:
				nofix=False
				sys.stdout.flush()	

	
    

