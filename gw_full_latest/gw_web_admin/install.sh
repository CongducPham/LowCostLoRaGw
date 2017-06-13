#!/bin/bash

echo "Installing gateway's web admin interface..."

if [ -d /var/www/html/admin ]
	then
		echo "have detected a previous installation"
		new_install=0
	else
		echo "new installation"
		new_install=1			 
fi

echo "Removing any existing admin folder (from previous install)"
sudo rm -rf /var/www/html/admin

if [ -d ./admin ]
	then
		echo "have detected a local web admin folder" 
		echo "***************************************"
		echo "*** install from local version Y/N  ***"
		echo "***************************************"
		read ouinon

		if [ "$ouinon" = "y" ] || [ "$ouinon" = "Y" ]
			then
				echo "Copying web admin interface files"
				sudo cp -r admin /var/www/html/
				echo "Done"
		fi
fi

if [ ! -d ./admin ] || [ "$ouinon" = "n" ] || [ "$ouinon" = "N" ]
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


if [ $new_install = 1 ]
	then
		######################################
		# Editing /etc/sudoers file
		######################################
		echo "Edit /etc/sudoers to obtain root privileges for www-data"
		# Take a backup of sudoers file and change the backup file.
		sudo cp /etc/sudoers /tmp/sudoers.bak
		sudo echo "www-data ALL=(ALL) NOPASSWD: /sbin/reboot, /sbin/shutdown" >> /tmp/sudoers.bak
		sudo echo "www-data ALL=(ALL) NOPASSWD: /etc/hostapd/hostapd.conf, /var/www/html/admin/libs/sh/wifi.sh" >> /tmp/sudoers.bak
		sudo echo "www-data ALL=(ALL) NOPASSWD: /var/www/html/admin//libs/sh/thingspeak_key.sh, /var/www/html/admin//libs/sh/waziup_key.sh" >> /tmp/sudoers.bak
		sudo echo "www-data ALL=(ALL) NOPASSWD: /home/pi/lora_gateway/scripts/update_gw.sh, /var/www/html/admin//libs/sh/install_gw.sh, /var/www/html/admin//libs/sh/update_gw_file.sh" >> /tmp/sudoers.bak
		sudo echo "www-data ALL=(ALL) NOPASSWD: /var/www/html/admin/libs/sh/gateway_conf.sh, /var/www/html/admin//libs/sh/waziup_conf.sh, /var/www/html/admin//libs/sh/thingspeak_conf.sh" >> /tmp/sudoers.bak
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
fi		

echo "Install gateway's web admin interface done."
echo "Connect to http://gw_ip_addr/admin"

