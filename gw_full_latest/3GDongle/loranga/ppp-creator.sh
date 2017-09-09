#!/bin/sh

#echo "install ppp"
#sudo apt-get install ppp

echo "creating directories"
mkdir -p /etc/chatscripts
mkdir -p /etc/ppp/peers

echo "creating script file : /etc/chatscripts/quectel-chat-connect"
echo "
ABORT \"BUSY\"
ABORT \"NO CARRIER\"
ABORT \"NO DIALTONE\"
ABORT \"ERROR\"
ABORT \"NO ANSWER\"
TIMEOUT 30
\"\" AT
OK ATE0
OK ATI;+CSUB;+CSQ;+CPIN?;+COPS?;+CGREG?;&D2
# Insert the APN provided by your network operator, default apn is $1
OK AT+CGDCONT=1,\"IP\",\"\\T\",,0,0
OK ATD*99#
CONNECT" > /etc/chatscripts/quectel-chat-connect


echo "creating script file : /etc/chatscripts/quectel-chat-disconnect"
echo "
ABORT \"ERROR\"
ABORT \"NO DIALTONE\"
SAY \"\nSending break to the modem\n\"
""  +++
""  +++
""  +++
SAY \"\nGoodbay\n\"" > /etc/chatscripts/quectel-chat-disconnect


echo "creating script file : /etc/ppp/peers/gprs"
echo "
/dev/$2 115200
# The chat script, customize your APN in this file
connect 'chat -s -v -f /etc/chatscripts/quectel-chat-connect -T $1'
# The close script
disconnect 'chat -s -v -f /etc/chatscripts/quectel-chat-disconnect'
# Hide password in debug messages
hide-password
# The phone is not required to authenticate
noauth
# Debug info from pppd
debug
# If you want to use the HSDPA link as your gateway
defaultroute
# pppd must not propose any IP address to the peer
noipdefault
# No ppp compression
novj
novjccomp
noccp
ipcp-accept-local
ipcp-accept-remote
local
# For sanity, keep a lock on the serial line
lock
modem
dump
nodetach
# Hardware flow control
nocrtscts
remotename 3gppp
ipparam 3gppp
ipcp-max-failure 30
# Ask the peer for up to 2 DNS server addresses
usepeerdns" > /etc/ppp/peers/gprs

echo "\n\nUse \"sudo pppd call gprs\" command and Surf"