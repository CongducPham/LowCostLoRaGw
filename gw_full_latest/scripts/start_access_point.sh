#!/bin/bash

/home/pi/lora_gateway/scripts/prepare_access_point.sh

#launching services dnsmasq and hostapd
echo "Starting hostapd and dnsmasq services"
sudo systemctl unmask dnsmasq
sudo systemctl unmask hostapd
sudo systemctl start dnsmasq
sudo systemctl start hostapd
sudo service dhcpcd restart
sleep 3
wpa_cli -i wlan0 reconfigure
sleep 3
sudo systemctl restart hostapd
