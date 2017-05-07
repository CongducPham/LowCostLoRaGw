#!/bin/bash

INTERFACES_NOT_AP_FILE=/etc/network/interfaces_not_ap
INTERFACES_AP_FILE=/etc/network/interfaces_ap
INTERFACES_FILE=/etc/network/interfaces

#if "interfaces_not_ap" exists, then replace current interfaces file by this one
if [ -f $INTERFACES_NOT_AP_FILE ]; 
  then
	sudo service hostapd stop
	sudo service dnsmasq stop
	sudo service networking stop
	sudo mv $INTERFACES_FILE $INTERFACES_AP_FILE
	sudo mv $INTERFACES_NOT_AP_FILE $INTERFACES_FILE
	sudo service networking start
	echo "The access point is now disabled, using the file /etc/wpa_supplicant/wpa_supplicant.conf to connect to an access point."
  else
	echo "The access point is already disabled."
fi
