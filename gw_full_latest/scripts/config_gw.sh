#!/bin/bash

#------------------------------------------------------------
# Copyright 2016-2017 Congduc Pham, University of Pau, France.
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


#
# example: ./config_gw.sh

cd /home/pi/lora_gateway/scripts

board=`cat /proc/cpuinfo | grep "Revision" | cut -d ':' -f 2 | tr -d " \t\n\r"`

echo "********************************************"
echo "*** compile lora_gateway executable Y/N  ***"
echo "********************************************"
read ouinon

if [ "$ouinon" = "y" ] || [ "$ouinon" = "Y" ]
	then
		cd ..
		
		downlink=`jq ".gateway_conf.downlink" gateway_conf.json`
 
		if [ "$downlink" != "0" ]
		then
			echo "Detecting downlink timer, will compile with downlink support"
		fi	
		
		if [ "$board" = "a01041" ] || [ "$board" = "a21041" ] || [ "$board" = "a22042" ]
			then
				echo "You have a Raspberry 2"
				echo "Compiling for Raspberry 2 and 3"
				if [ "$downlink" = "0" ]
					then 
						make lora_gateway_pi2
					else
						make lora_gateway_pi2_downlink 
				fi		
		elif [ "$board" = "a02082" ] || [ "$board" = "a22082" ]
			then
				echo "You have a Raspberry 3"
				echo "Compiling for Raspberry 2 and 3"
				if [ "$downlink" = "0" ]
					then 
						make lora_gateway_pi2
					else
						make lora_gateway_pi2_downlink 
				fi	
		elif [ "$board" = "900092" ] || [ "$board" = "900093" ]
			then
				echo "You have a Raspberry Zero"
				echo "Compiling for Raspberry Zero (same as Raspberry 1)"
				if [ "$downlink" = "0" ]
					then 
						make lora_gateway
					else
						make lora_gateway_downlink 
				fi	
		elif [ "$board" = "9000c1" ]
			then
				echo "You have a Raspberry Zero W"
				echo "Compiling for Raspberry Zero W (same as Raspberry 1)"
				if [ "$downlink" = "0" ]
					then 
						make lora_gateway
					else
						make lora_gateway_downlink 
				fi	
		else
			echo "You have a Raspberry 1"		
			echo "Compiling for Raspberry 1"
			if [ "$downlink" = "0" ]
				then 
					make lora_gateway
				else
					make lora_gateway_downlink 
			fi	
		fi		
		
		cd scripts
fi

if [ $# == 1 ]
then
	echo "Taking provided address: $1"
	gwid="$1"
else	

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
fi

echo "Keep a copy of /home/pi/lora_gateway/scripts/update_gw.sh"
mkdir /home/pi/scripts
cp /home/pi/lora_gateway/scripts/update_gw.sh /home/pi/scripts
echo "Done"

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

if [ ! -d /home/pi/Dropbox/LoRa-test ]
	then
		echo "************************************************************"
		echo "*** create /home/pi/Dropbox/LoRa-test (recommended) Y/N  ***"
		echo "************************************************************"
		read ouinon

		if [ "$ouinon" = "y" ] || [ "$ouinon" = "Y" ]
			then
				echo "Creating /home/pi/Dropbox/LoRa-test"
				mkdir -p /home/pi/Dropbox/LoRa-test
				echo "Done"
		fi
	else
		echo "/home/pi/Dropbox/LoRa-test already exist. OK."
fi
				
echo "***************************************************************"
echo "*** create log symb link to /home/pi/Dropbox/LoRa-test Y/N  ***"
echo "***************************************************************"
read ouinon

if [ "$ouinon" = "y" ] || [ "$ouinon" = "Y" ]
	then
		rm /home/pi/lora_gateway/log
		echo "Creating log -> /home/pi/Dropbox/LoRa-test"
		ln -s /home/pi/Dropbox/LoRa-test /home/pi/lora_gateway/log
		echo "Done"		
fi	

echo "***********************************"
if [ -f /etc/hostapd/custom_ssid.txt ];
then
	echo "Customized hot-spot ssid detected: `cat /etc/hostapd/custom_ssid.txt`"
	echo "Configuring hostapd.conf will replace with WAZIUP_PI_GW_$gwid"
	echo "Password will also be reset to loragateway"		
fi

echo "***********************************"
echo "*** configure hostapd.conf Y/N  ***"
echo "***********************************"
read ouinon

if [ "$ouinon" = "y" ] || [ "$ouinon" = "Y" ]
	then
		echo "Replacing hot-spot ssid in /etc/hostapd/hostapd.conf"
		sudo sed -i 's/^ssid.*/ssid=WAZIUP_PI_GW_'"$gwid"'/g' /etc/hostapd/hostapd.conf
		echo "Done"
		echo "Gateway WiFi ssid is WAZIUP_PI_GW_$gwid"
		
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
				echo "Removing /etc/hostapd/custom_ssid.txt if any"
				sudo rm -rf /etc/hostapd/custom_ssid.txt
				echo "Done"			
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
				echo "Done"
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
		sudo sed -i 's/^#Name *=.*/Name=WAZIUP_PI_BT_GW_'"$gwid"'/g' /etc/bluetooth/main.conf
		# then in all cases we replace Name so that if we already have an uncommented Name it will also work
		sudo sed -i 's/^Name *=.*/Name=WAZIUP_PI_BT_GW_'"$gwid"'/g' /etc/bluetooth/main.conf
		echo "Done"
		echo "Gateway Bluetooth network name is WAZIUP_PI_BT_GW_$gwid"
fi

## TODO: /lib/systemd/system/bluetooth.service

echo "**********************************"
echo "*** install DHT22 support Y/N  ***"
echo "**********************************"
read ouinon

if [ "$ouinon" = "y" ] || [ "$ouinon" = "Y" ]
	then
		echo "Compiling the DHT22 library"
		cd /home/pi/lora_gateway/sensors_in_raspi
		cd PIGPIO
		make -j4
		echo "Done"
		echo "Installing the DHT22 library"
		sudo make install
		echo "Done"
		
		echo "Change in gateway_conf.json the value in seconds between 2 readings."
		echo "--> \"dht22\" : 3600"
		echo "for one reading every hour. Set to 0 to disable DHT reading."
		echo "***************************************"
		echo "*** edit gateway_conf.json now? Y/N ***"
		echo "***************************************"
		read ouinon
		if [ "$ouinon" = "y" ] || [ "$ouinon" = "Y" ]
			then
				echo "Editing gateway_conf.json. CTRL-O to save, CTRL-X to return. Press any key to start editing."
				read k 
				nano /home/pi/lora_gateway/gateway_conf.json	
		fi			
		
fi

echo ""
echo "Current status for DHT22 MongoDB is:"
grep "dht22_mongo" /home/pi/lora_gateway/gateway_conf.json
echo ""
echo "*************************************"
echo "*** activate DHT22 MongoDB Y/N/Q  ***"
echo "*************************************"
read ouinon

if [ "$ouinon" = "y" ] || [ "$ouinon" = "Y" ]
	then
		echo "Activating DHT22 MongoDB in gateway_conf.json"
		sed -i 's/"dht22_mongo".*:.*false/"dht22_mongo" : true/g' /home/pi/lora_gateway/gateway_conf.json
		echo "Done"
fi

if [ "$ouinon" = "n" ] || [ "$ouinon" = "N" ]
	then
		echo "Deactivating DHT22 MongoDB in gateway_conf.json"
		sed -i 's/"dht22_mongo".*:.*true/"dht22_mongo" : false/g' /home/pi/lora_gateway/gateway_conf.json
		echo "Done"
fi

echo "*********************************************************"
echo "*** edit LoRa data MongoDB local storage option? Y/N  ***"
echo "*********************************************************"
read ouinon

if [ "$ouinon" = "y" ] || [ "$ouinon" = "Y" ]
	then
		echo "INSTRUCTIONS: set the local gateway MongoDB cloud enabled field to true or false."
		echo "Editing clouds.json. CTRL-O to save, CTRL-X to return. Press any key to start editing."
		read k 
		nano /home/pi/lora_gateway/clouds.json
fi

if [ "$ouinon" = "n" ] || [ "$ouinon" = "N" ]
	then
		echo "WARNING: LoRa data MongoDB local storage status is unknown"
fi

echo "*******************************"
echo "*** run gateway at boot Y/N ***"
echo "*******************************"
read ouinon

# we always remove so that there will be no duplicate lines
echo "Removing /home/pi/lora_gateway/scripts/start_gw.sh in /etc/rc.local if any"
sudo sed -i '\/home\/pi\/lora_gateway\/scripts\/start_gw.sh/d' /etc/rc.local
echo "Done"

if [ "$ouinon" = "y" ] || [ "$ouinon" = "Y" ]
	then
		echo "Add /home/pi/lora_gateway/scripts/start_gw.sh in /etc/rc.local"
		sudo sed -i 's/^exit 0/\/home\/pi\/lora_gateway\/scripts\/start_gw.sh\nexit 0/g' /etc/rc.local
		echo "Done"
fi

		
echo "**********************************************"
echo "*** check configuration (recommended) Y/N  ***"
echo "**********************************************"
read ouinon

if [ "$ouinon" = "y" ] || [ "$ouinon" = "Y" ]
	then
		echo "Displaying /home/pi/lora_gateway/gateway_id.txt with less command. Press key to start. Scroll with Space, Q to quit."
		read k
		less /home/pi/lora_gateway/gateway_id.txt

		echo "Displaying /home/pi/lora_gateway/gateway_conf.json with less command. Press key to start. Scroll with Space, Q to quit."
		read k
		less /home/pi/lora_gateway/gateway_conf.json

		echo "Displaying /home/pi/lora_gateway/clouds.json with less command. Press key to start. Scroll with Space, Q to quit."
		read k
		less /home/pi/lora_gateway/clouds.json
				
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

		echo "Displaying /etc/rc.local with less command. Press key to start. Scroll with Space, Q to quit."
		read k
		sudo less /etc/rc.local
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
