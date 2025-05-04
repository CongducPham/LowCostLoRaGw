import RPi.GPIO as GPIO
import time

GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)

#Pin 33 in number is pin 13 on Rpi
GPIO.setup(13,GPIO.OUT)

#Pin 31 in number is pin 6 on Rpi
GPIO.setup(6,GPIO.IN)
status=GPIO.input(6)

if status==GPIO.LOW:
	print "Loranga3G is already ON"
else:
	print "Turn on Loranga3G "
	GPIO.output(13,GPIO.HIGH)
	time.sleep(2)
	GPIO.output(13,GPIO.LOW)
