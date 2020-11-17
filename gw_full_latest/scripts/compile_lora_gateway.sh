#!/bin/bash
#
# example: ./compile_lora_gateway.sh

pushd /home/pi/lora_gateway

#remove old binaries
make clean

getoptissue=`cat /etc/os-release | grep buster`

if [ "$getoptissue" = "" ]
then
	sudo sed -i 's/^CFLAGS+=-DGETOPT_ISSUE/#CFLAGS+=-DGETOPT_ISSUE/g' /home/pi/lora_gateway/raspberry.makefile
else
	sudo sed -i 's/#*CFLAGS+=-DGETOPT_ISSUE/CFLAGS+=-DGETOPT_ISSUE/g' /home/pi/lora_gateway/raspberry.makefile
fi	

revision=`cat /proc/cpuinfo | grep "Revision" | cut -d ':' -f 2 | tr -d " \t\n\r"`

rev_hex="0x$revision"

#uuuuuuuuFMMMCCCCPPPPTTTTTTTTRRRR
type=$(( rev_hex & 0x00000FF0 ))

board=$(( type >> 4 ))

if [ "$board" = "0" ] || [ "$board" = "1" ] || [ "$board" = "2" ] || [ "$board" = "3" ]
	then
		echo "You have a Raspberry 1"		
		echo "Compiling for Raspberry 1"
		echo "RPI1/0" > /home/pi/lora_gateway/arch_compiled.txt
		make lora_gateway
elif [ "$board" = "4" ]
	then
		echo "You have a Raspberry 2"
		echo "Compiling for Raspberry 2 and 3"
		echo "RPI2/3" > /home/pi/lora_gateway/arch_compiled.txt		
		make lora_gateway_pi2
elif [ "$board" = "8" ]
	then
		echo "You have a Raspberry 3B"
		echo "Compiling for Raspberry 2 and 3"
		echo "RPI2/3" > /home/pi/lora_gateway/arch_compiled.txt		
		make lora_gateway_pi2
elif [ "$board" = "13" ] || [ "$board" = "14" ]
	then
		echo "You have a Raspberry 3B+/3A+"
		echo "Compiling for Raspberry 2 and 3"
		echo "RPI2/3" > /home/pi/lora_gateway/arch_compiled.txt		
		make lora_gateway_pi2		
elif [ "$board" = "9" ]
	then
		echo "You have a Raspberry Zero"
		echo "Compiling for Raspberry Zero (same as Raspberry 1)"
		echo "RPI1/0" > /home/pi/lora_gateway/arch_compiled.txt		 
		make lora_gateway
elif [ "$board" = "12" ]
	then
		echo "You have a Raspberry Zero W"
		echo "Compiling for Raspberry Zero W (same as Raspberry 1)"
		echo "RPI1/0" > /home/pi/lora_gateway/arch_compiled.txt		
		make lora_gateway
elif [ "$board" = "17" ]
	then
		echo "You have a Raspberry 4B"
		echo "Compiling for Raspberry 4B"
		echo "RPI4" > /home/pi/lora_gateway/arch_compiled.txt		 
		make lora_gateway_pi4
else
	echo "Not supported, trying"		
	echo "Compiling for Raspberry 1"
	echo "RPI1/0" > /home/pi/lora_gateway/arch_compiled.txt	
	make lora_gateway
fi
		
sudo chown -R pi:pi /home/pi/lora_gateway/
		
popd

echo "You should reboot your Raspberry"
echo "Bye."
