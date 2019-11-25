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

echo "compiling the gateway program"

downlink=`jq ".gateway_conf.downlink" gateway_conf.json`
 
if [ "$downlink" != "0" ]
then
	echo "Detecting downlink timer, will compile with downlink support"
fi	

revision=`cat /proc/cpuinfo | grep "Revision" | cut -d ':' -f 2 | tr -d " \t\n\r"`

rev_hex="0x$revision"

#uuuuuuuuFMMMCCCCPPPPTTTTTTTTRRRR
type=$(( rev_hex & 0x00000FF0 ))

board=$(( type >> 4 ))

if [ "$board" = "0" ] || [ "$board" = "1" ] || [ "$board" = "2" ] || [ "$board" = "3" ]
	then
		echo "You have a Raspberry 1"		
		echo "Compiling for Raspberry 1"
		if [ "$downlink" = "0" ]
			then 
				make lora_gateway
			else
				make lora_gateway_downlink 
		fi	
elif [ "$board" = "4" ]
	then
		echo "You have a Raspberry 2"
		echo "Compiling for Raspberry 2 and 3"
		if [ "$downlink" = "0" ]
			then 
				make lora_gateway_pi2
			else
				make lora_gateway_pi2_downlink 
		fi		
elif [ "$board" = "8" ]
	then
		echo "You have a Raspberry 3B"
		echo "Compiling for Raspberry 2 and 3"
		if [ "$downlink" = "0" ]
			then 
				make lora_gateway_pi2
			else
				make lora_gateway_pi2_downlink 
		fi	
elif [ "$board" = "13" ] || [ "$board" = "14" ]
	then
		echo "You have a Raspberry 3B+/3A+"
		echo "Compiling for Raspberry 2 and 3"
		if [ "$downlink" = "0" ]
			then 
				make lora_gateway_pi2
			else
				make lora_gateway_pi2_downlink 
		fi			
elif [ "$board" = "9" ]
	then
		echo "You have a Raspberry Zero"
		echo "Compiling for Raspberry Zero (same as Raspberry 1)"
		if [ "$downlink" = "0" ]
			then 
				make lora_gateway
			else
				make lora_gateway_downlink 
		fi	
elif [ "$board" = "12" ]
	then
		echo "You have a Raspberry Zero W"
		echo "Compiling for Raspberry Zero W (same as Raspberry 1)"
		if [ "$downlink" = "0" ]
			then 
				make lora_gateway
			else
				make lora_gateway_downlink 
		fi	
elif [ "$board" = "17" ]
	then
		echo "You have a Raspberry 4B"
		echo "not supported yet"
		#echo "Compiling for Raspberry Zero W (same as Raspberry 1)"
		#if [ "$downlink" = "0" ]
		#	then 
		#		make lora_gateway
		#	else
		#		make lora_gateway_downlink 
		#fi
else
	echo "Not support, trying"		
	echo "Compiling for Raspberry 1"
	if [ "$downlink" = "0" ]
		then 
			make lora_gateway
		else
			make lora_gateway_downlink 
	fi	
fi

#board=`cat /proc/cpuinfo | grep "Revision" | cut -d ':' -f 2 | tr -d " \t\n\r"`	
	
#if [ "$board" = "a01040" ] || [ "$board" = "a01041" ] || [ "$board" = "a21041" ] || [ "$board" = "a22042" ]
#	then
#		echo "You have a Raspberry 2"
#		echo "Compiling for Raspberry 2 and 3"
#		if [ "$downlink" = "0" ]
#			then 
# 				make lora_gateway_pi2
# 			else
# 				make lora_gateway_pi2_downlink 
# 		fi		
# elif [ "$board" = "a02082" ] || [ "$board" = "a22082" ] || [ "$board" = "a32082" ] || [ "$board" = "a52082" ] || [ "$board" = "a22083" ]
# 	then
# 		echo "You have a Raspberry 3B"
# 		echo "Compiling for Raspberry 2 and 3"
# 		if [ "$downlink" = "0" ]
# 			then 
# 				make lora_gateway_pi2
# 			else
# 				make lora_gateway_pi2_downlink 
# 		fi	
# elif [ "$board" = "a020d3" ]
# 	then
# 		echo "You have a Raspberry 3B+"
# 		echo "Compiling for Raspberry 2 and 3"
# 		if [ "$downlink" = "0" ]
# 			then 
# 				make lora_gateway_pi2
# 			else
# 				make lora_gateway_pi2_downlink 
# 		fi			
# elif [ "$board" = "900092" ] || [ "$board" = "900093" ]
# 	then
# 		echo "You have a Raspberry Zero"
# 		echo "Compiling for Raspberry Zero (same as Raspberry 1)"
# 		if [ "$downlink" = "0" ]
# 			then 
# 				make lora_gateway
# 			else
# 				make lora_gateway_downlink 
# 		fi	
# elif [ "$board" = "9000c1" ]
# 	then
# 		echo "You have a Raspberry Zero W"
# 		echo "Compiling for Raspberry Zero W (same as Raspberry 1)"
# 		if [ "$downlink" = "0" ]
# 			then 
# 				make lora_gateway
# 			else
# 				make lora_gateway_downlink 
# 		fi	
# elif [ "$board" = "a03111" ] || [ "$board" = "b03111" ] || [ "$board" = "c03111" ]
# 	then
# 		echo "You have a Raspberry 4B"
# 		echo "not supported yet"
# 		#echo "Compiling for Raspberry Zero W (same as Raspberry 1)"
# 		#if [ "$downlink" = "0" ]
# 		#	then 
# 		#		make lora_gateway
# 		#	else
# 		#		make lora_gateway_downlink 
# 		#fi
# else
# 	echo "You might have a Raspberry 1"		
# 	echo "Compiling for Raspberry 1"
# 	if [ "$downlink" = "0" ]
# 		then 
# 			make lora_gateway
# 		else
# 			make lora_gateway_downlink 
# 	fi	
# fi

echo "update of gateway done."
sudo chown -R pi:pi /home/pi/lora_gateway/
echo "if it is a new installation, go into lora_gateway/scripts folder"
echo "and run ./basic_config_gw.sh"
