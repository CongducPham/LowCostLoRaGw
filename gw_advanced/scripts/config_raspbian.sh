#!/bin/bash

#------------------------------------------------------------
# Copyright 2016 Congduc Pham, University of Pau, France.
# 
# Congduc.Pham@univ-pau.fr
#
# This file is part of the low-cost LoRa gateway developped at University of Pau
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with the program.  If not, see <http://www.gnu.org/licenses/>.
#------------------------------------------------------------


#provide to the script the last 5 hex number of your eth0 interface, i.e. 27EBBEDA21 
#
# example: ./config_raspbian.sh 27EBBEDA21

if [ "$#" -ne 1 ]
  then
    echo "Usage: $0 last_fixe_hex_byte_of_eth0_iface"
    exit
fi

echo "Replacing gw id in ../local_conf.json"
sed -i -- 's/"000000.*"/"000000'"$1"'"/g' ../local_conf.json
echo "Done"

if [ ! -d ~/Dropbox/LoRa-test ]
        then
                echo "*****************************************************"
                echo "*** create ~/Dropbox/LoRa-test (recommended) Y/N  ***"
                echo "*****************************************************"
                read ouinon

                if [ "$ouinon" = "y" ] || [ "$ouinon" = "Y" ]
                        then
                                echo "Creating ~/Dropbox/LoRa-test"
                                mkdir -p ~/Dropbox/LoRa-test
                                echo "Done"
                fi
        else
                echo "~/Dropbox/LoRa-test already exist. OK."
fi				
	
echo "***********************************"
echo "*** configure hostapd.conf Y/N  ***"
echo "***********************************"
read ouinon

if [ "$ouinon" = "y" ] || [ "$ouinon" = "Y" ]
	then
		echo "Replacing hot-spot ssid in /etc/hostapd/hostapd.conf"
		sudo sed -i 's/^ssid.*/ssid=WAZIUP_PI_GW_'"$1"'/g' /etc/hostapd/hostapd.conf
		echo "Done"
		echo "Gateway WiFi ssid is WAZIUP_PI_GW_$1"
		
		echo "Setting wpa_passphrase in /etc/hostapd/hostapd.conf"
		sudo sed -i 's/^wpa_passphrase.*/wpa_passphrase=loragateway/g' /etc/hostapd/hostapd.conf
		echo "Done"
		echo "Gateway WiFi wpa_passphrase is loragateway"		
		echo "Gateway is not currently configured as access-point. run ./start_access_point.sh to do so"
		
		echo "****************************************************************"
		echo "*** configure a newly installed hostapd/dnsmasq package Y/N  ***"
		echo "****************************************************************"
		read ouinon

		if [ "$ouinon" = "y" ] || [ "$ouinon" = "Y" ]
			then			
				echo "Indicating in /etc/default/hostapd that /etc/hostapd/hostapd.conf is the config file"
				# here we replace #DAEMON_CONF to DAEMON_CONF in case DAEMON_CONF was commented
				sudo sed -i 's/^#DAEMON_CONF.*/DAEMON_CONF="/etc/hostapd/hostapd.conf"/g' /etc/default/hostapd
				# then in all cases we replace DAEMON_CONF so that if we already have an uncommented DAEMON_CONF it will also work
				sudo sed -i 's/^DAEMON_CONF.*/DAEMON_CONF="/etc/hostapd/hostapd.conf"/g' /etc/default/hostapd
				echo "Done"
				echo "Copying access-point interface configuration file in /etc/network"
				sudo cp interfaces_ap /etc/network
				echo "Done"
				echo "Add in /etc/dnsmasq.conf DHCP lease IP range, the RPI acces-point will have IP 192.168.200.1"
				echo "interface=wlan0" >>  /etc/dnsmasq.conf
				echo "dhcp-range=192.168.200.100,192.168.200.120,255.255.255.0,1h" >>  /etc/dnsmasq.conf
		fi

fi
		
echo "*********************************************"
echo "*** configure bluetooth network name Y/N  ***"
echo "*********************************************"
read ouinon

if [ "$ouinon" = "y" ] || [ "$ouinon" = "Y" ]
	then
		echo "Disabling pnat plugin in /etc/bluetooth/main.conf"
		# insert right after [General] section
		sudo sed -i 's/\[General\]/\[General\]\nDisablePlugins=pnat/g' /etc/bluetooth/main.conf
		echo "Done"
		echo "Renaming Bluetooth network name in /etc/bluetooth/main.conf"
		# here we replace #Name to Name in case Name was commented
		sudo sed -i 's/^#Name *=.*/Name=WAZIUP_PI_BT_GW_'"$1"'/g' /etc/bluetooth/main.conf
		# then in all cases we replace Name so that if we already have an uncommented Name it will also work
		sudo sed -i 's/^Name *=.*/Name=WAZIUP_PI_BT_GW_'"$1"'/g' /etc/bluetooth/main.conf
		echo "Done"
		echo "Gateway Bluetooth network name is WAZIUP_PI_BT_GW_$1"
fi

## TODO: /lib/systemd/system/bluetooth.service

echo "**********************************"
echo "*** install DHT22 support Y/N  ***"
echo "**********************************"
read ouinon

if [ "$ouinon" = "y" ] || [ "$ouinon" = "Y" ]
	then
		echo "Compiling the DHT22 library"
		cd sensors_in_raspi
		cd PIGPIO
		make -j4
		echo "Done"
		echo "Installing the DHT22 library"
		sudo make install
		echo "Done"
		cd ../..
		echo "Change in local_conf.json the value in seconds between 2 readings"
		echo "--> \"dht22\" : 3600"
		echo "for one reading every hour"
fi

echo "*****************************"
echo "*** activate MongoDB Y/N  ***"
echo "*****************************"
read ouinon

if [ "$ouinon" = "y" ] || [ "$ouinon" = "Y" ]
	then
		echo "Activating MongoDB in global_conf.json"
		sed -i 's/"is_used".*:.*false/"is_used" : true/g' ../global_conf.json
		echo "Done"
fi
		
echo "**********************************************"
echo "*** check configuration (recommended) Y/N  ***"
echo "**********************************************"
read ouinon

if [ "$ouinon" = "y" ] || [ "$ouinon" = "Y" ]
	then
		echo "Displaying ../global_conf.json with less command. Press key to start. Scroll with Space, Q to quit."
		read k
		less ../global_conf.json
			
		echo "Displaying ../local_conf.json with less command. Press key to start. Scroll with Space, Q to quit."
		read k
		less ../local_conf.json
		
		echo "Displaying /etc/hostapd/hostapd.conf with less command. Press key to start. Scroll with Space, Q to quit."
		read k
		sudo less /etc/hostapd/hostapd.conf
		
		echo "Displaying /etc/default/hostapd with less command. Press key to start. Scroll with Space, Q to quit."
		read k
		sudo less /etc/default/hostapd
		
		echo "Displaying /etc/dnsmasq.conf with less command. Press key to start. Scroll with Space, Q to quit."
		read k
		sudo less /etc/dnsmasq.conf
		
		echo "Displaying /etc/bluetooth/main.conf with less command. Press key to start. Scroll with Space, Q to quit."
		read k
		sudo less /etc/bluetooth/main.conf
				
		echo "Displaying /lib/systemd/system/bluetooth.service with less command. Press key to start. Scroll with Space, Q to quit."
		read k
		sudo less /lib/systemd/system/bluetooth.service											
fi	

echo "You should reboot your Raspberry"
echo "*******************"
echo "*** reboot Y/N  ***"
echo "*******************"
read ouinon

if [ "$ouinon" = "y" ] || [ "$ouinon" = "Y" ]
	then
		echo "Rebooting, please reconnect"
		sudo shutdown -r now
fi

echo "Bye."
