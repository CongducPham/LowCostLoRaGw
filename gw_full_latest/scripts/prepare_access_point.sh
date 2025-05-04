#!/bin/bash

#use minimal config for access point, everything will be handled by dhcpd
sudo cp /etc/network/interfaces.orig /etc/network/interfaces 

#just in case
sudo systemctl stop hostapd
sudo systemctl stop dnsmasq 

sudo systemctl enable hostapd.service
echo "The access point is now enabled for next reboot."
#in case information are not in sync and there are already these lines, so just delete them
sudo sed -i '/^nohook wpa_supplicant/d' /etc/dhcpcd.conf
sudo sed -i '/^interface wlan0/d' /etc/dhcpcd.conf
sudo sed -i '/^static ip_address=192\.168\.200\.1\/24/d' /etc/dhcpcd.conf
#add info to for access point configuration
sudo echo "nohook wpa_supplicant" >> /etc/dhcpcd.conf
sudo echo "interface wlan0" >> /etc/dhcpcd.conf
sudo echo "static ip_address=192.168.200.1/24" >> /etc/dhcpcd.conf

