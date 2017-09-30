#!/bin/sh

sudo python wake-2G.py
sleep 2
sudo pppd $1 call gprs &