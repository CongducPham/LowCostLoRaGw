Captive WiFi
============

If `nodogsplash` is not installed, install it with:

	> cd
	> cd Downloads
	> sudo apt install libmicrohttpd-dev
	> git clone https://github.com/nodogsplash/nodogsplash.git
	> cd nodogsplash
	> make
	> sudo make install

Then update the LowCostLoRa gateway distribution. That will install the `lora_gateway/splash` folder and content.

	> cd
	> cd lora_gateway/scripts
	> ./update_gw.sh
	
Then, install the specific `nodogsplash` configuration:

	> cd splash
	> ./install.sh
	
Once installed, use a smartphone or tablet to connect to the gateway's WiFi. You should see the captive portal allowing you to display the sensor data web page.		
	
Enjoy!
C. Pham		