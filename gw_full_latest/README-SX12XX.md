Supporting the SX12XX LoRa chip family
======================================

In October 2020, we decided to provide support for the whole SX12XX LoRa chip family: SX126X, SX127X and SX128X. SX126X are considered as the next generation LoRa chip in the sub-GHz band while SX128X are targeting the ISM 2.4GHz band to remove some of the tight limitations of sub-GHz band's regulations.

Therefore, we moved from the Libelium's SX1272 library that we have been using and adapting from 2015 to the SX12XX LoRa library written by Stuart Robinson (https://github.com/StuartsProjects/SX12XX-LoRa). Note that we have modified the Stuart Robinson SX12XX library to add our advanced features (CAD, Carrier Sense, PA_BOOST for SX127X, ...) and to adapt it to the framework that we have been using for the low-cost LoRa gateway. The original version from Stuart Robinson has been forked to our github (https://github.com/CongducPham/SX12XX-LoRa) to serve as reference. Our modified version is directly included in the LowCostLoRaGw repository (https://github.com/CongducPham/LowCostLoRaGw), see below.

What have been modified
-----------------------

The objective is to seamlessly support the whole SX12XX family chip with the same source code for the low-level LoRa radio bridge. For that purpose, the following changes have been made:

- the old `lora_gateway.cpp` code that was written to work exclusively with SX127X chips using the modified SX1272 Libelium's library has been renamed as `SX1272_lora_gateway.cpp`

- the old SX1272 library (`SX1272.cpp` and `SX1272.h`) are moved to a folder called `SX1272`

- a new version of the gateway low-level radio bridge using the SX12XX library is called `SX12XX_lora_gateway.cpp`. There are 3 `.h` files for specific configurations of the SX126X, SX127X and SX128X chips: `SX126X_lora_gateway.h`, `SX127X_lora_gateway.h` and `SX128X_lora_gateway.h`. For more information on how to use the SX12XX library, please refer to the official SX12XX github repository at https://github.com/StuartsProjects/SX12XX-LoRa.

- folder `SX12XX` contains the modified SX12XX library. Only files related to SX126X, SX127X and SX128X have been modified for our purposes.

- the `makefile` has been updated
  - you can change the definition of `lora_gateway`, `loragateway_pi2` and `lora_gateway_pi4` targets according to to your radio module (SX126X, SX127X or SX128X) and low-level lib (Libelium 1272/1276 lib or Stuart Robinson's SX12XX lib)
  - default target is for SX127X family using the new SX12XX library: `SX127X_lora_gateway` or `SX127X_lora_gateway_pi2` or `SX127X_lora_gateway_pi4` 
  	- if you want to keep the old Libelium SX1272 library version, then simply change `lora_gateway`, `loragateway_pi2` and `lora_gateway_pi4` to `SX1272_lora_gateway`, `SX1272_lora_gateway_pi2` and `SX1272_lora_gateway_pi4` respectively
  - then
    - `make lora_gateway`, or
  	- `make lora_gateway_pi2`, or
    - `make lora_gateway_pi4`, or
    - simply using the `scripts/compile_lora_gw.sh` script that automatically determines the Raspberry version to call the appropriate target
  - this is the recommended option with `scripts/compile_lora_gw.sh` because compilation of the low-level LoRa radio bridge will be realized seamlessly as in the previous framework
  - for testing purposes, you can however compile for a specific target
    - `make SX1272_lora_gateway` or `make SX1272_lora_gateway_pi2` or `make SX1272_lora_gateway_pi2` (Libelium 1272/1276 lib)
    - `make SX126X_lora_gateway` or `make SX126X_lora_gateway_pi2` or `make SX126X_lora_gateway_pi4` (Stuart Robinson's SX12XX lib)	
    - `make SX127X_lora_gateway` or `make SX127X_lora_gateway_pi2` or `make SX127X_lora_gateway_pi4` (Stuart Robinson's SX12XX lib)	
    - `make SX128X_lora_gateway` or `make SX128X_lora_gateway_pi2` or `make SX128X_lora_gateway_pi4` (Stuart Robinson's SX12XX lib)	

-  so-called LoRa mode from mode 1 to mode 10, and the additional mode 11 for LoRaWAN are still preserved to maintain compatibility. However, they are not handled by the SX12XX library. Instead, the `start_gw.py` Python script that launches the gateway will convert the LoRa mode into their BW and SF combination as defined previously (see https://github.com/CongducPham/LowCostLoRaGw#annexa-lora-mode-and-predefined-channels).

- pre-defined frequency channels (see https://github.com/CongducPham/LowCostLoRaGw#annexa-lora-mode-and-predefined-channels) are also preserved but they are defined in the `SX126X_lora_gateway.h`, `SX127X_lora_gateway.h` and `SX128X_lora_gateway.h` files. 3 additional channel for the 2.4GHz band of the SX128X chip are: CH_00_2400 = 2403000000Hz, CH_01_2400 = 2425000000Hz and CH_02_2400 = 2479000000Hz. CH_00_2400 is the default frequency in 2.4GHz.

- you can select the frequency band in `radio.makefile` as it was done previously by indicating `-DBAND868` or `-DBAND900` or `-DBAND433`. There is no `-DBAND2400` in `radio.makefile` because compiling for SX128X target automatically set `-DBAND2400`.

- for LoRaWAN mode in 2.4GHz, as there is no LoRaWAN specifications for frequency usage, the uplink and the downlink frequency (as well as data rate) are the same, even if RX2 is targeted. By defaut, it is CH_00_2400 = 2403000000Hz.

Notice
------

- At time of writing, not all our advanced features have been ported to SX128X chips. Especially the computation of the time-on-air. We are working to these issues but there is very little impact on the gateway functionalities.
	
Enjoy!
C. Pham		