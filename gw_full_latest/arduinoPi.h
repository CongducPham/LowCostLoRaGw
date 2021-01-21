/*
*  SerialPI class based on arduPi.cpp
*  Copyright (C) 2012 Libelium Comunicaciones Distribuidas S.L.
*  http://www.libelium.com
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*  Version 2.0 (For Raspberry Pi Rev2)
*  Author: Sergio Martinez
*/

/*
*  SPIPI class based rewritten to use wiringPI
*  by C. Pham, Jan 2021
*/

// Raspberry PI pin mapping
// Pin number in this global_conf.json are Wiring Pi number (wPi colunm)
// issue a `gpio readall` on PI command line to see mapping
// +-----+-----+---------+--B Plus--+---------+-----+-----+
// | BCM | wPi |   Name  | Physical | Name    | wPi | BCM |
// +-----+-----+---------+----++----+---------+-----+-----+
// |     |     |    3.3v |  1 || 2  | 5v      |     |     |
// |   2 |   8 |   SDA.1 |  3 || 4  | 5V      |     |     |
// |   3 |   9 |   SCL.1 |  5 || 6  | 0v      |     |     |
// |   4 |   7 | GPIO. 7 |  7 || 8  | TxD     | 15  | 14  |
// |     |     |      0v |  9 || 10 | RxD     | 16  | 15  |
// |  17 |   0 | GPIO. 0 | 11 || 12 | GPIO. 1 | 1   | 18  |
// |  27 |   2 | GPIO. 2 | 13 || 14 | 0v      |     |     |
// |  22 |   3 | GPIO. 3 | 15 || 16 | GPIO. 4 | 4   | 23  |
// |     |     |    3.3v | 17 || 18 | GPIO. 5 | 5   | 24  |
// |  10 |  12 |    MOSI | 19 || 20 | 0v      |     |     |
// |   9 |  13 |    MISO | 21 || 22 | GPIO. 6 | 6   | 25  |
// |  11 |  14 |    SCLK | 23 || 24 | CE0     | 10  | 8   |
// |     |     |      0v | 25 || 26 | CE1     | 11  | 7   |
// |   0 |  30 |   SDA.0 | 27 || 28 | SCL.0   | 31  | 1   |
// |   5 |  21 | GPIO.21 | 29 || 30 | 0v      |     |     |
// |   6 |  22 | GPIO.22 | 31 || 32 | GPIO.26 | 26  | 12  |
// |  13 |  23 | GPIO.23 | 33 || 34 | 0v      |     |     |
// |  19 |  24 | GPIO.24 | 35 || 36 | GPIO.27 | 27  | 16  |
// |  26 |  25 | GPIO.25 | 37 || 38 | GPIO.28 | 28  | 20  |
// |     |     |      0v | 39 || 40 | GPIO.29 | 29  | 21  |
// +-----+-----+---------+----++----+---------+-----+-----+
// | BCM | wPi |   Name  | Physical | Name    | wPi | BCM |
// +-----+-----+---------+--B Plus--+---------+-----+-----+

#ifndef arduinoPi_h
#define arduinoPi_h

#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>
#include <termios.h> 
#include <ctype.h>
#include <sys/ioctl.h>
#include <limits.h>
#include <algorithm>
#include <limits.h>
#include <pthread.h>
#include <poll.h>
#include <stdarg.h> //Include forva_start, va_arg and va_end strings functions

#define CS		10
#define MOSI	11
#define MISO	12
#define SCK		13

static int REV = 0;

#define LSBFIRST  0  ///< LSB First
#define MSBFIRST  1   ///< MSB First

#define SPI_MODE0 0  ///< CPOL = 0, CPHA = 0
#define SPI_MODE1 1  ///< CPOL = 0, CPHA = 1
#define SPI_MODE2 2  ///< CPOL = 1, CPHA = 0
#define SPI_MODE3 3

#define SPI_CLOCK_DIV65536 0       ///< 65536 = 256us = 4kHz
#define SPI_CLOCK_DIV32768 32768   ///< 32768 = 126us = 8kHz
#define SPI_CLOCK_DIV16384 16384   ///< 16384 = 64us = 15.625kHz
#define SPI_CLOCK_DIV8192 8192    ///< 8192 = 32us = 31.25kHz
#define SPI_CLOCK_DIV4096 4096    ///< 4096 = 16us = 62.5kHz
#define SPI_CLOCK_DIV2048 2048    ///< 2048 = 8us = 125kHz
#define SPI_CLOCK_DIV1024 1024    ///< 1024 = 4us = 250kHz
#define SPI_CLOCK_DIV512 512     ///< 512 = 2us = 500kHz
#define SPI_CLOCK_DIV256 256     ///< 256 = 1us = 1MHz
#define SPI_CLOCK_DIV128 128     ///< 128 = 500ns = = 2MHz
#define SPI_CLOCK_DIV64 64      ///< 64 = 250ns = 4MHz
#define SPI_CLOCK_DIV32 32      ///< 32 = 125ns = 8MHz
#define SPI_CLOCK_DIV16 16      ///< 16 = 50ns = 20MHz
#define SPI_CLOCK_DIV8 8       ///< 8 = 25ns = 40MHz
#define SPI_CLOCK_DIV4 4       ///< 4 = 12.5ns 80MHz
#define SPI_CLOCK_DIV2 2       ///< 2 = 6.25ns = 160MHz
#define SPI_CLOCK_DIV1 1       ///< 0 = 256us = 4kHz

namespace unistd {
	//All functions of unistd.h must be called like this: unistd::the_function()
    #include <unistd.h>
};


enum Representation{
	BIN,
	OCT,
	DEC,
	HEX,
	BYTE
};

/*
typedef enum
{
	LOW = 0,
	HIGH = 1,
	RISING = 2,
	FALLING = 3,
	BOTH = 4
} Digivalue;
*/

typedef bool boolean;
typedef unsigned char byte;

static const int SPI_CHANNEL = 0;

/* SerialPi Class
 * Class that provides the functionality of arduino Serial library
 */
class SerialPi {

private:
	int sd,status;
	const char *serialPort;
	unsigned char c;
	struct termios options;
	int speed;
	long timeOut;
	timespec time1, time2;
	timespec timeDiff(timespec start, timespec end);
	char * int2bin(int i);
	char * int2hex(int i);
	char * int2oct(int i);

public:

	SerialPi();
	void begin(int serialSpeed);
	int available();
	char read();
	int readBytes(char message[], int size);
	int readBytesUntil(char character,char buffer[],int length);
	bool find(const char *target);
	bool findUntil(const char *target, const char *terminal);
	long parseInt();
	float parseFloat();
	char peek();
	void print(const char *message);
	void print(char message);
	void print(unsigned char i,Representation rep);
	void print(float f, int precission);
	void println(const char *message);
	void println(char message);
	void println(int i, Representation rep);
	void println(float f, int precission);
	int write(unsigned char message);
	int write(const char *message);
	int write (char *message, int size);   
	void flush();
	void setTimeout(long millis);
	void end();
};

class SPIPi{
	public:
	
		SPIPi();
  	void begin();
    void end();
    void setBitOrder(uint8_t order);
 		void setClockDivider(uint16_t divider);
		void setDataMode(uint8_t mode);
 		uint8_t transfer(uint8_t value);
 		void transfernb(char* tbuf, char* rbuf, uint32_t len);
};

extern SPIPi SPI;
extern SerialPi Serial;
#endif
