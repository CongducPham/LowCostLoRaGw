#!/bin/sh

sudo kill $(ps aux | grep -e pppd | awk '{print $2}')
sleep 2
#on the 2G version, it is similar to a toggle switch
sudo python wake-2G.py
#it is not possible to power OFF the 3G version once it has been powered ON
#so can't do nothing for the 3G version
