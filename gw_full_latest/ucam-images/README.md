Configuring the gateway for image features
==========================================

Received encoded images will be saved by post_processing_gw.py in /home/pi/Dropbox/LoRa-test/images. post_processing_gw.py will call the image decoding program (see below) to decode in BMP the encoded image and will move the BMP file into /var/www/html/images/uploads.

You can use the config_image.sh script to create all these folders and set the correct file permissions.

	> cd ucam-images
	> sudo ./config_images.sh
	
A very simple PHP script (index.php) will be copied into /var/www/html/images/ allowing you to visualize with a web browser all received images:

	http://192.168.200.1/images/index.php	
	
assuming that you are connected to the gateway's WiFi network. If you use Ethernet with a DHCP address, use that address.	

Standalone image decoding program
=================================

decode_to_bmp is a standalone image decoding command line tool that decodes in BMP format an image that have been compressed by our image sensor platform (see http://cpham.perso.univ-pau.fr/WSN-MODEL/tool-html/imagesensor.html). It is intended to be used as part of our low-cost LoRa platform: post_processing_gw.py Python script handles image packets sent by the image sensor and will call decode_to_bmp to decode incoming images into BMP format. 

You may need to compile the program on the Raspberry:

	> cd ucam-images
	> g++ -o decode_to_bmp decode_to_bmp.c

If you don't use it as a standalone program, but only from post_processing_gw.py then you don't have anything to do more.

decode_to_bmp is called with some parameters that allows it to name the decoded image accordingly. See below for the parameter list. For instance, if you receive a first image from sensor 3 taken by camera 0 and encoded with a quality factor of 20, then the BMP image will be named: 

	ucam_0-node_0003-cam_0-Q20-P2-S347
	
where P2 means that there are 2 image packets and S347 means that the real encoded size of the image is 347 bytes.	

Test
====

Test decode_to_bmp with:

	> ./decode_to_bmp -received test-Q20.dat -SN 0 -src 6 -camid 0 -Q 20 128x128-test.bmp
	
Parameters
==========

	-received: indicates that the image is received from an image sensor
	-SN n: indicate an image sequence number n
	-src a: indicates a source image sensor address
	-camid c: indicates the source camid (in case of multiple camera sensor)
	-Q q: the quality factor
	file: this is the BMP file used for color information 	