#!/bin/bash

#-------------------------------------------------------------------------------
# Copyright 2016 Congduc Pham, University of Pau, France.
# 
# Congduc.Pham@univ-pau.fr
#
# This file is part of the low-cost LoRa gateway developped at University of Pau
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with the program.  If not, see <http://www.gnu.org/licenses/>.
#-------------------------------------------------------------------------------

choice="Z"

while [ "$choice" != "Q" ]
do
echo "========================================================* Gateway *==="
echo "0- nohup python start_gw.py &                                        +"
echo "1- sudo ./lora_gateway --mode 1                                      +"
echo "2- sudo ./lora_gateway --mode 1|python post_processing_gw.py -t -m 2 +"
echo "3- ps aux | grep -e start_gw -e lora_gateway -e post_proc -e log_gw  +"
echo "4- tail --line=15 ../Dropbox/LoRa-test/post-processing_*.log         +"
echo "5- tail -f ../Dropbox/LoRa-test/post-processing_*.log                +"
echo "------------------------------------------------------* Bluetooth *--+"
echo "a- run: sudo hciconfig hci0 piscan                                   +"
echo "b- run: sudo python rfcomm-server.py                                 +"
echo "c- run: nohup sudo python rfcomm-server.py -bg > rfcomm.log &        +"
echo "d- run: ps aux | grep rfcomm                                         +"
echo "e- run: tail -f rfcomm.log                                           +"
echo "---------------------------------------------------* Connectivity *--+"
echo "f- test: ping www.univ-pau.fr                                        +"
echo "--------------------------------------------------* Configuration *--+"
echo "A- show global_conf.json                                             +"
echo "B- show local_conf.json                                              +"
echo "-----------------------------------------------------------* kill *--+"
echo "K- kill all gateway related processes                                +"
echo "k- kill rfcomm-server process                                        +"
echo "---------------------------------------------------------------------+"
echo "Q- quit                                                              +"
echo "======================================================================" 
echo "Enter your choice: "
read choice
echo "----------------------------------------------------------------------"
echo "BEGIN OUTPUT"

if [ "$choice" = "0" ]
	then
		echo "Running in background the full LoRa gateway" 
		nohup python start_gw.py &
		echo "Check ../Dropbox/LoRa-test/post-processing_*.log file (select command 5)"
fi

if [ "$choice" = "1" ] 
	then
		echo "Running simple lora_gateway... CTRL-C to exit"
		sudo ./lora_gateway --mode 1
fi

if [ "$choice" = "2" ] 
	then
		echo "Running lora_gateway with post-processing... CTRL-C to exit" 
		sudo ./lora_gateway --mode 1 | python post_processing_gw.py -t -m 2
fi

if [ "$choice" = "3" ] 
	then
		echo "Check for lora_gateway process"
		echo "##############################"
		ps aux | grep -e start_gw -e lora_gateway -e post_processing -e log_gw
		echo "##############################"
		echo "The gateway is running if you see the lora_gateway process"

fi

if [ "$choice" = "4" ] 
	then
		echo "Displaying last 15 lines of ../Dropbox/LoRa-test/post-processing_*.log"
		echo "Current UTC date is"
		date --utc
		tail --line=15 ../Dropbox/LoRa-test/post-processing_*.log
fi

if [ "$choice" = "5" ] 
	then
		echo "Following last lines of ../Dropbox/LoRa-test/post-processing_*.log. CTRL-C to return"
		echo "Current UTC date is"
		date --utc
		trap "echo" SIGINT
		tail -f ../Dropbox/LoRa-test/post-processing_*.log
fi

if [ "$choice" = "a" ] 
	then
		echo "Testing the Bluetooth interface"
		echo "###############################"
		sudo hciconfig hci0 piscan
		echo "###############################"
		echo "if no error then the RPI is visible with Bluetooth"
fi

if [ "$choice" = "b" ] 
	then
		echo "Running rfcomm server for Bluetooth queries... CTRL-C to exit"
		sudo python rfcomm-server.py
fi

if [ "$choice" = "c" ] 
	then
		echo "Running in background the rfcomm server for Bluetooth queries. Logs in rfcomm.log"
		nohup sudo python rfcomm-server.py -bg > rfcomm.log &
fi


if [ "$choice" = "d" ] 
	then
		echo "Check for rfcomm-server process"
		echo "###############################"
		ps aux | grep rfcomm-server
		echo "###############################"
		echo "You should see python rfcomm-server.py for Bluetooth queries"
fi

if [ "$choice" = "e" ] 
	then
		echo "Following last lines of rfcomm.log. CTRL-C to return"
		echo "Current UTC date is"
		date --utc
		trap "echo" SIGINT
		tail -f rfcomm.log
fi

if [ "$choice" = "f" ] 
	then
		echo "Test Internet connectivity. CTRL-C to return"
		trap "echo" SIGINT
		ping www.univ-pau.fr
fi

if [ "$choice" = "A" ] 
        then
                echo "Showing global_conf.json"
                cat global_conf.json
fi

if [ "$choice" = "B" ] 
        then
                echo "Showing local_conf.json"
                cat local_conf.json
fi

if [ "$choice" = "K" ] 
        then
		echo "Killing all gateway related processes"
		sudo kill $(ps aux | grep -e start_gw -e lora_gateway -e post_processing -e log_gw | awk '{print $2}')
fi

if [ "$choice" = "k" ] 
        then
		echo "Killing rfcomm-server process"
		sudo kill $(ps aux | grep -e rfcomm-server | awk '{print $2}')
fi

echo "END OUTPUT"

if [ "$choice" != "Q" ]
	then
		echo "Press RETURN/ENTER..."
		read k
fi

done

echo "Bye."
