#!/bin/sh

cd /home/pi/lora_gateway

if [ -f 3GDongle/loranga/use_loranga_internet_on_boot.txt ]
then
	cd 3GDongle/loranga
	# the script will turn on the modem and launch pppd
	./start-internet.sh
	sleep 10
	cd ../../
elif [ -f 3GDongle/loranga/use_loranga_SMS_on_boot.txt ]
then
	cd 3GDongle/loranga
	# just turn on the modem
	sudo python wake-2G.py
	cd ../..
fi

python start_gw.py &
