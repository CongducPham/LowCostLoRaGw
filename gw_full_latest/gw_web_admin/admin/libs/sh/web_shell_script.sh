#!/bin/bash

if [ "$1" = "test" ]
then
	echo "test"
fi

if [ "$1" = "low_level_gw_status" ]
then
	grep -a "Low-level gw status ON" /home/pi/Dropbox/LoRa-test/post-processing.log | tail -1
fi

#//////////////////////////////////////////////////////////////////////////////////////////////////////
# gateway conf radio
#//////////////////////////////////////////////////////////////////////////////////////////////////////

if [ "$1" = "download_sx1301_conf" ]
then
	cd /var/www/html/admin/log	
	
	fname=`basename $2`
	
	wget --backups=1 $2
	
	mv $fname global_conf.json
	
	cp global_conf.json /opt/ttn-gateway/packet_forwarder/lora_pkt_fwd
	 
	sudo chown -R pi:www-data /var/www/html/admin/log
fi

	
if [ "$1" = "paboost" ]
then
	###################################
	# update paboost
	###################################

	if [ $2 == "true" ]
	then
		sudo sed -i 's/#*CFLAGS+=-DPABOOST/CFLAGS+=-DPABOOST/g' /home/pi/lora_gateway/radio.makefile	
	else
		sudo sed -i 's/^CFLAGS+=-DPABOOST/#CFLAGS+=-DPABOOST/g' /home/pi/lora_gateway/radio.makefile
	fi	
fi

#//////////////////////////////////////////////////////////////////////////////////////////////////////
# gateway lorawan conf
#//////////////////////////////////////////////////////////////////////////////////////////////////////

if [ "$1" = "gw_lorawan_conf" ]
then
	###################################
	# configure gateway for LoRaWAN
	###################################

	#For example to update channel number: radio_conf => ch => value
	#jq '.radio_conf.ch = 12' gateway_conf.json > "$tmp" && mv "$tmp" gateway_conf.json
	tmp=$(mktemp)

	#set LoRa mode to 11
	jq '.'"radio_conf"'.'"mode"' = '11' ' /home/pi/lora_gateway/gateway_conf.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/gateway_conf.json
	#set raw format to true
	jq '.'"gateway_conf"'.'"raw"' = 'true' ' /home/pi/lora_gateway/gateway_conf.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/gateway_conf.json
	#set aes_lorawan to false
	jq '.'"gateway_conf"'.'"aes_lorawan"' = 'false' ' /home/pi/lora_gateway/gateway_conf.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/gateway_conf.json
	
	status_interval=`jq ".gateway_conf.status" /home/pi/lora_gateway/gateway_conf.json`
	
	if [ $status_interval == 0 ]
	then
		#set status to 600s(5min)
		jq '.'"gateway_conf"'.'"status"' = '600' ' /home/pi/lora_gateway/gateway_conf.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/gateway_conf.json
	fi

	#disable non-LoRaWAN downlink mechanism
	jq '.'"gateway_conf"'.'"downlink"' = '0' ' /home/pi/lora_gateway/gateway_conf.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/gateway_conf.json
	#set downlink_lorawan to true
	jq '.'"gateway_conf"'.'"downlink_lorawan"' = 'true' ' /home/pi/lora_gateway/gateway_conf.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/gateway_conf.json

	lorawan_server="127.0.0.1"
			
	#enable TTN settings if needed
	if [ $(($2 & 1)) == 1 ]
	then
		jq '.lorawan_encrypted_clouds=([.lorawan_encrypted_clouds[]  | select(.script == "python CloudTTN.py") .'enabled' = 'true'])' /home/pi/lora_gateway/clouds.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/clouds.json
		#set ttn_status to true
		jq '.'"status_conf"'.'"ttn_status"' = 'true' ' /home/pi/lora_gateway/gateway_conf.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/gateway_conf.json
		#read lorawan_server from key_TTN.py
		lorawan_server=`grep "lorawan_server" /home/pi/lora_gateway/key_TTN.py | cut -d '=' -f 2 | tr -d '"'`
	else
		jq '.lorawan_encrypted_clouds=([.lorawan_encrypted_clouds[]  | select(.script == "python CloudTTN.py") .'enabled' = 'false'])' /home/pi/lora_gateway/clouds.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/clouds.json
		#set ttn_status to false
		jq '.'"status_conf"'.'"ttn_status"' = 'false' ' /home/pi/lora_gateway/gateway_conf.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/gateway_conf.json
	fi

	#enable ChirpStack settings if needed
	if [ $(($2 & 2)) == 2 ]
	then
		jq '.lorawan_encrypted_clouds=([.lorawan_encrypted_clouds[]  | select(.script == "python CloudChirpStack.py") .'enabled' = 'true'])' /home/pi/lora_gateway/clouds.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/clouds.json
		#set cs_status to true
		jq '.'"status_conf"'.'"cs_status"' = 'true' ' /home/pi/lora_gateway/gateway_conf.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/gateway_conf.json
		#read lorawan_server from key_ChirpStack.py
		lorawan_server=`grep "lorawan_server" /home/pi/lora_gateway/key_ChirpStack.py | cut -d '=' -f 2 | tr -d '"'`		
	else
		jq '.lorawan_encrypted_clouds=([.lorawan_encrypted_clouds[]  | select(.script == "python CloudChirpStack.py") .'enabled' = 'false'])' /home/pi/lora_gateway/clouds.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/clouds.json
		#set cs_status to false
		jq '.'"status_conf"'.'"cs_status"' = 'false' ' /home/pi/lora_gateway/gateway_conf.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/gateway_conf.json
	fi

	#give priority to lorawan_server from key_ChirpStack.py if it is enabled
	jq '.'"gateway_conf"'.'"downlink_network_server"' = "'$lorawan_server'" ' /home/pi/lora_gateway/gateway_conf.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/gateway_conf.json
			
	sudo chown -R pi:pi /home/pi/lora_gateway/
	sudo chmod +r /home/pi/lora_gateway/gateway_conf.json
	sudo chmod +r /home/pi/lora_gateway/clouds.json
	
fi

#//////////////////////////////////////////////////////////////////////////////////////////////////////
# gateway conf gateway
#//////////////////////////////////////////////////////////////////////////////////////////////////////

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

if [ "$1" = "chirpstack_status" ]
then
	###################################
	# enable/disable chirpstack netserv
	###################################
	cd /home/pi/lora_gateway/scripts/chirpstack
	
	if [ $2 == "true" ]
	then
		sudo ./enable_chirpstack.sh
	else
		sudo ./disable_chirpstack.sh
	fi	
fi

if [ "$1" = "cloudchirpstack_conf" ]
then
	###################################
	# Configure CloudChirpStack
	###################################

	tmp=$(mktemp)
	if [ $# == 3 ] 
	then
		jq '.lorawan_encrypted_clouds=([.lorawan_encrypted_clouds[]  | select(.script == "python CloudChirpStack.py") .'$2' = '$3'])' /home/pi/lora_gateway/clouds.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/clouds.json
		jq '.'"status_conf"'.'"cs_status"' = '$3' ' /home/pi/lora_gateway/gateway_conf.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/gateway_conf.json
		sudo chown -R pi:pi /home/pi/lora_gateway/
		sudo chmod +r /home/pi/lora_gateway/gateway_conf.json
		sudo chmod +r /home/pi/lora_gateway/clouds.json
	fi
fi

#//////////////////////////////////////////////////////////////////////////////////////////////////////
# gateway conf alert mail
#//////////////////////////////////////////////////////////////////////////////////////////////////////

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

#//////////////////////////////////////////////////////////////////////////////////////////////////////
# gateway conf alert SMS
#//////////////////////////////////////////////////////////////////////////////////////////////////////

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

#//////////////////////////////////////////////////////////////////////////////////////////////////////
# gateway conf downlink request
#//////////////////////////////////////////////////////////////////////////////////////////////////////

if [ "$1" = "downlink_request" ]
then
	cd /home/pi/lora_gateway/downlink
	echo "{\"status\":\"send_request\",\"dst\":$2,\"data\":\"$3\"}" >> downlink-post.txt
	sudo chown -R pi:pi /home/pi/lora_gateway/
fi

#//////////////////////////////////////////////////////////////////////////////////////////////////////
# gateway conf alert get post-processing log file
#//////////////////////////////////////////////////////////////////////////////////////////////////////

if [ "$1" = "copy_log_file" ]
then
	sudo cp /home/pi/lora_gateway/log/post-processing.log /var/www/html/admin/log
	sudo tail -n 500 /home/pi/lora_gateway/log/post-processing.log > /var/www/html/admin/log/post-processing-500L.log
	sudo chown -R pi:www-data /var/www/html/admin/log
fi

#//////////////////////////////////////////////////////////////////////////////////////////////////////
# gateway update install
#//////////////////////////////////////////////////////////////////////////////////////////////////////

if [ "$1" = "get_git_version" ]
then
	sudo svn info https://github.com/CongducPham/LowCostLoRaGw/trunk/gw_full_latest | grep "Revision:" | cut -d ' ' --field=2 > /home/pi/git-VERSION.txt
fi

if [ "$1" = "no_git_version" ]
then
	sudo echo "N/A" > /home/pi/git-VERSION.txt
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

#//////////////////////////////////////////////////////////////////////////////////////////////////////
# gateway update download and install file
#//////////////////////////////////////////////////////////////////////////////////////////////////////

if [ "$1" = "download_gw_file" ]
then
	cd /home/pi/lora_gateway
	
	fname=`basename $2`
	
	case "$fname" in
	*.zip)
		if [ "$3" = "erase" ] 
		then
			sudo rm -rf *
		fi
		;;
	*)
        # it's not
        ;;
	esac	
	
	wget --backups=1 $2
	
	case "$fname" in
	*.zip)
        unzip -o $fname;;
	*)
        # it's not
        ;;
	esac
	 
	sudo chown -R pi:pi /home/pi/lora_gateway/
fi

#//////////////////////////////////////////////////////////////////////////////////////////////////////
# cloud 
#//////////////////////////////////////////////////////////////////////////////////////////////////////

if [ "$1" = "waziup_conf" ]
then
	tmp=$(mktemp)
	if [ $# == 3 ] 
	then
		jq '.clouds=([.clouds[]  | select(.script == "python CloudWAZIUP.py") .'$2' = '$3'])' /home/pi/lora_gateway/clouds.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/clouds.json
		sudo chown -R pi:pi /home/pi/lora_gateway/
		sudo chmod +r /home/pi/lora_gateway/clouds.json
	fi
fi

if [ "$1" = "waziup_key" ]
then
	###################################
	# Configure WAZIUP Key
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
	elif [ $2 == "username" ]
	then
		sudo sed -i 's/^username.*/username=\"'"$3"'\"/g' /home/pi/lora_gateway/key_WAZIUP.py	
	elif [ $2 == "password" ]
	then
		sudo sed -i 's/^password.*/password=\"'"$3"'\"/g' /home/pi/lora_gateway/key_WAZIUP.py			
	elif [ $2 == "visibility" ]
	then
		sudo sed -i 's/^visibility.*/visibility=\"'"$3"'\"/g' /home/pi/lora_gateway/key_WAZIUP.py	
	fi	
fi

if [ "$1" = "thingspeak_conf" ]
then
	###################################
	# Configure ThingSpeak
	###################################
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

if [ "$1" = "nodered_key" ]
then
	if [ "$2" = "source_list" ]
	then
		sudo sed -i 's/^source_list.*/source_list='"$3"'/g'''  /home/pi/lora_gateway/key_NodeRed.py
	fi
fi

if [ "$1" = "cloudttn_conf" ]
then
	###################################
	# Configure CloudTTN
	###################################

	tmp=$(mktemp)
	if [ $# == 3 ] 
	then
		jq '.lorawan_encrypted_clouds=([.lorawan_encrypted_clouds[]  | select(.script == "python CloudTTN.py") .'$2' = '$3'])' /home/pi/lora_gateway/clouds.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/clouds.json
		jq '.'"status_conf"'.'"ttn_status"' = '$3' ' /home/pi/lora_gateway/gateway_conf.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/gateway_conf.json
		sudo chown -R pi:pi /home/pi/lora_gateway/
		sudo chmod +r /home/pi/lora_gateway/gateway_conf.json
		sudo chmod +r /home/pi/lora_gateway/clouds.json
	fi
fi

if [ "$1" = "ttn_key" ]
then
	if [ "$2" = "source_list" ]
	then
		sudo sed -i 's/^source_list.*/source_list='"$3"'/g'''  /home/pi/lora_gateway/key_TTN.py
	fi
fi

if [ "$1" = "chirpstack_key" ]
then
	if [ "$2" = "lorawan_server" ]
	then
		sudo sed -i 's/^lorawan_server.*/lorawan_server=\"'"$3"'\"/g'''  /home/pi/lora_gateway/key_ChirpStack.py
	elif [ "$2" = "source_list" ]
	then
		sudo sed -i 's/^source_list.*/source_list='"$3"'/g'''  /home/pi/lora_gateway/key_ChirpStack.py
	fi
fi

#//////////////////////////////////////////////////////////////////////////////////////////////////////
# system GW access point
#//////////////////////////////////////////////////////////////////////////////////////////////////////

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

#//////////////////////////////////////////////////////////////////////////////////////////////////////
# system wifi client
#//////////////////////////////////////////////////////////////////////////////////////////////////////

if [ "$1" = "wificlient" ]
then
	###################################
	# configure wifi client
	###################################
	#adding the WiFi network into /etc/wpa_supplicant/wpa_supplicant.conf
	wpa_passphrase "$2" "$3" | sudo tee -a /etc/wpa_supplicant/wpa_supplicant.conf > /dev/null
	
	cd /home/pi/lora_gateway/scripts
	./prepare_wifi_client.sh	
fi

if [ "$1" = "wificlientnow" ]
then
	###################################
	# start wifi client now
	###################################
	cd /home/pi/lora_gateway/scripts
	./stop_access_point.sh	
fi

#//////////////////////////////////////////////////////////////////////////////////////////////////////
# system switch to AP mode
#//////////////////////////////////////////////////////////////////////////////////////////////////////

if [ "$1" = "apmode" ]
then
	###################################
	# configure wifi AP
	###################################
	cd /home/pi/lora_gateway/scripts
	./prepare_access_point.sh
fi

if [ "$1" = "apmodenow" ]
then
	###################################
	# configure wifi AP now
	###################################
	cd /home/pi/lora_gateway/scripts
	./start_access_point.sh
fi

#//////////////////////////////////////////////////////////////////////////////////////////////////////
# system cellular
#//////////////////////////////////////////////////////////////////////////////////////////////////////

if [ "$1" = "dongle_on" ]
then
	###################################
	# configure dongle
	###################################
	cd /home/pi/lora_gateway/3GDongle/loranga
	./disable-loranga-internet-on-boot.sh	
	cd /home/pi/lora_gateway/3GDongle
	./enable-3GDongle-internet-on-boot.sh
fi

if [ "$1" = "dongle_off" ]
then
	###################################
	# configure dongle
	###################################
	cd /home/pi/lora_gateway/3GDongle
	./disable-3GDongle-internet-on-boot.sh
fi

if [ "$1" = "loranga_on" ]
then
	###################################
	# configure loranga
	###################################
	cd /home/pi/lora_gateway/3GDongle
	./disable-3GDongle-internet-on-boot.sh	
	cd /home/pi/lora_gateway/3GDongle/loranga
	./enable-loranga-internet-on-boot.sh
fi

if [ "$1" = "loranga_off" ]
then
	###################################
	# configure loranga
	###################################
	cd /home/pi/lora_gateway/3GDongle/loranga
	./disable-loranga-internet-on-boot.sh
fi

if [ "$1" = "loranga_2G" ]
then
	###################################
	# configure loranga
	###################################
	cd /home/pi/lora_gateway/3GDongle/loranga
	rm -rf loranga3G.txt
fi

if [ "$1" = "loranga_3G" ]
then
	###################################
	# configure loranga
	###################################
	cd /home/pi/lora_gateway/3GDongle/loranga
	./select-loranga3G-board.sh
fi

#//////////////////////////////////////////////////////////////////////////////////////////////////////
# system web admin login setting
#//////////////////////////////////////////////////////////////////////////////////////////////////////

if [ "$1" = "set_profile" ]
then
	tmp=$(mktemp)
	jq '.username="'$2'" ' /etc/gw_web_admin/database.json > "$tmp" && mv "$tmp" /etc/gw_web_admin/database.json
	jq '.password="'$3'" ' /etc/gw_web_admin/database.json > "$tmp" && mv "$tmp" /etc/gw_web_admin/database.json
	sudo chmod +r /etc/gw_web_admin/database.json
fi
