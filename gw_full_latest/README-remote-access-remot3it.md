Remote access with `remot3.it`
==============================

This README describes a very simple and convenient way to access (and then administrate) your gateway, or a deployed one, from anywhere in the world using the [`remot3.it`](https://www.remot3.it/web/index.html) tool.

`remot3.it` works as follows: there are software running on your RPI that will connect the RPI to the `remot3.it` platform. When you register an RPI, you will give it a name, e.g. MY_RPI_GW, that will appear on the `remot3.it` platform when you logged into your `remot3.it` account. When selecting a remote RPI, you will see the various services that are installed on the RPI, e.g. MY_RPI_GW_SSH for SSH service MY_RPI_GW_WEB for TCP (web) service. When selecting a service, `remot3.it` will provide you dynamic information such as URL and port number for you to `ssh` (with SSH service) or access a web page (with TCP web service) on your remote RPI. The advantage of `remot3.it` is that you can perform all these steps remotely even though the URL can changed from one session to another as you will get them from your `remot3.it` account.

Sign in and enable your `remot3.it` account
-------------------------------------------

You need to create an account on `https://www.remot3.it/web/index.html`. A mail will be sent to you to activate the account. An account will allow you to manage as many RPI gateways as you want.

Install `remot3.it` tool
------------------------

You have to use `ssh` to connect to your RPI gateway, then:

	> sudo apt-get update
	> sudo apt-get install weavedconnectd

Then use `weavedinstaller` to install services that you want to use. 

	> sudo weavedinstaller
	
It is recommended to add SSH and TCP (web) services

For `sudo weavedinstaller`, follow instructions [here](http://forum.weaved.com/t/how-to-get-started-with-remot3-it-for-pi/1029).

For instance, create SSH service, name it MY_RPI_GW_SSH, and TCP service for web access, name it MY_RPI_GW_WEB.

Start `ssh` remote access
-------------------------

Log in your `remot3.it` account, select the RPI gateway you want to administrate, then select the SSH service. `remot3.it` will provide you with the URL and port number to run an `ssh` session to access your RPI.

For instance, you can remotely perform complex configuration tasks by editing various files such as `gateway_conf.json` or `clouds.json` to install a new cloud script.

Start `TCP web` remote access
----------------------------- 	

Log in your `remot3.it` account, select the RPI you want to access, then select the TCP service. `remot3.it` will provide you with the URL to copy/paste into a web browser to open the corresponding web pages on the RPI.

The web access will allow you to access the gateway's web admin interface to, for instance, update the gateway software, although this update can easily be performed with the SSH service by running the `update_gw.sh` script.
	
	
Enjoy!
C. Pham		