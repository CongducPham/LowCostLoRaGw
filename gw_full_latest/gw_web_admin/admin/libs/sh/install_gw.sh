#!/bin/bash

cd /home/pi

cp lora_gateway/scripts/update_gw.sh .

rm -r lora_gateway

./update_gw.sh

rm update_gw.sh