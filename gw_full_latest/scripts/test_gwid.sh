#!/bin/bash

if [ $# == 1 ]
then
	echo "Taking provided address: $1"
	gwid="$1"
else	

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
fi
echo "Detecting gw id as 0000$gwid"

