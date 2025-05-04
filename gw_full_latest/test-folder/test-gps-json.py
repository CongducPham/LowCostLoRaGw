import json
import codecs
import os
import code
import datetime
from dateutil import parser

# when gw receive GPS message from a device, it will update/add info from that device into the existing list of devices: gps.json
# then, it will update the "active" field for devices within a given time windows
# then, it will extract those that active in active_gps.json

now=parser.parse('2017-12-31T17:25:05')

#------------------------------------------------------------
# Open gps.json file 
#------------------------------------------------------------

#name of json file containing the gps devices
gps_json_filename = "gps.json"

#open json file to retrieve active devices
f = open(os.path.expanduser(gps_json_filename),"r")
string = f.read()
f.close()

#change it into a python array
gps_json_array = json.loads(string)

#retrieving all gps devices declarations
gps_devices = gps_json_array["devices"]

src='waziup_UPPA_Sensor15'

found_device=False

distance='0.1234'

for device in gps_devices:
	if src==device['src']:
		found_device=True
		device['distance']=distance

new_device={}

new_device["src"]=u'waziup_UPPA_Sensor18'
new_device["snr"]=0
new_device["rssi"]=0
new_device["seq"]=0
new_device["bc"]=0
new_device["time"]=u'2017-12-31T17:25:05'
new_device["gw"]=u'000000XXXXXXXXXX'
new_device["fxt"]=0
new_device["lat"]=0.0
new_device["lgt"]=0.0
new_device["distance"]=0.5678
new_device["active"]='yes'

gps_devices.append(new_device)

#localdt = datetime.datetime.now()
#tnow = localdt.replace(microsecond=0).isoformat()

#code.interact(local=locals())

new_gps_json_array = json.loads('{"devices":[]}')
new_gps_devices = new_gps_json_array["devices"]

for device in gps_devices:
	t1=parser.parse(device['time'])
	diff=now-t1
	
	if diff.total_seconds() < 11*60:
		device['active']=u'yes'
		new_gps_devices.append(device)
	else:
		device['active']=u'no'	

with open('gps.json', 'w') as f:
    json.dump(gps_json_array, codecs.getwriter('utf-8')(f), ensure_ascii=False, indent=4)
    		
with open('active_gps.json', 'w') as f:
    json.dump(new_gps_json_array, codecs.getwriter('utf-8')(f), ensure_ascii=False, indent=4)		
    
    
    
    		