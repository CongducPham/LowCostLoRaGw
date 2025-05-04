#!/bin/bash

/home/pi/lora_gateway/scripts/prepare_wifi_client.sh

sudo systemctl stop hostapd
sudo systemctl stop dnsmasq
echo "The access point is now disabled"
echo "Use /etc/wpa_supplicant/wpa_supplicant.conf to connect to an existing access point"
echo "Trying to connect to a configured WiFi network"
#sudo service dhcpcd restart
sudo ip link set dev wlan0 down
sleep 1
sudo ip link set dev wlan0 up
sleep 1
sudo wpa_supplicant -B -i wlan0 -c /etc/wpa_supplicant/wpa_supplicant.conf
sleep 1
wpa_cli -i wlan0 reconfigure

