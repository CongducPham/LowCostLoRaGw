gammurc_file="/home/pi/.gammurc"

PIN="0000"

contacts=["+1XXXXXXXXX"]

#Note how we can indicate a device source addr that are allowed to use the script
#Use decimal between 2-255 and use 4-byte hex format for LoRaWAN devAddr
#leave empty to allow all devices
#source_list=["3", "255", "0x01020304"]
source_list=[]