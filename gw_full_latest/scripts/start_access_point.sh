#!/bin/bash

/home/pi/lora_gateway/scripts/prepare_access_point.sh

sudo systemctl unmask dnsmasq
sudo systemctl unmask hostapd
sleep 1
sudo ip link set dev wlan0 down
sleep 1
sudo ip link set dev wlan0 up
sleep 1
#launching services dnsmasq and hostapd
echo "Starting hostapd service"
sudo systemctl start hostapd
sleep 3
echo "restarting dhcpd"
sudo service dhcpcd restart
sleep 3
echo "Starting dnsmasq services"
sudo systemctl start dnsmasq
#sleep 1
#wpa_cli -i wlan0 reconfigure
#sleep 1
#sudo systemctl restart hostapd
