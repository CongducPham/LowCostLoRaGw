#!/bin/bash

if [ "$1" = "test" ]
then
	echo "test"
fi

if [ "$1" = "contact_mail" ]
then
	###################################
	# configure gateway - contact_mail
	###################################

	#sudo ./contact_mail.sh aaa@gmail.com,bbb@gmail.com,ccc@gmail.com,ddd@gmail.com

	tmp=$(mktemp)

	jq '.alert_conf.contact_mail="'$2'" ' /home/pi/lora_gateway/gateway_conf.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/gateway_conf.json

	sudo chown -R pi:pi /home/pi/lora_gateway/
	sudo chmod +r /home/pi/lora_gateway/gateway_conf.json

fi

if [ "$1" = "contact_sms" ]
then
	###################################
	# configure gateway - contact_sms
	###################################

	tmp=$(mktemp)

	jq '.alert_conf.contact_sms='$2' ' /home/pi/lora_gateway/gateway_conf.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/gateway_conf.json

	sudo chown -R pi:pi /home/pi/lora_gateway/
	sudo chmod +r /home/pi/lora_gateway/gateway_conf.json
fi

if [ "$1" = "copy_log_file" ]
then
	sudo cp /home/pi/lora_gateway/log/post-processing.log /var/www/html/admin/log
	sudo tail -n 500 /home/pi/lora_gateway/log/post-processing.log > /var/www/html/admin/log/post-processing-500L.log
	sudo chown -R pi:www-data /var/www/html/admin/log
fi

if [ "$1" = "downlink_request" ]
then
	cd /home/pi/lora_gateway/downlink
	echo "{\"status\":\"send_request\",\"dst\":$2,\"data\":\"$3\"}" >> downlink-post.txt
	sudo chown -R pi:pi /home/pi/lora_gateway/
fi

if [ "$1" = "gateway_conf" ]
then
	###################################
	# configure gateway
	###################################

	#For example to update channel number: radio_conf => ch => value
	#jq '.radio_conf.ch = 12' gateway_conf.json > "$tmp" && mv "$tmp" gateway_conf.json

	tmp=$(mktemp)
	if [ $# == 4 ] 
	then
		if [ $3 == "gateway_ID" ] || [ $3 == "contact_mail" ] || [ $3 == "mail_from" ] || [ $3 == "mail_passwd" ] || [ $3 == "mail_server" ] || [ $3 == "pin" ]
		then
				jq '.'"$2"'.'"$3"' = "'$4'" ' /home/pi/lora_gateway/gateway_conf.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/gateway_conf.json
		else
				jq '.'"$2"'.'"$3"' = '$4' ' /home/pi/lora_gateway/gateway_conf.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/gateway_conf.json
		fi
		sudo chown -R pi:pi /home/pi/lora_gateway/
		sudo chmod +r /home/pi/lora_gateway/gateway_conf.json
	fi
fi

if [ "$1" = "install_gw" ]
then
	cd /home/pi
	cp lora_gateway/scripts/update_gw.sh .
	rm -r lora_gateway
	./update_gw.sh
	rm update_gw.sh
fi

if [ "$1" = "low_level_gw_status" ]
then
	grep -a "Low-level gw status ON" /home/pi/Dropbox/LoRa-test/post-processing.log | tail -1
fi

if [ "$1" = "set_profile" ]
then
	tmp=$(mktemp)
	jq '.username="'$2'" ' /etc/gw_web_admin/database.json > "$tmp" && mv "$tmp" /etc/gw_web_admin/database.json
	jq '.password="'$3'" ' /etc/gw_web_admin/database.json > "$tmp" && mv "$tmp" /etc/gw_web_admin/database.json
	sudo chmod +r /etc/gw_web_admin/database.json
fi

if [ "$1" = "thingspeak_conf" ]
then
	###################################
	# Configure ThingSpeak
	###################################

	#For example to update write_key for ThingSpeak cloud: 
	# jq '.[] | map((select(.script == "python CloudThingSpeak.py") | .write_key) |= "")' clouds.json
	tmp=$(mktemp)
	if [ $# == 3 ] 
	then
		jq '.clouds=([.clouds[]  | select(.script == "python CloudThingSpeak.py") .'$2' = '$3'])' /home/pi/lora_gateway/clouds.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/clouds.json
		sudo chown -R pi:pi /home/pi/lora_gateway/
		sudo chmod +r /home/pi/lora_gateway/clouds.json
	fi
fi

if [ "$1" = "thingspeak_key" ]
then
	###################################
	# Configure ThingSpeak Write Key
	###################################

	#Replacing data in /home/pi/lora_gateway/key_ThingSpeak.py
	sudo sed -i 's/^_def_thingspeak_channel_key.*/_def_thingspeak_channel_key=\"'"$2"'\"/g' /home/pi/lora_gateway/key_ThingSpeak.py
fi

if [ "$1" = "update_gw_file" ]
then
	cd /home/pi/lora_gateway
	wget --backups=1 $2 
	sudo chown -R pi:pi /home/pi/lora_gateway/
fi

if [ "$1" = "waziup_conf" ]
then
	#For example to update write_key for ThingSpeak cloud: 
	# jq '.[] | map((select(.script == "python CloudThingSpeak.py") | .write_key) |= "")' clouds.json

	tmp=$(mktemp)
	if [ $# == 3 ] 
	then
			jq '.clouds=([.clouds[]  | select(.script == "python CloudOrion.py") .'$2' = '$3'])' /home/pi/lora_gateway/clouds.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/clouds.json
			sudo chown -R pi:pi /home/pi/lora_gateway/
			sudo chmod +r /home/pi/lora_gateway/clouds.json
	fi
fi

if [ "$1" = "waziup_key" ]
then
	###################################
	# Configure Orion Key
	###################################

	if [ $2 == "project_name" ] 
	then
		sudo sed -i 's/^project_name.*/project_name=\"'"$3"'\"/g' /home/pi/lora_gateway/key_Orion.py
	elif [ $2 == "organization_name" ]
	then
		sudo sed -i 's/^organization_name.*/organization_name=\"'"$3"'\"/g' /home/pi/lora_gateway/key_Orion.py
	else #service_tree
		sudo sed -i 's/^service_tree.*/service_tree=\"'"$3"'\"/g' /home/pi/lora_gateway/key_Orion.py
	fi
fi

if [ "$1" = "wifi" ]
then
	###################################
	# configure hostapd.conf
	###################################

	#Replacing hot-spot ssid in /etc/hostapd/hostapd.conf
	sudo sed -i 's/^ssid.*/ssid='"$2"'/g' /etc/hostapd/hostapd.conf
	#Setting wpa_passphrase in /etc/hostapd/hostapd.conf
	sudo sed -i 's/^wpa_passphrase.*/wpa_passphrase='"$2"'/g' /etc/hostapd/hostapd.conf
fi

