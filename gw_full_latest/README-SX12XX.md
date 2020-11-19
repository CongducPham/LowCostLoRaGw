Supporting the SX12XX LoRa chip family
======================================

In October 2020, we decided to provide support for the whole SX12XX LoRa chip family: SX126X, SX127X and SX128X. SX126X are considered as the next generation LoRa chip in the sub-GHz band while SX128X are targeting the ISM 2.4GHz band to remove some of the tight limitations of sub-GHz band's regulations.

Therefore, we moved from the Libelium's SX1272 library that we have been using and adapting since 2015 to the SX12XX LoRa library written by Stuart Robinson (https://github.com/StuartsProjects/SX12XX-LoRa). Note that we have modified the Stuart Robinson SX12XX library to add our advanced features (CAD, Carrier Sense, PA_BOOST for SX127X, ack transactions,...) and to adapt it to the framework that we have been using for the low-cost LoRa gateway. The original version from Stuart Robinson has been forked to our github (https://github.com/CongducPham/SX12XX-LoRa) to serve as reference. Our modified version is directly included in the LowCostLoRaGw repository [https://github.com/CongducPham/LowCostLoRaGw](https://github.com/CongducPham/LowCostLoRaGw), see below.

What have been modified
-----------------------

The objective is to seamlessly support the whole SX12XX family chip with the same source code for the low-level LoRa radio bridge. For that purpose, the following changes have been made:

- the old `lora_gateway.cpp` code that was written to work exclusively with SX127X chips using the modified SX1272 Libelium's library has been renamed as `SX1272_lora_gateway.cpp`

- the old SX1272 library (`SX1272.cpp` and `SX1272.h`) are moved to a folder called `SX1272`

- a new version of the gateway low-level radio bridge using the SX12XX library is called `SX12XX_lora_gateway.cpp`. There are 3 `.h` files for specific configurations of the SX126X, SX127X and SX128X chips: `SX126X_lora_gateway.h`, `SX127X_lora_gateway.h` and `SX128X_lora_gateway.h`. For more information on how to use the SX12XX library, please refer to the official SX12XX github repository at https://github.com/StuartsProjects/SX12XX-LoRa.

- folder `SX12XX` contains the modified SX12XX library. Only files related to SX126X, SX127X and SX128X have been modified for our purposes.

- the `makefile` has been updated
  - you can change the definition of `lora_gateway`, `lora_gateway_pi2` and `lora_gateway_pi4` targets according to selected radio module (SX126X, SX127X or SX128X) and low-level lib (Libelium 1272/1276 lib or Stuart Robinson's SX12XX lib)
  - default target is for SX127X family using the new SX12XX library: `SX127X_lora_gateway` or `SX127X_lora_gateway_pi2` or `SX127X_lora_gateway_pi4` 
  	- if you want to keep the old Libelium SX1272 library version, then simply change `lora_gateway`, `lora_gateway_pi2` and `lora_gateway_pi4` targets to `SX1272_lora_gateway`, `SX1272_lora_gateway_pi2` and `SX1272_lora_gateway_pi4` respectively
  - then
    - `make lora_gateway`, or
  	- `make lora_gateway_pi2`, or
    - `make lora_gateway_pi4`, or
    - simply using the `scripts/compile_lora_gw.sh` script that will automatically determine the Raspberry model to compile for the appropriate target
    - using `scripts/compile_lora_gw.sh` is the recommended option because compilation of the low-level LoRa radio bridge will be realized seamlessly as in the previous framework
  - for testing purposes, you can however manually compile for a specific target
    - `make SX1272_lora_gateway` or `make SX1272_lora_gateway_pi2` or `make SX1272_lora_gateway_pi2` (Libelium 1272/1276 lib)
    - `make SX126X_lora_gateway` or `make SX126X_lora_gateway_pi2` or `make SX126X_lora_gateway_pi4` (Stuart Robinson's SX12XX lib)	
    - `make SX127X_lora_gateway` or `make SX127X_lora_gateway_pi2` or `make SX127X_lora_gateway_pi4` (Stuart Robinson's SX12XX lib)	
    - `make SX128X_lora_gateway` or `make SX128X_lora_gateway_pi2` or `make SX128X_lora_gateway_pi4` (Stuart Robinson's SX12XX lib)	

-  so-called LoRa mode from mode 1 to mode 10, and the additional mode 11 for LoRaWAN are still preserved to maintain compatibility. However, they are not handled by the SX12XX library. Instead, the `start_gw.py` Python script that launches the gateway will convert the LoRa mode into their BW and SF combination as defined previously (see https://github.com/CongducPham/LowCostLoRaGw#annexa-lora-mode-and-predefined-channels).

- pre-defined frequency channels (see https://github.com/CongducPham/LowCostLoRaGw#annexa-lora-mode-and-predefined-channels) are also preserved but they are defined in the `SX126X_lora_gateway.h`, `SX127X_lora_gateway.h` and `SX128X_lora_gateway.h` files. 3 additional channel for the 2.4GHz band of the SX128X chip are: CH_00_2400 = 2403000000Hz, CH_01_2400 = 2425000000Hz and CH_02_2400 = 2479000000Hz. CH_00_2400 is the default frequency in 2.4GHz.

- the wiring of NSS, NRESET, BUSY and DIO1 pins are defined in the `SX126X_lora_gateway.h`, `SX127X_lora_gateway.h` and `SX128X_lora_gateway.h` files. You can overwrite these definitions in `radio.makefile`: for instance `-DNRESET=7`. Important notice: the pin numbering follows the Arduino numbering. See below for the mapping between Arduino pin number to Raspberry pin number. By default: NSS=10->GPIO8, BUSY=5->GPIO25, NRESET=6->GPIO4, DIO1=2->GPIO18. See [https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/RFMonRPI.jpg](https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/RFMonRPI.jpg).

- you can select the frequency band in `radio.makefile` as it was done previously by indicating `-DBAND868` or `-DBAND900` or `-DBAND433`. There is no `-DBAND2400` in `radio.makefile` because compiling for SX128X target automatically uses 2400MHz band.

- for LoRaWAN mode in 2.4GHz, as there is no LoRaWAN specifications for frequency usage, the uplink and the downlink frequency (as well as data rate) are identical, even if RX2 is targeted. By defaut, it is CH_00_2400 = 2403000000Hz.

- `SX12XX_simple_lora_gateway.cpp` is a very simple receiver for testing purposes. You can use `make SX127X_simple_lora_gateway_pi2` to build it for RPI2 using SX127X LoRa chip. Other versions (RPI and RPI4, SX126X and SX128X) are also available, similarly to `SX12XX_lora_gateway.cpp`.

- `SX12XX_simple_lora_transmitter` is a very simple LoRa transmitter for the RPI that can also be used for testing purposes. You can use `make SX127X_simple_lora_transmitter_pi2` to build it for RPI2 using SX127X LoRa chip. Other versions (RPI and RPI4, SX126X and SX128X) are also available, similarly to `SX12XX_simple_lora_gateway.cpp`.

Mapping Arduino pin number to Raspberry pin number
--------------------------------------------------

We use the `arduPi` layer to make Arduino-like program running on the Raspberry. The advantage is that the same code can be compiled for both platforms, at least for the main features. The `arduPi` layer translate pin number expressed in the Arduino system into Raspberry pin number. The mapping is as follows:

```
       Arduino                         Raspberry
          D2                            GPIO18
          D3                            GPIO23
          D4                            GPIO24
          D5                            GPIO25
          D6                            GPIO4
          D7                            GPIO17
          D8                            GPIO21 or GPIO27
          D9                            GPIO22
          D10                           GPIO8 (CE0)
          D11                           GPIO10 (MOSI)
          D12                           GPIO9 (MISO)
          D13                           GPIO11 (SCK)
```

GPIO pin are as follows on the Raspberry header:

<img src="https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/GPIO_Pi2.png" width="400">

Arduino examples
----------------

Along with the support of these new LoRa chips, we updated our Arduino examples. See [https://github.com/CongducPham/LowCostLoRaGw/tree/master/Arduino](https://github.com/CongducPham/LowCostLoRaGw/tree/master/Arduino)
			

SX128X: Using LoRaWAN-like communications and uploading to LoRaWAN Network Servers with LoRa 2.4GHz
---------------------------------------------------------------------------------------------------

Although there is an increasing interest and demand for LoRa 2.4GHz in order to facilitate worldwide deployment, there is no LoRaWAN specifications for this band. Therefore most of LoRaWAN network servers do not support yet datarate offered by LoRa 2.4GHz. For instance, if the LoRa bandwidth differs from 125kHz, most of network servers will refuse the uplink packet. Also, there is no definition of frequencies for both uplink and downlink.

With the support of LoRa 2.4GHz, our single-channel can easily push received packets to LoRaWAN network servers such as TTN or ChirpStack by declaring the bandwidth to be 125kHz although the real bandwidth can be different. `CloudTTN.py` and `CloudChirpStack.py` python scripts set the LoRa modulation's bandwidth to be 125kHz to comply with actual LoRaWAN specifications.

Once uplink packets are pushed to the network server, downlink transmission might occur. With LoRa 2.4GHz the low-level gateway will not change the downlink frequency even when receive window 2 (RX2) is targeted. 

With Arduino boards, one of the most popular LoRaWAN library is the LMIC library that has been mainly written to work with SX127X chips and, most importantly, to deal with sub-GHz regulations such as radio duty-cycle. Therefore, when using LoRaWAN-like communication with LoRa 2.4GHz, we are not using LMIC but our light and limited LoRaWAN support targeting mainly communication with a single-channel gateway. [`Arduino_LoRa_SX12XX_temp`](https://github.com/CongducPham/LowCostLoRaGw/tree/master/Arduino/Arduino_LoRa_SX12XX_temp) is an example of LoRaWAN-like communication with support of only downlink in ABP mode that can be used for uploading LoRaWAN packet to a LoRaWAN network server. 
			
Early pictures
--------------

Here are some pictures of our early test with Ebyte SX1280. As you can see both device and gateway are fully operational. Here frequency is 2403000000Hz and datarate if SF12BW203125.

<img src="https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/SX1280-earlydevice.jpg" width="400"><img src="https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/SX1280-earlygw.jpg" width="400">

Upload to WAZIUP cloud is shown below. Now, you can benefit from the versatile low-cost single-channel gateway with all LoRa chip family: SX126X, SX127X and SX128X.

<img src="https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/SX1280-fullgw.png">

Upload to LoRaWAN TTN network server with LoRa 2.4GHz is shown below. Data are sent with a device running `Arduino_LoRa_SX12XX_temp`. See how the LoRa modulation's bandwith is advertised as 125kHz while the real bandwidth that was used for the communication was 203.125kHz.

<img src="https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/SX1280-ttn.png">
	
Enjoy!
C. Pham		