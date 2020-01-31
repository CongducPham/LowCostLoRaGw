#!/bin/bash

echo "Removing /home/pi/lora_gateway/scripts/start_gw.sh in /etc/rc.local if any"
sudo sed -i '\/home\/pi\/lora_gateway\/scripts\/start_gw.sh/d' /etc/rc.local
echo "Removing /home/pi/lora_gateway/start_upl_pprocessing_gw.sh in /etc/rc.local if any"
sudo sed -i '\/home\/pi\/lora_gateway\/start_upl_pprocessing_gw.sh/d' /etc/rc.local
echo "Removing /home/pi/lora_gateway/start_lpf_pprocessing_gw.sh in /etc/rc.local if any"
sudo sed -i '\/home\/pi\/lora_gateway\/start_lpf_pprocessing_gw.sh/d' /etc/rc.local

echo "Add /home/pi/lora_gateway/scripts/start_gw.sh in /etc/rc.local, but without starting the gateway"
sudo sed -i 's/^exit 0/\/home\/pi\/lora_gateway\/scripts\/start_gw.sh startup_only\nexit 0/g' /etc/rc.local
echo "Done"