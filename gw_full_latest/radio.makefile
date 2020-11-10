#options are: -DPABOOST -DMAX_DBM=v -DBAND868|-DBAND900|-DBAND433 -DNRESET=6 -DNSS=10
# default: NRESET=6 means Arduino pin 6 which will be translated into GPIO4 by ArduPi
# use NRESET=7 for WaziHat, translated into GPI17 by ArduPi
# default: NSS=10 means Arduino pin 10 which will be translated into GPIO8/SPI_CE0 by ArduPi

CFLAGS=-DPABOOST -DMAX_DBM=14 -DBAND868 -DNRESET=6 -DNSS=10

