#!/bin/bash

cd /home/pi/lora_gateway/downlink

#echo "{\"status\":\"send_request\",\"dst\":$1,\"data\":\"$2\"}" > downlink-post.txt
echo "{\"status\":\"send_request\",\"dst\":$1,\"data\":\"$2\"}" >> downlink-post.txt

sudo chown -R pi:pi /home/pi/lora_gateway/
