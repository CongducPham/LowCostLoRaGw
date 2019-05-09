#!/bin/bash

cd /home/pi/lora_gateway

cp /opt/ttn-gateway/packet_forwarder/lora_pkt_fwd/*.json .

# Reset iC880a PIN
SX1301_RESET_BCM_PIN=17
echo "$SX1301_RESET_BCM_PIN"  > /sys/class/gpio/export
echo "out" > /sys/class/gpio/gpio$SX1301_RESET_BCM_PIN/direction
echo "0"   > /sys/class/gpio/gpio$SX1301_RESET_BCM_PIN/value
sleep 0.1
echo "1"   > /sys/class/gpio/gpio$SX1301_RESET_BCM_PIN/value
sleep 0.1
echo "0"   > /sys/class/gpio/gpio$SX1301_RESET_BCM_PIN/value
sleep 0.1
echo "$SX1301_RESET_BCM_PIN" > /sys/class/gpio/unexport

sleep 3

if [ $# -eq 0 ]
then
        /opt/ttn-gateway/lora_gateway/util_pkt_logger/util_pkt_logger | python util_pkt_logger_formatter.py | python post_processing_gw.py | python log_gw.py &
else
        /opt/ttn-gateway/lora_gateway/util_pkt_logger/util_pkt_logger | python util_pkt_logger_formatter.py 
fi