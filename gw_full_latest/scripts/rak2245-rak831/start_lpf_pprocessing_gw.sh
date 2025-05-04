#!/bin/bash
(
if [ $# -eq 0 ] 
then 
        sleep 1
else
        #wait until the RPI has completed startup
        #we observed that it is not very reliable to start the radio concentrator right away
        sleep 60       
fi

cd /home/pi/lora_gateway
echo "Copying configuration file *.json from /opt/ttn-gateway/packet_forwarder/lora_pkt_fwd/"
cp /opt/ttn-gateway/packet_forwarder/lora_pkt_fwd/*.json .
#this is for the web admin interface
cp /opt/ttn-gateway/packet_forwarder/lora_pkt_fwd/*.json /var/www/html/admin/log
sudo chown -R pi:www-data /var/www/html/admin/log

#create the gw id so that a newly installed gateway is always configured with a correct id
/home/pi/lora_gateway/scripts/create_gwid.sh

gatewayid=`cat /home/pi/lora_gateway/gateway_id.txt`
gatewayttnid="${gatewayid:4:6}FFFF${gatewayid:10}"

echo "Replacing gw id in /home/pi/lora_gateway/global_conf.json"
sed -i -- 's/gateway_ID".*".*"/gateway_ID": "'"$gatewayttnid"'"/g' /home/pi/lora_gateway/global_conf.json

echo "Replacing gw id in /home/pi/lora_gateway/local_conf.json"
sed -i -- 's/gateway_ID".*".*"/gateway_ID": "'"$gatewayttnid"'"/g' /home/pi/lora_gateway/local_conf.json

SERVICE="lora_pkt_fwd"

MAX_RETRY=10
BACKOFF_RETRY_TIME=15
MAX_SLEEP_TIME=60
PID_CHECK_PERIOD=30	
MIN_RUN_TIME=120

retry=1
run_time=0

while [ $retry -lt $MAX_RETRY ]
do

        run_time=0
        
        echo "Launching lora_pkt_fwd: retry = $retry" >&2
        
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

        /opt/ttn-gateway/packet_forwarder/lora_pkt_fwd/lora_pkt_fwd | python lora_pkt_fwd_formatter.py | python post_processing_gw.py | python log_gw.py &
        sleep 10
        
        #if the radio concentrator has failed to start then the while loop will not run
        while pgrep -x "$SERVICE" >/dev/null: ; do
                sleep $PID_CHECK_PERIOD
                
                if [ $run_time -lt $MIN_RUN_TIME ]; then
                	run_time=$[$run_time + $PID_CHECK_PERIOD]
                fi	
                
                if [ $run_time -ge $MIN_RUN_TIME ]; then
                        #if util_pkt_logger has been running for more than MIN_RUN_TIME then we reset retry
                        #because the radio concentrator has successfully started at least once for a while
                        retry=1
                fi
        done
        
        echo "lora_pkt_fwd not running for some reason" >&2
        echo "trying to restart" >&2
        sleep 2
        
        kill $(ps aux | grep -e post_processing -e log_gw -e lora_pkt_fwd | awk '{print $2}') >&2
        
        sleep_time=$[$BACKOFF_RETRY_TIME * $retry]
        
        if [ $sleep_time -gt $MAX_SLEEP_TIME ]; then
        	sleep_time=$MAX_SLEEP_TIME
        fi
        	
        echo "retry=$retry. Will retry in $sleep_time seconds" >&2
        sleep $sleep_time
        
        retry=$[$retry + 1]
done
echo "Maximum retries. Exiting" >&2
exit 0
) &
