Remote access with `ngrok`
==========================

This README describes a very simple and convenient way to access (and then administrate) your gateway, or a deployed one, from anywhere in the world using the [`ngrok`](https://ngrok.com/docs/2#getting-started) service.

Install `ngrok`
---------------

If you update the gateway software (using the [web admin interface procedure](https://github.com/CongducPham/LowCostLoRaGw#option-i)) `ngrok` support will be installed. If you just want to add the `ngrok` feature without a full update of your gateway, then can you just use the web admin interface `Gateway Update`/`Download and install a file` to sequentially download and install the 2 following files by copy/paste the URL (`ngrok`is about 13MB so it can take some time depending on your Internet access):

- `ngrok` binary: `https://www.dropbox.com/s/9zdbx9g78q7qree/ngrok`
- `cmd.sh` update=: `https://www.dropbox.com/s/0t8evgnanvzeqpz/cmd.sh`  

If you want to manually install the latest version of `ngrok` from `ngrok.com` use `ssh` to connect to your gateway, then:

	> cd /home/pi/Downloads
	> sudo wget https://bin.equinox.io/c/4VmDzA7iaHb/ngrok-stable-linux-arm.zip
	> unzip ngrok-stable-linux-arm.zip

You should now have the `ngrok` utility in `/home/pi/Downloads`. Copy it in `/home/pi/lora_gateway` folder:

	> cp ngrok /home/pi/lora_gateway

Sign in and enable your `ngrok` access
--------------------------------------

You need to create an account on `https://ngrok.com/signup`. Then go to `https://dashboard.ngrok.com` to get your authentication token. On the Raspberry, run the following command:

	> cd /home/pi/lora_gateway
	> ./ngrok authtoken <your_auth_token>
	
of course, replace `your_auth_token` by the token indicated in your `ngrok` dashboard. You need to do that step only once for a given gateway.

You can also use command `N` of the text command interface to enter the `ngrok` auth token.

Start `ssh` remote access
-------------------------

To start `ssh` remote access from anywhere in the world, even if your gateway is behind a local LAN, run the following command:

	> cd /home/pi/lora_gateway
	> ./ngrok tcp 22
	
`ngrok` will display something similar to:

	ngrok by @inconshreveable
	
	Session Status                online
	Account                       Congduc Pham (Plan: Free)
	Version                       2.2.8
	Region                        United States (us)
	Web Interface                 http://127.0.0.1:4040
	Forwarding                    tcp://0.tcp.ngrok.io:48590 -> localhost:22
	
	Connections                   ttl     opn     rt1     rt5     p50     p90
	                              0       0       0.00    0.00    0.00    0.00

Then from a terminal you can get access to your gateway with:

	> ssh -p 48590 pi@0.tcp.ngrok.io
	
Note that you can also start HTTP access with:

	> ./ngrok http 80
	
to make the gateway's web admin interface accessible from the outside. However, as the web admin interface has limited functionalities, a deep troubleshooting will most likely use `ssh` access.

You can use command `O` of the text command interface to start the `ngrok tcp 22` process.	 	

Typical usage of the `ngrok` service
------------------------------------

We use this `ngrok` service to administrate a deployed gateway when some troubleshooting and maintenance is needed. It is not intended for permanent access to a gateway but rather like a `Teamviewer`-like service. For instance, suppose that organization X with administrator person Y has a deployed gateway and Y needs some assistance. Y creates an account with `ngrok`, installs and runs `ngrok tcp 22`, then gives you (via phone, mail, SMS, `Whatsapp`,...) the `ngrok`'s URL, e.g. `tcp://0.tcp.ngrok.io:48590`. Then, from your office, you can just run `ssh -p 48590 pi@0.tcp.ngrok.io` to manage Y's gateway: upgrade the gateway, configure a particular service, troubleshooting,... When the maintenance is finished, Y will just tear down the `ngrok` tunnel.

Use the `cmd.sh` text command interface
---------------------------------------

`cmd.sh` has been extended with 3 commands:

	----------------------------------------------------------* ngrok *--+
	M- get and install ngrok                                             +
	N- ngrok authtoken                                                   +
	O- ngrok tcp 22                                                      + 
	---------------------------------------------------------------------+	
	
A user can use its smartphone are tablet to connect on the gateway's WiFi and with an `ssh` app, log on 192.168.200.1. On iOS we tested with `Termius` and on Android we tested with `JuiceSSH`.	Once log into the gateway, the text command interface in automatically launched and the user can choose and run pre-defined commands. Here, typically, he can use command `M` to install `ngrok` (if it is needed), command `N` to register its `ngrok` access (after signing up with `ngrok`) and command `O` to start remote access each time he needs someone else to get access to the gateway, following the previously described procedure. 
	
	
Enjoy!
C. Pham		