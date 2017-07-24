#!/bin/sh

echo "Copying updated cmd.sh with image options"
cp cmd.sh .. 
echo "configuring for image storage"
echo "creating images folders in /home/pi/Dropbox/LoRa-test"
mkdir -p /home/pi/Dropbox/LoRa-test/images
echo "creating images related folders in /var/www/html"
mkdir -p /var/www/html/images/uploads
mkdir -p /var/www/html/images/bak
echo "copying gw_images_web as /var/www/html/images"
cp -r gw_images_web/* /var/www/html/images
echo "change group to www-data"
chown -R pi:www-data /var/www/html/images
echo "Done"