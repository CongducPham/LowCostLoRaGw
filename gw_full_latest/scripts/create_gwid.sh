#!/bin/bash

#get the last 5 bytes of the eth0 MAC addr
gwid=`ifconfig | grep 'eth0' | awk '{print $NF}' | sed 's/://g' | awk '{ print toupper($1) }' | cut -c 3-`

#get the last 5 bytes of the wlan0 MAC addr
if [ "$gwid" = "" ]
	then
		gwid=`ifconfig | grep 'wlan0' | awk '{print $NF}' | sed 's/://g' | awk '{ print toupper($1) }' | cut -c 3-`
		
		#it means that the wlan0 interface works in access point mode or has no IP address assigned
		#so get the address from the ether field
		if [ "$gwid" = "00" ]
		then
			gwid=`ifconfig | grep 'ether' | awk '{print $2}' | sed 's/://g' | awk '{ print toupper($1) }' | cut -c 3-`
		fi
fi

#set a default value
if [ "$gwid" = "" ]
	then
		gwid="XXXXXXDEF0"
fi

echo "Creating /home/pi/lora_gateway/gateway_id.txt file"
echo "Writing 000000$gwid"
echo "000000$gwid" > /home/pi/lora_gateway/gateway_id.txt
echo "Done"

echo "Creating /home/pi/lora_gateway/gateway_id.md5 file"
tr -d '\n' < /home/pi/lora_gateway/gateway_id.txt | md5sum - | cut -d ' ' --field=1 > /home/pi/lora_gateway/gateway_id.md5
#the version below actually takes the \n at the end of the gw id which is not what we want, thus the version above
#md5sum /home/pi/lora_gateway/gateway_id.txt | cut -d ' ' --field=1 > /home/pi/lora_gateway/gateway_id.md5
echo "Done"

echo "Replacing gw id in /home/pi/lora_gateway/gateway_conf.json"
sed -i -- 's/"000000.*"/"000000'"$gwid"'"/g' /home/pi/lora_gateway/gateway_conf.json
echo "Done"

