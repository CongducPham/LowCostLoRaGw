from loraWAN import loraWAN_process_pkt

#PKT='40 04 05 01 02 00 00 00 01 E4 85 FD 1F 77 91 B9 D9 34 FF 1F F9 A8 BC 3D 7E'
#pkt=bytearray.fromhex(PKT)
pkt='\x40\x04\x05\x01\x02\x00\x00\x00\x01\xE4\x85\xFD\x1F\x77\x91\xB9\xD9\x34\xFF\x1F\xF9\xA8\xBC\x3D\x7E'
lorapkt=[]
for i in range (0,len(pkt)):
	lorapkt.append(ord(pkt[i]))
	
plain_payload=loraWAN_process_pkt(lorapkt)

print "plain payload is : "+plain_payload