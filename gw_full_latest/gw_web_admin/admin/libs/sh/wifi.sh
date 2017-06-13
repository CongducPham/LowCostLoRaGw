#!/bin/bash

###################################
# configure hostapd.conf
###################################

#Replacing hot-spot ssid in /etc/hostapd/hostapd.conf
sudo sed -i 's/^ssid.*/ssid='"$1"'/g' /etc/hostapd/hostapd.conf
#Setting wpa_passphrase in /etc/hostapd/hostapd.conf
sudo sed -i 's/^wpa_passphrase.*/wpa_passphrase='"$2"'/g' /etc/hostapd/hostapd.conf
