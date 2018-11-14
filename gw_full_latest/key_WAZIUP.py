####################################################
#server: CAUTION must exist
waziup_server="http://api.waziup.io/api/v1"

#project name
project_name="waziup"

#your organization: CHANGE HERE
#choose one of the following: "DEF", "UPPA", "EGM", "IT21", "CREATENET", "CTIC", "UI", "ISPACE", "UGB", "WOELAB", "FARMERLINE", "C4A", "PUBD"
#organization_name="UPPA"
organization_name="ORG"

#service tree: CHANGE HERE at your convenience
#can be empty
#should start with a '-'
#service_tree='-TESTS'
service_tree=''

#sensor name: CHANGE HERE but maybe better to leave it as Sensor
#the final name will contain the sensor address
sensor_name="Sensor"

#service path: DO NOT CHANGE HERE
service_path=organization_name+service_tree

#SUMMARY
#
#the entity name will then be service_path+'_'+sensor_name+scr_addr, e.g. "UPPA-TESTS_Sensor2"
#

#use ONLY letters and numbers [A-Za-z0-9] for the username and the password
username="guest"
password="guest"

#here "private" or "public" for the managed sensors
visibility="public"

#Note how we can indicate a device source addr that are allowed to use the script
#Use decimal between 2-255 and use 4-byte hex format for LoRaWAN devAddr
#leave empty to allow all devices
#source_list=["3", "255", "01020304"]
source_list=[]

####################################################