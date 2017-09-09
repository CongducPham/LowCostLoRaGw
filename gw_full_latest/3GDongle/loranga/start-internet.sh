#!/bin/sh

sudo python wake-2G.py
sleep 2
sudo pppd call gprs &