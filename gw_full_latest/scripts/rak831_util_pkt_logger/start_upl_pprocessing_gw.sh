#!/bin/bash
(
if [ $# -eq 0 ] 
then 
	sleep 1
else
	#wait until the RPI starts
	sleep 60	
fi

cd /home/pi/lora_gateway
cp /opt/ttn-gateway/packet_forwarder/lora_pkt_fwd/*.json .

#create the gw id so that a newly installed gateway is always configured with a correct id
/home/pi/lora_gateway/scripts/create_gwid.sh

SERVICE="util_pkt_logger"

while : ; do

	echo "Launching util_pkt_logger"
	
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

	/opt/ttn-gateway/lora_gateway/util_pkt_logger/util_pkt_logger | python util_pkt_logger_formatter.py | python post_processing_gw.py | python log_gw.py &
	sleep 10
	
	while pgrep -x "$SERVICE" >/dev/null: ; do
		sleep 20
	done
	
	echo "util_pkt_logger not running for some reason"
	echo "trying to restart"
	
	kill $(ps aux | grep -e post_processing -e log_gw -e util_pkt_logger | awk '{print $2}')
	
	sleep 15	
done
) &