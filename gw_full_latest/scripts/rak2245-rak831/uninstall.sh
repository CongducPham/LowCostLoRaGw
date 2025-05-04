#!/bin/bash

./disable_at_boot.sh

pushd /home/pi/lora_gateway

rm -rf start_lpf.log start_upl.log start_lpf_pprocessing_gw.sh start_upl_pprocessing_gw.sh lora_pkt_fwd_formatter.py util_pkt_logger_formatter.py local_conf.json global_conf.json
popd
