/*****************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 16/03/20

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/* 
 *  Adapted by Congduc Pham, Nov. 2020
 *  
 *  When using the Arduino Serial Monitor, you can indicate the real distance if you know it
 *  in order to determine the distance_ajustment factor. Simply enter 10# if the real
 *  distance is 10m. You can do this procedure several time.
 *
 */
 
#define Program_Version "V1.0"

#include <SPI.h>
//this is the standard behaviour of library, use SPI Transaction switching
#define USE_SPI_TRANSACTION

#include <SX128XLT.h>
#include "Settings.h"

SX128XLT LT;

///////////////////////////////////////////////////////////////////
//new small OLED screen, mostly based on SSD1306 
#define OLED
#define OLED_9GND876
//#define OLED_6GND789

#ifdef OLED
#include <U8x8lib.h>
//you can also power the OLED screen with a digital pin, here pin 8
#define OLED_PWR_PIN 8
// connection may depend on the board. Use A5/A4 for most Arduino boards. On ESP8266-based board we use GPI05 and GPI04. Heltec ESP32 has embedded OLED.
#if defined ARDUINO_Heltec_WIFI_LoRa_32 || defined ARDUINO_WIFI_LoRa_32 || defined HELTEC_LORA
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);
#elif defined ESP8266 || defined ARDUINO_ESP8266_ESP01
//U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 5, /* data=*/ 4, /* reset=*/ U8X8_PIN_NONE);
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 12, /* data=*/ 14, /* reset=*/ U8X8_PIN_NONE);
#else
#ifdef OLED_GND234
  #ifdef OLED_PWR_PIN
    #undef OLED_PWR_PIN
    #define OLED_PWR_PIN 2
  #endif
  U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 3, /* data=*/ 4, /* reset=*/ U8X8_PIN_NONE);
#elif defined OLED_9GND876
  #ifdef OLED_PWR_PIN
    #undef OLED_PWR_PIN
    #define OLED_PWR_PIN 8
  #endif  
  U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 7, /* data=*/ 6, /* reset=*/ U8X8_PIN_NONE);
#elif defined OLED_6GND789
  #ifdef OLED_PWR_PIN
    #undef OLED_PWR_PIN
    #define OLED_PWR_PIN 7
  #endif  
  U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 8, /* data=*/ 9, /* reset=*/ U8X8_PIN_NONE);  
#elif defined OLED_7GND654
  #ifdef OLED_PWR_PIN
    #undef OLED_PWR_PIN
    #define OLED_PWR_PIN 6
  #endif  
  U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 5, /* data=*/ 4, /* reset=*/ U8X8_PIN_NONE);  
#elif defined OLED_GND13_12_11
  #ifdef OLED_PWR_PIN
    #undef OLED_PWR_PIN
    #define OLED_PWR_PIN 13
  #endif  
  U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 12, /* data=*/ 11, /* reset=*/ U8X8_PIN_NONE); 
#else
  U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ A5, /* data=*/ A4, /* reset=*/ U8X8_PIN_NONE);
#endif
#endif
char oled_msg[20];
#endif

uint16_t rangeing_errors, rangeings_valid, rangeing_results;
uint16_t IrqStatus;
uint32_t endwaitmS, startrangingmS, range_result_sum, range_result_average;
float raw_distance, distance, raw_distance_sum, distance_sum, raw_distance_average, distance_average;
bool ranging_error;
int32_t range_result;

//input command buffer
char cmd[20]="**********";

long getCmdValue(int &i, char* strBuff=NULL) {
        
        char seqStr[10]="******";

        int j=0;
        // character '#' will indicate end of cmd value
        while ((char)cmd[i]!='#' && (i < strlen(cmd)) && j<strlen(seqStr)) {
                seqStr[j]=(char)cmd[i];
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

void setup()
{
  Serial.begin(38400);
  Serial.println();
  Serial.print(F(__TIME__));
  Serial.print(F(" "));
  Serial.println(F(__DATE__));
  Serial.println(F(Program_Version));
  Serial.println();
  Serial.println(F("54_Ranging_Master Starting"));

#ifdef OLED
#ifdef OLED_PWR_PIN
  pinMode(OLED_PWR_PIN, OUTPUT);
  digitalWrite(OLED_PWR_PIN, HIGH);
#ifdef OLED_9GND876
  //use pin 9 as ground
  pinMode(9, OUTPUT);
  digitalWrite(9, LOW);
#elif defined OLED_6GND789
  //use pin 6 as ground
  pinMode(6, OUTPUT);
  digitalWrite(6, LOW);  
#elif defined OLED_7GND654
  //use pin 7 as ground
  pinMode(7, OUTPUT);
  digitalWrite(7, LOW);
#endif
#endif
#endif

  SPI.begin();

  if (LT.begin(NSS, NRESET, RFBUSY, DIO1, LORA_DEVICE))
  {
    Serial.println(F("Device found"));
    delay(1000);
  }
  else
  {
    Serial.println(F("No device responding"));
    while (1)
    {
    }
  }

  LT.setupRanging(Frequency, Offset, SpreadingFactor, Bandwidth, CodeRate, RangingAddress, RANGING_MASTER);

  //LT.setRangingCalibration(Calibration);               //override automatic lookup of calibration value from library table

#ifdef OLED
  u8x8.begin();
  //u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.setFont(u8x8_font_pxplustandynewtv_r);
  u8x8.drawString(0, 0, "Ranging master");
  sprintf(oled_msg,"SF%dBW%d", LT.getLoRaSF(), LT.returnBandwidth()/1000);
  u8x8.drawString(0, 1, oled_msg);
  sprintf(oled_msg,"Cal %d", LT.getSetCalibrationValue());
  u8x8.drawString(0, 2, oled_msg);
  sprintf(oled_msg,"Adjust %d", (uint8_t)distance_adjustment);
  u8x8.drawString(0, 3, oled_msg);    
#endif   

  Serial.print(F("Address "));
  Serial.println(RangingAddress);
  Serial.print(F("CalibrationValue "));
  Serial.println(LT.getSetCalibrationValue());
  Serial.println(F("Ranging master RAW ready"));

  delay(2000);
}

void loop()
{
  uint8_t index;
  int i;
  raw_distance_sum = 0;
  distance_sum = 0;
  range_result_sum = 0;
  rangeing_results = 0;                           //count of valid results in each loop

 // check if we received data from the input serial port
  if (Serial.available()) {

    i=0;  

    while (Serial.available() && i<80) {
      cmd[i]=Serial.read();
      i++;
      delay(50);
    }
    
    cmd[i]='\0';

    Serial.print("Rcv serial: ");
    Serial.println(cmd);

    i=0;
    
    uint16_t real_distance=getCmdValue(i);
    
    Serial.print("Take real distance: ");
    Serial.println(real_distance);  

    distance_adjustment=(float)real_distance/raw_distance_average;

    Serial.print("Set distance adjustment to: ");
    Serial.println(distance_adjustment);  
  }
  
  for (index = 1; index <= rangeingcount; index++)
  {

    startrangingmS = millis();

    LT.transmitRanging(RangingAddress, TXtimeoutmS, RangingTXPower, NO_WAIT);

    endwaitmS = millis() + TXtimeoutmS;
  
    do {
      IrqStatus = LT.readIrqStatus();
      delay(1);
    } while ( !(IrqStatus & IRQ_TX_DONE) && !(IrqStatus & IRQ_RANGING_MASTER_RESULT_VALID) && !(IrqStatus & IRQ_RANGING_MASTER_RESULT_TIMEOUT) && (millis() <= endwaitmS));

    if (IrqStatus & IRQ_RANGING_MASTER_RESULT_VALID)
    {
      rangeing_results++;
      rangeings_valid++;
      Serial.print(F("Valid"));
      range_result = LT.getRangingResultRegValue(RANGING_RESULT_RAW);
      Serial.print(F(",Register,"));
      Serial.print(range_result);

      if (range_result > 800000)
      {
        range_result = 0;
      }
      range_result_sum = range_result_sum + range_result;

      raw_distance = LT.getRangingDistance(RANGING_RESULT_RAW, range_result, 1.0);
      distance = LT.getRangingDistance(RANGING_RESULT_RAW, range_result, distance_adjustment);
      distance_sum = distance_sum + distance;
      raw_distance_sum = raw_distance_sum + raw_distance;
      
      Serial.print(F(",Distance,"));
      Serial.print(distance, 1);
    }
    else
    {
      rangeing_errors++;
      distance = 0;
      range_result = 0;
      Serial.print(F("NotValid"));
      Serial.print(F(",Irq,"));
      Serial.print(IrqStatus, HEX);
    }
    delay(packet_delaymS);

    if (index == rangeingcount)
    {
      range_result_average = (range_result_sum / rangeing_results);

      if (rangeing_results == 0)
      {
        distance_average = 0;
        raw_distance_average = 0;
      }
      else
      {
        distance_average = (distance_sum / rangeing_results);
        raw_distance_average = (raw_distance_sum / rangeing_results);
      }
      
      Serial.print(F(",TotalValid,"));
      Serial.print(rangeings_valid);
      Serial.print(F(",TotalErrors,"));
      Serial.print(rangeing_errors);
      Serial.print(F(",AverageRAWResult,"));
      Serial.print(range_result_average);
      Serial.print(F(",AverageRawDistance,"));
      Serial.print(raw_distance_average, 1);      
      Serial.print(F(",AverageDistance,"));
      Serial.print(distance_average, 1);

#ifdef OLED
  u8x8.clearLine(4);
  u8x8.clearLine(5);
  sprintf(oled_msg,"Distance %dm", (uint16_t)distance_average);
  u8x8.drawString(0, 4, oled_msg); 
  sprintf(oled_msg,"OK %d Err %d", rangeings_valid, rangeing_errors);
  u8x8.drawString(0, 5, oled_msg);   
#endif 
      delay(2000);
    }
    Serial.println();
  }
}
