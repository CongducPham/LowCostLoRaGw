#!/bin/bash

sudo systemctl enable hostapd.service
echo "The access point is now enabled for next reboot."
#in case information are not in sync
sudo sed -i '/nohook wpa_supplicant/d' /etc/dhcpcd.conf
sudo sed -i '/interface wlan0/d' /etc/dhcpcd.conf
sudo sed -i '/static ip_address=192\.168\.200\.1\/24/d' /etc/dhcpcd.conf
#add info to start access point
sudo echo "nohook wpa_supplicant" >> /etc/dhcpcd.conf
sudo echo "interface wlan0" >> /etc/dhcpcd.conf
sudo echo "static ip_address=192.168.200.1/24" >> /etc/dhcpcd.conf

