# file: rfcomm-server.py
# auth: Albert Huang <albert@csail.mit.edu>
# desc: simple demonstration of a server application that uses RFCOMM sockets
#
# $Id: rfcomm-server.py 518 2007-08-10 07:20:07Z albert $

# adapted by N. Bertuol and C. Pham
# Congduc.Pham@univ-pau.fr
# University of Pau, France
# 

from bluetooth import *
from MongoDB import get_documents, get_nodes_names
import datetime
import signal
import sys

server_sock=BluetoothSocket( RFCOMM )
server_sock.bind(("",PORT_ANY))
server_sock.listen(1)

port = server_sock.getsockname()[1]

uuid = "94f39d29-7d6d-437d-973b-fba39e49d4ee"

advertise_service( server_sock, "SampleServer",
                   service_id = uuid,
                   service_classes = [ uuid, SERIAL_PORT_CLASS ],
                   profiles = [ SERIAL_PORT_PROFILE ], 
#                   protocols = [ OBEX_UUID ] 
                    )

def signal_handler(signal, frame):
	print('You pressed Ctrl+C or killed the process!')
	print("disconnected")
	#client_sock.close()
	server_sock.close()
	print "Bye."
	sys.exit(0)

if (len(sys.argv)==2 and sys.argv[1]=="-bg"):
	# if we run in background, we do not want to catch the CTRL-C signal, only the SIGTERM from kill -9 
	signal.signal(signal.SIGTERM, signal_handler)
else:	
	signal.signal(signal.SIGINT, signal_handler)
						
while True:          
	print "Waiting for connection on RFCOMM channel %d" % port
	sys.stdout.flush()
	
	client_sock, client_info = server_sock.accept()
	print "Accepted connection from ", client_info
	sys.stdout.flush()
	
	try:
		data = client_sock.recv(1024)
		if len(data) == 0: break
		print "received [%s]" % data

		#send data in a string (date and data only)
		if data.startswith('data'):
			#data = 'data$nodes$dates'
			#getting nodes names and dates(begin and end)
			preferences = data.split('$')
			
			nodeArray = None
			beginDate = None
			endDate = None
			
			#check if nodes and dates were set
			if(not preferences[1] == ''):
				nodeArray = preferences[1].split(';') #array of nodes names
			if(not preferences[2] == ''):
				dateArray = preferences[2].split('-') #array of strings representing beginDate and endDate
				
				#beginDate is the first date retrieved from preferences
				if(dateArray[0].startswith('begin_')):
					auxDate = dateArray[0].split('_') #retrieving {'begin',day,month-1,year}
					beginDate = datetime.datetime(int(auxDate[3]), int(auxDate[2])+1, int(auxDate[1]), 0, 0, 0, 0)
					
					auxDate = dateArray[1].split('_') #retrieving {'end',day,month-1,year}
					endDate = datetime.datetime(int(auxDate[3]), int(auxDate[2])+1, int(auxDate[1]), 23, 59, 59, 999999)
					
				#endDate is the first date retrieved from preferences
				else:
					auxDate = dateArray[1].split('_') #retrieving {'begin',day,month-1,year}
					beginDate = datetime.datetime(int(auxDate[3]), int(auxDate[2])+1, int(auxDate[1]), 0, 0, 0, 0)
					
					auxDate = dateArray[0].split('_') #retrieving {'end',day,month-1,year}
					endDate = datetime.datetime(int(auxDate[3]), int(auxDate[2])+1, int(auxDate[1]), 23, 59, 59, 999999)
			
			#reinitialize data
			data = ''
			
			#check if no node is checked, send an error message
			if((nodeArray is None) and (not beginDate is None) and (not endDate is None)):
				data = 'ERROR : no node is selected, please select at least one node in the preferences page.!'
				print('ERROR: no node is checked')
				
			#check if beginDate <= endDate
			elif((not beginDate is None) and (not endDate is None) and (beginDate > endDate)):
				data = 'ERROR : the begin date ('+beginDate.strftime("%d-%m-%Y")+') you have selected in the preferences page has passed after the selected end date ('+endDate.strftime("%d-%m-%Y")+'). Please, you must change your choosen dates in the preferences page.!'
				print('ERROR : beginDate > endDate')
				
			#check if good preferences are set (all preferences set, no preferences set or only nodes preferences set)
			else:
				documents = get_documents(nodeArray, beginDate, endDate)
				
				for doc in documents:
					data += 'NODE: '+str(doc['node_eui'])+' DATE: '+str(doc['time'])+' DATA: '+str(doc['data'])+'\n'
				data += '!'
				
				print("sending data")
			sys.stdout.flush()	
			
		#send data which will be saved in a csv file (values separated by ";")
		elif data.startswith('file'):
			#data = 'data$nodes$dates'
			#getting nodes names and dates(begin and end)
			preferences = data.split('$')
			
			nodeArray = None
			beginDate = None
			endDate = None
			
			#check if nodes and dates were set
			if(not preferences[1] == ''):
				nodeArray = preferences[1].split(';') #array of nodes names
			if(not preferences[2] == ''):
				dateArray = preferences[2].split('-') #array of strings representing beginDate and endDate
				
				#beginDate is the first date retrieved from preferences
				if(dateArray[0].startswith('begin_')):
					auxDate = dateArray[0].split('_') #retrieving {'begin',day,month-1,year}
					beginDate = datetime.datetime(int(auxDate[3]), int(auxDate[2])+1, int(auxDate[1]), 0, 0, 0, 0)
					
					auxDate = dateArray[1].split('_') #retrieving {'end',day,month-1,year}
					endDate = datetime.datetime(int(auxDate[3]), int(auxDate[2])+1, int(auxDate[1]), 23, 59, 59, 999999)
					
				#endDate is the first date retrieved from preferences
				else:
					auxDate = dateArray[1].split('_') #retrieving {'begin',day,month-1,year}
					beginDate = datetime.datetime(int(auxDate[3]), int(auxDate[2])+1, int(auxDate[1]), 0, 0, 0, 0)
					
					auxDate = dateArray[0].split('_') #retrieving {'end',day,month-1,year}
					endDate = datetime.datetime(int(auxDate[3]), int(auxDate[2])+1, int(auxDate[1]), 23, 59, 59, 999999)
			
			#reinitialize data
			data = ''
			
			#check if no node is checked, send an error message
			if((nodeArray is None) and (not beginDate is None) and (not endDate is None)):
				data = 'ERROR : no node is selected, please select at least one node in the preferences page.!'
				print('ERROR: no node is checked')
				
			#check if beginDate <= endDate
			elif((not beginDate is None) and (not endDate is None) and (beginDate > endDate)):
				data = 'ERROR : the begin date ('+beginDate.strftime("%d-%m-%Y")+') you have selected in the preferences page has passed after the selected end date ('+endDate.strftime("%d-%m-%Y")+'). Please, you must change your choosen dates in the preferences page.!'
				print('ERROR : beginDate > endDate')
				
			#check if good preferences are set (all preferences set, no preferences set or only nodes preferences set)
			else:
				data = 'type;gateway_eui;node_eui;snr;rssi;cr;datarate;time;data\n'
				
				documents = get_documents(nodeArray, beginDate, endDate)
				
				for doc in documents:
					data += str(doc['type'])+';'
					data += str(doc["gateway_eui"])+";"
					data += str(doc["node_eui"])+";"
					data += str(doc["snr"])+";"
					data += str(doc["rssi"])+";"
					data += str(doc["cr"])+";"
					data += str(doc["datarate"])+";"
					data += str(doc["time"].isoformat())+";"
					data += str(doc["data"])
					data += "\n"
				data += '!'
			
			print("sending data to be in the csv file")
			
		#send node names in a string ("node1#datebegin_dateend;node2#datebegin_dateend;...")
		elif data == 'nodes':
			data = get_nodes_names()
			data += '!'
			
			print("sending nodes names")
			
		#send a message which explains that the entry is wrong
		else:
			data = 'ERROR : BAD_ENTRY, PLEASE USE THE APP!'
			print("bad entry")
		
		sys.stdout.flush()	
		client_sock.send(data)

	except IOError:
		pass

	except KeyboardInterrupt:

		print("disconnected")

		client_sock.close()
		server_sock.close()
		print "all done"

		break
		
	sys.stdout.flush()	
