#!/bin/bash

sudo python wake-2G.py
sleep 2
(
    while : ; do
        sudo pppd $1 call gprs
        sleep 10
    done
) &
