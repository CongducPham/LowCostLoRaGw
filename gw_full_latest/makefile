include radio.makefile
include raspberry.makefile

#these definitions, if files exist will overwrite previous definitions
-include /boot/radio.makefile
-include /boot/raspberry.makefile


RPIMODEL=$(shell /home/pi/lora_gateway/scripts/test_raspberry_model.sh | grep ">" | cut -d ">" -f2)

#to test compilation on various models by writing an RPI model in the file
#RPIMODEL=$(shell cat /home/pi/lora_gateway/arch.txt)
	
ifeq ($(RPIMODEL),RPI1/0)
rpimodel=
else ifeq ($(RPIMODEL),RPI2/3)
rpimodel=_pi2
else ifeq ($(RPIMODEL),RPI4)
rpimodel=_pi4
endif
	
ifeq ($(SPILIB),ARDUPI)
ifeq ($(SX12XX),SX126X)
lora_gateway: SX126X_lora_gateway$(rpimodel)
#lora_gateway_pi2: SX126X_lora_gateway_pi2
#lora_gateway_pi4: SX126X_lora_gateway_pi4
else ifeq ($(SX12XX),SX127X)
lora_gateway: SX127X_lora_gateway$(rpimodel)
#lora_gateway_pi2: SX127X_lora_gateway_pi2
#lora_gateway_pi4: SX127X_lora_gateway_pi4
else ifeq ($(SX12XX),SX128X)
lora_gateway: SX128X_lora_gateway$(rpimodel)
#lora_gateway_pi2: SX128X_lora_gateway_pi2
#lora_gateway_pi4: SX128X_lora_gateway_pi4
else ifeq ($(SX12XX),SX1272)
lora_gateway: SX1272_lora_gateway$(rpimodel)
#lora_gateway_pi2: SX1272_lora_gateway_pi2
#lora_gateway_pi4: SX1272_lora_gateway_pi4
endif
else ifeq ($(SX12XX),SX126X)
lora_gateway: SX126X_lora_gateway_wiring
#lora_gateway_pi2: SX126X_lora_gateway_wiring
#lora_gateway_pi4: SX126X_lora_gateway_wiring
else ifeq ($(SX12XX),SX127X)
lora_gateway: SX127X_lora_gateway_wiring
#lora_gateway_pi2: SX127X_lora_gateway_wiring
#lora_gateway_pi4: SX127X_lora_gateway_wiring
else ifeq ($(SX12XX),SX128X)
lora_gateway: SX128X_lora_gateway_wiring
#lora_gateway_pi2: SX128X_lora_gateway_wiring
#lora_gateway_pi4: SX128X_lora_gateway_wiring
else ifeq ($(SX12XX),SX1272)
lora_gateway: SX1272_lora_gateway_wiring
#lora_gateway_pi2: SX1272_lora_gateway_wiring
#lora_gateway_pi4: SX1272_lora_gateway_wiring
endif

################################
# usage
#
#	- use radio.makefile to indicate the radio type
#		* SX12XX=SX1272 | SX126X | SX127X | SX128X
#
#	- use raspberry.makefile to indicate the SPI lib 
#		* SPILIB=ARDUPI | WIRINGPI
#
# - then make lora_gateway
#
################################

################################
# detailed targets. There are several possible targets
#
# - with arduPi, which differenciate between RPI models and only works on 32-bit OS:
#		* make SX1272_lora_gateway or make SX1272_lora_gateway_pi2 or make SX1272_lora_gateway_pi2 (Libelium 1272/1276 lib)
#		* make SX126X_lora_gateway or make SX126X_lora_gateway_pi2 or make SX126X_lora_gateway_pi4 (StuartProject's SX12XX lib)	
#		* make SX127X_lora_gateway or make SX127X_lora_gateway_pi2 or make SX127X_lora_gateway_pi4 (StuartProject's SX12XX lib)	
#		* make SX128X_lora_gateway or make SX128X_lora_gateway_pi2 or make SX128X_lora_gateway_pi4 (StuartProject's SX12XX lib)	
#
# - with wiringPi, with no differenciation between RPI models and works with 32-bit and 64-bit OS:
#		* make SX1272_lora_gateway_wiring (Libelium 1272/1276 lib)
#		* make SX126X_lora_gateway_wiring (StuartProject's SX12XX lib)	
#		* make SX127X_lora_gateway_wiring (StuartProject's SX12XX lib)	
#		* make SX128X_lora_gateway_wiring (StuartProject's SX12XX lib)
#
# - on 64-bit OS, you need the "UNOFICIAL WiringPi library for RPi 64bit OS" (https://github.com/TheNextLVL/wiringPi)
#		* but take this updated version where RPI4 has been added as detected platform
#		* https://github.com/CongducPham/LowCostLoRaGw/blob/master/zip/wiringPi-64bit.tgz
#		* then
#			- tar xvfz wiringPi-64bit.tgz
#			- cd wiringPi
#			- sudo ./build
#
# - IMPORTANT: with wiringPi, make sure that the pin definition defined in radio.makefile are those of wiringPi
#
# - then to run the gateway
#		* sudo ./lora_gateway
#
# - there is a simple lora gateway program with fixed LoRa parameters to test the StuartProject's SX12XX lib
#		* make SX126X_simple_lora_gateway or make SX126X_simple_lora_gateway_pi2 or make SX126X_simple_lora_gateway_pi4 (StuartProject's SX12XX lib)	
#		* make SX127X_simple_lora_gateway or make SX127X_simple_lora_gateway_pi2 or make SX127X_simple_lora_gateway_pi4 (StuartProject's SX12XX lib)	
#		* make SX128X_simple_lora_gateway or make SX128X_simple_lora_gateway_pi2 or make SX128X_simple_lora_gateway_pi4 (StuartProject's SX12XX lib)
#
# - then to run the simple gateway
#		* sudo ./simple_lora_gateway
#
################################

################################
# targets
################################

# --------------> RPI1 & RPI0
################################

#/////////////////////////////////
#Libelium SX1272/1276 library – only support SX1272/SX1276 radio modules
#lora_gateway: SX1272_lora_gateway
#/////////////////////////////////
#if moving to SX12XX library
#///// uncomment 1 choice ////////
#lora_gateway: SX126X_lora_gateway
#lora_gateway: SX127X_lora_gateway
#lora_gateway: SX128X_lora_gateway
#/////////////////////////////////

#SX1272 lora gateway 
####################
SX1272_lora_gateway: SX1272_lora_gateway.o arduPi.o SX1272/SX1272_arduPi.o base64.o
	g++ -lrt -lpthread SX1272_lora_gateway.o arduPi.o SX1272/SX1272_arduPi.o base64.o -o SX1272_lora_gateway
	rm -f lora_gateway
	ln -s SX1272_lora_gateway ./lora_gateway

#SX12XX test gateway 
####################
SX126X_simple_lora_gateway: SX126X_simple_lora_gateway.o arduPi.o SX12XX/SX126XLT_arduPi.o base64.o
	g++ -lrt -lpthread SX126X_simple_lora_gateway.o arduPi.o SX12XX/SX126XLT_arduPi.o base64.o -o SX126X_simple_lora_gateway
	rm -f simple_lora_gateway
	ln -s SX126X_simple_lora_gateway ./simple_lora_gateway
	
SX127X_simple_lora_gateway: SX127X_simple_lora_gateway.o arduPi.o SX12XX/SX127XLT_arduPi.o base64.o
	g++ -lrt -lpthread SX127X_simple_lora_gateway.o arduPi.o SX12XX/SX127XLT_arduPi.o base64.o -o SX127X_simple_lora_gateway
	rm -f simple_lora_gateway
	ln -s SX127X_simple_lora_gateway ./simple_lora_gateway

SX128X_simple_lora_gateway: SX128X_simple_lora_gateway.o arduPi.o SX12XX/SX128XLT_arduPi.o base64.o
	g++ -lrt -lpthread SX128X_simple_lora_gateway.o arduPi.o SX12XX/SX128XLT_arduPi.o base64.o -o SX128X_simple_lora_gateway
	rm -f simple_lora_gateway
	ln -s SX128X_simple_lora_gateway ./simple_lora_gateway
	
#SX12XX transmitter 
####################
SX126X_simple_lora_transmitter: SX126X_simple_lora_transmitter.o arduPi.o SX12XX/SX126XLT_arduPi.o base64.o
	g++ -lrt -lpthread SX126X_simple_lora_transmitter.o arduPi.o SX12XX/SX126XLT_arduPi.o base64.o -o SX126X_simple_lora_transmitter
	rm -f simple_lora_transmitter
	ln -s SX126X_simple_lora_transmitter ./simple_lora_transmitter
	
SX127X_simple_lora_transmitter: SX127X_simple_lora_transmitter.o arduPi.o SX12XX/SX127XLT_arduPi.o base64.o
	g++ -lrt -lpthread SX127X_simple_lora_transmitter.o arduPi.o SX12XX/SX127XLT_arduPi.o base64.o -o SX127X_simple_lora_transmitter
	rm -f simple_lora_transmitter
	ln -s SX127X_simple_lora_transmitter ./simple_lora_transmitter

SX128X_simple_lora_transmitter: SX128X_simple_lora_transmitter.o arduPi.o SX12XX/SX128XLT_arduPi.o base64.o
	g++ -lrt -lpthread SX128X_simple_lora_transmitter.o arduPi.o SX12XX/SX128XLT_arduPi.o base64.o -o SX128X_simple_lora_transmitter
	rm -f simple_lora_transmitter
	ln -s SX128X_simple_lora_transmitter ./simple_lora_transmitter	
	
#SX12XX lora gateway 
####################
SX126X_lora_gateway: SX126X_lora_gateway.o arduPi.o SX12XX/SX126XLT_arduPi.o base64.o
	g++ -lrt -lpthread SX126X_lora_gateway.o arduPi.o SX12XX/SX126XLT_arduPi.o base64.o -o SX126X_lora_gateway
	rm -f lora_gateway
	ln -s SX126X_lora_gateway ./lora_gateway	

SX127X_lora_gateway: SX127X_lora_gateway.o arduPi.o SX12XX/SX127XLT_arduPi.o base64.o
	g++ -lrt -lpthread SX127X_lora_gateway.o arduPi.o SX12XX/SX127XLT_arduPi.o base64.o -o SX127X_lora_gateway
	rm -f lora_gateway
	ln -s SX127X_lora_gateway ./lora_gateway	
	
SX128X_lora_gateway: SX128X_lora_gateway.o arduPi.o SX12XX/SX128XLT_arduPi.o base64.o
	g++ -lrt -lpthread SX128X_lora_gateway.o arduPi.o SX12XX/SX128XLT_arduPi.o base64.o -o SX128X_lora_gateway
	rm -f lora_gateway
	ln -s SX128X_lora_gateway ./lora_gateway	
	
#SX1280 ranging slave 
#####################
SX1280_ranging_slave: SX1280_ranging_slave.o arduPi.o SX12XX/SX128XLT_arduPi.o base64.o
	g++ -lrt -lpthread SX1280_ranging_slave.o arduPi.o SX12XX/SX128XLT_arduPi.o base64.o -o SX1280_ranging_slave
	rm -f ranging_slave
	ln -s SX1280_ranging_slave ./ranging_slave	
		
# --------------> RPI2 & RPI3
################################

#/////////////////////////////////
#Libelium SX1272/1276 library – only support SX1272/SX1276 radio modules
#lora_gateway_pi2: SX1272_lora_gateway_pi2
#/////////////////////////////////
#if moving to SX12XX library
#///// uncomment 1 choice ////////////////
#lora_gateway_pi2: SX126X_lora_gateway_pi2
#lora_gateway_pi2: SX127X_lora_gateway_pi2
#lora_gateway_pi2: SX128X_lora_gateway_pi2
#/////////////////////////////////////////

#SX1272 lora gateway 
####################
SX1272_lora_gateway_pi2: SX1272_lora_gateway.o arduPi_pi2.o SX1272/SX1272_arduPi.o base64.o
	g++ -lrt -lpthread SX1272_lora_gateway.o arduPi_pi2.o SX1272/SX1272_arduPi.o base64.o -o SX1272_lora_gateway_pi2
	rm -f lora_gateway
	ln -s SX1272_lora_gateway_pi2 ./lora_gateway

#SX12XX test gateway 
####################
SX126X_simple_lora_gateway_pi2: SX126X_simple_lora_gateway.o arduPi_pi2.o SX12XX/SX126XLT_arduPi.o base64.o
	g++ -lrt -lpthread SX126X_simple_lora_gateway.o arduPi_pi2.o SX12XX/SX126XLT_arduPi.o base64.o -o SX126X_simple_lora_gateway_pi2
	rm -f simple_lora_gateway
	ln -s SX126X_simple_lora_gateway_pi2 ./simple_lora_gateway
	
SX127X_simple_lora_gateway_pi2: SX127X_simple_lora_gateway.o arduPi_pi2.o SX12XX/SX127XLT_arduPi.o base64.o
	g++ -lrt -lpthread SX127X_simple_lora_gateway.o arduPi_pi2.o SX12XX/SX127XLT_arduPi.o base64.o -o SX127X_simple_lora_gateway_pi2
	rm -f simple_lora_gateway
	ln -s SX127X_simple_lora_gateway_pi2 ./simple_lora_gateway

SX128X_simple_lora_gateway_pi2: SX128X_simple_lora_gateway.o arduPi_pi2.o SX12XX/SX128XLT_arduPi.o base64.o
	g++ -lrt -lpthread SX128X_simple_lora_gateway.o arduPi_pi2.o SX12XX/SX128XLT_arduPi.o base64.o -o SX128X_simple_lora_gateway_pi2
	rm -f simple_lora_gateway
	ln -s SX128X_simple_lora_gateway_pi2 ./simple_lora_gateway	

#SX12XX transmitter 
####################
SX126X_simple_lora_transmitter_pi2: SX126X_simple_lora_transmitter.o arduPi_pi2.o SX12XX/SX126XLT_arduPi.o base64.o
	g++ -lrt -lpthread SX126X_simple_lora_transmitter.o arduPi_pi2.o SX12XX/SX126XLT_arduPi.o base64.o -o SX126X_simple_lora_transmitter_pi2
	rm -f simple_lora_transmitter
	ln -s SX126X_simple_lora_transmitter_pi2 ./simple_lora_transmitter
	
SX127X_simple_lora_transmitter_pi2: SX127X_simple_lora_transmitter.o arduPi_pi2.o SX12XX/SX127XLT_arduPi.o base64.o
	g++ -lrt -lpthread SX127X_simple_lora_transmitter.o arduPi_pi2.o SX12XX/SX127XLT_arduPi.o base64.o -o SX127X_simple_lora_transmitter_pi2
	rm -f simple_lora_transmitter
	ln -s SX127X_simple_lora_transmitter_pi2 ./simple_lora_transmitter

SX128X_simple_lora_transmitter_pi2: SX128X_simple_lora_transmitter.o arduPi_pi2.o SX12XX/SX128XLT_arduPi.o base64.o
	g++ -lrt -lpthread SX128X_simple_lora_transmitter.o arduPi_pi2.o SX12XX/SX128XLT_arduPi.o base64.o -o SX128X_simple_lora_transmitter_pi2
	rm -f simple_lora_transmitter
	ln -s SX128X_simple_lora_transmitter_pi2 ./simple_lora_transmitter	
	
#SX12XX lora gateway 
####################
SX126X_lora_gateway_pi2: SX126X_lora_gateway.o arduPi_pi2.o SX12XX/SX126XLT_arduPi.o base64.o
	g++ -lrt -lpthread SX126X_lora_gateway.o arduPi_pi2.o SX12XX/SX126XLT_arduPi.o base64.o -o SX126X_lora_gateway_pi2
	rm -f lora_gateway
	ln -s SX126X_lora_gateway_pi2 ./lora_gateway	
		
SX127X_lora_gateway_pi2: SX127X_lora_gateway.o arduPi_pi2.o SX12XX/SX127XLT_arduPi.o base64.o
	g++ -lrt -lpthread SX127X_lora_gateway.o arduPi_pi2.o SX12XX/SX127XLT_arduPi.o base64.o -o SX127X_lora_gateway_pi2
	rm -f lora_gateway
	ln -s SX127X_lora_gateway_pi2 ./lora_gateway	

SX128X_lora_gateway_pi2: SX128X_lora_gateway.o arduPi_pi2.o SX12XX/SX128XLT_arduPi.o base64.o
	g++ -lrt -lpthread SX128X_lora_gateway.o arduPi_pi2.o SX12XX/SX128XLT_arduPi.o base64.o -o SX128X_lora_gateway_pi2
	rm -f lora_gateway
	ln -s SX128X_lora_gateway_pi2 ./lora_gateway	

#SX1280 ranging slave 
#####################
SX1280_ranging_slave_pi2: SX1280_ranging_slave.o arduPi_pi2.o SX12XX/SX128XLT_arduPi.o base64.o
	g++ -lrt -lpthread SX1280_ranging_slave.o arduPi_pi2.o SX12XX/SX128XLT_arduPi.o base64.o -o SX1280_ranging_slave_pi2
	rm -f ranging_slave
	ln -s SX1280_ranging_slave_pi2 ./ranging_slave
	
# --------------> RPI4 
################################

#/////////////////////////////////
#Libelium SX1272/1276 library – only support SX1272/SX1276 radio modules
#lora_gateway_pi4: SX1272_lora_gateway_pi4
#/////////////////////////////////
#if moving to SX12XX library
#///// uncomment 1 choice ////////////////
#lora_gateway_pi4: SX126X_lora_gateway_pi4
#lora_gateway_pi4: SX127X_lora_gateway_pi4
#lora_gateway_pi4: SX128X_lora_gateway_pi4
#/////////////////////////////////////////

#SX1272 lora gateway 
####################
SX1272_lora_gateway_pi4: SX1272_lora_gateway.o arduPi_pi4.o SX1272/SX1272_arduPi.o base64.o
	g++ -lrt -lpthread SX1272_lora_gateway.o arduPi_pi4.o SX1272/SX1272_arduPi.o base64.o -o SX1272_lora_gateway_pi4
	rm -f lora_gateway
	ln -s SX1272_lora_gateway_pi4 ./lora_gateway

#SX12XX test gateway 
####################
SX126X_simple_lora_gateway_pi4: SX126X_simple_lora_gateway.o arduPi_pi4.o SX12XX/SX126XLT_arduPi.o base64.o
	g++ -lrt -lpthread SX126X_simple_lora_gateway.o arduPi_pi4.o SX12XX/SX126XLT_arduPi.o base64.o -o SX126X_simple_lora_gateway_pi4
	rm -f lora_gateway
	ln -s SX126X_simple_lora_gateway_pi4 ./simple_lora_gateway
	
SX127X_simple_lora_gateway_pi4: SX127X_simple_lora_gateway.o arduPi_pi4.o SX12XX/SX127XLT_arduPi.o base64.o
	g++ -lrt -lpthread SX127X_simple_lora_gateway.o arduPi_pi4.o SX12XX/SX127XLT_arduPi.o base64.o -o SX127X_simple_lora_gateway_pi4
	rm -f lora_gateway
	ln -s SX127X_simple_lora_gateway_pi4 ./simple_lora_gateway

SX128X_simple_lora_gateway_pi4: SX128X_simple_lora_gateway.o arduPi_pi4.o SX12XX/SX128XLT_arduPi.o base64.o
	g++ -lrt -lpthread SX128X_simple_lora_gateway.o arduPi_pi4.o SX12XX/SX128XLT_arduPi.o base64.o -o SX128X_simple_lora_gateway_pi4
	rm -f lora_gateway
	ln -s SX128X_simple_lora_gateway_pi4 ./simple_lora_gateway	
		
#SX12XX transmitter 
####################
SX126X_simple_lora_transmitter_pi4: SX126X_simple_lora_transmitter.o arduPi_pi4.o SX12XX/SX126XLT_arduPi.o base64.o
	g++ -lrt -lpthread SX126X_simple_lora_transmitter.o arduPi_pi4.o SX12XX/SX126XLT_arduPi.o base64.o -o SX126X_simple_lora_transmitter_pi4
	rm -f simple_lora_transmitter
	ln -s SX126X_simple_lora_transmitter_pi4 ./simple_lora_transmitter
	
SX127X_simple_lora_transmitter_pi4: SX127X_simple_lora_transmitter.o arduPi_pi4.o SX12XX/SX127XLT_arduPi.o base64.o
	g++ -lrt -lpthread SX127X_simple_lora_transmitter.o arduPi_pi4.o SX12XX/SX127XLT_arduPi.o base64.o -o SX127X_simple_lora_transmitter_pi4
	rm -f simple_lora_transmitter
	ln -s SX127X_simple_lora_transmitter_pi4 ./simple_lora_transmitter

SX128X_simple_lora_transmitter_pi4: SX128X_simple_lora_transmitter.o arduPi_pi4.o SX12XX/SX128XLT_arduPi.o base64.o
	g++ -lrt -lpthread SX128X_simple_lora_transmitter.o arduPi_pi4.o SX12XX/SX128XLT_arduPi.o base64.o -o SX128X_simple_lora_transmitter_pi4
	rm -f simple_lora_transmitter
	ln -s SX128X_simple_lora_transmitter_pi4 ./simple_lora_transmitter	
	
#SX12XX lora gateway 
####################
SX126X_lora_gateway_pi4: SX126X_lora_gateway.o arduPi_pi4.o SX12XX/SX126XLT_arduPi.o base64.o
	g++ -lrt -lpthread SX126X_lora_gateway.o arduPi_pi4.o SX12XX/SX126XLT_arduPi.o base64.o -o SX126X_lora_gateway_pi4
	rm -f lora_gateway
	ln -s SX126X_lora_gateway_pi4 ./lora_gateway	
		
SX127X_lora_gateway_pi4: SX127X_lora_gateway.o arduPi_pi4.o SX12XX/SX127XLT_arduPi.o base64.o
	g++ -lrt -lpthread SX127X_lora_gateway.o arduPi_pi4.o SX12XX/SX127XLT_arduPi.o base64.o -o SX127X_lora_gateway_pi4
	rm -f lora_gateway
	ln -s SX127X_lora_gateway_pi4 ./lora_gateway	

SX128X_lora_gateway_pi4: SX128X_lora_gateway.o arduPi_pi4.o SX12XX/SX128XLT_arduPi.o base64.o
	g++ -lrt -lpthread SX128X_lora_gateway.o arduPi_pi4.o SX12XX/SX128XLT_arduPi.o base64.o -o SX128X_lora_gateway_pi4
	rm -f lora_gateway
	ln -s SX128X_lora_gateway_pi4 ./lora_gateway	
	
#SX1280 ranging slave 
#####################
SX1280_ranging_slave_pi4: SX1280_ranging_slave.o arduPi_pi4.o SX12XX/SX128XLT_arduPi.o base64.o
	g++ -lrt -lpthread SX1280_ranging_slave.o arduPi_pi4.o SX12XX/SX128XLT_arduPi.o base64.o -o SX1280_ranging_slave_pi4
	rm -f ranging_slave
	ln -s SX1280_ranging_slave_pi4 ./ranging_slave	


# --------------> ALL RPI using wiringPi
########################################

#/////////////////////////////////
#if moving to SX12XX library
#///// uncomment 1 choice ////////////////
#lora_gateway_wiring: SX1272_lora_gateway_wiring
#lora_gateway_wiring: SX126X_lora_gateway_wiring
#lora_gateway_wiring: SX127X_lora_gateway_wiring
#lora_gateway_wiring: SX128X_lora_gateway_wiring
#/////////////////////////////////////////

#SX12XX test gateway 
####################
SX126X_simple_lora_gateway_wiring: SX126X_simple_lora_gateway_wiring.o arduinoPi.o SX12XX/SX126XLT_wiring.o base64.o
	g++ -lrt -lpthread -lwiringPi SX126X_simple_lora_gateway_wiring.o arduinoPi.o SX12XX/SX126XLT_wiring.o base64.o -o SX126X_simple_lora_gateway_wiring
	rm -f lora_gateway
	ln -s SX126X_simple_lora_gateway_wiring ./simple_lora_gateway
	
SX127X_simple_lora_gateway_wiring: SX127X_simple_lora_gateway_wiring.o arduinoPi.o SX12XX/SX127XLT_wiring.o base64.o
	g++ -lrt -lpthread -lwiringPi SX127X_simple_lora_gateway_wiring.o arduinoPi.o SX12XX/SX127XLT_wiring.o base64.o -o SX127X_simple_lora_gateway_wiring
	rm -f lora_gateway
	ln -s SX127X_simple_lora_gateway_wiring ./simple_lora_gateway

SX128X_simple_lora_gateway_wiring: SX128X_simple_lora_gateway_wiring.o arduinoPi.o SX12XX/SX128XLT_wiring.o base64.o
	g++ -lrt -lpthread -lwiringPi SX128X_simple_lora_gateway_wiring.o arduinoPi.o SX12XX/SX128XLT_wiring.o base64.o -o SX128X_simple_lora_gateway_wiring
	rm -f lora_gateway
	ln -s SX128X_simple_lora_gateway_wiring ./simple_lora_gateway	
		
#SX12XX transmitter 
####################
SX126X_simple_lora_transmitter_wiring: SX126X_simple_lora_transmitter_wiring.o arduinoPi.o SX12XX/SX126XLT_wiring.o base64.o
	g++ -lrt -lpthread -lwiringPi SX126X_simple_lora_transmitter_wiring.o arduinoPi.o SX12XX/SX126XLT_wiring.o base64.o -o SX126X_simple_lora_transmitter_wiring
	rm -f simple_lora_transmitter
	ln -s SX126X_simple_lora_transmitter_wiring ./simple_lora_transmitter
	
SX127X_simple_lora_transmitter_wiring: SX127X_simple_lora_transmitter_wiring.o arduinoPi.o SX12XX/SX127XLT_wiring.o base64.o
	g++ -lrt -lpthread -lwiringPi SX127X_simple_lora_transmitter_wiring.o arduinoPi.o SX12XX/SX127XLT_wiring.o base64.o -o SX127X_simple_lora_transmitter_wiring
	rm -f simple_lora_transmitter
	ln -s SX127X_simple_lora_transmitter_wiring ./simple_lora_transmitter

SX128X_simple_lora_transmitter_wiring: SX128X_simple_lora_transmitter_wiring.o arduinoPi.o SX12XX/SX128XLT_wiring.o base64.o
	g++ -lrt -lpthread -lwiringPi SX128X_simple_lora_transmitter_wiring.o arduinoPi.o SX12XX/SX128XLT_wiring.o base64.o -o SX128X_simple_lora_transmitter_wiring
	rm -f simple_lora_transmitter
	ln -s SX128X_simple_lora_transmitter_wiring ./simple_lora_transmitter	

#SX1272 lora gateway 
####################
SX1272_lora_gateway_wiring: SX1272_lora_gateway.o arduinoPi.o SX1272/SX1272_wiring.o base64.o
	g++ -lrt -lpthread -lwiringPi SX1272_lora_gateway.o arduinoPi.o SX1272/SX1272_wiring.o base64.o -o SX1272_lora_gateway_wiring
	rm -f lora_gateway
	ln -s SX1272_lora_gateway_wiring ./lora_gateway	
		
#SX12XX lora gateway 
####################
SX126X_lora_gateway_wiring: SX126X_lora_gateway_wiring.o arduinoPi.o SX12XX/SX126XLT_wiring.o base64.o
	g++ -lrt -lpthread -lwiringPi SX126X_lora_gateway_wiring.o arduinoPi.o SX12XX/SX126XLT_wiring.o base64.o -o SX126X_lora_gateway_wiring
	rm -f lora_gateway
	ln -s SX126X_lora_gateway_wiring ./lora_gateway	
		
SX127X_lora_gateway_wiring: SX127X_lora_gateway_wiring.o arduinoPi.o SX12XX/SX127XLT_wiring.o base64.o
	g++ -lrt -lpthread -lwiringPi SX127X_lora_gateway_wiring.o arduinoPi.o SX12XX/SX127XLT_wiring.o base64.o -o SX127X_lora_gateway_wiring
	rm -f lora_gateway
	ln -s SX127X_lora_gateway_wiring ./lora_gateway	

SX128X_lora_gateway_wiring: SX128X_lora_gateway_wiring.o arduinoPi.o SX12XX/SX128XLT_wiring.o base64.o
	g++ -lrt -lpthread -lwiringPi SX128X_lora_gateway_wiring.o arduinoPi.o SX12XX/SX128XLT_wiring.o base64.o -o SX128X_lora_gateway_wiring
	rm -f lora_gateway
	ln -s SX128X_lora_gateway_wiring ./lora_gateway	
	
#SX1280 ranging slave 
#####################
SX1280_ranging_slave_wiring: SX1280_ranging_slave_wiring.o arduinoPi.o SX12XX/SX128XLT_wiring.o base64.o
	g++ -lrt -lpthread -lwiringPi SX1280_ranging_slave_wiring.o arduinoPi.o SX12XX/SX128XLT_wiring.o base64.o -o SX1280_ranging_slave_wiring
	rm -f ranging_slave
	ln -s SX1280_ranging_slave_wiring ./ranging_slave	


################################
# rules
################################
	
SX1272_lora_gateway.o: SX1272_lora_gateway.cpp radio.makefile raspberry.makefile
	g++ $(CFLAGS) -I SX1272 -I . -c SX1272_lora_gateway.cpp -o SX1272_lora_gateway.o
	
##

SX126X_simple_lora_gateway.o: SX12XX_simple_lora_gateway.cpp SX12XX_simple_lora_gateway.h
	g++ $(CFLAGS) -I SX12XX -I . -DUSE_ARDUPI -DSX126X -c SX12XX_simple_lora_gateway.cpp -o SX126X_simple_lora_gateway.o
	
SX127X_simple_lora_gateway.o: SX12XX_simple_lora_gateway.cpp SX12XX_simple_lora_gateway.h
	g++ $(CFLAGS) -I SX12XX -I . -DUSE_ARDUPI -DSX127X -c SX12XX_simple_lora_gateway.cpp -o SX127X_simple_lora_gateway.o

SX128X_simple_lora_gateway.o: SX12XX_simple_lora_gateway.cpp SX12XX_simple_lora_gateway.h
	g++ $(CFLAGS) -I SX12XX -I . -DUSE_ARDUPI -DSX128X -c SX12XX_simple_lora_gateway.cpp -o SX128X_simple_lora_gateway.o
	
SX126X_simple_lora_gateway_wiring.o: SX12XX_simple_lora_gateway.cpp SX12XX_simple_lora_gateway.h
	g++ $(CFLAGS) -I SX12XX -I . -DSX126X -c SX12XX_simple_lora_gateway.cpp -o SX126X_simple_lora_gateway_wiring.o
	
SX127X_simple_lora_gateway_wiring.o: SX12XX_simple_lora_gateway.cpp SX12XX_simple_lora_gateway.h
	g++ $(CFLAGS) -I SX12XX -I . -DSX127X -c SX12XX_simple_lora_gateway.cpp -o SX127X_simple_lora_gateway_wiring.o

SX128X_simple_lora_gateway_wiring.o: SX12XX_simple_lora_gateway.cpp SX12XX_simple_lora_gateway.h
	g++ $(CFLAGS) -I SX12XX -I . -DSX128X -c SX12XX_simple_lora_gateway.cpp -o SX128X_simple_lora_gateway_wiring.o	

##
	
SX126X_simple_lora_transmitter.o: SX12XX_simple_lora_transmitter.cpp SX12XX_simple_lora_transmitter.h
	g++ $(CFLAGS) -I SX12XX -I . -DUSE_ARDUPI -DSX126X -c SX12XX_simple_lora_transmitter.cpp -o SX126X_simple_lora_transmitter.o
	
SX127X_simple_lora_transmitter.o: SX12XX_simple_lora_transmitter.cpp SX12XX_simple_lora_transmitter.h
	g++ $(CFLAGS) -I SX12XX -I . -DUSE_ARDUPI -DSX127X -c SX12XX_simple_lora_transmitter.cpp -o SX127X_simple_lora_transmitter.o

SX128X_simple_lora_transmitter.o: SX12XX_simple_lora_transmitter.cpp SX12XX_simple_lora_transmitter.h
	g++ $(CFLAGS) -I SX12XX -I . -DUSE_ARDUPI -DSX128X -c SX12XX_simple_lora_transmitter.cpp -o SX128X_simple_lora_transmitter.o	
	
SX126X_simple_lora_transmitter_wiring.o: SX12XX_simple_lora_transmitter.cpp SX12XX_simple_lora_transmitter.h
	g++ $(CFLAGS) -I SX12XX -I . -DSX126X -c SX12XX_simple_lora_transmitter.cpp -o SX126X_simple_lora_transmitter_wiring.o
	
SX127X_simple_lora_transmitter_wiring.o: SX12XX_simple_lora_transmitter.cpp SX12XX_simple_lora_transmitter.h
	g++ $(CFLAGS) -I SX12XX -I . -DSX127X -c SX12XX_simple_lora_transmitter.cpp -o SX127X_simple_lora_transmitter_wiring.o

SX128X_simple_lora_transmitter_wiring.o: SX12XX_simple_lora_transmitter.cpp SX12XX_simple_lora_transmitter.h
	g++ $(CFLAGS) -I SX12XX -I . -DSX128X -c SX12XX_simple_lora_transmitter.cpp -o SX128X_simple_lora_transmitter_wiring.o		

##
			
SX126X_lora_gateway.o: SX12XX_lora_gateway.cpp SX126X_lora_gateway.h radio.makefile raspberry.makefile
	g++ $(CFLAGS) -I SX12XX -I . -DUSE_ARDUPI -DSX126X -c SX12XX_lora_gateway.cpp -o SX126X_lora_gateway.o

SX127X_lora_gateway.o: SX12XX_lora_gateway.cpp SX127X_lora_gateway.h radio.makefile raspberry.makefile
	g++ $(CFLAGS) -I SX12XX -I . -DUSE_ARDUPI -DSX127X -c SX12XX_lora_gateway.cpp -o SX127X_lora_gateway.o

SX128X_lora_gateway.o: SX12XX_lora_gateway.cpp SX128X_lora_gateway.h radio.makefile raspberry.makefile
	g++ $(CFLAGS) -I SX12XX -I . -DUSE_ARDUPI -DSX128X -c SX12XX_lora_gateway.cpp -o SX128X_lora_gateway.o
	
SX126X_lora_gateway_wiring.o: SX12XX_lora_gateway.cpp SX126X_lora_gateway.h radio.makefile raspberry.makefile
	g++ $(CFLAGS) -I SX12XX -I . -DSX126X -c SX12XX_lora_gateway.cpp -o SX126X_lora_gateway_wiring.o

SX127X_lora_gateway_wiring.o: SX12XX_lora_gateway.cpp SX127X_lora_gateway.h radio.makefile raspberry.makefile
	g++ $(CFLAGS) -I SX12XX -I . -DSX127X -c SX12XX_lora_gateway.cpp -o SX127X_lora_gateway_wiring.o

SX128X_lora_gateway_wiring.o: SX12XX_lora_gateway.cpp SX128X_lora_gateway.h radio.makefile raspberry.makefile
	g++ $(CFLAGS) -I SX12XX -I . -DSX128X -c SX12XX_lora_gateway.cpp -o SX128X_lora_gateway_wiring.o	

##
	
SX1280_ranging_slave.o: SX1280_ranging_slave.cpp SX1280_ranging_slave.h radio.makefile raspberry.makefile
	g++ $(CFLAGS) -I SX12XX -I . -DUSE_ARDUPI -DSX128X -c SX1280_ranging_slave.cpp -o SX1280_ranging_slave.o

SX1280_ranging_slave_wiring.o: SX1280_ranging_slave.cpp SX1280_ranging_slave.h radio.makefile raspberry.makefile
	g++ $(CFLAGS) -I SX12XX -I . -DSX128X -c SX1280_ranging_slave.cpp -o SX1280_ranging_slave_wiring.o
			
##

arduPi.o: arduPi.cpp arduPi.h
	g++ $(CFLAGS) -c arduPi.cpp -o arduPi.o	

arduPi_pi2.o: arduPi.cpp arduPi.h
	g++ $(CFLAGS) -DRASPBERRY2 -c arduPi.cpp -o arduPi_pi2.o	

arduPi_pi4.o: arduPi.cpp arduPi.h
	g++ $(CFLAGS) -DRASPBERRY4 -c arduPi.cpp -o arduPi_pi4.o
	
arduinoPi.o: arduinoPi.cpp arduinoPi.h
	g++ $(CFLAGS) -c arduinoPi.cpp -o arduinoPi.o		

SX1272/SX1272_arduPi.o: SX1272/SX1272.cpp SX1272/SX1272.h
	g++ $(CFLAGS) -I SX1272 -I . -DUSE_ARDUPI -c SX1272/SX1272.cpp -o SX1272/SX1272_arduPi.o

SX1272/SX1272_wiring.o: SX1272/SX1272.cpp SX1272/SX1272.h
	g++ $(CFLAGS) -I SX1272 -I . -c SX1272/SX1272.cpp -o SX1272/SX1272_wiring.o
	
SX12XX/SX126XLT_wiring.o: SX12XX/SX126XLT.cpp SX12XX/SX126XLT.h SX12XX/SX126XLT_Definitions.h
	g++ $(CFLAGS) -I SX12XX -I . -c SX12XX/SX126XLT.cpp -o SX12XX/SX126XLT_wiring.o

SX12XX/SX127XLT_wiring.o: SX12XX/SX127XLT.cpp SX12XX/SX127XLT.h SX12XX/SX127XLT_Definitions.h
	g++ $(CFLAGS) -I SX12XX -I . -c SX12XX/SX127XLT.cpp -o SX12XX/SX127XLT_wiring.o
	
SX12XX/SX128XLT_wiring.o: SX12XX/SX128XLT.cpp SX12XX/SX128XLT.h SX12XX/SX128XLT_Definitions.h
	g++ $(CFLAGS) -I SX12XX -I . -c SX12XX/SX128XLT.cpp -o SX12XX/SX128XLT_wiring.o

SX12XX/SX126XLT_arduPi.o: SX12XX/SX126XLT.cpp SX12XX/SX126XLT.h SX12XX/SX126XLT_Definitions.h
	g++ $(CFLAGS) -I SX12XX -I . -DUSE_ARDUPI -c SX12XX/SX126XLT.cpp -o SX12XX/SX126XLT_arduPi.o

SX12XX/SX127XLT_arduPi.o: SX12XX/SX127XLT.cpp SX12XX/SX127XLT.h SX12XX/SX127XLT_Definitions.h
	g++ $(CFLAGS) -I SX12XX -I . -DUSE_ARDUPI -c SX12XX/SX127XLT.cpp -o SX12XX/SX127XLT_arduPi.o
	
SX12XX/SX128XLT_arduPi.o: SX12XX/SX128XLT.cpp SX12XX/SX128XLT.h SX12XX/SX128XLT_Definitions.h
	g++ $(CFLAGS) -I SX12XX -I . -DUSE_ARDUPI -c SX12XX/SX128XLT.cpp -o SX12XX/SX128XLT_arduPi.o
	
base64.o: base64.c base64.h
	g++ -c base64.c -o base64.o	

clean:
	cd SX1272; rm -rf *.o; cd ../SX12XX; rm -rf *.o; cd ..
	rm -rf *.o lora_gateway simple_lora_gateway simple_lora_transmitter ranging_slave SX1272_lora_gateway SX1272_lora_gateway_pi* SX12?X_lora_gateway SX12?X_lora_gateway_pi* SX12?X_simple_lora_gateway SX12?X_simple_lora_gateway_pi* SX12?X_simple_lora_transmitter SX12?X_simple_lora_transmitter_pi* SX1280_ranging_slave SX1280_ranging_slave_pi* *_wiring

