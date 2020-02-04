#!/bin/sh

#List of /boot file to control the startup behavior
#These file can be created by inserting the SD card 
#into any computer as the /boot partition is in FAT
#format. The content has no meaning, just the file.
#
#	- /boot/rak831.txt
#	- /boot/basic_config_on_boot.txt
#	- /boot/do_not_start_lora_gw.txt
###################################################

#leave some time for system to completely setup
sleep 10

#if there is a lora_gateway folder in the boot partition then we copy the whole content
#this is usefull when you want to configure the gateway by writing on the SD card from a computer
if [ -d /boot/lora_gateway ]
then
	echo "Detecting /boot/lora_gateway folder, copy content in /home/pi/lora_gateway"
	cp -r /boot/lora_gateway/* /home/pi/lora_gateway
	mv /boot/lora_gateway /boot/lora_gateway_`date +%F`
fi
	
cd /home/pi/lora_gateway

#run script for the shutdown button
if [ -f /boot/rak831.txt ] || grep start_upl /etc/rc.local>/dev/null || grep start_lpf /etc/rc.local>/dev/null: ; then
	#because of pin conflict with the RAK831/2245 shield the piShutdown script to shutdown the RPI is disabled
	#besides the pins are not really accessible on these shields
	echo "disabling piShutdown"
else
	echo "enabling piShutdown"
	python /home/pi/lora_gateway/scripts/piShutdown.py &
fi

#check whether the low-level radio bridge has been compiled for the correct RPI architecture
arch_compiled="NA"

if [ -f /home/pi/lora_gateway/arch_compiled.txt ]
then
	arch_compiled=`cat /home/pi/lora_gateway/arch_compiled.txt`
	echo "lora_gateway compiled for $arch_compiled"
fi

arch=`/home/pi/lora_gateway/scripts/test_raspberry_model.sh | grep ">" | cut -d ">" -f2`
echo "detecting RPI arch to be $arch"

#we recompile if architecture has changed, or if we are forced to run basic_config_gw.sh
if [ -f /boot/basic_config_on_boot.txt ] || [ $arch != $arch_compiled ] || [ $arch_compiled = "NA" ]
then
	echo "Running basic_config_gw.sh"
	sudo /home/pi/lora_gateway/scripts/basic_config_gw.sh
fi

#create the gw id so that a newly installed gateway is always configured with a correct id
/home/pi/lora_gateway/scripts/create_gwid.sh

###
### Start Internet access with 3G dongle
############################################

if [ -f 3GDongle/use_3GDongle_internet_on_boot.txt ]
then
	echo "Start Internet with 3G Dongle"
	cd 3GDongle
	# the script will turn on the modem and launch pppd
	# it will keep re-launching wvdial is case wvdial crashes
	sudo ./wvdial_wrapper.sh Dongle &
	sleep 15
	cd ..
###
### Start Internet access with Loranga board
############################################
elif [ -f 3GDongle/loranga/use_loranga_internet_on_boot.txt ]
then
	echo "Start Internet with Loranga board"
	cd 3GDongle/loranga
	# the script will turn on the modem and launch pppd
	./start-internet.sh &
	sleep 15
	cd ../../
elif [ -f 3GDongle/loranga/use_loranga_SMS_on_boot.txt ]
then
	echo "Use Loranga board for SMS"
	cd 3GDongle/loranga	
	# just turn on the modem
	if [ -f loranga3G.txt ]
	then
		sudo python wake-3G.py
	else	
		sudo python wake-2G.py
	fi
	cd ../..
fi
############################################

###
### Start Node-Red if needed
############################################

#try to find whether NodeRed cloud is enabled or not
i=`jq '.clouds[].script|index("CloudNodeRed")|values' clouds.json`
#echo $i
j=`jq '.clouds[].script|index("CloudNodeRed")' clouds.json | grep -n "$i" | cut -d ":" -f1`
#echo $j
nodered=`jq ".clouds[$j-1].enabled" clouds.json`
#echo $nodered
if [ $nodered = "true" ]
then
	echo "CloudNodeRed is enabled, start Node-Red"
	sudo -u pi node-red-start &
fi
############################################

###
### Start GPS web interface nodejs if needed
############################################

#try to find whether CloudGpsFile cloud is enabled or not
i=`jq '.clouds[].script|index("CloudGpsFile")|values' clouds.json`
#echo $i
j=`jq '.clouds[].script|index("CloudGpsFile")' clouds.json | grep -n "$i" | cut -d ":" -f1`
#echo $j
gpsfile=`jq ".clouds[$j-1].enabled" clouds.json`
#echo $gpsfile
if [ $gpsfile = "true" ] && [ -f gps/server.js ]
then
	echo "CloudGpsFile is enabled, start GPS nodejs web interface"
	cd gps
	node server.js &
	
	if [ -f gps.txt ]
	then
		ftime=`stat -c %Y gps.txt`
		ctime=`date +%s`
		ddiff=$(( (ctime - ftime) / 86400 ))
		if [ "$ddiff" != "0" ]
		then	
			echo "gps.txt file is $ddiff day old"
			echo "deleting gps.txt and gps.csv files"
			rm -rf gps.txt gps.csv
		fi	
	fi		
	cd ..
fi
############################################

###
### Internet routing for WiFi access point
############################################

#check if the gateway is an access point
#if yes, then enable IP forwarding to give internet connectivity to connected devices, e.g. smartphone, tablets,...
pid=`pgrep hostapd`

if [ "$pid" != "" ]
then
		echo "Gateway is acting as WiFi access point, configuring routing rules"
		sudo sh -c "echo 1 > /proc/sys/net/ipv4/ip_forward"
		sudo sed -i 's/^#net.ipv4.ip_forward=1/net.ipv4.ip_forward=1/g' /etc/sysctl.conf

		if [ -f 3GDongle/use_3GDongle_internet_on_boot.txt ] || [ -f 3GDongle/loranga/use_loranga_internet_on_boot.txt ]
		then
				echo "Internet connectivity is provided by cellular network, configuring routing rules"
				sudo iptables -t nat -A POSTROUTING -o ppp0 -j MASQUERADE
				sudo iptables -A FORWARD -i ppp0 -o wlan0 -m state --state RELATED,ESTABLISHED -j ACCEPT
				sudo iptables -A FORWARD -i wlan0 -o ppp0 -j ACCEPT
		else
				sudo iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
				sudo iptables -A FORWARD -i eth0 -o wlan0 -m state --state RELATED,ESTABLISHED -j ACCEPT
				sudo iptables -A FORWARD -i wlan0 -o eth0 -j ACCEPT
		fi
fi
############################################

###
### Version management and auto update
############################################

if [ -f /home/pi/lora_gateway/VERSION.txt ]
then
	installed_version=`cat /home/pi/lora-gateway/VERSION.txt`
else
	installed_version=`cat /home/pi/VERSION.txt`
fi	
	
echo "Current installed version is $installed_version"

if ping -q -c 1 -W 1 8.8.8.8 >/dev/null; then
	#get git version of distribution
	echo "svn trying to get version info from github"
	svn info https://github.com/CongducPham/LowCostLoRaGw/trunk/gw_full_latest | grep "Revision:" | cut -d ' ' --field=2 > /home/pi/git-VERSION-tmp.txt

	#just in case
	git_version_size=`stat -c %b /home/pi/git-VERSION-tmp.txt`
		
	if [ "$git_version_size" != "0" ]
	then
		mv /home/pi/git-VERSION-tmp.txt /home/pi/git-VERSION.txt
		git_version=`cat /home/pi/git-VERSION.txt`
		echo "github version is $git_version"
	
		auto_update=`jq ".gateway_conf.auto_update" gateway_conf.json`

		if [ "$auto_update" = "true" ]
		then
			echo "Auto update is on, trying to update to latest version from github"
		
			if [ -f /home/pi/lora_gateway/VERSION.txt ]
			then
				echo "Previous installation from .zip archive detected"
				echo "Aborting auto-update from github"
			else
				if [ "$installed_version" != "$git_version" ]
				then
					echo "Launching update script to install latest version from github"
					echo "Equivalent to full update with the web admin interface"
					/home/pi/lora_gateway/scripts/update_gw.sh
					sleep 5
				else
					echo "Installed version is up-to-date"
				fi
			fi
		else
			echo "Auto update is off."
		fi
	fi
else
	echo "N/A" > /home/pi/git-VERSION.txt
	git_version=`cat /home/pi/git-VERSION.txt`
	echo "svn could not get version info from github, mark as $git_version."
	rm -rf /home/pi/git-VERSION-tmp.txt
fi	
############################################

###
### LoRaWAN downlink from a network server for a single-channel gateway
############################################

#try to find whether LoRaWAN downlink is used or not
downlink=`jq ".gateway_conf.downlink" gateway_conf.json`
downlink_lorawan=`jq ".gateway_conf.downlink_lorawan" gateway_conf.json`
lora_mode=`jq ".radio_conf.mode" gateway_conf.json`
 
if [ $lora_mode -eq 11 ] && [ "$downlink_lorawan" = "true" ] && [ $downlink -ge 0 ]
then
	if grep start_upl /etc/rc.local>/dev/null || grep start_lpf /etc/rc.local>/dev/null: ; then
		echo "Multi-channel concentrator configuration detected, not using single-channel downlink mechanism"
	else
		echo "Downlink message will be pulled from LoRaWAN Network Server for single-channel gateway"
		python /home/pi/lora_gateway/scripts/lorawan_stats/downlink_lorawan.py & 
	fi
fi
############################################

###
### Last item, run the gateway
############################################

#run the gateway
if [ -f /boot/do_not_start_lora_gw.txt ]
then
	echo 'forced to not start lora_gateway'
elif [ $# -eq 0 ]
then
	python start_gw.py &
fi

############################################

#leave some time for mongo to start, otherwise there might be a problem
sleep 10

#repair if needed the mongodb database connection
/home/pi/lora_gateway/scripts/mongo_repair.sh
