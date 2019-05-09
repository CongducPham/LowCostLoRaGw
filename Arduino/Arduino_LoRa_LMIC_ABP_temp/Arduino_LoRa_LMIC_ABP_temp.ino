/*******************************************************************************
 * Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman
 *
 * Permission is hereby granted, free of charge, to anyone
 * obtaining a copy of this document and accompanying files,
 * to do whatever they want with them without any restriction,
 * including, but not limited to, copying, modification and redistribution.
 * NO WARRANTY OF ANY KIND IS PROVIDED.
 *
 * This example sends a valid LoRaWAN packet with payload "Hello,
 * world!", using frequency and encryption settings matching those of
 * the The Things Network.
 *
 * This uses ABP (Activation-by-personalisation), where a DevAddr and
 * Session keys are preconfigured (unlike OTAA, where a DevEUI and
 * application key is configured, while the DevAddr and session keys are
 * assigned/generated in the over-the-air-activation procedure).
 *
 * Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in
 * g1, 0.1% in g2), but not the TTN fair usage policy (which is probably
 * violated by this sketch when left running for longer)!
 *
 * To use this sketch, first register your application and device with
 * the things network, to set or generate a DevAddr, NwkSKey and
 * AppSKey. Each device should have their own unique values for these
 * fields.
 *
 * Do not forget to define the radio type correctly in config.h.
 *
 *******************************************************************************/

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include "my_temp_sensor_code.h"

/********************************************************************
 _____              __ _                       _   _             
/  __ \            / _(_)                     | | (_)            
| /  \/ ___  _ __ | |_ _  __ _ _   _ _ __ __ _| |_ _  ___  _ __  
| |    / _ \| '_ \|  _| |/ _` | | | | '__/ _` | __| |/ _ \| '_ \ 
| \__/\ (_) | | | | | | | (_| | |_| | | | (_| | |_| | (_) | | | |
 \____/\___/|_| |_|_| |_|\__, |\__,_|_|  \__,_|\__|_|\___/|_| |_|
                          __/ |                                  
                         |___/                                   
********************************************************************/

///////////////////////////////////////////////////////////////////
// COMMENT OR UNCOMMENT TO CHANGE FEATURES. 
// ONLY IF YOU KNOW WHAT YOU ARE DOING!!! OTHERWISE LEAVE AS IT IS
// 
#define WITH_EEPROM
//if you are low on program memory, comment STRING_LIB to save about 2K
#define STRING_LIB
#define LOW_POWER
#define LOW_POWER_HIBERNATE
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// ADD HERE OTHER PLATFORMS THAT DO NOT SUPPORT EEPROM NOR LOW POWER
#if defined ARDUINO_SAM_DUE || defined __SAMD21G18A__
#undef WITH_EEPROM
#endif

#if defined ARDUINO_SAM_DUE
#undef LOW_POWER
#endif
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
unsigned int TX_INTERVAL = 1*60;
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//For TTN, use specify ABP mode on the TTN device setting

// LoRaWAN NwkSKey, network session key
//Enter here your Network Session Key from the TTN device info (same order, i.e. msb)
static const PROGMEM u1_t NWKSKEY[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// LoRaWAN AppSKey, application session key
//Enter here your App Session Key from the TTN device info (same order, i.e. msb)
static const u1_t PROGMEM APPSKEY[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// LoRaWAN end-device address (DevAddr)
//Enter here your Device Address from the TTN device info (same order, i.e. msb)
//example 0x12345678
static const u4_t DEVADDR = 0x12345678;

///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// IF YOU SEND A LONG STRING, INCREASE THE SIZE OF MESSAGE
uint8_t message[80];
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// COMMENT THIS LINE IF YOU WANT TO DYNAMICALLY SET THE NODE'S ADDR 
// OR SOME OTHER PARAMETERS BY REMOTE RADIO COMMANDS (WITH_RCVW)
// LEAVE THIS LINE UNCOMMENTED IF YOU WANT TO USE THE DEFAULT VALUE
// AND CONFIGURE YOUR DEVICE BY CHANGING MANUALLY THESE VALUES IN 
// THE SKETCH.
//
// ONCE YOU HAVE FLASHED A BOARD WITHOUT FORCE_DEFAULT_VALUE, YOU 
// WILL BE ABLE TO DYNAMICALLY CONFIGURE IT AND SAVE THIS CONFIGU-
// RATION INTO EEPROM. ON RESET, THE BOARD WILL USE THE SAVED CON-
// FIGURATION.

// IF YOU WANT TO REINITIALIZE A BOARD, YOU HAVE TO FIRST FLASH IT 
// WITH FORCE_DEFAULT_VALUE, WAIT FOR ABOUT 10s SO THAT IT CAN BOOT
// AND FLASH IT AGAIN WITHOUT FORCE_DEFAULT_VALUE. THE BOARD WILL 
// THEN USE THE DEFAULT CONFIGURATION UNTIL NEXT CONFIGURATION.

#define FORCE_DEFAULT_VALUE
///////////////////////////////////////////////////////////////////

// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

static osjob_t sendjob;

// Pin mapping

const lmic_pinmap lmic_pins = {
    .nss = 10,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 4,
    .dio = {7, 8, 9},
};

/*****************************
 _____           _      
/  __ \         | |     
| /  \/ ___   __| | ___ 
| |    / _ \ / _` |/ _ \
| \__/\ (_) | (_| |  __/
 \____/\___/ \__,_|\___|
*****************************/

// we wrapped Serial.println to support the Arduino Zero or M0
#if defined __SAMD21G18A__ && not defined ARDUINO_SAMD_FEATHER_M0
#define PRINTLN                   SerialUSB.println("")              
#define PRINT_CSTSTR(fmt,param)   SerialUSB.print(F(param))
#define PRINT_STR(fmt,param)      SerialUSB.print(param)
#define PRINT_VALUE(fmt,param)    SerialUSB.print(param)
#define PRINT_HEX(fmt,param)      SerialUSB.print(param,HEX)
#define FLUSHOUTPUT               SerialUSB.flush();
#else
#define PRINTLN                   Serial.println("")
#define PRINT_CSTSTR(fmt,param)   Serial.print(F(param))
#define PRINT_STR(fmt,param)      Serial.print(param)
#define PRINT_VALUE(fmt,param)    Serial.print(param)
#define PRINT_HEX(fmt,param)      Serial.print(param,HEX)
#define FLUSHOUTPUT               Serial.flush();
#endif

#ifdef WITH_EEPROM
#include <EEPROM.h>
#endif

#ifdef LOW_POWER
// this is for the Teensy36, Teensy35, Teensy31/32 & TeensyLC
// need v6 of Snooze library
#if defined __MK20DX256__ || defined __MKL26Z64__ || defined __MK64FX512__ || defined __MK66FX1M0__
#define LOW_POWER_PERIOD 60
#include <Snooze.h>
SnoozeTimer timer;
SnoozeBlock sleep_config(timer);
#else // for all other boards based on ATMega168, ATMega328P, ATMega32U4, ATMega2560, ATMega256RFR2, ATSAMD21G18A
#define LOW_POWER_PERIOD 8
// you need the LowPower library from RocketScream
// https://github.com/rocketscream/Low-Power
#include "LowPower.h"

#ifdef __SAMD21G18A__
// use the RTC library
#include "RTCZero.h"
/* Create an rtc object */
RTCZero rtc;
#endif
#endif
unsigned int nCycle = TX_INTERVAL/LOW_POWER_PERIOD;
#endif

unsigned long nextTransmissionTime=0L;

#ifdef WITH_EEPROM
struct sx1272config {

  uint8_t flag1;
  uint8_t flag2;
  uint8_t seq;
  unsigned int idle_period;  
  uint8_t overwrite;
  // can add other fields such as LoRa mode,...
};

sx1272config my_sx1272config;
#endif

//this function is provided to parse the downlink command which is assumed to be in the format /@A6#
//
long getCmdValue(int &i, char* strBuff=NULL) {
  
    char seqStr[7]="******";
    
    int j=0;
    // character '#' will indicate end of cmd value
    while ((char)message[i]!='#' && (i < strlen((char*)message)) && j<strlen(seqStr)) {
            seqStr[j]=(char)message[i];
            i++;
            j++;
    }
    
    // put the null character at the end
    seqStr[j]='\0';
    
    if (strBuff) {
            strcpy(strBuff, seqStr);        
    }
    else
            return (atol(seqStr));
} 

#ifndef STRING_LIB

char *ftoa(char *a, double f, int precision)
{
 long p[] = {0,10,100,1000,10000,100000,1000000,10000000,100000000};
 
 char *ret = a;
 long heiltal = (long)f;
 itoa(heiltal, a, 10);
 while (*a != '\0') a++;
 *a++ = '.';
 long desimal = abs((long)((f - heiltal) * p[precision]));
 if (desimal < p[precision-1]) {
  *a++ = '0';
 } 
 itoa(desimal, a, 10);
 return ret;
}
#endif

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            break;
        case EV_RFU1:
            Serial.println(F("EV_RFU1"));
            break;
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
              Serial.println(F("Received "));
              Serial.println(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
              for (int i = 0; i < LMIC.dataLen; i++) {
              if (LMIC.frame[LMIC.dataBeg + i] < 0x10) {
              Serial.print(F("0"));
              }
              Serial.print(LMIC.frame[LMIC.dataBeg + i], HEX);
              }
            }

#if defined LOW_POWER && not defined _VARIANT_ARDUINO_DUE_X_
            PRINT_CSTSTR("%s","Switch to power saving mode\n");
            FLUSHOUTPUT
            
            //TODO set the radio module in sleep mode
      
#ifdef __SAMD21G18A__
            // For Arduino M0 or Zero we use the built-in RTC
            rtc.setTime(17, 0, 0);
            rtc.setDate(1, 1, 2000);
            rtc.setAlarmTime(17, TX_INTERVAL, 0);
            // for testing with 20s
            //rtc.setAlarmTime(17, 0, 20);
            rtc.enableAlarm(rtc.MATCH_HHMMSS);
            //rtc.attachInterrupt(alarmMatch);
            rtc.standbyMode();
            
            LowPower.standby();
      
            PRINT_CSTSTR("%s","SAMD21G18A wakes up from standby\n");      
            FLUSHOUTPUT
#else
            nCycle = TX_INTERVAL/LOW_POWER_PERIOD;

#if defined __MK20DX256__ || defined __MKL26Z64__ || defined __MK64FX512__ || defined __MK66FX1M0__
            // warning, setTimer accepts value from 1ms to 65535ms max
            // milliseconds
            timer.setTimer(LOW_POWER_PERIOD*1000);
      
            nCycle = TX_INTERVAL/LOW_POWER_PERIOD;
#endif
            delay(10);
            
            for (uint8_t i=0; i<nCycle; i++) {  

#if defined ARDUINO_AVR_PRO || defined ARDUINO_AVR_NANO || defined ARDUINO_AVR_UNO || defined ARDUINO_AVR_MINI || defined __AVR_ATmega32U4__         
                // ATmega328P, ATmega168, ATmega32U4
                LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
                
                //LowPower.idle(SLEEP_8S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, 
                //              SPI_OFF, USART0_OFF, TWI_OFF);
#elif defined ARDUINO_AVR_MEGA2560
                // ATmega2560
                LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
                
                //LowPower.idle(SLEEP_8S, ADC_OFF, TIMER5_OFF, TIMER4_OFF, TIMER3_OFF, 
                //      TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART3_OFF, 
                //      USART2_OFF, USART1_OFF, USART0_OFF, TWI_OFF);
#elif defined __MK20DX256__ || defined __MKL26Z64__ || defined __MK64FX512__ || defined __MK66FX1M0__
                // Teensy31/32 & TeensyLC
#ifdef LOW_POWER_HIBERNATE
                Snooze.hibernate(sleep_config);
#else            
                Snooze.deepSleep(sleep_config);
#endif  
#else
                // use the delay function
                delay(LOW_POWER_PERIOD*1000);
#endif                        
                PRINT_CSTSTR("%s",".");
                FLUSHOUTPUT
                delay(10);                        
            }

            do_send(&sendjob);
#endif  
      
#else
            // Schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
#endif            
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
         default:
            Serial.println(F("Unknown event"));
            break;
    }
}

void do_send(osjob_t* j){
    // THIS IS WHERE YOU INSERT THE USEFUL TASKS
    ////////////////////////////////////////////

#ifdef LOW_POWER
    digitalWrite(PIN_POWER,HIGH);
    // security?
    delay(200);    
#endif

    float temp=0.0;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // change here how the temperature should be computed depending on your sensor type
    //  
  
    for (int i=0; i<5; i++) {
        temp += sensor_getValue();  
        delay(100);
    }

    //
    // 
    // /////////////////////////////////////////////////////////////////////////////////////////////////////////// 
    
#ifdef LOW_POWER
    digitalWrite(PIN_POWER,LOW);
#endif
    
    PRINT_CSTSTR("%s","Mean temp is ");
    temp = temp/5;
    PRINT_VALUE("%f", temp);
    PRINTLN;

    //for testing
    //temp = 22.5;

    uint8_t r_size;

    // the recommended format if now \!TC/22.5
#ifdef STRING_LIB
    r_size=sprintf((char*)message,"\\!%s/%s",nomenclature_str,String(temp).c_str());
#else
    char float_str[10];
    ftoa(float_str,temp,2);
    // this is for testing, uncomment if you just want to test, without a real temp sensor plugged
    //strcpy(float_str, "21.55567");
    r_size=sprintf((char*)message,"\\!%s/%s",nomenclature_str,float_str);
#endif

    PRINT_CSTSTR("%s","Sending ");
    PRINT_STR("%s",(char*)(message));
    PRINTLN;
    
    PRINT_CSTSTR("%s","Real payload size is ");
    PRINT_VALUE("%d", r_size);
    PRINTLN;   
  
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, message, r_size, 0);
        Serial.println(F("Packet queued"));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

/*****************************
 _____      _               
/  ___|    | |              
\ `--.  ___| |_ _   _ _ __  
 `--. \/ _ \ __| | | | '_ \ 
/\__/ /  __/ |_| |_| | |_) |
\____/ \___|\__|\__,_| .__/ 
                     | |    
                     |_|    
******************************/

void setup() {

    // initialization of the temperature sensor
    sensor_Init();

#ifdef LOW_POWER
#ifdef __SAMD21G18A__
    rtc.begin();
#endif  
#else
    digitalWrite(PIN_POWER,HIGH);
#endif
  
    delay(3000);
    // Open serial communications and wait for port to open:
#if defined __SAMD21G18A__ && not defined ARDUINO_SAMD_FEATHER_M0 
    SerialUSB.begin(38400);
#else
    Serial.begin(38400);  
#endif  
    // Print a start message
    PRINT_CSTSTR("%s","LoRa temperature sensor, extended version\n");

#ifdef ARDUINO_AVR_PRO
    PRINT_CSTSTR("%s","Arduino Pro Mini detected\n");  
#endif
#ifdef ARDUINO_AVR_NANO
    PRINT_CSTSTR("%s","Arduino Nano detected\n");   
#endif
#ifdef ARDUINO_AVR_MINI
    PRINT_CSTSTR("%s","Arduino MINI/Nexus detected\n");  
#endif
#ifdef ARDUINO_AVR_MEGA2560
    PRINT_CSTSTR("%s","Arduino Mega2560 detected\n");  
#endif
#ifdef ARDUINO_SAM_DUE
    PRINT_CSTSTR("%s","Arduino Due detected\n");  
#endif
#ifdef __MK66FX1M0__
    PRINT_CSTSTR("%s","Teensy36 MK66FX1M0 detected\n");
#endif
#ifdef __MK64FX512__
    PRINT_CSTSTR("%s","Teensy35 MK64FX512 detected\n");
#endif
#ifdef __MK20DX256__
    PRINT_CSTSTR("%s","Teensy31/32 MK20DX256 detected\n");
#endif
#ifdef __MKL26Z64__
    PRINT_CSTSTR("%s","TeensyLC MKL26Z64 detected\n");
#endif
#if defined ARDUINO_SAMD_ZERO && not defined ARDUINO_SAMD_FEATHER_M0
    PRINT_CSTSTR("%s","Arduino M0/Zero detected\n");
#endif
#ifdef ARDUINO_AVR_FEATHER32U4 
    PRINT_CSTSTR("%s","Adafruit Feather32U4 detected\n"); 
#endif
#ifdef  ARDUINO_SAMD_FEATHER_M0
    PRINT_CSTSTR("%s","Adafruit FeatherM0 detected\n");
#endif

// See http://www.nongnu.org/avr-libc/user-manual/using_tools.html
// for the list of define from the AVR compiler

#ifdef __AVR_ATmega328P__
    PRINT_CSTSTR("%s","ATmega328P detected\n");
#endif 
#ifdef __AVR_ATmega32U4__
    PRINT_CSTSTR("%s","ATmega32U4 detected\n");
#endif 
#ifdef __AVR_ATmega2560__
    PRINT_CSTSTR("%s","ATmega2560 detected\n");
#endif 
#ifdef __SAMD21G18A__ 
    PRINT_CSTSTR("%s","SAMD21G18A ARM Cortex-M0+ detected\n");
#endif
#ifdef __SAM3X8E__ 
    PRINT_CSTSTR("%s","SAM3X8E ARM Cortex-M3 detected\n");
#endif

#ifdef WITH_EEPROM
    // get config from EEPROM
    EEPROM.get(0, my_sx1272config);
  
    // found a valid config?
    if (my_sx1272config.flag1==0x12 && my_sx1272config.flag2==0x35) {
        PRINT_CSTSTR("%s","Get back previous sx1272 config\n");
    
        // set sequence number for SX1272 library
        // TODO use fcount
        PRINT_CSTSTR("%s","Using packet sequence number of ");
        PRINT_VALUE("%d", 0);
        PRINTLN;

#ifdef FORCE_DEFAULT_VALUE
        PRINT_CSTSTR("%s","Forced to use default parameters\n");
        my_sx1272config.flag1=0x12;
        my_sx1272config.flag2=0x35;
        my_sx1272config.seq=0;
        my_sx1272config.idle_period=TX_INTERVAL;    
        my_sx1272config.overwrite=0;
        EEPROM.put(0, my_sx1272config);
#else
        // get back the idle period
        if (my_sx1272config.idle_period!=0 && my_sx1272config.overwrite==1) {
          
            PRINT_CSTSTR("%s","Used stored idle period\n");
            idlePeriodInMin=my_sx1272config.idle_period;        
        }
        else
            PRINT_CSTSTR("%s","Stored idle period is null\n");                 
#endif    
    
        PRINT_CSTSTR("%s","Using idle period of ");
        PRINT_VALUE("%d", TX_INTERVAL);
        PRINTLN;     
    }
    else {
        // otherwise, write config and start over
        my_sx1272config.flag1=0x12;
        my_sx1272config.flag2=0x35;
        my_sx1272config.seq=0;
        my_sx1272config.idle_period=TX_INTERVAL;
        my_sx1272config.overwrite=0;
    }
#endif

    // LMIC init
    os_init();
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    // Set static session parameters. Instead of dynamically establishing a session
    // by joining the network, precomputed session parameters are be provided.
    #ifdef PROGMEM
    // On AVR, these values are stored in flash and only copied to RAM
    // once. Copy them to a temporary buffer here, LMIC_setSession will
    // copy them into a buffer of its own again.
    uint8_t appskey[sizeof(APPSKEY)];
    uint8_t nwkskey[sizeof(NWKSKEY)];
    memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
    memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
    LMIC_setSession (0x1, DEVADDR, nwkskey, appskey);
    #else
    // If not running an AVR with PROGMEM, just use the arrays directly
    LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);
    #endif

    #if defined(CFG_eu868)
    // Set up the channels used by the Things Network, which corresponds
    // to the defaults of most gateways. Without this, only three base
    // channels from the LoRaWAN specification are used, which certainly
    // works, so it is good for debugging, but can overload those
    // frequencies, so be sure to configure the full frequency range of
    // your network here (unless your network autoconfigures them).
    // Setting up channels should happen after LMIC_setSession, as that
    // configures the minimal channel set.
    // NA-US channels 0-71 are configured automatically
    
    //LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    //LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
    //LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    //LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    //LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    //LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    //LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    //LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    //LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band
    
    // TTN defines an additional channel at 869.525Mhz using SF9 for class B
    // devices' ping slots. LMIC does not have an easy way to define set this
    // frequency and support for class B is spotty and untested, so this
    // frequency is not configured here.
    #elif defined(CFG_us915)
    // NA-US channels 0-71 are configured automatically
    // but only one group of 8 should (a subband) should be active
    // TTN recommends the second sub band, 1 in a zero based count.
    // https://github.com/TheThingsNetwork/gateway-conf/blob/master/US-global_conf.json
    LMIC_selectSubBand(1);
    #endif
    
    // Disable link check validation
    LMIC_setLinkCheckMode(0);

    // TTN uses SF9 for its RX2 window.
    LMIC.dn2Dr = DR_SF9;

    //added by C. Pham
    //uncomment to only have the first frequency when sending to our low-cost gateway
    //Disable all channels, except for the 0 
    //FOR TESTING ONLY!
    
    for (int i=1; i<9; i++) { // For EU; for US use i<71
      if(i != 0) {
        LMIC_disableChannel(i);
      }
    }
    
    // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
    LMIC_setDrTxpow(DR_SF9,14);

    // Start job
    do_send(&sendjob);
}

/*****************************
 _                       
| |                      
| |     ___   ___  _ __  
| |    / _ \ / _ \| '_ \ 
| |___| (_) | (_) | |_) |
\_____/\___/ \___/| .__/ 
                  | |    
                  |_|    
*****************************/

void loop() {
    os_runloop_once();
}
