#!/bin/bash

sudo systemctl disable hostapd.service
sudo sed -i '/nohook wpa_supplicant/d' /etc/dhcpcd.conf
sudo sed -i '/interface wlan0/d' /etc/dhcpcd.conf
sudo sed -i '/static ip_address=192\.168\.200\.1\/24/d' /etc/dhcpcd.conf	
echo "The access point will be disabled at next reboot"
echo "Will use /etc/wpa_supplicant/wpa_supplicant.conf to connect to an existing access point"
