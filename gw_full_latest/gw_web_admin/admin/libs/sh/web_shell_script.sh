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
			if [ $3 == "gateway_ID" ]
			then
				echo "$4" > /home/pi/lora_gateway/gateway_id.txt
				tr -d '\n' < /home/pi/lora_gateway/gateway_id.txt | md5sum - | cut -d ' ' --field=1 > /home/pi/lora_gateway/gateway_id.md5
			fi
			
		else
			jq '.'"$2"'.'"$3"' = '$4' ' /home/pi/lora_gateway/gateway_conf.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/gateway_conf.json	
		fi
		sudo chown -R pi:pi /home/pi/lora_gateway/
		sudo chmod +r /home/pi/lora_gateway/gateway_conf.json
	fi
fi

if [ "$1" = "paboost" ]
then
	###################################
	# update paboost
	###################################

	if [ $2 == "Enabled" ]
	then
		sudo sed -i 's/#*CFLAGS/CFLAGS/g' /home/pi/lora_gateway/radio.makefile	
	else
		sudo sed -i 's/^CFLAGS/#CFLAGS/g' /home/pi/lora_gateway/radio.makefile
	fi	
fi	


if [ "$1" = "install_gw" ]
then

	wget -q --spider http://google.com

	if [ $? -eq 0 ]
	then
		#online
		cd /home/pi
		
		if [ -f lora_gateway/scripts/update_gw.sh ]
		then
			cp lora_gateway/scripts/update_gw.sh .
		elif [ -f scripts/update_gw.sh ]
		then
			cp scripts/update_gw.sh .
		else
			wget https://raw.githubusercontent.com/CongducPham/LowCostLoRaGw/master/gw_full_latest/scripts/update_gw.sh	
		fi
		
		chmod +x update_gw.sh
			
		mv lora_gateway lora_gateway_bak
		./update_gw.sh
		
		#test if a lora_gateway folder has been created
		if [ -d lora_gateway ]
		then
			rm -rf lora_gateway_bak
		else
			mv lora_gateway_bak lora_gateway
		fi		
		
		rm update_gw.sh
	fi
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

if [ "$1" = "cloudnointernet_conf" ]
then
	###################################
	# Configure CloudNoInternet
	###################################

	tmp=$(mktemp)
	if [ $# == 3 ] 
	then
		jq '.clouds=([.clouds[]  | select(.script == "python CloudNoInternet.py") .'$2' = '$3'])' /home/pi/lora_gateway/clouds.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/clouds.json
		sudo chown -R pi:pi /home/pi/lora_gateway/
		sudo chmod +r /home/pi/lora_gateway/clouds.json
	fi
fi

if [ "$1" = "cloudgpsfile_conf" ]
then
	###################################
	# Configure CloudGpsFile
	###################################

	tmp=$(mktemp)
	if [ $# == 3 ] 
	then
		jq '.clouds=([.clouds[]  | select(.script == "python CloudGpsFile.py") .'$2' = '$3'])' /home/pi/lora_gateway/clouds.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/clouds.json
		sudo chown -R pi:pi /home/pi/lora_gateway/
		sudo chmod +r /home/pi/lora_gateway/clouds.json
	fi
fi

if [ "$1" = "cloudmqtt_conf" ]
then
	###################################
	# Configure CloudMQTT
	###################################

	tmp=$(mktemp)
	if [ $# == 3 ] 
	then
		jq '.clouds=([.clouds[]  | select(.script == "python CloudMQTT.py") .'$2' = '$3'])' /home/pi/lora_gateway/clouds.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/clouds.json
		sudo chown -R pi:pi /home/pi/lora_gateway/
		sudo chmod +r /home/pi/lora_gateway/clouds.json
	fi
fi

if [ "$1" = "cloudnodered_conf" ]
then
	###################################
	# Configure CloudNodeRed
	###################################

	tmp=$(mktemp)
	if [ $# == 3 ] 
	then
		jq '.clouds=([.clouds[]  | select(.script == "python CloudNodeRed.py") .'$2' = '$3'])' /home/pi/lora_gateway/clouds.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/clouds.json
		sudo chown -R pi:pi /home/pi/lora_gateway/
		sudo chmod +r /home/pi/lora_gateway/clouds.json
	fi
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

	#Replacing data in /home/pi/lora_gateway/key_ThingSpeak.p
	if [ $2 = "source_list" ]
	then
		sudo sed -i 's/^source_list.*/source_list='"$3"'/g'''  /home/pi/lora_gateway/key_ThingSpeak.py
	else
		sudo sed -i 's/^_def_thingspeak_channel_key.*/_def_thingspeak_channel_key=\"'"$2"'\"/g' /home/pi/lora_gateway/key_ThingSpeak.py
	fi
	
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
		jq '.clouds=([.clouds[]  | select(.script == "python CloudWAZIUP.py") .'$2' = '$3'])' /home/pi/lora_gateway/clouds.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/clouds.json
		sudo chown -R pi:pi /home/pi/lora_gateway/
		sudo chmod +r /home/pi/lora_gateway/clouds.json
	fi
fi


if [ "$1" = "mqtt_key" ]
then
	if [ "$2" = "MQTT_server" ]
	then
		sudo sed -i 's/^MQTT_server.*/MQTT_server=\"'"$3"'\"/g'''  /home/pi/lora_gateway/key_MQTT.py
	elif [ "$2" = "project_name" ]
	then
		sudo sed -i 's/^project_name.*/project_name=\"'"$3"'\"/g'''  /home/pi/lora_gateway/key_MQTT.py
	elif [ "$2" = "organization_name" ]
	then
		sudo sed -i 's/^organization_name.*/organization_name=\"'"$3"'\"/g'''  /home/pi/lora_gateway/key_MQTT.py
	elif [ "$2" = "sensor_name" ]
	then
		sudo sed -i 's/^sensor_name.*/sensor_name=\"'"$3"'\"/g'''  /home/pi/lora_gateway/key_MQTT.py
	elif [ "$2" = "source_list" ]
	then
		sudo sed -i 's/^source_list.*/source_list='"$3"'/g'''  /home/pi/lora_gateway/key_MQTT.py
	fi
fi


if [ "$1" = "waziup_key" ]
then
	###################################
	# Configure Orion Key
	###################################
	#Replacing data in /home/pi/lora_gateway/key_Orion.p
	if [ $2 = "source_list" ]
	then
		sudo sed -i 's/^source_list.*/source_list='"$3"'/g'''  /home/pi/lora_gateway/key_WAZIUP.py
	elif [ $2 == "project_name" ] 
	then
		sudo sed -i 's/^project_name.*/project_name=\"'"$3"'\"/g' /home/pi/lora_gateway/key_WAZIUP.py
	elif [ $2 == "organization_name" ]
	then
		sudo sed -i 's/^organization_name.*/organization_name=\"'"$3"'\"/g' /home/pi/lora_gateway/key_WAZIUP.py
	elif [ $2 == "service_tree" ]
	then
		sudo sed -i 's/^service_tree.*/service_tree=\"'"$3"'\"/g' /home/pi/lora_gateway/key_WAZIUP.py
	elif [ $2 == "auth_token" ]
	then
		sudo sed -i 's/^auth_token.*/auth_token=\"'"$3"'\"/g' /home/pi/lora_gateway/key_WAZIUP.py	
	fi
fi

if [ "$1" = "gpsfile_key" ]
then
	###################################
	# Configure GpsFile Key
	###################################

	if [ $2 == "project_name" ] 
	then
		sudo sed -i 's/^project_name.*/project_name=\"'"$3"'\"/g' /home/pi/lora_gateway/key_GpsFile.py
	elif [ $2 == "organization_name" ]
	then
		sudo sed -i 's/^organization_name.*/organization_name=\"'"$3"'\"/g' /home/pi/lora_gateway/key_GpsFile.py
	elif [ $2 == "sensor_name" ]
	then
		sudo sed -i 's/^sensor_name.*/sensor_name=\"'"$3"'\"/g' /home/pi/lora_gateway/key_GpsFile.py
	elif [ $2 == "source_list" ]
	then
		sudo sed -i 's/^source_list.*/source_list='"$3"'/g''' /home/pi/lora_gateway/key_GpsFile.py 
	elif [ $2 == "active_interval_minutes" ]
	then
		sudo sed -i 's/^active_interval_minutes.*/active_interval_minutes='$3$n'/g' /home/pi/lora_gateway/key_GpsFile.py	
	elif [ $2 == "sms" ]
	then
		if [ $3 == "true" ]
		then
			sudo sed -i 's/^SMS.*/SMS=True/g' /home/pi/lora_gateway/key_GpsFile.py	
		else
			sudo sed -i 's/^SMS.*/SMS=False/g' /home/pi/lora_gateway/key_GpsFile.py
		fi
	elif [ $2 == "pin" ]
	then
		sudo sed -i 's/^PIN.*/PIN=\"'"$3"'\"/g' /home/pi/lora_gateway/key_GpsFile.py
	elif [ $2 == "contacts" ]
	then
		sudo sed -i 's/^contacts.*/contacts='"$3"'/g' /home/pi/lora_gateway/key_GpsFile.py
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
	sudo sed -i 's/^wpa_passphrase.*/wpa_passphrase='"$3"'/g' /etc/hostapd/hostapd.conf
	#indicate that a custom ssid has been defined by the user
	echo "$2" | sudo tee /etc/hostapd/custom_ssid.txt > /dev/null
fi

if [ "$1" = "wificlient" ]
then
	###################################
	# configure wifi client
	###################################
	cd /home/pi/lora_gateway/scripts
	./prepare_wifi_client.sh
	#adding the WiFi network into /etc/wpa_supplicant/wpa_supplicant.conf
	wpa_passphrase "$2" "$3" | sudo tee -a /etc/wpa_supplicant/wpa_supplicant.conf > /dev/null
fi

if [ "$1" = "apmode" ]
then
	###################################
	# configure wifi client
	###################################
	cd /home/pi/lora_gateway/scripts
	./prepare_access_point.sh
fi

if [ "$1" = "apmodenow" ]
then
	###################################
	# configure wifi client
	###################################
	cd /home/pi/lora_gateway/scripts
	./start_access_point.sh
fi
