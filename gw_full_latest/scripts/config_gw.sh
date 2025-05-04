#!/bin/bash
#
# example: ./config_gw.sh

cd /home/pi/lora_gateway/scripts

echo "********************************************"
echo "*** compile lora_gateway executable Y/N  ***"
echo "********************************************"
read ouinon

if [ "$ouinon" = "y" ] || [ "$ouinon" = "Y" ]
	then
		./compile_lora_gateway.sh
fi

if [ $# == 1 ]
then
	echo "Taking provided address: $1"
	gwid="$1"
else	
	#get the eth0 MAC addr
	gwid=`ifconfig | grep eth0 | grep HWaddr | awk '{print $NF}' | sed 's/://g' | awk '{ print toupper($1) }'`
	
	if [ "$gwid" = "" ]
		then
			gwid=`ifconfig eth0 | grep 'ether' | awk '{print $2}' | sed 's/://g' | awk '{ print toupper($1) }'`
		fi
		
	#get the wlan0 MAC addr
	if [ "$gwid" = "" ]
		then
			gwid=`ifconfig | grep 'wlan0' | grep HWaddr | awk '{print $NF}' | sed 's/://g' | awk '{ print toupper($1) }'`
			
		if [ "$gwid" = "" ]
			then
				gwid=`ifconfig wlan0 | grep 'ether' | awk '{print $2}' | sed 's/://g' | awk '{ print toupper($1) }'`
			fi	
			
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

echo "Keep a copy of /home/pi/lora_gateway/scripts/update_gw.sh"
mkdir /home/pi/scripts
cp /home/pi/lora_gateway/scripts/update_gw.sh /home/pi/scripts
echo "Done"

echo "Creating /home/pi/lora_gateway/gateway_id.txt file"
echo "Writing 0000$gwid"
echo "0000$gwid" > /home/pi/lora_gateway/gateway_id.txt
echo "Done"

echo "Creating /home/pi/lora_gateway/gateway_id.md5 file"
tr -d '\n' < /home/pi/lora_gateway/gateway_id.txt | md5sum - | cut -d ' ' --field=1 > /home/pi/lora_gateway/gateway_id.md5
#the version below actually takes the \n at the end of the gw id which is not what we want, thus the version above
#md5sum /home/pi/lora_gateway/gateway_id.txt | cut -d ' ' --field=1 > /home/pi/lora_gateway/gateway_id.md5
echo "Done"

echo "Replacing gw id in /home/pi/lora_gateway/gateway_conf.json"
sed -i -- 's/gateway_ID".*"0000.*"/gateway_ID": "0000'"$gwid"'"/g' /home/pi/lora_gateway/gateway_conf.json
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
				#echo "Copying access-point interface configuration file in /etc/network"
				#sudo cp interfaces_ap /etc/network
				##########################
				#new way to set interfaces
				##########################
				sudo mv /etc/network/interfaces /etc/network/interfaces.backup
				sudo echo "source-directory /etc/network/interfaces.d" > /etc/network/interfaces
				sudo echo "nohook wpa_supplicant" >> /etc/dhcpcd.conf
				sudo echo "interface wlan0" >> /etc/dhcpcd.conf
				sudo echo "static ip_address=192.168.200.1/24" >> /etc/dhcpcd.conf 
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

if grep start_upl /etc/rc.local || grep start_lpf /etc/rc.local ; then
	echo "util_pkt_logger or lora_pkt_fwd is currently used as low-level gateway"
	echo "preserving this setting"
else
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
