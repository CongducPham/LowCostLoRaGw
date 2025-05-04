import sys

#regular data packet from device 6, without appkey
#payload is \!TC/22.5
print "^p1,16,6,0,9,8,-45"
print "^r125,5,12"
print "^t2017-11-20T14:18:54+01:00"
print "\xFF\xFE\!TC/22.5"

sys.stdout.flush()