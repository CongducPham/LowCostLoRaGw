#!/bin/bash

if [ -f loranga3G.txt ]
then
	sudo python wake-3G.py
else	
	sudo python wake-2G.py
fi
	
sleep 2
(
    while : ; do
        sudo pppd $1 call gprs
        sleep 10
    done
) &
