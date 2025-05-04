
import RPi.GPIO as GPIO
import time

GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)

#Pin 15 in number is pin 22 on Rpi
GPIO.setup(22,GPIO.OUT)
print "Turn on Sim Module"
GPIO.output(22,GPIO.HIGH)
time.sleep(2)
#print "LED off"
GPIO.output(22,GPIO.LOW)
