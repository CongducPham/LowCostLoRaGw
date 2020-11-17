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
			if [ -f /home/pi/lora_gateway/scripts/repo.txt ]
				then
					repo=`tr -d '\n' < /home/pi/lora_gateway/scripts/repo.txt`
					echo "using user-defined repository: $repo"
				else
					repo="https://github.com/CongducPham/LowCostLoRaGw/trunk/gw_full_latest"
					echo "using default repository: $repo"
			fi		
			
			svn checkout "$repo" > svn.txt
			grep "Checked out revision" svn.txt | cut -d ' ' --field=4 > /home/pi/git-VERSION.txt
			sed -i -- 's/\.//g' /home/pi/git-VERSION.txt
			cp /home/pi/git-VERSION.txt /home/pi/VERSION.txt
			rm -rf svn.txt
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
		cp gateway_conf.json clouds.json *.makefile key_* config_backup
		cd ../gw_full_latest
		echo "copying new distrib into /home/pi/lora_gateway"
		cp --preserve -r * /home/pi/lora_gateway
		cd ../lora_gateway
		echo "reinstalling your configuration and key files"
		cp config_backup/* .
		#if we update from a github repo we remove VERSION.txt in /home/pi/lora_gateway
		rm VERSION.txt	
	else
		echo "new installation"
		echo "simply renaming gw_full_latest in lora_gateway"
		mv gw_full_latest lora_gateway	
fi	

cd /home/pi/lora_gateway

echo "update of gateway done."
sudo chown -R pi:pi /home/pi/lora_gateway/
echo "go into lora_gateway/scripts folder and run ./basic_config_gw.sh"
