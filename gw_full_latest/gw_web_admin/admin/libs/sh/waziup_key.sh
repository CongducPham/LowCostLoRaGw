#!/bin/bash

###################################
# Configure WAZIUP Key
###################################

#Replacing data in /home/pi/lora_gateway/key_WAZIUP.py
#project name
sudo sed -i 's/^project_name.*/project_name=\"'"$1"'\"/g' /home/pi/lora_gateway/key_WAZIUP.py
#organization
sudo sed -i 's/^organization_name.*/organization_name=\"'"$2"'\"/g' /home/pi/lora_gateway/key_WAZIUP.py
#service tree
sudo sed -i 's/^service_tree.*/service_tree=\"'"$3"'\"/g' /home/pi/lora_gateway/key_WAZIUP.py 
