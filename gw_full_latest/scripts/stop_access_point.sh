#!/bin/bash

pid = `pgrep hostapd`

if [ "$pid" == "" ]; 
  then 
	echo "The access point is already disabled."  
  else
	sudo service hostapd stop
	sudo service dnsmasq stop
	echo "The access point is now disabled, using the file /etc/wpa_supplicant/wpa_supplicant.conf to connect to an access point."
fi
