#!/bin/bash

### BEGIN INIT INFO
# Provides:          ap
# Required-Start:
# Required-Stop:
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: Init access point
# Description:       Init access point
### END INIT INFO

INTERFACES_NOT_AP_FILE=/etc/network/interfaces_not_ap
INTERFACES_AP_FILE=/etc/network/interfaces_ap
INTERFACES_FILE=/etc/network/interfaces

# checking if right interfaces file or not
if [ -f $INTERFACES_AP_FILE ]; 
  then
	sudo service networking stop
	sudo mv $INTERFACES_FILE $INTERFACES_NOT_AP_FILE
	sudo mv $INTERFACES_AP_FILE $INTERFACES_FILE
	sudo service networking start
	echo "The access point is now enabled."
  else
	echo "The access point is already enabled."
fi

#launching services dnsmasq and hostapd
sudo service dnsmasq start
sudo service hostapd start
