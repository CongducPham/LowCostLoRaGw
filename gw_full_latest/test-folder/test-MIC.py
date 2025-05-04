----
# step-by-step test

import LoRaWAN
from LoRaWAN.MHDR import MHDR
appskey = [0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c]

#you can make nwkskey different from appskey

nwkskey = [0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c]

#use appskey to encode payload

lorawan = LoRaWAN.new(appskey)

# 0x00000164 = 356

devaddr = [0x64, 0x01, 0x00, 0x00]
lorawan.create(MHDR.UNCONF_DATA_UP, {'devaddr': devaddr, 'data': list(map(ord, 'hello')) })
lorawan.to_raw()

#[64, 100, 1, 0, 0, 0, 0, 0, 1, 106, 76, 113, 233, 165, 195, 182, 118, 40]

print '[{}]'.format(', '.join(hex(x) for x in lorawan.to_raw()))

#[0x40, 0x64, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x6a, 0x4c, 0x71, 0xe9, 0xa5, 0xc3, 0xb6, 0x76, 0x28]

lorawan.get_mic()

#[195, 182, 118, 40]

#now set key to nwkskey to get MIC

lorawan.__init__(nwkskey)
lorawan.compute_mic()

#[195, 182, 118, 40]

print '[{}]'.format(', '.join(hex(x) for x in lorawan.compute_mic()))

#[0xc3, 0xb6, 0x76, 0x28]

#set the correct MIC

lorawan.set_mic(lorawan.compute_mic())

print '[{}]'.format(', '.join(hex(x) for x in lorawan.to_raw()))

#[0x40, 0x64, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x6a, 0x4c, 0x71, 0xe9, 0xa5, 0xc3, 0xb6, 0x76, 0x28]

----

# using loraWAN_get_MIC

from loraWAN import loraWAN_get_MIC

print loraWAN_get_MIC(356,'\\!TC/20.10/HU/1.00')

#[100, 1, 0, 0]
#[0x64, 0x1, 0x0, 0x0]
#[0x58, 0xa5, 0xbb, 0x8c]

----

LoRa temperature sensor, extended version
Arduino Pro Mini detected
ATmega328P detected
SX1276
SX1276 LF/HF calibration
...
Get back previous sx1272 config
Using packet sequence number of 15
Forced to use default parameters
Using node addr of 356
Using idle period of 30
Setting Mode: state 0
Setting Channel: state 0
Setting Power: state 0
Setting node addr: state 0
SX1272 successfully configured
Sending hello
Real payload size is 5
hello
plain payload hex
68 65 6C 6C 6F 
Encrypting
encrypted payload
6A 4C 71 E9 A5 
calculate MIC with NwkSKey
transmitted LoRaWAN-like packet:
MHDR[1] | DevAddr[4] | FCtrl[1] | FCnt[2] | FPort[1] | EncryptedPayload | MIC[4]
40 64 01 00 00 00 00 00 01 6A 4C 71 E9 A5 C3 B6 76 28 
end-device uses encapsulated LoRaWAN packet format only for encryption
--> CarrierSense1
--> CAD duration 553
OK1
--> waiting for 1 CAD = 62
--> CAD duration 553
OK2
LoRa pkt size 18
LoRa pkt seq 15
LoRa Sent in 1738
LoRa Sent w/CAD in 2294
Packet sent, state 0
Wait for 4000
Wait for incoming packet
No packet
Switch to power saving mode
Successfully switch LoRa module in sleep mode