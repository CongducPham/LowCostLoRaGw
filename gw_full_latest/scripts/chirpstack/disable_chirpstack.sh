#!/bin/bash

#systemctl list-unit-files

echo "disabling chirpstack-network-server"
sudo systemctl disable chirpstack-network-server
sudo systemctl stop chirpstack-network-server

echo "disabling chirpstack-application-server"
sudo systemctl disable chirpstack-application-server
sudo systemctl stop chirpstack-application-server

echo "disabling chirpstack-gateway-bridge"
sudo systemctl disable chirpstack-gateway-bridge
sudo systemctl stop chirpstack-gateway-bridge

echo "indicating router.eu.thethings.network as LoRaWAN network server in /opt/ttn-gateway/packet_forwarder/lora_pkt_fwd/global_conf.json"
#sudo sed -i -- 's/server_address".*".*"/server_address": "router.eu.thethings.network"/g' /home/pi/lora_gateway/global_conf.json
sudo sed -i -- 's/server_address".*".*"/server_address": "router.eu.thethings.network"/g' /opt/ttn-gateway/packet_forwarder/lora_pkt_fwd/global_conf.json
