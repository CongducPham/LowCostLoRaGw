#!/bin/bash

cd /home/pi/lora_gateway/gw_web_admin

echo "Installing gateway's web admin interface..."

echo "Removing any existing admin folder (from previous install)"
sudo rm -rf /var/www/html/admin

if [ -d ./admin ]
	then
		echo "have detected a local web admin folder" 
		echo "Copying web admin interface files"
		sudo cp -r admin /var/www/html/
		echo "Done"
fi

if [ ! -d ./admin ]
	then
		cd /home/pi
		#sudo apt-get update

		echo "Check if subversion is installed and install it if it is not"
		[[ -z `dpkg --get-selections | grep -w ^subversion[^-]` ]] && aptitude install subversion

		echo "Getting new gw_web_admin from github"
		svn checkout https://github.com/CongducPham/LowCostLoRaGw/trunk/gw_full_latest/gw_web_admin
		
		echo "Copying web admin interface files"
		sudo cp -r gw_web_admin/admin /var/www/html/
		
		echo "Removing downloaded gw_web_admin folder"
		sudo rm -rf gw_web_admin
fi		

######################################
# Editing /etc/sudoers file
######################################
echo "Edit /etc/sudoers to obtain root privileges for www-data"
# Take a backup of sudoers file and change the backup file.
sudo cp /etc/sudoers /tmp/sudoers.bak
# delete existing files
sudo sed -i '/www-data/d' /tmp/sudoers.bak

#for raspap-webgui
#from https://github.com/billz/raspap-webgui
sudo echo "www-data ALL=(ALL) NOPASSWD:/sbin/ifdown wlan0" >> /tmp/sudoers.bak
sudo echo "www-data ALL=(ALL) NOPASSWD:/sbin/ifup wlan0" >> /tmp/sudoers.bak
sudo echo "www-data ALL=(ALL) NOPASSWD:/bin/cat /etc/wpa_supplicant/wpa_supplicant.conf" >> /tmp/sudoers.bak
sudo echo "www-data ALL=(ALL) NOPASSWD:/bin/cp /tmp/wifidata /etc/wpa_supplicant/wpa_supplicant.conf" >> /tmp/sudoers.bak
sudo echo "www-data ALL=(ALL) NOPASSWD:/sbin/wpa_cli scan_results" >> /tmp/sudoers.bak
sudo echo "www-data ALL=(ALL) NOPASSWD:/sbin/wpa_cli scan" >> /tmp/sudoers.bak
sudo echo "www-data ALL=(ALL) NOPASSWD:/sbin/wpa_cli reconfigure" >> /tmp/sudoers.bak
sudo echo "www-data ALL=(ALL) NOPASSWD:/bin/cp /tmp/hostapddata /etc/hostapd/hostapd.conf" >> /tmp/sudoers.bak
sudo echo "www-data ALL=(ALL) NOPASSWD:/etc/init.d/hostapd start" >> /tmp/sudoers.bak
sudo echo "www-data ALL=(ALL) NOPASSWD:/etc/init.d/hostapd stop" >> /tmp/sudoers.bak
sudo echo "www-data ALL=(ALL) NOPASSWD:/etc/init.d/dnsmasq start" >> /tmp/sudoers.bak
sudo echo "www-data ALL=(ALL) NOPASSWD:/etc/init.d/dnsmasq stop" >> /tmp/sudoers.bak
sudo echo "www-data ALL=(ALL) NOPASSWD:/bin/cp /tmp/dhcpddata /etc/dnsmasq.conf" >> /tmp/sudoers.bak
sudo echo "www-data ALL=(ALL) NOPASSWD:/sbin/shutdown -h now" >> /tmp/sudoers.bak
sudo echo "www-data ALL=(ALL) NOPASSWD:/sbin/reboot" >> /tmp/sudoers.bak
sudo echo "www-data ALL=(ALL) NOPASSWD:/sbin/ip link set wlan0 down" >> /tmp/sudoers.bak
sudo echo "www-data ALL=(ALL) NOPASSWD:/sbin/ip link set wlan0 up" >> /tmp/sudoers.bak
sudo echo "www-data ALL=(ALL) NOPASSWD:/sbin/ip -s a f label wlan0" >> /tmp/sudoers.bak
sudo echo "www-data ALL=(ALL) NOPASSWD:/bin/cp /etc/raspap/networking/dhcpcd.conf /etc/dhcpcd.conf" >> /tmp/sudoers.bak
sudo echo "www-data ALL=(ALL) NOPASSWD:/etc/raspap/hostapd/enablelog.sh" >> /tmp/sudoers.bak
sudo echo "www-data ALL=(ALL) NOPASSWD:/etc/raspap/hostapd/disablelog.sh" >> /tmp/sudoers.bak

#for our simple web admin interface
sudo echo ""
sudo echo "www-data ALL=(ALL) NOPASSWD: /sbin/reboot, /sbin/shutdown, /etc/hostapd/hostapd.conf" >> /tmp/sudoers.bak
sudo echo "www-data ALL=(ALL) NOPASSWD: /var/www/html/admin/libs/sh/web_shell_script.sh" >> /tmp/sudoers.bak
sudo echo "www-data ALL=(ALL) NOPASSWD: /usr/bin/python" >> /tmp/sudoers.bak
sudo echo "www-data ALL=(ALL) NOPASSWD: /var/www/html/admin/libs/python/key_clouds.py, /var/www/html/admin/libs/python/key_thingspeak.py" >> /tmp/sudoers.bak
sudo echo "www-data ALL=(ALL) NOPASSWD: /home/pi/lora_gateway/scripts/basic_config_gw.sh" >> /tmp/sudoers.bak
sudo echo "www-data ALL=(ALL) NOPASSWD: /home/pi/lora_gateway/scripts/update_gw.sh" >> /tmp/sudoers.bak
sudo echo "www-data ALL=(ALL) NOPASSWD: /home/pi/lora_gateway/scripts/prepare_wifi_client.sh" >> /tmp/sudoers.bak
sudo echo "www-data ALL=(ALL) NOPASSWD: /home/pi/lora_gateway/scripts/prepare_access_point.sh" >> /tmp/sudoers.bak
sudo echo "www-data ALL=(ALL) NOPASSWD: /home/pi/lora_gateway/scripts/start_access_point.sh" >> /tmp/sudoers.bak
sudo echo "www-data ALL=(ALL) NOPASSWD: /home/pi/lora_gateway/gw_web_admin/install.sh" >> /tmp/sudoers.bak
sudo echo "www-data ALL=(ALL) NOPASSWD: /var/www/html/images/libs/sh/move_img.sh" >> /tmp/sudoers.bak

		
# Check syntax of the backup file to make sure it is correct.
sudo visudo -cf /tmp/sudoers.bak
if [ $? -eq 0 ]; then
  # Replace the sudoers file with the new only if syntax is correct.
  sudo cp /tmp/sudoers.bak /etc/sudoers
  echo "/etc/sudoers file updated"
  echo "Obtaining root privileges for www-data done."
else
  echo "Could not modify /etc/sudoers file. Please do this manually."
fi

######################################
# End editing /etc/sudoers file
######################################

echo "Check if jq is installed and install it if it is not."
[[ -z `dpkg --get-selections | grep -w ^jq[^-]` ]] && apt-get install jq

echo "Give rights to the Apache folder in order to easily administer the website."
sudo chown -R pi:www-data /var/www/html/
sudo chmod -R 770 /var/www/html/

###################
# Setting profile 
##################
if [ ! -d /etc/gw_web_admin ]
	then
		mkdir /etc/gw_web_admin
		# username : admin && password : loragateway
		echo "Creating the json authentifation file /etc/gw_web_admin/database.json"
		echo "Default connection settings : username = admin & password = loragateway"
		echo "{\"username\":\"admin\", \"password\":\"loragateway\"}" > /etc/gw_web_admin/database.json		
fi
		
echo "Install gateway's web admin interface done."
echo "Connect to http://gw_ip_addr/admin"

