####################################################
#server: CAUTION must exist
#the server indication is only for uplink messages
#if lora_pkt_fwd is used at low-level, then indicate the server
#in global_conf.json so that other messages (e.g. downlink, stats,...)
#are correctly handled
#do not use localhost as it may be converted into ::1 for in IPv6 notation
lorawan_server="127.0.0.1"
lorawan_port=1700

#Note how we can indicate a device source addr that are allowed to use the script
#Use decimal between 2-255 and use 4-byte hex format for LoRaWAN devAddr
#leave empty to allow all devices
#source_list=["6", "7", "0x01020304"]
source_list=[]

