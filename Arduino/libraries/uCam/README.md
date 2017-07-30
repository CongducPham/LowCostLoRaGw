uCam - 4D Systems uCam Control Code
====

This is a piece of code that we wrote long back to control uCam-TTL by 4D 
Systems with an Arduino.

We suggest using an Arduino Mega as it has multiple serial port and this code 
requires two. One to communicate with the Camera and the other to send the 
captured image over Serial port.

You should also have the data sheet of the camera handy to write some additional
piece of code that this library will require.


How to use
----

Just dump the .h and .cpp files in your project folder.

Then initialize the camera somewhere in your code like so:

    UCAM529 camera(inSerial, outSerial, CAMERA_ON_PIN, PICTURE_QUALITY);

Where,

* inSerial is the pointer to the serial port where camera is connected
* outSerial is the pointer to the serial port where data is to be sent
* CAMERA_ON_PIN is the pin number where camera reset is connected
* PICTURE_QUALITY is one of the hex codes given in the camera's data sheet.

> Important note: You will need to increase the RX TX buffer limits in the 
> Arduino Serial library code.

Then you have to initialize the camera:

    camera.init()
    
Then take a picture using the following command:

    camera.takePicture()
    
Simple enough.


Additional Notes
----

This code will require some additional work to make it work properly but at 
least it will give people a good start with their own implementation.

You will also have to write some code on the receiving end to parse the image 
data and save it somewhere or display it somewhere or send it somewhere. If you
are using a serial modem or some kind of serial transport you could send the 
picture directly over that using this code.


