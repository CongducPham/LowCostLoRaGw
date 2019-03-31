#!/bin/bash

#get the eth0 MAC addr
gwid=`ifconfig | grep 'eth0' | awk '{print $NF}' | sed 's/://g' | awk '{ print toupper($1) }'`

#get the wlan0 MAC addr
if [ "$gwid" = "" ]
	then
		gwid=`ifconfig | grep 'wlan0' | awk '{print $NF}' | sed 's/://g' | awk '{ print toupper($1) }'`
		
		#it means that the wlan0 interface works in access point mode or has no IP address assigned
		#so get the address from the ether field
		if [ "$gwid" = "00" ]
		then
			gwid=`ifconfig | grep 'ether' | awk '{print $2}' | sed 's/://g' | awk '{ print toupper($1) }'`
		fi
fi

#set a default value
if [ "$gwid" = "" ]
	then
		gwid="XXXXXXXXDEF0"
fi

echo "Creating /home/pi/lora_gateway/gateway_id.txt file"
echo "Writing 0000$gwid"
echo "0000$gwid" > /home/pi/lora_gateway/gateway_id.txt
echo "Done"

echo "Creating /home/pi/lora_gateway/gateway_id.md5 file"
tr -d '\n' < /home/pi/lora_gateway/gateway_id.txt | md5sum - | cut -d ' ' --field=1 > /home/pi/lora_gateway/gateway_id.md5
#the version below actually takes the \n at the end of the gw id which is not what we want, thus the version above
#md5sum /home/pi/lora_gateway/gateway_id.txt | cut -d ' ' --field=1 > /home/pi/lora_gateway/gateway_id.md5
echo "Done"

echo "Replacing gw id in /home/pi/lora_gateway/gateway_conf.json"
sed -i -- 's/gateway_ID".*"0000.*"/gateway_ID": "0000'"$gwid"'"/g' /home/pi/lora_gateway/gateway_conf.json
echo "Done"

