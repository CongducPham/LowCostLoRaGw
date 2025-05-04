#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// OneWire DS18B20 Temperature Sensor Setup
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// LoRaWAN Network Session Key
static const PROGMEM u1_t NWKSKEY[16] = { 0x55, 0xF7, 0x14, 0xA6, 0x58, 0x11, 0xDE, 0xAF, 0xF4, 0xAC, 0x6B, 0x65, 0x3F, 0xA5, 0xAB, 0xAD };

// LoRaWAN Application Session Key
static const u1_t PROGMEM APPSKEY[16] = { 0xBC, 0xA5, 0x0B, 0x27, 0x85, 0x31, 0xF0, 0x28, 0xBE, 0x75, 0x9A, 0x66, 0x84, 0x15, 0x76, 0xED };

// LoRaWAN end-device address (DevAddr)
static const u4_t DEVADDR = 0x260BE0A2; // Change this address for every node!

// Callbacks for over-the-air activation (left empty)
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

// Temperature data buffer
static uint8_t mydata[4];
static osjob_t sendjob;

// Transmission interval
const unsigned TX_INTERVAL = 30;

// Pin mapping for LoRa shield
const lmic_pinmap lmic_pins = {
    .nss = 10,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 9,
    .dio = {2, 6, 7},
};

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if(LMIC.dataLen) {
                // data received in rx slot after tx
                Serial.print(F("Data Received: "));
                Serial.write(LMIC.frame+LMIC.dataBeg, LMIC.dataLen);
                Serial.println();
            }
            // Schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        default:
            Serial.println(F("Other event"));
            break;
    }
}

void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        // Get temperature reading
        sensors.requestTemperatures(); 
        float tempC = sensors.getTempCByIndex(0);
        
        // Convert temperature to 2 bytes (16-bit integer)
        int16_t tempInt = tempC * 100; // Multiply by 100 to preserve 2 decimal places
        mydata[0] = highByte(tempInt);
        mydata[1] = lowByte(tempInt);
        
        // Prepare upstream data transmission
        LMIC_setTxData2(1, mydata, sizeof(mydata), 0);
        Serial.print(F("Sending temperature: "));
        Serial.print(tempC);
        Serial.println(F(" °C"));
    }
}

void setup() {
    Serial.begin(38400);
    Serial.println(F("Starting Temperature Sensor & TTN"));

    // Initialize temperature sensor
    sensors.begin();

    // LMIC init
    os_init();
    // Reset the MAC state
    LMIC_reset();

    // Set static session parameters
    #ifdef PROGMEM
    uint8_t appskey[sizeof(APPSKEY)];
    uint8_t nwkskey[sizeof(NWKSKEY)];
    memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
    memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
    LMIC_setSession (0x1, DEVADDR, nwkskey, appskey);
    #else
    LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);
    #endif

    // Configure LoRaWAN channels
    LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);

    // Disable link check validation
    LMIC_setLinkCheckMode(0);

    // Set data rate and transmit power
    LMIC_setDrTxpow(DR_SF12,14);

    // Trigger first send job
    do_send(&sendjob);
}

void loop() {
    os_runloop_once();
}
