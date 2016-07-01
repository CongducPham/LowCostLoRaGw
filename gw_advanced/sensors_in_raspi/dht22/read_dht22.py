"""
Remember to start the daemon with the command
'sudo pigpiod' before running this script. It
needs to be restarted every time your pi
is restarted.
"""
import os
import pigpio
import DHT22
from time import sleep

try:
	if(not os.path.isfile('/var/run/pigpio.pid')):
		os.system("sudo pigpiod")
		print("PIGPIO: the deamon is launched")
		sleep(3)
	else:
		print("PIGPIO: the deamon was already launched")
		
except Exception: pass

# Initiate GPIO for pigpio
pi = pigpio.pi()
# Setup the sensor
dht22 = DHT22.sensor(pi, 4) # use the actual GPIO pin name
dht22.trigger()

# We want our sleep time to be above 2 seconds.
sleepTime = 3

def get_dht22_values():
	# Wait few seconds before getting values
	sleep(sleepTime)
	
	# Get a new reading
	dht22.trigger()
	
	# Save our values
	humidity  = '%.2f' % (dht22.humidity())
	temp = '%.2f' % (dht22.temperature())
	return (humidity, temp)

if __name__ == '__main__':
	while True:
		humidity, temperature = readDHT22()
		print("Humidity is: " + humidity + "%")
		print("Temperature is: " + temperature + "*C")

