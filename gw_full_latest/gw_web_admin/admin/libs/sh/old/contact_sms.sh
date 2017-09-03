#!/bin/bash

###################################
# configure gateway
###################################

#sudo ./contact_sms.sh [\"+33299002233\",\"+23299338822\",\"+47922337788"]

tmp=$(mktemp)

jq '.alert_conf.contact_sms='$1' ' /home/pi/lora_gateway/gateway_conf.json > "$tmp" && mv "$tmp" /home/pi/lora_gateway/gateway_conf.json

sudo chown -R pi:pi /home/pi/lora_gateway/
sudo chmod +r /home/pi/lora_gateway/gateway_conf.json


