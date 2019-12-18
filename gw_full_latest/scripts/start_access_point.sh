#!/bin/bash

pid = `pgrep hostapd`

if [ "$pid" == "" ]; 
  then	
	sudo systemctl enable hostapd.service
	echo "The access point is now enabled."
  else
	echo "The access point is already enabled."
fi

#launching services dnsmasq and hostapd
echo "Starting hostapd and dnsmasq services"
sudo systemctl unmask dnsmasq
sudo systemctl unmask hostapd
sudo service dnsmasq start
sudo service hostapd start
