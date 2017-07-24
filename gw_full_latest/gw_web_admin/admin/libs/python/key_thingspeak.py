import json
import sys

sys.path.insert(0, '/home/pi/lora_gateway')
import key_ThingSpeak

key_data = {'thingspeak_channel_key':key_ThingSpeak._def_thingspeak_channel_key}
print json.dumps(key_data)
