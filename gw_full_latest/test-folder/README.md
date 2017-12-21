Python test scripts
===================

First, copy the test files in /home/pi/lora_gateway

Testing the complete post-processing stage
------------------------------------------

	> python test-fakePacket.py | python post_processing.gw.py

Directly testing a cloud script
-------------------------------

Recall that a message will be uploaded to cloud only if it is prefixed with `\!`. So assuming that you are sending `\!TC/22.5` when your script is launched, `post_processing_gw.py` provides 5 parameters. 

- ldata: the received data (without the prefix)
	- e.g. TC/22.5 as 1st argument (sys.argv[1] in python)
- pdata: packet information
	- e.g. "1,16,6,0,9,8,-45" as 2nd argument (sys.argv[2] in python)
	- interpreted as dst,ptype,src,seq,len,SNR,RSSI for the last received packet
- rdata: the LoRa radio information
	- e.g. "125,5,12" as 3rd argument (sys.argv[3] in python)
	- interpreted as bw,cr,sf for the last received packet
- tdata: the timestamp information
	- e.g. "2017-11-20T14:18:54+01:00" as 4th argument (sys.argv[4] in python)
- gwid: the gateway id
	- e.g. 00000027EBBEDA21 as 5th argument (sys.argv[5] in python)	
	
These parameters are passed to the script. It is up to the cloud script to use these parameters or not. For instance:

	> python CloudThingSpeak.py "TC/22.5" "1,16,6,0,9,8,-45" "125,5,12" "2017-11-20T14:18:54+01:00" "00000027EBBEDA21"

	> python CloudNodeRed.py "TC/22.5" "1,16,6,0,9,8,-45" "125,5,12" "2017-11-20T14:18:54+01:00" "00000027EBBEDA21"
	
Testing a GPS device:

	> python CloudGpsFile.py "BC/9/LAT/43.31402/LGT/-0.36370/FXT/4180" "1,16,6,0,9,8,-45" "125,5,12" "2017-11-20T14:18:54+01:00" "00000027EBBEDA21"	

