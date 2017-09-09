#!/bin/sh

sudo kill $(ps aux | grep -e pppd | awk '{print $2}')
sleep 2
sudo python wake-2G.py
