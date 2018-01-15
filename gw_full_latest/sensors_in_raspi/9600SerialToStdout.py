import serial
import sys

if len(sys.argv) < 2:
    thePort = "/dev/ttyACM0"
else:
    thePort = sys.argv[1]

if len(sys.argv) < 3:
    theBaud = 9600
else:
    theBaud = sys.argv[2]
    
ser = serial.Serial(thePort, theBaud, timeout=0)

# flush everything that may have been received on the port to make sure that we start with a clean serial input
ser.flushInput()

while True:
    out = ''
    sys.stdout.write(ser.read(1024))
    sys.stdout.flush()
