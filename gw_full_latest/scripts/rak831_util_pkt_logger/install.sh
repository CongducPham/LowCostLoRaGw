#!/bin/bash

echo "Installing util_pkt_logger_formatter for RAK831 gateway"
echo "-------------------------------------------------------"
echo "You MUST have installed the RAK831 related software from"
echo "https://github.com/RAKWireless/RAK2245-RAK831-LoRaGateway-RPi-Raspbian-OS"
echo "cd /home/pi; git clone https://github.com/RAKWireless/RAK2245-RAK831-LoRaGateway-RPi-Raspbian-OS.git RAK831"
echo "cd RAK831/lora; sudo ./install"
echo "If you encounter error with apt-get install dialog, then edit RAK831's install.sh and comment the corresponding line"
echo "Continue (y/n)"
read input

if [ "$input" = "y" ] || [ "$input" = "Y" ]
	then
		if [ -d "/opt/ttn-gateway/lora_gateway/util_pkt_logger" ]
			then
				echo "/opt/ttn_gateway folder exists - ok"		
				
				echo "copy util_pkt_logger_formatter.py in /home/pi/lora_gateway"
				cp util_pkt_logger_formatter.py /home/pi/lora_gateway
				echo "copy start_upl_pprocessing_gw.sh in /home/pi/lora_gateway"
				cp start_upl_pprocessing_gw.sh /home/pi/lora_gateway
				echo "copy test_upl_pprocessing_gw.sh in /home/pi/lora_gateway"
				cp test_upl_pprocessing_gw.sh /home/pi/lora_gateway				
				echo "renaming original util_pkt_logger.c into util_pkt_logger_original.c"
				sudo mv /opt/ttn-gateway/lora_gateway/util_pkt_logger/src/util_pkt_logger.c /opt/ttn-gateway/lora_gateway/util_pkt_logger/src/util_pkt_logger_original.c 
				echo "copy util_pkt_logger.c in /opt/ttn-gateway/lora_gateway/util_pkt_logger/src"
				sudo cp util_pkt_logger.c /opt/ttn-gateway/lora_gateway/util_pkt_logger/src
				echo "recompiling util_pkt_logger..."
				cd /opt/ttn-gateway/lora_gateway/util_pkt_logger
				sudo make
				
				echo
				echo "Disabling original ttn-gateway service"
				sudo systemctl disable ttn-gateway.service

				echo				
				echo "*******************************"
				echo "*** run gateway at boot Y/N ***"
				echo "*******************************"
				read input

				# we always remove so that there will be no duplicate lines
				echo "Removing /home/pi/lora_gateway/scripts/start_gw.sh in /etc/rc.local if any"
				sudo sed -i '\/home\/pi\/lora_gateway\/scripts\/start_gw.sh/d' /etc/rc.local
				echo "Removing /home/pi/lora_gateway/start_upl_pprocessing_gw.sh in /etc/rc.local if any"
				sudo sed -i '\/home\/pi\/lora_gateway\/start_upl_pprocessing_gw.sh/d' /etc/rc.local

				if [ "$input" = "y" ] || [ "$input" = "Y" ]
					then
						echo "Add /home/pi/lora_gateway/scripts/start_gw.sh in /etc/rc.local, but without starting the gateway"
						sudo sed -i 's/^exit 0/\/home\/pi\/lora_gateway\/scripts\/start_gw.sh startup_only\nexit 0/g' /etc/rc.local
						echo "Add /home/pi/lora_gateway/start_upl_pprocessing_gw in /etc/rc.local"
						sudo sed -i 's/^exit 0/\/home\/pi\/lora_gateway\/start_upl_pprocessing_gw.sh wait 2> \/home\/pi\/lora_gateway\/start_upl.log\nexit 0/g' /etc/rc.local
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
				echo "/opt/ttn_gateway/lora_gateway/util_pkt_logger folder does not exist"
				echo "Please, install the RAK831 related software first"
				echo "cd /home/pi; git clone https://github.com/RAKWireless/RAK2245-RAK831-LoRaGateway-RPi-Raspbian-OS.git RAK831"
				echo "cd RAK831/lora; sudo ./install"
				echo "If you encounter error with apt-get install dialog, then edit RAK831's install.sh and comment the corresponding line"
				exit 1
		fi		
	else
		echo "Installation cancelled"
		exit 1		
fi		
	



