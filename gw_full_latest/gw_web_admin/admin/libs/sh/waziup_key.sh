#!/bin/bash

###################################
# Configure Orion Key
###################################


if [ $1 == "project_name" ] 
then
    sudo sed -i 's/^project_name.*/project_name=\"'"$2"'\"/g' /home/pi/lora_gateway/key_Orion.py
elif [ $1 == "organization_name" ]
then
	sudo sed -i 's/^organization_name.*/organization_name=\"'"$2"'\"/g' /home/pi/lora_gateway/key_Orion.py
else #service_tree
    sudo sed -i 's/^service_tree.*/service_tree=\"'"$2"'\"/g' /home/pi/lora_gateway/key_Orion.py
fi