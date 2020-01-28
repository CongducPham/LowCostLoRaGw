include radio.makefile
include raspberry.makefile

lora_gateway: lora_gateway.o arduPi.o SX1272.o base64.o
	g++ -lrt -lpthread lora_gateway.o arduPi.o SX1272.o base64.o -o lora_gateway	

lora_gateway_pi2: lora_gateway_pi2.o arduPi_pi2.o SX1272_pi2.o base64.o
	g++ -lrt -lpthread lora_gateway_pi2.o arduPi_pi2.o SX1272_pi2.o base64.o -o lora_gateway_pi2
	rm -f lora_gateway
	ln -s lora_gateway_pi2 ./lora_gateway

lora_gateway_pi4: lora_gateway_pi2.o arduPi_pi4.o SX1272_pi2.o base64.o
	g++ -lrt -lpthread lora_gateway_pi2.o arduPi_pi4.o SX1272_pi2.o base64.o -o lora_gateway_pi4
	rm -f lora_gateway
	ln -s lora_gateway_pi4 ./lora_gateway
	
lora_gateway.o: lora_gateway.cpp radio.makefile raspberry.makefile
	g++ $(CFLAGS) -c lora_gateway.cpp -o lora_gateway.o

lora_gateway_pi2.o: lora_gateway.cpp radio.makefile raspberry.makefile
	g++ $(CFLAGS) -DRASPBERRY2 -c lora_gateway.cpp -o lora_gateway_pi2.o

arduPi.o: arduPi.cpp arduPi.h
	g++ $(CFLAGS) -c arduPi.cpp -o arduPi.o	

arduPi_pi2.o: arduPi_pi2.cpp arduPi_pi2.h
	g++ $(CFLAGS) -c arduPi_pi2.cpp -o arduPi_pi2.o	

arduPi_pi4.o: arduPi_pi2.cpp arduPi_pi2.h
	g++ $(CFLAGS) -DRASPBERRY4 -c arduPi_pi2.cpp -o arduPi_pi4.o
	
SX1272.o: SX1272.cpp SX1272.h
	g++ $(CFLAGS) -c SX1272.cpp -o SX1272.o

SX1272_pi2.o: SX1272.cpp SX1272.h
	g++ $(CFLAGS) -DRASPBERRY2 -c SX1272.cpp -o SX1272_pi2.o

base64.o: base64.c base64.h
	g++ -c base64.c -o base64.o	

clean:
	rm *.o lora_*gateway
