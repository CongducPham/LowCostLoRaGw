import json
import sys

sys.path.insert(0, '/home/pi/lora_gateway')
import key_Orion
import key_ThingSpeak

key_data = {'project_name':key_Orion.project_name, 
		'organization_name':key_Orion.organization_name, 
		'service_tree':key_Orion.service_tree,
		'orion_token':key_Orion.orion_token, 
		'thingspeak_channel_key':key_ThingSpeak._def_thingspeak_channel_key
		}
print json.dumps(key_data)
