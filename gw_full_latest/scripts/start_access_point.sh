#!/bin/bash

/home/pi/lora_gateway/scripts/prepare_access_point.sh

#launching services dnsmasq and hostapd
echo "Starting hostapd and dnsmasq services"
sudo systemctl unmask dnsmasq
sudo systemctl unmask hostapd
sudo systemctl start dnsmasq
sudo systemctl start hostapd
sleep 1
sudo ip link set dev wlan0 down
sleep 1
sudo ip link set dev wlan0 up
echo "restarting dhcpd"
sudo service dhcpcd restart
sleep 1
wpa_cli -i wlan0 reconfigure
sleep 1
sudo systemctl restart hostapd
