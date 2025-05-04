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

/home/pi/lora_gateway/scripts/test_raspberry_model.sh
/home/pi/lora_gateway/scripts/test_raspberry_model.sh | grep ">" | cut -d ">" -f2 > /home/pi/lora_gateway/arch_compiled.txt

make lora_gateway
		
sudo chown -R pi:pi /home/pi/lora_gateway/
		
popd

echo "You should reboot your Raspberry"
echo "Bye."
