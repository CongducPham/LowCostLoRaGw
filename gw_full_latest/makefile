include radio.makefile
include raspberry.makefile

################################
# usage options
# 
# - by defining lora_gateway, lora_gateway_pi2 or lora_gateway_pi4 target according to
#		* to your radio module (SX126X, SX127X or SX128X)
#		* and low-level lib (Libelium 1272/1276 lib or StuartProject's SX12XX lib)
#   * then:
#			- make lora_gateway, or
# 		- make lora_gateway_pi2, or
# 		- make lora_gateway_pi4
#
# - by calling the specific target
#		* make SX1272_lora_gateway or make SX1272_lora_gateway_pi2 or make SX1272_lora_gateway_pi2 (Libelium 1272/1276 lib)
#		* make SX126X_lora_gateway or make SX126X_lora_gateway_pi2 or make SX126X_lora_gateway_pi4 (StuartProject's SX12XX lib)	
#		* make SX127X_lora_gateway or make SX127X_lora_gateway_pi2 or make SX127X_lora_gateway_pi4 (StuartProject's SX12XX lib)	
#		* make SX128X_lora_gateway or make SX128X_lora_gateway_pi2 or make SX128X_lora_gateway_pi4 (StuartProject's SX12XX lib)	
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
lora_gateway: SX127X_lora_gateway
#lora_gateway: SX128X_lora_gateway
#/////////////////////////////////

#SX1272 lora gateway 
####################
SX1272_lora_gateway: SX1272_lora_gateway.o arduPi.o SX1272/SX1272.o base64.o
	g++ -lrt -lpthread SX1272_lora_gateway.o arduPi.o SX1272/SX1272.o base64.o -o SX1272_lora_gateway
	rm -f lora_gateway
	ln -s SX1272_lora_gateway ./lora_gateway

#SX12XX test gateway 
####################
SX126X_simple_lora_gateway: SX126X_simple_lora_gateway.o arduPi.o SX12XX/SX126XLT.o base64.o
	g++ -lrt -lpthread SX126X_simple_lora_gateway.o arduPi.o SX12XX/SX126XLT.o base64.o -o SX126X_simple_lora_gateway
	rm -f simple_lora_gateway
	ln -s SX126X_simple_lora_gateway ./simple_lora_gateway
	
SX127X_simple_lora_gateway: SX127X_simple_lora_gateway.o arduPi.o SX12XX/SX127XLT.o base64.o
	g++ -lrt -lpthread SX127X_simple_lora_gateway.o arduPi.o SX12XX/SX127XLT.o base64.o -o SX127X_simple_lora_gateway
	rm -f simple_lora_gateway
	ln -s SX127X_simple_lora_gateway ./simple_lora_gateway

SX128X_simple_lora_gateway: SX128X_simple_lora_gateway.o arduPi.o SX12XX/SX128XLT.o base64.o
	g++ -lrt -lpthread SX128X_simple_lora_gateway.o arduPi.o SX12XX/SX128XLT.o base64.o -o SX128X_simple_lora_gateway
	rm -f simple_lora_gateway
	ln -s SX128X_simple_lora_gateway ./simple_lora_gateway
	
#SX12XX transmitter 
####################
SX126X_simple_lora_transmitter: SX126X_simple_lora_transmitter.o arduPi.o SX12XX/SX126XLT.o base64.o
	g++ -lrt -lpthread SX126X_simple_lora_transmitter.o arduPi.o SX12XX/SX126XLT.o base64.o -o SX126X_simple_lora_transmitter
	rm -f simple_lora_transmitter
	ln -s SX126X_simple_lora_transmitter ./simple_lora_transmitter
	
SX127X_simple_lora_transmitter: SX127X_simple_lora_transmitter.o arduPi.o SX12XX/SX127XLT.o base64.o
	g++ -lrt -lpthread SX127X_simple_lora_transmitter.o arduPi.o SX12XX/SX127XLT.o base64.o -o SX127X_simple_lora_transmitter
	rm -f simple_lora_transmitter
	ln -s SX127X_simple_lora_transmitter ./simple_lora_transmitter

SX128X_simple_lora_transmitter: SX128X_simple_lora_transmitter.o arduPi.o SX12XX/SX128XLT.o base64.o
	g++ -lrt -lpthread SX128X_simple_lora_transmitter.o arduPi.o SX12XX/SX128XLT.o base64.o -o SX128X_simple_lora_transmitter
	rm -f simple_lora_transmitter
	ln -s SX128X_simple_lora_transmitter ./simple_lora_transmitter	
	
#SX12XX lora gateway 
####################
SX126X_lora_gateway: SX126X_lora_gateway.o arduPi.o SX12XX/SX126XLT.o base64.o
	g++ -lrt -lpthread SX126X_lora_gateway.o arduPi.o SX12XX/SX126XLT.o base64.o -o SX126X_lora_gateway
	rm -f lora_gateway
	ln -s SX126X_lora_gateway ./lora_gateway	

SX127X_lora_gateway: SX127X_lora_gateway.o arduPi.o SX12XX/SX127XLT.o base64.o
	g++ -lrt -lpthread SX127X_lora_gateway.o arduPi.o SX12XX/SX127XLT.o base64.o -o SX127X_lora_gateway
	rm -f lora_gateway
	ln -s SX127X_lora_gateway ./lora_gateway	
	
SX128X_lora_gateway: SX128X_lora_gateway.o arduPi.o SX12XX/SX128XLT.o base64.o
	g++ -lrt -lpthread SX128X_lora_gateway.o arduPi.o SX12XX/SX128XLT.o base64.o -o SX128X_lora_gateway
	rm -f lora_gateway
	ln -s SX128X_lora_gateway ./lora_gateway	
	
#SX1280 ranging slave 
#####################
SX1280_ranging_slave: SX1280_ranging_slave.o arduPi.o SX12XX/SX128XLT.o base64.o
	g++ -lrt -lpthread SX1280_ranging_slave.o arduPi.o SX12XX/SX128XLT.o base64.o -o SX1280_ranging_slave
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
lora_gateway_pi2: SX127X_lora_gateway_pi2
#lora_gateway_pi2: SX128X_lora_gateway_pi2
#/////////////////////////////////////////

#SX1272 lora gateway 
####################
SX1272_lora_gateway_pi2: SX1272_lora_gateway.o arduPi_pi2.o SX1272/SX1272.o base64.o
	g++ -lrt -lpthread SX1272_lora_gateway.o arduPi_pi2.o SX1272/SX1272.o base64.o -o SX1272_lora_gateway_pi2
	rm -f lora_gateway
	ln -s SX1272_lora_gateway_pi2 ./lora_gateway

#SX12XX test gateway 
####################
SX126X_simple_lora_gateway_pi2: SX126X_simple_lora_gateway.o arduPi_pi2.o SX12XX/SX126XLT.o base64.o
	g++ -lrt -lpthread SX126X_simple_lora_gateway.o arduPi_pi2.o SX12XX/SX126XLT.o base64.o -o SX126X_simple_lora_gateway_pi2
	rm -f simple_lora_gateway
	ln -s SX126X_simple_lora_gateway_pi2 ./simple_lora_gateway
	
SX127X_simple_lora_gateway_pi2: SX127X_simple_lora_gateway.o arduPi_pi2.o SX12XX/SX127XLT.o base64.o
	g++ -lrt -lpthread SX127X_simple_lora_gateway.o arduPi_pi2.o SX12XX/SX127XLT.o base64.o -o SX127X_simple_lora_gateway_pi2
	rm -f simple_lora_gateway
	ln -s SX127X_simple_lora_gateway_pi2 ./simple_lora_gateway

SX128X_simple_lora_gateway_pi2: SX128X_simple_lora_gateway.o arduPi_pi2.o SX12XX/SX128XLT.o base64.o
	g++ -lrt -lpthread SX128X_simple_lora_gateway.o arduPi_pi2.o SX12XX/SX128XLT.o base64.o -o SX128X_simple_lora_gateway_pi2
	rm -f simple_lora_gateway
	ln -s SX128X_simple_lora_gateway_pi2 ./simple_lora_gateway	

#SX12XX transmitter 
####################
SX126X_simple_lora_transmitter_pi2: SX126X_simple_lora_transmitter.o arduPi_pi2.o SX12XX/SX126XLT.o base64.o
	g++ -lrt -lpthread SX126X_simple_lora_transmitter.o arduPi_pi2.o SX12XX/SX126XLT.o base64.o -o SX126X_simple_lora_transmitter_pi2
	rm -f simple_lora_transmitter
	ln -s SX126X_simple_lora_transmitter_pi2 ./simple_lora_transmitter
	
SX127X_simple_lora_transmitter_pi2: SX127X_simple_lora_transmitter.o arduPi_pi2.o SX12XX/SX127XLT.o base64.o
	g++ -lrt -lpthread SX127X_simple_lora_transmitter.o arduPi_pi2.o SX12XX/SX127XLT.o base64.o -o SX127X_simple_lora_transmitter_pi2
	rm -f simple_lora_transmitter
	ln -s SX127X_simple_lora_transmitter_pi2 ./simple_lora_transmitter

SX128X_simple_lora_transmitter_pi2: SX128X_simple_lora_transmitter.o arduPi_pi2.o SX12XX/SX128XLT.o base64.o
	g++ -lrt -lpthread SX128X_simple_lora_transmitter.o arduPi_pi2.o SX12XX/SX128XLT.o base64.o -o SX128X_simple_lora_transmitter_pi2
	rm -f simple_lora_transmitter
	ln -s SX128X_simple_lora_transmitter_pi2 ./simple_lora_transmitter	
	
#SX12XX lora gateway 
####################
SX126X_lora_gateway_pi2: SX126X_lora_gateway.o arduPi_pi2.o SX12XX/SX126XLT.o base64.o
	g++ -lrt -lpthread SX126X_lora_gateway.o arduPi_pi2.o SX12XX/SX126XLT.o base64.o -o SX126X_lora_gateway_pi2
	rm -f lora_gateway
	ln -s SX126X_lora_gateway_pi2 ./lora_gateway	
		
SX127X_lora_gateway_pi2: SX127X_lora_gateway.o arduPi_pi2.o SX12XX/SX127XLT.o base64.o
	g++ -lrt -lpthread SX127X_lora_gateway.o arduPi_pi2.o SX12XX/SX127XLT.o base64.o -o SX127X_lora_gateway_pi2
	rm -f lora_gateway
	ln -s SX127X_lora_gateway_pi2 ./lora_gateway	

SX128X_lora_gateway_pi2: SX128X_lora_gateway.o arduPi_pi2.o SX12XX/SX128XLT.o base64.o
	g++ -lrt -lpthread SX128X_lora_gateway.o arduPi_pi2.o SX12XX/SX128XLT.o base64.o -o SX128X_lora_gateway_pi2
	rm -f lora_gateway
	ln -s SX128X_lora_gateway_pi2 ./lora_gateway	

#SX1280 ranging slave 
#####################
SX1280_ranging_slave_pi2: SX1280_ranging_slave.o arduPi_pi2.o SX12XX/SX128XLT.o base64.o
	g++ -lrt -lpthread SX1280_ranging_slave.o arduPi_pi2.o SX12XX/SX128XLT.o base64.o -o SX1280_ranging_slave_pi2
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
lora_gateway_pi4: SX127X_lora_gateway_pi4
#lora_gateway_pi4: SX128X_lora_gateway_pi4
#/////////////////////////////////////////

#SX1272 lora gateway 
####################
SX1272_lora_gateway_pi4: SX1272_lora_gateway.o arduPi_pi4.o SX1272/SX1272.o base64.o
	g++ -lrt -lpthread SX1272_lora_gateway.o arduPi_pi4.o SX1272/SX1272.o base64.o -o SX1272_lora_gateway_pi4
	rm -f lora_gateway
	ln -s SX1272_lora_gateway_pi4 ./lora_gateway

#SX12XX test gateway 
####################
SX126X_simple_lora_gateway_pi4: SX126X_simple_lora_gateway.o arduPi_pi4.o SX12XX/SX126XLT.o base64.o
	g++ -lrt -lpthread SX126X_simple_lora_gateway.o arduPi_pi4.o SX12XX/SX126XLT.o base64.o -o SX126X_simple_lora_gateway_pi4
	rm -f lora_gateway
	ln -s SX126X_simple_lora_gateway_pi4 ./simple_lora_gateway
	
SX127X_simple_lora_gateway_pi4: SX127X_simple_lora_gateway.o arduPi_pi4.o SX12XX/SX127XLT.o base64.o
	g++ -lrt -lpthread SX127X_simple_lora_gateway.o arduPi_pi4.o SX12XX/SX127XLT.o base64.o -o SX127X_simple_lora_gateway_pi4
	rm -f lora_gateway
	ln -s SX127X_simple_lora_gateway_pi4 ./simple_lora_gateway

SX128X_simple_lora_gateway_pi4: SX128X_simple_lora_gateway.o arduPi_pi4.o SX12XX/SX128XLT.o base64.o
	g++ -lrt -lpthread SX128X_simple_lora_gateway.o arduPi_pi4.o SX12XX/SX128XLT.o base64.o -o SX128X_simple_lora_gateway_pi4
	rm -f lora_gateway
	ln -s SX128X_simple_lora_gateway_pi4 ./simple_lora_gateway	
		
#SX12XX transmitter 
####################
SX126X_simple_lora_transmitter_pi4: SX126X_simple_lora_transmitter.o arduPi_pi4.o SX12XX/SX126XLT.o base64.o
	g++ -lrt -lpthread SX126X_simple_lora_transmitter.o arduPi_pi4.o SX12XX/SX126XLT.o base64.o -o SX126X_simple_lora_transmitter_pi4
	rm -f simple_lora_transmitter
	ln -s SX126X_simple_lora_transmitter_pi4 ./simple_lora_transmitter
	
SX127X_simple_lora_transmitter_pi4: SX127X_simple_lora_transmitter.o arduPi_pi4.o SX12XX/SX127XLT.o base64.o
	g++ -lrt -lpthread SX127X_simple_lora_transmitter.o arduPi_pi4.o SX12XX/SX127XLT.o base64.o -o SX127X_simple_lora_transmitter_pi4
	rm -f simple_lora_transmitter
	ln -s SX127X_simple_lora_transmitter_pi4 ./simple_lora_transmitter

SX128X_simple_lora_transmitter_pi4: SX128X_simple_lora_transmitter.o arduPi_pi4.o SX12XX/SX128XLT.o base64.o
	g++ -lrt -lpthread SX128X_simple_lora_transmitter.o arduPi_pi4.o SX12XX/SX128XLT.o base64.o -o SX128X_simple_lora_transmitter_pi4
	rm -f simple_lora_transmitter
	ln -s SX128X_simple_lora_transmitter_pi4 ./simple_lora_transmitter	
	
#SX12XX lora gateway 
####################
SX126X_lora_gateway_pi4: SX126X_lora_gateway.o arduPi_pi4.o SX12XX/SX126XLT.o base64.o
	g++ -lrt -lpthread SX126X_lora_gateway.o arduPi_pi4.o SX12XX/SX126XLT.o base64.o -o SX126X_lora_gateway_pi4
	rm -f lora_gateway
	ln -s SX126X_lora_gateway_pi4 ./lora_gateway	
		
SX127X_lora_gateway_pi4: SX127X_lora_gateway.o arduPi_pi4.o SX12XX/SX127XLT.o base64.o
	g++ -lrt -lpthread SX127X_lora_gateway.o arduPi_pi4.o SX12XX/SX127XLT.o base64.o -o SX127X_lora_gateway_pi4
	rm -f lora_gateway
	ln -s SX127X_lora_gateway_pi4 ./lora_gateway	

SX128X_lora_gateway_pi4: SX128X_lora_gateway.o arduPi_pi4.o SX12XX/SX128XLT.o base64.o
	g++ -lrt -lpthread SX128X_lora_gateway.o arduPi_pi4.o SX12XX/SX128XLT.o base64.o -o SX128X_lora_gateway_pi4
	rm -f lora_gateway
	ln -s SX128X_lora_gateway_pi4 ./lora_gateway	
	
#SX1280 ranging slave 
#####################
SX1280_ranging_slave_pi4: SX1280_ranging_slave.o arduPi_pi4.o SX12XX/SX128XLT.o base64.o
	g++ -lrt -lpthread SX1280_ranging_slave.o arduPi_pi4.o SX12XX/SX128XLT.o base64.o -o SX1280_ranging_slave_pi4
	rm -f ranging_slave
	ln -s SX1280_ranging_slave_pi4 ./ranging_slave	

################################
# rules
################################
	
SX1272_lora_gateway.o: SX1272_lora_gateway.cpp radio.makefile raspberry.makefile
	g++ $(CFLAGS) -I SX1272 -I . -c SX1272_lora_gateway.cpp -o SX1272_lora_gateway.o

SX1272_lora_gateway_pi2.o: SX1272_lora_gateway.cpp radio.makefile raspberry.makefile
	g++ $(CFLAGS) -I SX1272 -I . -DRASPBERRY2 -c SX1272_lora_gateway.cpp -o SX1272_lora_gateway_pi2.o	

##

SX126X_simple_lora_gateway.o: SX12XX_simple_lora_gateway.cpp SX12XX_simple_lora_gateway.h
	g++ $(CFLAGS) -I SX12XX -I . -DSX126X -c SX12XX_simple_lora_gateway.cpp -o SX126X_simple_lora_gateway.o
	
SX127X_simple_lora_gateway.o: SX12XX_simple_lora_gateway.cpp SX12XX_simple_lora_gateway.h
	g++ $(CFLAGS) -I SX12XX -I . -DSX127X -c SX12XX_simple_lora_gateway.cpp -o SX127X_simple_lora_gateway.o

SX128X_simple_lora_gateway.o: SX12XX_simple_lora_gateway.cpp SX12XX_simple_lora_gateway.h
	g++ $(CFLAGS) -I SX12XX -I . -DSX128X -c SX12XX_simple_lora_gateway.cpp -o SX128X_simple_lora_gateway.o
	
SX126X_simple_lora_transmitter.o: SX12XX_simple_lora_transmitter.cpp SX12XX_simple_lora_transmitter.h
	g++ $(CFLAGS) -I SX12XX -I . -DSX126X -c SX12XX_simple_lora_transmitter.cpp -o SX126X_simple_lora_transmitter.o
	
SX127X_simple_lora_transmitter.o: SX12XX_simple_lora_transmitter.cpp SX12XX_simple_lora_transmitter.h
	g++ $(CFLAGS) -I SX12XX -I . -DSX127X -c SX12XX_simple_lora_transmitter.cpp -o SX127X_simple_lora_transmitter.o

SX128X_simple_lora_transmitter.o: SX12XX_simple_lora_transmitter.cpp SX12XX_simple_lora_transmitter.h
	g++ $(CFLAGS) -I SX12XX -I . -DSX128X -c SX12XX_simple_lora_transmitter.cpp -o SX128X_simple_lora_transmitter.o	
			
SX126X_lora_gateway.o: SX12XX_lora_gateway.cpp SX126X_lora_gateway.h radio.makefile raspberry.makefile
	g++ $(CFLAGS) -I SX12XX -I . -DSX126X -c SX12XX_lora_gateway.cpp -o SX126X_lora_gateway.o

SX127X_lora_gateway.o: SX12XX_lora_gateway.cpp SX127X_lora_gateway.h radio.makefile raspberry.makefile
	g++ $(CFLAGS) -I SX12XX -I . -DSX127X -c SX12XX_lora_gateway.cpp -o SX127X_lora_gateway.o

SX128X_lora_gateway.o: SX12XX_lora_gateway.cpp SX128X_lora_gateway.h radio.makefile raspberry.makefile
	g++ $(CFLAGS) -I SX12XX -I . -DSX128X -c SX12XX_lora_gateway.cpp -o SX128X_lora_gateway.o
	
SX1280_ranging_slave.o: SX1280_ranging_slave.cpp SX1280_ranging_slave.h radio.makefile raspberry.makefile
	g++ $(CFLAGS) -I SX12XX -I . -DSX128X -c SX1280_ranging_slave.cpp -o SX1280_ranging_slave.o

##
	
#SX126X_simple_lora_gateway_pi2.o: SX12XX_simple_lora_gateway.cpp SX12XX_simple_lora_gateway.h
#	g++ $(CFLAGS) -I SX12XX -I . -DSX126X -DRASPBERRY2 -c SX12XX_simple_lora_gateway.cpp -o SX126X_simple_lora_gateway_pi2.o
	
#SX127X_simple_lora_gateway_pi2.o: SX12XX_simple_lora_gateway.cpp SX12XX_simple_lora_gateway.h
#	g++ $(CFLAGS) -I SX12XX -I . -DSX127X -DRASPBERRY2 -c SX12XX_simple_lora_gateway.cpp -o SX127X_simple_lora_gateway_pi2.o

#SX128X_simple_lora_gateway_pi2.o: SX12XX_simple_lora_gateway.cpp SX12XX_simple_lora_gateway.h
#	g++ $(CFLAGS) -I SX12XX -I . -DSX128X -DRASPBERRY2 -c SX12XX_simple_lora_gateway.cpp -o SX128X_simple_lora_gateway_pi2.o	

#SX126X_simple_lora_transmitter_pi2.o: SX12XX_simple_lora_transmitter.cpp SX12XX_simple_lora_transmitter.h
#	g++ $(CFLAGS) -I SX12XX -I . -DSX126X -DRASPBERRY2 -c SX12XX_simple_lora_transmitter.cpp -o SX126X_simple_lora_transmitter_pi2.o
	
#SX127X_simple_lora_transmitter_pi2.o: SX12XX_simple_lora_transmitter.cpp SX12XX_simple_lora_transmitter.h
#	g++ $(CFLAGS) -I SX12XX -I . -DSX127X -DRASPBERRY2 -c SX12XX_simple_lora_transmitter.cpp -o SX127X_simple_lora_transmittery_pi2.o

#SX128X_simple_lora_transmitter_pi2.o: SX12XX_simple_lora_transmitter.cpp SX12XX_simple_lora_transmitter.h
#	g++ $(CFLAGS) -I SX12XX -I . -DSX128X -DRASPBERRY2 -c SX12XX_simple_lora_transmitter.cpp -o SX128X_simple_lora_transmitter_pi2.o	
			
#SX126X_lora_gateway_pi2.o: SX12XX_lora_gateway.cpp SX126X_lora_gateway.h radio.makefile raspberry.makefile
#	g++ $(CFLAGS) -I SX12XX -I . -DSX126X -DRASPBERRY2 -c SX12XX_lora_gateway.cpp -o SX126X_lora_gateway_pi2.o

#SX127X_lora_gateway_pi2.o: SX12XX_lora_gateway.cpp SX127X_lora_gateway.h radio.makefile raspberry.makefile
#	g++ $(CFLAGS) -I SX12XX -I . -DSX127X -DRASPBERRY2 -c SX12XX_lora_gateway.cpp -o SX127X_lora_gateway_pi2.o
	
#SX128X_lora_gateway_pi2.o: SX12XX_lora_gateway.cpp SX128X_lora_gateway.h radio.makefile raspberry.makefile
#	g++ $(CFLAGS) -I SX12XX -I . -DSX128X -DRASPBERRY2 -c SX12XX_lora_gateway.cpp -o SX128X_lora_gateway_pi2.o
		
##

arduPi.o: arduPi.cpp arduPi.h
	g++ $(CFLAGS) -c arduPi.cpp -o arduPi.o	

arduPi_pi2.o: arduPi.cpp arduPi.h
	g++ $(CFLAGS) -DRASPBERRY2 -c arduPi.cpp -o arduPi_pi2.o	

arduPi_pi4.o: arduPi.cpp arduPi.h
	g++ $(CFLAGS) -DRASPBERRY4 -c arduPi.cpp -o arduPi_pi4.o

SX1272/SX1272.o: SX1272/SX1272.cpp SX1272/SX1272.h
	g++ $(CFLAGS) -I SX1272 -I . -c SX1272/SX1272.cpp -o SX1272/SX1272.o

SX12XX/SX126XLT.o: SX12XX/SX126XLT.cpp SX12XX/SX126XLT.h SX12XX/SX126XLT_Definitions.h
	g++ $(CFLAGS) -I SX12XX -I . -c SX12XX/SX126XLT.cpp -o SX12XX/SX126XLT.o

SX12XX/SX127XLT.o: SX12XX/SX127XLT.cpp SX12XX/SX127XLT.h SX12XX/SX127XLT_Definitions.h
	g++ $(CFLAGS) -I SX12XX -I . -c SX12XX/SX127XLT.cpp -o SX12XX/SX127XLT.o
	
SX12XX/SX128XLT.o: SX12XX/SX128XLT.cpp SX12XX/SX128XLT.h SX12XX/SX128XLT_Definitions.h
	g++ $(CFLAGS) -I SX12XX -I . -c SX12XX/SX128XLT.cpp -o SX12XX/SX128XLT.o

base64.o: base64.c base64.h
	g++ -c base64.c -o base64.o	

clean:
	cd SX1272; rm -rf *.o; cd ../SX12XX; rm -rf *.o; cd ..
	rm -rf *.o lora_gateway simple_lora_gateway simple_lora_transmitter ranging_slave SX1272_lora_gateway SX1272_lora_gateway_pi* SX12?X_lora_gateway SX12?X_lora_gateway_pi* SX12?X_simple_lora_gateway SX12?X_simple_lora_gateway_pi* SX12?X_simple_lora_transmitter SX12?X_simple_lora_transmitter_pi* SX1280_ranging_slave SX1280_ranging_slave_pi*
