#!/bin/bash

#get back original file
sudo cp /etc/network/interfaces.orig /etc/network/interfaces

echo "restarting dhcpd"
sudo service dhcpcd restart

#add wlan0 and eth0 definitions
echo "auto eth0" | sudo tee -a /etc/network/interfaces > /dev/null
echo "allow-hotplug eth0" | sudo tee -a /etc/network/interfaces > /dev/null
echo "iface eth0 inet dhcp" | sudo tee -a /etc/network/interfaces > /dev/null
echo "auto wlan0" | sudo tee -a /etc/network/interfaces > /dev/null
echo "allow-hotplug wlan0" | sudo tee -a /etc/network/interfaces > /dev/null
echo "iface wlan0 inet dhcp" | sudo tee -a /etc/network/interfaces > /dev/null
echo "wpa-conf /etc/wpa_supplicant/wpa_supplicant.conf" | sudo tee -a /etc/network/interfaces > /dev/null

sudo systemctl disable hostapd.service
sudo systemctl disable dnsmasq.service
sudo sed -i '/^nohook wpa_supplicant/d' /etc/dhcpcd.conf
sudo sed -i '/^interface wlan0/d' /etc/dhcpcd.conf
sudo sed -i '/^static ip_address=192\.168\.200\.1\/24/d' /etc/dhcpcd.conf	
echo "The access point will be disabled at next reboot"
echo "Will use /etc/wpa_supplicant/wpa_supplicant.conf to connect to an existing access point"
