####################################################

#project name
project_name="waziup"

#your organization: CHANGE HERE
#choose one of the following: "DEF", "UPPA", "EGM", "IT21", "CREATENET", "CTIC", "UI", "ISPACE", "UGB", "WOELAB", "FARMERLINE", "C4A", "PUBD"
organization_name="UPPA"
#organization_name="ORG"

#sensor name: CHANGE HERE but maybe better to leave it as Sensor
#the final name will contain the sensor address
sensor_name="Sensor"

# CloudNodeRed will built the source as waziup/UPPA/Sensor2 to easily connect an MQTT node after the gateway

#Note how we can indicate a device source addr that are allowed to use the script
#Use decimal between 2-255 and use 4-byte hex format for LoRaWAN devAddr
#leave empty to allow all devices
#source_list=["3", "255", "01020304"]
source_list=[]

####################################################