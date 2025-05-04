#option are SX1272, SX126X, SX127X or SX128X
#SX12XX=SX1272
SX12XX=SX126X
#SX12XX=SX127X
#SX12XX=SX128X

#options are: -DPABOOST -DMAX_DBM=v -DBAND868|-DBAND900|-DBAND433 -DNRESET=6 -DNSS=10 -DRFBUSY=5
# default: NRESET=6 means Arduino pin 6 which will be translated into GPIO4 by ArduPi
# use NRESET=7 for WaziHat, translated into GPI17 by ArduPi
# default: NSS=10 means Arduino pin 10 which will be translated into GPIO8/SPI_CE0 by ArduPi
# default: RFBUSY=5 means Arduino pin 5 which will be translated into GPIO25/GEN06 by ArduPi 
CFLAGS+=-DPABOOST
CFLAGS+=-DMAX_DBM=14
CFLAGS+=-DBAND868
#NRESET=0 is for wiringPi, correspond to GPIO17 (WaziHat)
#NRESET=7 is for wiringPi, correspond to GPIO4 
#RFBUSY=6 is for wiringPi, correspond to GPIO25 (SX1262/SX1282)
#NSS=10 remains the same for Arduino (arduPi) and wiringPi
#
#CFLAGS+=-DNSS=10 -DNRESET=7 -DRFBUSY=6
#
#Raspberry PI pin mapping
#issue a `gpio readall` on PI command line to see mapping
#+-----+-----+---------+--B Plus--+---------+-----+-----+
#| BCM | wPi |   Name  | Physical | Name    | wPi | BCM |
#+-----+-----+---------+----++----+---------+-----+-----+
#|     |     |    3.3v |  1 || 2  | 5v      |     |     |
#|   2 |   8 |   SDA.1 |  3 || 4  | 5V      |     |     |
#|   3 |   9 |   SCL.1 |  5 || 6  | 0v      |     |     |
#|   4 |   7 | GPIO. 7 |  7 || 8  | TxD     | 15  | 14  |
#|     |     |      0v |  9 || 10 | RxD     | 16  | 15  |
#|  17 |   0 | GPIO. 0 | 11 || 12 | GPIO. 1 | 1   | 18  |
#|  27 |   2 | GPIO. 2 | 13 || 14 | 0v      |     |     |
#|  22 |   3 | GPIO. 3 | 15 || 16 | GPIO. 4 | 4   | 23  |
#|     |     |    3.3v | 17 || 18 | GPIO. 5 | 5   | 24  |
#|  10 |  12 |    MOSI | 19 || 20 | 0v      |     |     |
#|   9 |  13 |    MISO | 21 || 22 | GPIO. 6 | 6   | 25  |
#|  11 |  14 |    SCLK | 23 || 24 | CE0     | 10  | 8   |
#|     |     |      0v | 25 || 26 | CE1     | 11  | 7   |
#|   0 |  30 |   SDA.0 | 27 || 28 | SCL.0   | 31  | 1   |
#|   5 |  21 | GPIO.21 | 29 || 30 | 0v      |     |     |
#|   6 |  22 | GPIO.22 | 31 || 32 | GPIO.26 | 26  | 12  |
#|  13 |  23 | GPIO.23 | 33 || 34 | 0v      |     |     |
#|  19 |  24 | GPIO.24 | 35 || 36 | GPIO.27 | 27  | 16  |
#|  26 |  25 | GPIO.25 | 37 || 38 | GPIO.28 | 28  | 20  |
#|     |     |      0v | 39 || 40 | GPIO.29 | 29  | 21  |
#+-----+-----+---------+----++----+---------+-----+-----+
#| BCM | wPi |   Name  | Physical | Name    | wPi | BCM |
#+-----+-----+---------+--B Plus--+---------+-----+-----+
#
