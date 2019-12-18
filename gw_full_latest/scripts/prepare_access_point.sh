#!/bin/bash

pid = `pgrep hostapd`

if [ "$pid" == "" ]; 
  then
	sudo systemctl enable hostapd.service
	echo "The access point is now enabled for next reboot."
  else
	echo "The access point is already enabled. Reboot for changes to take effect."
fi
