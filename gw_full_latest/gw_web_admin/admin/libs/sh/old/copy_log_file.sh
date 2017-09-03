#!/bin/bash

sudo cp /home/pi/lora_gateway/log/post-processing.log /var/www/html/admin/log
sudo tail -n 500 /home/pi/lora_gateway/log/post-processing.log > /var/www/html/admin/log/post-processing-500L.log
sudo chown -R pi:www-data /var/www/html/admin/log

