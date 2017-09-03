#!/bin/bash

###################################
# configure gateway
###################################

#For example to update channel number: radio_conf => ch => value
#jq '.radio_conf.ch = 12' gateway_conf.json > "$tmp" && mv "$tmp" gateway_conf.json

tmp=$(mktemp)
if [ $# == 3 ] 
then
	if [ $2 == "gateway_ID" ] || [ $2 == "contact_mail" ] || [ $2 == "mail_from" ] || [ $2 == "mail_passwd" ] || [ $2 == "mail_server" ] || [ $2 == "pin" ]
	then
        	jq '.'"$1"'.'"$2"' = "'$3'" ' /home/pi/lora_gateway/gateway_conf.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/gateway_conf.json
    else
        	jq '.'"$1"'.'"$2"' = '$3' ' /home/pi/lora_gateway/gateway_conf.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/gateway_conf.json
    fi
    sudo chown -R pi:pi /home/pi/lora_gateway/
    sudo chmod +r /home/pi/lora_gateway/gateway_conf.json
fi
