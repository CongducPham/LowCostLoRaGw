#!/bin/bash

input='0'

while [ $input != '1' ] && [ $input != '2' ]
do
	echo "***********************************************************"
	echo "Which module to start: (1) lora_pkt_fwd (2) util_pkt_logger"
	echo "***********************************************************"
	read input
done

if [ "$input" = "1" ]
	then
		./install_lpf.sh
elif [ "$input" = "2" ]
	then
		./install_upl.sh
fi	

echo "Done"	