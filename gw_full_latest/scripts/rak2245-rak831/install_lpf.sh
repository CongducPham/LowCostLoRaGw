#!/bin/bash

echo "Installing lora_pkt_fwd_formatter for RAK831/RAK2245 gateway"
echo "---------------------------------------------------------------"
echo "Normally you should have installed the RAK831/2245 related software from RAK"
echo "We provide an install_rak_lora.sh script that performs a light installation that it is recommended to use"
echo "Continue with installing lora_pkt_fwd_formatter (y/n)"
read input

if [ "$input" = "y" ] || [ "$input" = "Y" ]
	then
		if [ -d "/opt/ttn-gateway/packet_forwarder/lora_pkt_fwd" ]
			then
				SCRIPT_DIR=$(pwd)
				
				echo "/opt/ttn_gateway folder exists - ok"		
				
				echo "copy lora_pkt_fwd_formatter.py in /home/pi/lora_gateway"
				cp lora_pkt_fwd_formatter.py /home/pi/lora_gateway
				echo "copy start_lpf_pprocessing_gw.sh in /home/pi/lora_gateway"
				cp start_lpf_pprocessing_gw.sh /home/pi/lora_gateway
				
				is_modified_version=`grep DISABLE_DATA_UPLINK_UDP /opt/ttn-gateway/packet_forwarder/lora_pkt_fwd/src/lora_pkt_fwd.c`
				
				if [ "$is_modified_version" = "" ]
					then							
						echo "renaming original lora_pkt_fwd.c into lora_pkt_fwd_original.c"
						sudo mv /opt/ttn-gateway/packet_forwarder/lora_pkt_fwd/src/lora_pkt_fwd.c /opt/ttn-gateway/packet_forwarder/lora_pkt_fwd/src/lora_pkt_fwd_original.c 
						echo "copy lora_pkt_fwd.c in /opt/ttn-gateway/packet_forwarder/lora_pkt_fwd/src"
						sudo cp lora_pkt_fwd.c /opt/ttn-gateway/packet_forwarder/lora_pkt_fwd/src
						echo "recompiling lora_pkt_fwd...."
						cd /opt/ttn-gateway/packet_forwarder/lora_pkt_fwd
						sudo make
				fi
				echo
				echo "Disabling original ttn-gateway service"
				sudo systemctl disable ttn-gateway.service
				
				echo "Copying global_conf.json into /opt/ttn-gateway/packet_forwarder/lora_pkt_fwd/"
				cd $SCRIPT_DIR
				sudo cp global_conf-ttn.json /opt/ttn-gateway/packet_forwarder/lora_pkt_fwd/global_conf.json				

				echo				
				echo "*******************************************************************"
				echo "*** configure gateway for LoRaWAN Network Server (e.g. TTN) Y/N ***"
				echo "*******************************************************************"
				read input

				if [ "$input" = "y" ] || [ "$input" = "Y" ]
					then
						tmp=$(mktemp)
						
						#change LoRa mode to 11 in gateway_conf.json
						echo "Setting mode to 11 for LoRaWAN"
						jq '.radio_conf.mode = 11' /home/pi/lora_gateway/gateway_conf.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/gateway_conf.json
						
						#force ttn_status to true
						echo "Enabling TTN status report"
 						jq '.status_conf.ttn_status = true' /home/pi/lora_gateway/gateway_conf.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/gateway_conf.json						
						
						#change raw to true
						echo "Setting raw mode to accept LoRaWAN packet"
 						jq '.gateway_conf.raw = true' /home/pi/lora_gateway/gateway_conf.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/gateway_conf.json

						#change aes_lorawan to false
						echo "Disabling local AES LoRaWAN to push encrypted data to TTN"
						echo "If you want to locally decrypt LoRaWAN packets, edit key_LoRaWAN.py and provide NwkSKey/AppSKey for devices"
 						jq '.gateway_conf.aes_lorawan = false' /home/pi/lora_gateway/gateway_conf.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/gateway_conf.json
				 						
						#get gateway id
						/home/pi/lora_gateway/scripts/create_gwid.sh
						gatewayid=`cat /home/pi/lora_gateway/gateway_id.txt`
						gatewayttnid="${gatewayid:4:6}FFFF${gatewayid:10}"
						echo "Register gateway on TTN console ID $gatewayttnid using legacy packet_forwarder option"				
				fi
				
				echo				
				echo "***********************************"
				echo "*** run lpf gateway at boot Y/N ***"
				echo "***********************************"
				read input
				
				if [ "$input" = "y" ] || [ "$input" = "Y" ]
					then
						# we always remove so that there will be no duplicate lines
						echo "Removing /home/pi/lora_gateway/scripts/start_gw.sh in /etc/rc.local if any"
						sudo sed -i '\/home\/pi\/lora_gateway\/scripts\/start_gw.sh/d' /etc/rc.local
						echo "Removing /home/pi/lora_gateway/start_upl_pprocessing_gw.sh in /etc/rc.local if any"
						sudo sed -i '\/home\/pi\/lora_gateway\/start_upl_pprocessing_gw.sh/d' /etc/rc.local
						echo "Removing /home/pi/lora_gateway/start_lpf_pprocessing_gw.sh in /etc/rc.local if any"
						sudo sed -i '\/home\/pi\/lora_gateway\/start_lpf_pprocessing_gw.sh/d' /etc/rc.local
											
						echo "Add /home/pi/lora_gateway/scripts/start_gw.sh in /etc/rc.local, but without starting the gateway"
						sudo sed -i 's/^exit 0/\/home\/pi\/lora_gateway\/scripts\/start_gw.sh startup_only\nexit 0/g' /etc/rc.local
						echo "Add /home/pi/lora_gateway/start_lpf_pprocessing_gw in /etc/rc.local"
						sudo sed -i 's/^exit 0/\/home\/pi\/lora_gateway\/start_lpf_pprocessing_gw.sh wait 2> \/home\/pi\/lora_gateway\/start_lpf.log\nexit 0/g' /etc/rc.local
				fi
				
				echo				
				echo "Installation completed."
				echo 
				echo "You should reboot your Raspberry"
				echo "*******************"
				echo "*** reboot Y/N  ***"
				echo "*******************"
				read input

				if [ "$input" = "y" ] || [ "$input" = "Y" ]
					then
						echo "Rebooting, please reconnect"
						sudo shutdown -r now
				fi
				
			else
				echo "/opt/ttn_gateway/packet_forwarder/lora_pkt_fwd folder does not exist"
				echo "Please, install the RAK831/2245 related software first"
				echo "We provide an install_rak_lora.sh script that performs a light installation that it is recommended to use"
				exit 1
		fi		
	else
		echo "Installation cancelled"
		exit 1		
fi		
	



