#!/bin/bash

sudo cp -r /var/www/html/images/uploads/* /var/www/html/images/bak

sudo rm -rf /var/www/html/images/uploads/*

sudo chown -R pi:www-data /var/www/html/
sudo chmod -R 770 /var/www/html/