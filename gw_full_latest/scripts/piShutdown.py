#from https://core-electronics.com.au/tutorials/how-to-make-a-safe-shutdown-button-for-raspberry-pi.html
#
from gpiozero import Button #import button from the Pi GPIO library
import time # import time functions
import os #imports OS library for Shutdown control

stopButton = Button(26) # defines the button as an object and chooses GPIO 26
rebootButton = Button(21) # defines the button as an object and chooses GPIO 21

while True: #infinite loop
	if stopButton.is_pressed: #Check to see if button is pressed
		time.sleep(1) # wait for the hold time we want. 
		if stopButton.is_pressed: #check if the user let go of the button
			os.system("shutdown now -h") #shut down the Pi -h is or -r will reset
	if rebootButton.is_pressed: #Check to see if button is pressed
			time.sleep(1) # wait for the hold time we want. 
			if rebootButton.is_pressed: #check if the user let go of the button
					os.system("reboot") #reboot the Pi			
	time.sleep(1) # wait to loop again so we don't use the processor too much.