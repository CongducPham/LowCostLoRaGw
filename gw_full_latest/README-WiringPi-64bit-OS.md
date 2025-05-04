wiringPi as the low-level SPI library
=====================================

In January 2021, we decided to provide support for next 64-bit OS. The main reason is to prepare the whole low-cost Raspberry-based gateway to embed advance AI frameworks that are mostly based on 64-bit OS. To do so, the low-level SPI library on the Raspberry has moved from Libelium's `arduPi` lib to Gordon's `wiringPi` lib because `arduPi` relies heavily on direct memory accesses that are difficult to port. Note that on 32-bit OS, `arduPi`can still be used if you do not want to move to `wiringPi`.
	
What have been modified
----------------------- 

- on 32-bit OS, the original `wiringPi` distribution, although not maintained anymore by the author, can be installed (http://wiringpi.com/)

- on RPI4, even with 32-bit OS, it is necessary to get the unofficial Mirror/Fork from https://github.com/WiringPi/WiringPi to get `wiringPi` supporting the RPI4

- on 64-bit OS, the "UNOFICIAL WiringPi library for RPi 64bit OS" from https://github.com/TheNextLVL/wiringPi provided a solution but failed to run on an RPI4

- finally, I merged the contributions from both https://github.com/WiringPi/WiringPi and https://github.com/TheNextLVL/wiringPi to get `wiringPi` supporting RPI4 and also running on 64-bit OS:
	- take this updated version https://github.com/CongducPham/LowCostLoRaGw/raw/master/zip/wiringPi-64bit-RPI4.tgz
		- wget https://github.com/CongducPham/LowCostLoRaGw/raw/master/zip/wiringPi-64bit-RPI4.tgz, then
		- tar xvfz wiringPi-64bit-RPI4.tgz
		- cd wiringPi-64bit-RPI4
		- sudo ./build
		
- while `arduPi` uses Arduino-like pin numbering, `wiringPi` has its own pin numbering system. See section on pin numbering below

- even with `wiringPi`, the SPI-related functions offered by `arduPi` (which mimicks those of the Arduino SPI lib) is still used as API. The objective is also to be able to compile on the Raspberry codes that are written in Arduino style, possibly even for Arduino platforms. In this way, it is easier to maintain almost the same source code for both platforms. Therefore, with `wiringPi`, `arduinoPi.cpp` is a wrapper to `wiringPi` SPI calls.

Compiling
---------
		
- the `makefile` has been updated
	- use `raspberry.makefile` to indicate the SPI lib
		- `SPILIB=ARDUPI | WIRINGPI`
	- if there is a `/boot/raspberry.makefile` file, it will have priority
  - then `make lora_gateway` or
  - simply using the `scripts/compile_lora_gw.sh` script which is the recommended option		

- on 32-bit OS, `arduPi`can still be used 

- for testing purposes, you can however manually compile for a specific target
	- with `arduPi`
  	- `make SX1272_lora_gateway` or `make SX1272_lora_gateway_pi2` or `make SX1272_lora_gateway_pi2` (Libelium 1272/1276 lib)
		- `make SX126X_lora_gateway` or `make SX126X_lora_gateway_pi2` or `make SX126X_lora_gateway_pi4` (Stuart Robinson's SX12XX lib)	
  	- `make SX127X_lora_gateway` or `make SX127X_lora_gateway_pi2` or `make SX127X_lora_gateway_pi4` (Stuart Robinson's SX12XX lib)	
  	- `make SX128X_lora_gateway` or `make SX128X_lora_gateway_pi2` or `make SX128X_lora_gateway_pi4` (Stuart Robinson's SX12XX lib)	
	
	- with `wiringPi`
		- `make SX1272_lora_gateway_wiring` (Libelium 1272/1276 lib)
		- `make SX126X_lora_gateway_wiring` (StuartProject's SX12XX lib)	
		- `make SX127X_lora_gateway_wiring` (StuartProject's SX12XX lib)	
		- `make SX128X_lora_gateway_wiring` (StuartProject's SX12XX lib)

Pin numbering with wiringPi
---------------------------

Remember that pin numbering now follows the `wiringPi` which is illustrated below. Be sure to indicate the pin number from "wPi" column when you identified on which physical pin you connected the various wires needed by the radio module

```
+-----+-----+---------+--B Plus--+---------+-----+-----+
| BCM | wPi |   Name  | Physical | Name    | wPi | BCM |
+-----+-----+---------+----++----+---------+-----+-----+
|     |     |    3.3v |  1 || 2  | 5v      |     |     |
|   2 |   8 |   SDA.1 |  3 || 4  | 5V      |     |     |
|   3 |   9 |   SCL.1 |  5 || 6  | 0v      |     |     |
|   4 |   7 | GPIO. 7 |  7 || 8  | TxD     | 15  | 14  |
|     |     |      0v |  9 || 10 | RxD     | 16  | 15  |
|  17 |   0 | GPIO. 0 | 11 || 12 | GPIO. 1 | 1   | 18  |
|  27 |   2 | GPIO. 2 | 13 || 14 | 0v      |     |     |
|  22 |   3 | GPIO. 3 | 15 || 16 | GPIO. 4 | 4   | 23  |
|     |     |    3.3v | 17 || 18 | GPIO. 5 | 5   | 24  |
|  10 |  12 |    MOSI | 19 || 20 | 0v      |     |     |
|   9 |  13 |    MISO | 21 || 22 | GPIO. 6 | 6   | 25  |
|  11 |  14 |    SCLK | 23 || 24 | CE0     | 10  | 8   |
|     |     |      0v | 25 || 26 | CE1     | 11  | 7   |
|   0 |  30 |   SDA.0 | 27 || 28 | SCL.0   | 31  | 1   |
|   5 |  21 | GPIO.21 | 29 || 30 | 0v      |     |     |
|   6 |  22 | GPIO.22 | 31 || 32 | GPIO.26 | 26  | 12  |
|  13 |  23 | GPIO.23 | 33 || 34 | 0v      |     |     |
|  19 |  24 | GPIO.24 | 35 || 36 | GPIO.27 | 27  | 16  |
|  26 |  25 | GPIO.25 | 37 || 38 | GPIO.28 | 28  | 20  |
|     |     |      0v | 39 || 40 | GPIO.29 | 29  | 21  |
+-----+-----+---------+----++----+---------+-----+-----+
| BCM | wPi |   Name  | Physical | Name    | wPi | BCM |
+-----+-----+---------+--B Plus--+---------+-----+-----+
```

- the wiring of NSS, NRESET, BUSY and DIO1 pins are defined in the `SX126X_lora_gateway.h`, `SX127X_lora_gateway.h` and `SX128X_lora_gateway.h` files. You can overwrite these definitions in `radio.makefile`: for instance `-DNRESET=7`. By default, the numbering still refers to the `arduPi` pin numbering system: NSS=10 (GPIO8), BUSY=5->GPIO25, NRESET=6->GPIO4, DIO1=2->GPIO18. If you move to `wiringPi` then indicate in `radio.makefile`:

	- `-DNRESET=0` for WaziHat. Correspond to GPIO17
	- `-DNRESET=7` for our radio module breakout. Correspond to GPIO4
	- `-DRFBUSY=6` for our radio module breakout (SX1262 and SX1282). Correspond to GPIO25. Only needed for SX1262/SX1282 
	- NSS can remain to 10 because `wiringPi` and `arduPi` has same number
	- e.g. `CFLAGS+=-DNSS=10 -DNRESET=7 -DRFBUSY=6`

Enjoy!
C. Pham		