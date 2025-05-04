#!/bin/sh

echo "compiling the image decoding tool"
g++ -o decode_to_bmp decode_to_bmp.c
echo "configuring for image storage"
echo "creating images folders in /home/pi/Dropbox/LoRa-test"
mkdir -p /home/pi/Dropbox/LoRa-test/images
echo "creating images related folders in /var/www/html"
sudo mkdir -p /var/www/html/images/uploads
sudo mkdir -p /var/www/html/images/bak
echo "copying gw_images_web as /var/www/html/images"
sudo cp -r gw_images_web/* /var/www/html/images
echo "change group to www-data"
sudo chown -R pi:www-data /var/www/html/images
#echo "allowing script to be executed by www-data"
#sudo echo "www-data ALL=(ALL) NOPASSWD: /var/www/html/images/libs/sh/move_img.sh" >> /etc/sudoers
echo "Done"