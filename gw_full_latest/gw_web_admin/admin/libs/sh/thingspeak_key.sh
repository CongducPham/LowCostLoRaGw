#!/bin/bash

###################################
# Configure ThingSpeak Write Key
###################################

#Replacing data in /home/pi/lora_gateway/key_ThingSpeak.py
sudo sed -i 's/^_def_thingspeak_channel_key.*/_def_thingspeak_channel_key=\"'"$1"'\"/g' /home/pi/lora_gateway/key_ThingSpeak.py
