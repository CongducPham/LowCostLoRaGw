# Copyright 2015 Congduc Pham, University of Pau, France.
# 
# Conduc.Pham@univ-pau.fr
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

import sys
import os
import datetime
import getopt

#gateway id/addr, just for giving a different name to various log files
_gwaddr=1

def main(argv):
	try:
		opts, args = getopt.getopt(argv,'a:',['addr'])
	except getopt.GetoptError:
		print 'logParseGateway -a'
		sys.exit(2)
	
	for opt, arg in opts:		
		if opt in ("-a", "--addr"):
			global _gwaddr
			_gwaddr = arg
			print "will use _"+str(_gwaddr)+" for post-processing log file"			

		
if __name__ == "__main__":
	main(sys.argv[1:])
	

_parselog_filename = "~/Dropbox/LoRa-test/post-processing_"+str(_gwaddr)+".log"	

the_line=sys.stdin.readline()
	
while the_line:
	
	f=open(os.path.expanduser(_parselog_filename),"a")	
	now = datetime.datetime.now()
	f.write(now.isoformat()+'> ')
	f.write(the_line)
	f.close()
	sys.stdout.write(the_line)
	the_line=sys.stdin.readline()	
