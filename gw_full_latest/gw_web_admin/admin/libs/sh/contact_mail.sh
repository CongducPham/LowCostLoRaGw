#!/bin/bash

###################################
# configure gateway
###################################

#sudo ./contact_mail.sh aaa@gmail.com,bbb@gmail.com,ccc@gmail.com,ddd@gmail.com

tmp=$(mktemp)

jq '.alert_conf.contact_mail="'$1'" ' /home/pi/lora_gateway/gateway_conf.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/gateway_conf.json

sudo chown -R pi:pi /home/pi/lora_gateway/
sudo chmod +r /home/pi/lora_gateway/gateway_conf.json


