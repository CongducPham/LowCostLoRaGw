#!/bin/bash

#if [ ! -f /home/pi/lora_gateway/gateway_id.txt ]
#then
#	echo "ERROR: gateway_id.txt file not found"
#	#echo "Done"
#fi

#gatewayid=`cat /home/pi/lora_gateway/gateway_id.txt`

#echo $gatewayid

#echo "First low-level gateway status"
#grep -a -m 1 "Low-level gw status ON" /home/pi/Dropbox/LoRa-test/post-processing_$gatewayid.log

#echo "Last low-level gateway status"
grep -a "Low-level gw status ON" /home/pi/Dropbox/LoRa-test/post-processing.log | tail -1

