#include "arduinoPi.h"

/*********************************
 *                               *
 * SerialPi Class implementation *
 * ----------------------------- *
 *********************************/

/******************
 * Public methods *
 ******************/

//Constructor
SerialPi::SerialPi(){
    serialPort="/dev/ttyAMA0";
    timeOut = 1000;
}

//Sets the data rate in bits per second (baud) for serial data transmission
void SerialPi::begin(int serialSpeed){

	switch(serialSpeed){
		case     50:	speed =     B50 ; break ;
		case     75:	speed =     B75 ; break ;
		case    110:	speed =    B110 ; break ;
		case    134:	speed =    B134 ; break ;
		case    150:	speed =    B150 ; break ;
		case    200:	speed =    B200 ; break ;
		case    300:	speed =    B300 ; break ;
		case    600:	speed =    B600 ; break ;
		case   1200:	speed =   B1200 ; break ;
		case   1800:	speed =   B1800 ; break ;
		case   2400:	speed =   B2400 ; break ;
		case   9600:	speed =   B9600 ; break ;
		case  19200:	speed =  B19200 ; break ;
		case  38400:	speed =  B38400 ; break ;
		case  57600:	speed =  B57600 ; break ;
		case 115200:	speed = B115200 ; break ;
		default:	speed = B230400 ; break ;
			
	}


	if ((sd = open(serialPort, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK)) == -1){
		fprintf(stderr,"Unable to open the serial port %s - \n", serialPort);
		exit(-1);
	}
    
	fcntl (sd, F_SETFL, O_RDWR) ;
    
	tcgetattr(sd, &options);
	cfmakeraw(&options);
	cfsetispeed (&options, speed);
	cfsetospeed (&options, speed);

	options.c_cflag |= (CLOCAL | CREAD);
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	options.c_oflag &= ~OPOST;

	tcsetattr (sd, TCSANOW, &options);

	ioctl (sd, TIOCMGET, &status);

	status |= TIOCM_DTR;
	status |= TIOCM_RTS;

	ioctl (sd, TIOCMSET, &status);
	
	unistd::usleep (10000);
    
}

//Prints data to the serial port as human-readable ASCII text.
void SerialPi::print(const char *message){
    unistd::write(sd,message,strlen(message));
}

//Prints data to the serial port as human-readable ASCII text.
void SerialPi::print (char message){
	unistd::write(sd,&message,1);
}

/*Prints data to the serial port as human-readable ASCII text.
 * It can print the message in many format representations such as:
 * Binary, Octal, Decimal, Hexadecimal and as a BYTE. */
void SerialPi::print(unsigned char i,Representation rep){
    char * message;
    switch(rep){

        case BIN:
            message = int2bin(i);
            unistd::write(sd,message,strlen(message));
            break;
        case OCT:
            message = int2oct(i);
            unistd::write(sd,message,strlen(message));
            break;
        case DEC:
            sprintf(message,"%d",i);
            unistd::write(sd,message,strlen(message));
            break;
        case HEX:
            message = int2hex(i);
            unistd::write(sd,message,strlen(message));
            break;
        case BYTE:
            unistd::write(sd,&i,1);
            break;

    }
}

/* Prints data to the serial port as human-readable ASCII text.
 * precission is used to limit the number of decimals.
 */
void SerialPi::print(float f, int precission){
	/*
    const char *str1="%.";
    char * str2;
    char * str3;
    char * message;
    sprintf(str2,"%df",precission);
    asprintf(&str3,"%s%s",str1,str2);
    sprintf(message,str3,f);
	*/
	char message[10];
	sprintf(message, "%.1f", f );
    unistd::write(sd,message,strlen(message));
}

/* Prints data to the serial port as human-readable ASCII text followed
 * by a carriage retrun character '\r' and a newline character '\n' */
void SerialPi::println(const char *message){
	const char *newline="\r\n";
	char * msg = NULL;
	asprintf(&msg,"%s%s",message,newline);
    unistd::write(sd,msg,strlen(msg));
}

/* Prints data to the serial port as human-readable ASCII text followed
 * by a carriage retrun character '\r' and a newline character '\n' */
void SerialPi::println(char message){
	const char *newline="\r\n";
	char * msg = NULL;
	asprintf(&msg,"%s%s",&message,newline);
    unistd::write(sd,msg,strlen(msg));
}

/* Prints data to the serial port as human-readable ASCII text followed
 * by a carriage retrun character '\r' and a newline character '\n' */
void SerialPi::println(int i, Representation rep){
    char * message;
    switch(rep){

        case BIN:
            message = int2bin(i);
            break;
        case OCT:
            message = int2oct(i);
            break;
        case DEC:
            sprintf(message,"%d",i);
            break;
        case HEX:
            message = int2hex(i);
            break;

    }

    const char *newline="\r\n";
    char * msg = NULL;
    asprintf(&msg,"%s%s",message,newline);
    unistd::write(sd,msg,strlen(msg));
}

/* Prints data to the serial port as human-readable ASCII text followed
 * by a carriage retrun character '\r' and a newline character '\n' */
void SerialPi::println(float f, int precission){
    const char *str1="%.";
    char * str2;
    char * str3;
    char * message;
    sprintf(str2,"%df",precission);
    asprintf(&str3,"%s%s",str1,str2);
    sprintf(message,str3,f);

    const char *newline="\r\n";
    char * msg = NULL;
    asprintf(&msg,"%s%s",message,newline);
    unistd::write(sd,msg,strlen(msg));
}

/* Writes binary data to the serial port. This data is sent as a byte 
 * Returns: number of bytes written */
int SerialPi::write(unsigned char message){
	unistd::write(sd,&message,1);
	return 1;
}

/* Writes binary data to the serial port. This data is sent as a series
 * of bytes
 * Returns: number of bytes written */
int SerialPi::write(const char *message){
	int len = strlen(message);
	unistd::write(sd,&message,len);
	return len;
}

/* Writes binary data to the serial port. This data is sent as a series
 * of bytes placed in an buffer. It needs the length of the buffer
 * Returns: number of bytes written */
int SerialPi::write(char *message, int size){
	unistd::write(sd,message,size);
	return size;
}

/* Get the numberof bytes (characters) available for reading from 
 * the serial port.
 * Return: number of bytes avalable to read */
int SerialPi::available(){
    int nbytes = 0;
    if (ioctl(sd, FIONREAD, &nbytes) < 0)  {
		fprintf(stderr, "Failed to get byte count on serial.\n");
        exit(-1);
    }
    return nbytes;
}

/* Reads 1 byte of incoming serial data
 * Returns: first byte of incoming serial data available */
char SerialPi::read() {
	unistd::read(sd,&c,1);
    return c;
}

/* Reads characters from th serial port into a buffer. The function 
 * terminates if the determined length has been read, or it times out
 * Returns: number of bytes readed */
int SerialPi::readBytes(char message[], int size){
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
		int count;
		for (count=0;count<size;count++){
			if(available()) unistd::read(sd,&message[count],1);
			clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
			timespec t = timeDiff(time1,time2);
			if((t.tv_nsec/1000)>timeOut) break;
		}
		return count;
}

/* Reads characters from the serial buffer into an array. 
 * The function terminates if the terminator character is detected,
 * the determined length has been read, or it times out.
 * Returns: number of characters read into the buffer. */
int SerialPi::readBytesUntil(char character,char buffer[],int length){
    char lastReaded = character +1; //Just to make lastReaded != character
    int count=0;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    while(count != length && lastReaded != character){
        if(available()) unistd::read(sd,&buffer[count],1);
        lastReaded = buffer[count];
        count ++;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
        timespec t = timeDiff(time1,time2);
        if((t.tv_nsec/1000)>timeOut) break;
    }

    return count;
}


bool SerialPi::find(const char *target){
    return findUntil(target,NULL);
}

/* Reads data from the serial buffer until a target string of given length
 * or terminator string is found.
 * Returns: true if the target string is found, false if it times out */
bool SerialPi::findUntil(const char *target, const char *terminal){
    int index = 0;
    int termIndex = 0;
    int targetLen = strlen(target);
    int termLen = strlen(terminal);
    char readed;
    timespec t;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);

    if( *target == 0)
        return true;   // return true if target is a null string

    do{
        if(available()){
            unistd::read(sd,&readed,1);
            if(readed != target[index])
            index = 0; // reset index if any char does not match

            if( readed == target[index]){
                if(++index >= targetLen){ // return true if all chars in the target match
                    return true;
                }
            }

            if(termLen > 0 && c == terminal[termIndex]){
                if(++termIndex >= termLen) return false; // return false if terminate string found before target string
            }else{ 
                termIndex = 0;
            }
        }

        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
        t = timeDiff(time1,time2);

    }while((t.tv_nsec/1000)<=timeOut);

    return false;
}

/* returns the first valid (long) integer value from the current position.
 * initial characters that are not digits (or the minus sign) are skipped
 * function is terminated by the first character that is not a digit. */
long SerialPi::parseInt(){
    bool isNegative = false;
    long value = 0;
    char c;

    //Skip characters until a number or - sign found
    do{
        c = peek();
        if (c == '-') break;
        if (c >= '0' && c <= '9') break;
        unistd::read(sd,&c,1);  // discard non-numeric
    }while(1);

    do{
        if(c == '-')
            isNegative = true;
        else if(c >= '0' && c <= '9')// is c a digit?
            value = value * 10 + c - '0';
        unistd::read(sd,&c,1);  // consume the character we got with peek
        c = peek();

    }while(c >= '0' && c <= '9');

    if(isNegative)
        value = -value;
    return value;
}

float SerialPi::parseFloat(){
    boolean isNegative = false;
    boolean isFraction = false;
    long value = 0;
    char c;
    float fraction = 1.0;

    //Skip characters until a number or - sign found
    do{
        c = peek();
        if (c == '-') break;
        if (c >= '0' && c <= '9') break;
        unistd::read(sd,&c,1);  // discard non-numeric
    }while(1);

    do{
        if(c == '-')
            isNegative = true;
        else if (c == '.')
            isFraction = true;
        else if(c >= '0' && c <= '9')  {      // is c a digit?
            value = value * 10 + c - '0';
            if(isFraction)
                fraction *= 0.1;
        }
        unistd::read(sd,&c,1);  // consume the character we got with peek
        c = peek();
    }while( (c >= '0' && c <= '9')  || (c == '.' && isFraction==false));

    if(isNegative)
        value = -value;
    if(isFraction)
        return value * fraction;
    else
        return value;


}

// Returns the next byte (character) of incoming serial data without removing it from the internal serial buffer.
char SerialPi::peek(){
    //We obtain a pointer to FILE structure from the file descriptor sd
    FILE * f = fdopen(sd,"r+");
    //With a pointer to FILE we can do getc and ungetc
    c = getc(f);
    ungetc(c, f);
    return c;
}

// Remove any data remaining on the serial buffer
void SerialPi::flush(){
    while(available()){
        unistd::read(sd,&c,1);
    }
}

/* Sets the maximum milliseconds to wait for serial data when using SerialPi::readBytes()
 * The default value is set to 1000 */
void SerialPi::setTimeout(long millis){
	timeOut = millis;
}

//Disables serial communication
void SerialPi::end(){
	unistd::close(sd);
}

/*******************
 * Private methods *
 *******************/

//Returns a timespec struct with the time elapsed between start and end timespecs
timespec SerialPi::timeDiff(timespec start, timespec end){
	timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}

//Returns a binary representation of the integer passed as argument
char * SerialPi::int2bin(int i){
	size_t bits = sizeof(int) * CHAR_BIT;
    char * str = (char *)malloc(bits + 1);
    int firstCeros = 0;
    int size = bits;
    if(!str) return NULL;
    str[bits] = 0;

    // type punning because signed shift is implementation-defined
    unsigned u = *(unsigned *)&i;
    for(; bits--; u >>= 1)
        str[bits] = u & 1 ? '1' : '0';

    //Delete first 0's
    for (int i=0; i<bits; i++){
        if(str[i] == '0'){
            firstCeros++;
        }else{
            break;
        }
    }
    char * str_noceros = (char *)malloc(size-firstCeros+1);
    for (int i=0; i<(size-firstCeros);i++){
        str_noceros[i]=str[firstCeros+i];
    }

    return str_noceros;
}

//Returns an hexadecimal representation of the integer passed as argument
char * SerialPi::int2hex(int i){
	char buffer[32];
    sprintf(buffer,"%x",i);
    char * hex = (char *)malloc(strlen(buffer)+1);
    strcpy(hex,buffer);
    return hex;
}

//Returns an octal representation of the integer passed as argument
char * SerialPi::int2oct(int i){
    char buffer[32];
    sprintf(buffer,"%o",i);
    char * oct = (char *)malloc(strlen(buffer)+1);
    strcpy(oct,buffer);
    return oct;	
}


/*******************************
 *                             *
 * SPIPi Class implementation *
 * --------------------------- *
 *******************************/

/******************
 * Public methods *
 ******************/

SPIPi::SPIPi(){

}

void SPIPi::begin(){
		// Init SPI
  	wiringPiSPISetup(SPI_CHANNEL, 500000);
}

void SPIPi::end(){  

}

void SPIPi::setBitOrder(uint8_t order){
    // BCM2835_SPI_BIT_ORDER_MSBFIRST is the only one suported by SPI0
}

// defaults to 0, which means a divider of 65536.
// The divisor must be a power of 2. Odd numbers
// rounded down. The maximum SPI clock rate is
// of the APB clock
void SPIPi::setClockDivider(uint16_t divider){
		//here we are using wiringPiSPISetupMode (int channel, int speed, int mode)
}

void SPIPi::setDataMode(uint8_t mode){
		//here we are using wiringPiSPISetupMode (int channel, int speed, int mode)
}

// Writes (and reads) a single byte to SPI
// WARNING. With wiringPi, it seems not possible to make successive calls to SPI.transfer to send a series of bytes on the SPI bus
// it is recommended to use SPI.transfernb
uint8_t SPIPi::transfer(uint8_t value) {

		uint8_t spibuf[1];
		spibuf[0] = value;
		//spibuf[1] = 0x00;

  	wiringPiSPIDataRW(SPI_CHANNEL, spibuf, 1);
  
  	return spibuf[0];
}

void SPIPi::transfernb(char* tbuf, char* rbuf, uint32_t len) {

		wiringPiSPIDataRW(SPI_CHANNEL, (uint8_t*)tbuf, len);
		
		if (rbuf!=NULL) {
			for (int i=0; i<len; i++)
				rbuf[i]=tbuf[i];
		}
}

SerialPi Serial = SerialPi();
SPIPi SPI = SPIPi();
