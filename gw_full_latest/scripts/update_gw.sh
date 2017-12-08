#!/bin/sh

echo "updating gateway"
cd /home/pi/

echo "removing any existing gw_full_latest folder (from previous update)"
rm -rf gw_full_latest

if [ -d local_gw_full_latest ]
then
	echo "Installing from local_gw_full_latest folder"
	mv local_gw_full_latest gw_full_latest
else
	wget -q --spider http://google.com

	if [ $? -eq 0 ]
		then
			#online
			echo "getting new gw_full_latest from github"
			svn checkout https://github.com/CongducPham/LowCostLoRaGw/trunk/gw_full_latest
		else
			echo "No Internet connection, exiting"
			exit
	fi			
fi

if [ ! -d gw_full_latest ]
then
	echo "Failed to find a valid gw_full_latest folder for installation, exiting"
	exit
fi

if [ -d lora_gateway ]
	then
		echo "detecting an existing lora_gateway folder"
		cd lora_gateway
		echo "preserving your configuration and key files"		
		mkdir config_backup
		cp gateway_conf.json clouds.json radio.makefile key_* loraWAN_config.py config_backup
		cd ../gw_full_latest
		echo "copying new distrib into /home/pi/lora_gateway"
		cp --preserve -r * /home/pi/lora_gateway
		cd ../lora_gateway
		echo "reinstalling your configuration and key files"
		cp config_backup/* .	
	else
		echo "new installation"
		echo "simply renaming gw_full_latest in lora_gateway"
		mv gw_full_latest lora_gateway	
fi	

cd /home/pi/lora_gateway

echo "compiling the gateway program"

board=`cat /proc/cpuinfo | grep "Revision" | cut -d ':' -f 2 | tr -d " \t\n\r"`

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

echo "update of gateway done."
sudo chown -R pi:pi /home/pi/lora_gateway/
echo "if it is a new installation, go into lora_gateway/scripts folder"
echo "and run ./basic_config_gw.sh"
