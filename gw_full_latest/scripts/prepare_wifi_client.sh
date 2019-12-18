#!/bin/bash

pid = `pgrep hostapd`

if [ "$pid" == "" ]; 
  then
	echo "The access point is already disabled."
  else	
	sudo systemctl disable hostapd.service
	echo "The access point will be disabled at next reboot, using the file /etc/wpa_supplicant/wpa_supplicant.conf to connect to an access point."	
fi
