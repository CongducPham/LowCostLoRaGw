# LoRa test channel
_def_thingspeak_channel_key='SGSH52UGPVAUYG3S'

#Note how we can indicate a device source addr that are allowed to use the script
#Use decimal between 2-255 and use 4-byte hex format for LoRaWAN devAddr
#leave empty to allow all devices
#source_list=["3", "255", "01020304"]
source_list=[]

field_association=[]
# (6,2) means data from sensor 6 will use starting field index of 2
#field_association=[(6,2)]