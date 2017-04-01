#!/bin/sh

echo "updating gateway"
cd /home/pi
echo "removing any existing gw_full_latest folder (from previous update)"
rm -rf gw_full_latest
echo "getting new gw_full_latest from github"
svn checkout https://github.com/CongducPham/LowCostLoRaGw/trunk/gw_full_latest

if [ -d lora_gateway ]
	then
		echo "detecting an existing lora_gateway folder"
		cd gw_full_latest
		echo "preserving your key files"
		rm key_*
		echo "preserving your configuration files"
		rm gateway_conf.json clouds.json radio.makefile
		echo "copying new distrib into /home/pi/lora_gateway"
		cp -r * /home/pi/lora_gateway
	else
		echo "new installation"
		echo "simply renaming gw_full_latest in lora_gateway"
		mv gw_full_latest lora_gateway	
fi	
cd /home/pi/lora_gateway
echo "compiling the gateway program"
board=`cat /proc/cpuinfo | grep "Revision" | cut -d ':' -f 2 | tr -d " \t\n\r"`
if [ "$board" = "a01041" ] || [ "$board" = "a21041" ]
        then
                echo "You have a Raspberry 2"
                echo "Compiling for Raspberry 2 and 3"
                make lora_gateway_pi2
elif [ "$board" = "a02082" ] || [ "$board" = "a22082" ]
        then
                echo "You have a Raspberry 3"
                echo "Compiling for Raspberry 2 and 3"
                make lora_gateway_pi2
else
        echo "You have a Raspberry 1"
        echo "Compiling for Raspberry 1"
        make lora_gateway
fi
echo "update of gateway done."
echo "if it is a new installation, go into lora_gateway/scripts folder"
echo "and run ./config_gw.sh"
