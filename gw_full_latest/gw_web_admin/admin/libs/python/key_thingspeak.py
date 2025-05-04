import json
import sys

sys.path.insert(0, '/home/pi/lora_gateway')
import key_ThingSpeak
import key_NoInternet
import key_GpsFile
import key_MQTT
import key_NodeRed
import key_TTN
import key_ChirpStack

key_data = {'thingspeak_channel_key':key_ThingSpeak._def_thingspeak_channel_key,
		'thingspeak_source_list':key_ThingSpeak.source_list,
		'mqtt_server':key_MQTT.MQTT_server,
		'mqtt_project_name':key_MQTT.project_name,
		'mqtt_organization_name':key_MQTT.organization_name,
		'mqtt_sensor_name':key_MQTT.sensor_name,
		'mqtt_source_list':key_MQTT.source_list,
		'gpsfile_active_interval_minute':key_GpsFile.active_interval_minutes,
		'gpsfile_sms':key_GpsFile.SMS,
		'gpsfile_pin':key_GpsFile.PIN,
		'gpsfile_contacts':key_GpsFile.contacts,
		'gpsfile_project_name':key_GpsFile.project_name,
		'gpsfile_organization_name':key_GpsFile.organization_name,
		'gpsfile_sensor_name':key_GpsFile.sensor_name,
		'gpsfile_source_list':key_GpsFile.source_list,
		'nodered_project_name':key_NodeRed.project_name,
		'nodered_organization_name':key_NodeRed.organization_name,
		'nodered_sensor_name':key_NodeRed.sensor_name,
		'nodered_source_list':key_NodeRed.source_list,
		'nointernet_source_list':key_NoInternet.source_list,
		#'ttn_lorawan_server':key_TTN.lorawan_server,
		#'ttn_port':key_TTN.lorawan_port,		
		'ttn_source_list':key_TTN.source_list,
		'chirpstack_lorawan_server':key_ChirpStack.lorawan_server,
		'chirpstack_source_list':key_ChirpStack.source_list
		}
print json.dumps(key_data)
