#!/bin/bash

echo "copy nodogsplash.conf to /etc/nodogsplash/"
sudo cp nodogsplash.conf /etc/nodogsplash/
echo "done"

echo "stopping current nodogsplash"
sudo ndsctl stop
echo "done"

echo "starting nodogsplash"
sudo nodogsplash
echo "done"
