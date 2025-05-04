# Gateway Web Admin

Gateway Web Admin is a simple responsive web interface to manage/configure some parameters of the Raspberry Pi LoRa IoT gateway.

Use your browser, connect to the gateway's WiFi (gateway in Access Point mode) and enter the following url:

	192.168.200.1/admin
	
![](https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/gw-web-admin.png)

The current SD card image has everything you need, including the web admin interface installed. Use the web admin interface to update the gateway software and the web admin interface to the latest version from the github repository.

Look at the [Low-cost-LoRa-GW web admin interface tutorial](https://github.com/CongducPham/tutorials/blob/master/Low-cost-LoRa-GW-web-admin.pdf). For most end-users, the web admin interface is sufficient to configure and manage the gateway. For instance, to configure a new gateway, just use the web admin interface to update the gateway software and to run the basic configuration procedure.

![](https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/gw-web-admin-update.png)

## Manual installation

Go to `lora_gateway/gw_web_admin` folder

	> cd lora_gateway/gw_web_admin
	
Run the installation script from your RaspberryPi's shell prompt:

	> sudo ./install.sh 

The installer will complete all required steps for you.

Enjoy!
M. Diop and C. Pham	