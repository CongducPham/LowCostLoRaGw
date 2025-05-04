#!/bin/bash

# Stop on the first sign of trouble
set -e

if [ $UID != 0 ]; then
    echo "ERROR: Operation not permitted. Forgot sudo?"
    exit 1
fi

SCRIPT_DIR=$(pwd)

echo "Light version of the RAK 2245/831 installer"

# Install LoRaWAN packet forwarder repositories
INSTALL_DIR="/opt/ttn-gateway"
if [ ! -d "$INSTALL_DIR" ]; then mkdir $INSTALL_DIR; fi
pushd $INSTALL_DIR

# Build LoRa gateway app

echo "Get Semtech's lora_gateway github distribution"
git clone https://github.com/Lora-net/lora_gateway.git

pushd lora_gateway

cp $SCRIPT_DIR/rak_lora/library.cfg ./libloragw/library.cfg
cp $SCRIPT_DIR/rak_lora/loragw_spi.native.c ./libloragw/src/loragw_spi.native.c

echo "Compiling lora_gateway lib"
make

popd

# Build packet forwarder

echo "Get Semtech's packet_forwarder github distribution"
git clone https://github.com/Lora-net/packet_forwarder.git

pushd packet_forwarder

cp $SCRIPT_DIR/rak_lora/lora_pkt_fwd.c ./lora_pkt_fwd/src/lora_pkt_fwd.c

echo "Compiling lora_pkt_fwd"
make

popd

# add config "dtoverlay=pi3-disable-bt" to config.txt
linenum=`sed -n '/dtoverlay=pi3-disable-bt/=' /boot/config.txt`
if [ ! -n "$linenum" ]; then
        echo "dtoverlay=pi3-disable-bt" >> /boot/config.txt
fi

systemctl disable hciuart
cd $SCRIPT_DIR

echo "Done"
echo "Now run ./enable_at_boot.sh"

