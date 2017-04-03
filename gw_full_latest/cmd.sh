#!/bin/bash

#-------------------------------------------------------------------------------
# Copyright 2017 Congduc Pham, University of Pau, France.
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

if [ ! -f gateway_id.txt ]
then
	echo "ERROR: gateway_id.txt file not found"
	echo "should create it by running echo \"000000XXXXXXXXXX\" > gateway_id.txt"
	echo "where XXXXXXXXXX is the last 5 bytes of your MAC Ethernet interface address"
	echo "Example: echo \"00000027EBBEDA21\" > gateway_id.txt"
	echo "Here is your MAC Ethernet interface address:"
	echo "-------------------------------------------------------"
	ifconfig | grep "eth0"
    echo "-------------------------------------------------------"
	echo "Enter the last 5 hex bytes of your MAC Ethernet interface address"
	echo "in capital character and without the : separator"
	echo "example: HWaddr b8:27:eb:be:da:21 then just enter 27EBBEDA21"
	read macaddr
	echo "Will write 000000$macaddr into gateway_id.txt"
	echo "000000$macaddr" > gateway_id.txt
	echo "Done"
	echo "Replacing gw id in gateway_conf.json"
	sed -i -- 's/"000000.*"/"000000'"$macaddr"'"/g' gateway_conf.json
	echo "Done"	
fi

gatewayid=`cat gateway_id.txt`

while [ "$choice" != "Q" ]
do
echo "=======================================* Gateway $gatewayid *==="
echo "0- sudo python start_gw.py & ; disown %1                             +"
echo "1- sudo ./lora_gateway --mode 1                                      +"
echo "2- sudo ./lora_gateway --mode 1 | python post_processing_gw.py       +"
echo "3- ps aux | grep -e start_gw -e lora_gateway -e post_proc -e log_gw  +"
echo "4- tail --line=25 ../Dropbox/LoRa-test/post-processing_*.log         +"
echo "5- tail --line=25 -f ../Dropbox/LoRa-test/post-processing_*.log      +"
echo "6- less ../Dropbox/LoRa-test/post-processing_*.log                   +"
echo "------------------------------------------------------* Bluetooth *--+"
echo "a- run: sudo hciconfig hci0 piscan                                   +"
echo "b- run: sudo python rfcomm-server.py                                 +"
echo "c- run: nohup sudo python rfcomm-server.py -bg > rfcomm.log &        +"
echo "d- run: ps aux | grep rfcomm                                         +"
echo "e- run: tail -f rfcomm.log                                           +"
echo "---------------------------------------------------* Connectivity *--+"
echo "f- test: ping www.univ-pau.fr                                        +"
echo "--------------------------------------------------* Filtering msg *--+"
echo "l- List LoRa reception indications                                   +"
echo "m- List radio module reset indications                               +"
echo "n- List boot indications                                             +"
echo "o- List post-processing status                                       +"
echo "p- List low-level gateway status                                     +"
echo "--------------------------------------------------* Configuration *--+"
echo "A- show gateway_conf.json                                            +"
echo "B- edit gateway_conf.json                                            +"
echo "C- show clouds.json                                                  +"
echo "D- edit clouds.json                                                  +"
echo "-----------------------------------------------------------* kill *--+"
echo "K- kill all gateway related processes                                +"
echo "k- kill rfcomm-server process                                        +"
echo "R- reboot gateway                                                    +"
echo "S- shutdown gateway                                                  +"
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
		sudo python start_gw.py &
		disown %1
		echo "Check ../Dropbox/LoRa-test/post-processing_$gatewayid.log file (select command 5)"
fi

if [ "$choice" = "1" ] 
	then
		echo "Running simple lora_gateway... CTRL-C to exit"
		sudo ./lora_gateway --mode 1
fi

if [ "$choice" = "2" ] 
	then
		echo "Running lora_gateway with post-processing... CTRL-C to exit" 
		sudo ./lora_gateway --mode 1 | python post_processing_gw.py
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
		echo "Displaying last 25 lines of ../Dropbox/LoRa-test/post-processing_$gatewayid.log"
		echo "Current UTC date is"
		date --utc
		tail --line=25 ../Dropbox/LoRa-test/post-processing_$gatewayid.log
fi

if [ "$choice" = "5" ] 
	then
		echo "Following last lines of ../Dropbox/LoRa-test/post-processing_$gatewayid.log. CTRL-C to return"
		echo "Current UTC date is"
		date --utc
		trap "echo" SIGINT
		tail --line=25 -f ../Dropbox/LoRa-test/post-processing_$gatewayid.log
fi

if [ "$choice" = "6" ] 
	then
		echo "Display ../Dropbox/LoRa-test/post-processing_$gatewayid.log. Q to return"
		echo "Current UTC date is"
		date --utc
		trap "echo" SIGINT
		less ../Dropbox/LoRa-test/post-processing_$gatewayid.log
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

if [ "$choice" = "l" ] 
	then
		echo "List LoRa reception indications"
		grep "rxlora" ../Dropbox/LoRa-test/post-processing_$gatewayid.log
fi

if [ "$choice" = "m" ] 
	then
		echo "List radio module reset"
		grep "Resetting radio module" ../Dropbox/LoRa-test/post-processing_$gatewayid.log
fi

if [ "$choice" = "n" ] 
	then
		echo "List boot indications"
		grep "**********Power ON" ../Dropbox/LoRa-test/post-processing_$gatewayid.log
fi

if [ "$choice" = "o" ] 
	then
		echo "List post-processing status"
		grep "post status: gw ON" ../Dropbox/LoRa-test/post-processing_$gatewayid.log
fi

if [ "$choice" = "p" ] 
	then
		echo "List low-level gateway status"
		grep "Low-level gw status ON" ../Dropbox/LoRa-test/post-processing_$gatewayid.log
fi

if [ "$choice" = "A" ] 
        then
                echo "Showing global_conf.json"
                cat gateway_conf.json
fi

if [ "$choice" = "B" ] 
        then
        		if [ -f gateway_conf.json ]
        			then
                		echo "Editing gateway_conf.json. CTRL-O to save, CTRL-X to return"
                		nano gateway_conf.json
                	else
                		echo "Error: gateway_conf.json does not exist"
                fi	
fi

if [ "$choice" = "C" ] 
        then
                echo "Showing clouds.json"
                cat clouds.json
fi

if [ "$choice" = "D" ] 
        then
        		if [ -f clouds.json ]
        			then
                		echo "Editing clouds.json. CTRL-O to save, CTRL-X to return"
                		nano clouds.json
                	else
                		echo "Error: clouds.json does not exist"
                fi	
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

if [ "$choice" = "R" ] 
        then
		echo "Reboot gateway"
		sudo shutdown -r now
fi

if [ "$choice" = "S" ] 
        then
		echo "Shutdown gateway"
		sudo shutdown -h now
fi

echo "END OUTPUT"

if [ "$choice" != "Q" ]
	then
		echo "Press RETURN/ENTER..."
		read k
fi

done

echo "Bye."
