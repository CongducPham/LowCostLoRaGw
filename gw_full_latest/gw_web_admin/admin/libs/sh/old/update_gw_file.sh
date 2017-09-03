#!/bin/bash

cd /home/pi/lora_gateway

wget --backups=1 $1 

sudo chown -R pi:pi /home/pi/lora_gateway/
