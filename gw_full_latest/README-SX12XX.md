Notice
======

This `README-SX12XX.md` file has been written when the low-level SPI layer for Arduino compatibility was `arduPi`, mainly targeting 32-bit OS. In January 2021, we decided to provide support for next 64-bit OS, therefore we move to `wiringPi` with a light SPI wrapper `arduinoPi`. `wiringPi` is now the default low-level SPI layer in the GitHub source distribution. If you are using the latest distribution using `wiringPi`, please read the `README-WiringPi-64bit-OS.md` file.

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
	- use `radio.makefile` to indicate the radio type
		- `SX12XX=SX1272 | SX126X | SX127X | SX128X`
	- if there is a `/boot/radio.makefile` file, it will have priority
  - `SX12XX=SX1272` means that you want to keep the old Libelium SX1272 library version		
  - then `make lora_gateway` or
  - simply using the `scripts/compile_lora_gw.sh` script which is the recommended option
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

- `SX1280_ranging_slave` is based on the SX128XLT ranging slave example from the SX12XX library. It is exactly the same code than the Arduino example `Arduino_LoRa_SX1280_ranging_slave` that can be compiled on the Raspberry in order to use the gateway as the slave for distance computation at the end-device. Type `make SX1280_ranging_slave` or  `make SX1280_ranging_slave_pi2` or `make SX1280_ranging_slave_pi4`, then `sudo ./ranging_slave`. Default LoRa settings are defined in `SX1280_ranging_slave.h`. You can add arguments to dynamically change bw, sf and frequency: `sudo ./ranging_slave --bw 400 --sf 8 --freq 2403.0`. 

Mapping Arduino pin number to Raspberry pin number
--------------------------------------------------

For the latest distribution using `wiringPi` please read the notice at the beginning of this page.

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
			
			
New PCBs
--------

The RFM95W breakout initially designed for the HopeRF RFM95W (referred to as old RFM95W breakout) can actually also host the recent NiceRF SX1262 as both radio modules have similar pinout. However, we updated the old RFM95W breakout's wiring to also expose DIO2 and DIO1 on the Raspberry header to better support the NiceRF SX1262 module (DIO2 location on the RFM95W is the BUSY pin on the NiceRF SX1262 and DIO1 pin is used for RX/TX done interrupt on the NiceRF SX1262 instead of the DIO0 pin on the RFM95W). BUSY pin is needed for the SX126X so if you are using the old RFM95W breakout you need to solder a wire. Our modified communication library does not need DIO1 pin. When updating the RFM95W breakout we also added a 4-pin header to easily connect a small OLED screen to a Raspberry using the I2C bus (SCL and SDA pin). Pin order from left to right on the OLED should be GND, VCC, SCL and SDA.

We also made a variant to support the NiceRF SX1280 module which provides LoRa modulation on the 2.4GHz band (I really don't know why the SX1280 did not use the same pinout than the SX1262 which is similar to the RFM95W!). All these breakout PCBs are illustrated in the following figure.

![](https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/RFM95Breakout.jpg)

The ProMini LoRa RFM95W PCB for the Arduino ProMini has been slightly modified to reflect that it can work with the NiceRF SX1262 as well. It is then referred to as ProMini LoRa RFM95W/NiceRF SX1262. We then created the ProMini LoRa NiceRF SX1280 for the new SX1280.

![](https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/ProMiniLoRaBreakout.jpg)

These new !PCBs have 4 solder pads that can be soldered together if you want to connect various DIO pins. Some of these pads are by default connected. If you want to use the connected Arduino pins for other purposes, just cut the wire between the pads with a cutter and test for discontinuity.

- ProMini LoRa RFM95W/NiceRF SX1262 with an RFM95W. Connected by default: DIO0 to D2, DIO1 to D3, RST to D4. Not connected by default: DIO2 to D5. Since our modified SX12XX library is not using DIO0 nor DIO1 by default, you can disconnect to use pin D2 and D3 for your application.
- ProMini LoRa RFM95W/NiceRF SX1262 with a NiceRF SX1262. Connected by default: DIO1 to D3 and RST to D4. Not connected by default: BUSY to D5. You **need** to connect BUSY to D5 with the corresponding pad. We chose to keep it this way because RFM95W are still very popular. Since our modified SX12XX library is not using DIO1 by default, you can disconnect to use pin D3 for your application.
- ProMini LoRa NiceRF SX1280 with a NiceRF SX1280. Connected by default: DIO1 to D2, BUSY to D3 and RST to D4. Not connected by default: DIO2 to D5. Since our modified SX12XX library is not using DIO1 by default, you can disconnect to use pin D2 for your application.
			
Early pictures
--------------

Here are some pictures of our early test with Ebyte SX1280 (E28-2G4M12S SPI). As you can see both device and gateway are fully operational. Here frequency is 2403000000Hz and datarate is SF12BW203. The PCB breakouts for the Ebyte SX1280 has been provided by my colleague Mojtaba. Soldering the Ebyte SX1280 was quite challenging!

<img src="https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/SX1280-earlydevice.jpg" width="400"><img src="https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/SX1280-earlygw.jpg" width="400">

With the SX1280 for both device and gateway, upload to WAZIUP cloud is shown below.

<img src="https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/SX1280-gw-output.png">

Upload to LoRaWAN TTN network server with LoRa 2.4GHz is shown below. Data are sent with a device running `Arduino_LoRa_SX12XX_temp`. See how the LoRa modulation's bandwith is advertised as 125kHz while the real bandwidth that was used for the communication was 203.125kHz.

<img src="https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/SX1280-ttn.png">

Could not wait to receive the NiceRF SX1262 so here is a very "dirty" SX1262 device with an Ebyte SX1262 (E22-900M22S SPI 868MHz) connected to an Arduino ProMini. Again, soldering was challenging, especially that I did not have a PCB breakout for the Ebyte SX1262! After making one radio, I did not have the courage to solder a second one, so I only tested at device side to check that data can be received by an RFM95W (SX1276) gateway. Tests were successful and an SX1262 can talk to an SX1276 without any issue.

<img src="https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/SX1262-earlydevice.jpg" width="400"><img src="https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/SX1262-earlydevicecloseup.jpg" width="400">

Two days after the Ebyte SX1262 test, I finally received the 2 NiceRF SX1262 I've ordered from AliExpress! Since I have some old RFM95W breakout left, I just solder the NiceRF SX1262 on the old RFM95W breakout. As stated previously, both RFM95W and NiceRF SX1262 have almost same pinout. See how the BUSY pin of the SX1262 is connected with a wire to the Raspberry's GPIO25 pin. 

<img src="https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/SX1262-fullgw.jpg" width="400">

For the device, I did the same procedure with one of the old ProMini LoRa RFM95W PCB I still have without waiting for the updated ProMini LoRa RFM95W/NiceRF SX1262 PCB. Here the BUSY pin is connected to D5 using the DIO2 connecting pads.

<img src="https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/SX1262-ProMini.jpg" width="550">

At least, I've finally received my SX1280 breakout and ProMini LoRa SX1280 PCB on Nov 25th, 2020!

<img src="https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/SX1280-fullgw.jpg" width="550">

<img src="https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/SX1280-ProMini.jpg" width="550">

Now, you can fully benefit from the versatile low-cost single-channel gateway with all LoRa chip family SX126X, SX127X and SX128X!

Enjoy!
C. Pham		