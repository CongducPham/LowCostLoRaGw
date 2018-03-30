#!/bin/sh

cd /home/pi/lora_gateway

if [ -f 3GDongle/loranga/use_loranga_internet_on_boot.txt ]
then
	echo "Start Internet with Loranga board"
	cd 3GDongle/loranga
	# the script will turn on the modem and launch pppd
	./start-internet.sh
	sleep 10
	cd ../../
elif [ -f 3GDongle/loranga/use_loranga_SMS_on_boot.txt ]
then
	echo "Use Loranga board for SMS"
	cd 3GDongle/loranga
	# just turn on the modem
	sudo python wake-2G.py
	cd ../..
fi

#create the gw id so that a newly installed gateway is always configured with a correct id
./scripts/create_gwid.sh

#try to find whether NodeRed cloud is enabled or not
i=`jq '.clouds[].script|index("CloudNodeRed")|values' clouds.json`
#echo $i
j=`jq '.clouds[].script|index("CloudNodeRed")' clouds.json | grep -n "$i" | cut -c1`
#echo $j
nodered=`jq ".clouds[$j-1].enabled" clouds.json`
#echo $nodered
if [ $nodered = "true" ]
then
	echo "CloudNodeRed is enabled, start Node-Red"
	node-red-start &
fi

#run the gateway
python start_gw.py &
