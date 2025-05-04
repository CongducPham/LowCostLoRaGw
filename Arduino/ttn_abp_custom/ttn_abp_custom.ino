/*******************************************************************************
 * Modified LoRaWAN code for interactive serial input
 * This code sends data to TTN only when text is entered in the Serial Monitor
 *******************************************************************************/

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

// LoRaWAN NwkSKey, network session key
static const PROGMEM u1_t NWKSKEY[16] = { 0x55, 0xF7, 0x14, 0xA6, 0x58, 0x11, 0xDE, 0xAF, 0xF4, 0xAC, 0x6B, 0x65, 0x3F, 0xA5, 0xAB, 0xAD };

// LoRaWAN AppSKey, application session key
static const u1_t PROGMEM APPSKEY[16] = { 0xBC, 0xA5, 0x0B, 0x27, 0x85, 0x31, 0xF0, 0x28, 0xBE, 0x75, 0x9A, 0x66, 0x84, 0x15, 0x76, 0xED };

// LoRaWAN end-device address (DevAddr)
static const u4_t DEVADDR = 0x260BE0A2;

void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

// Buffer for incoming Serial data
#define MAX_MSG_LEN 64
char message[MAX_MSG_LEN] = "";
int msgIndex = 0;
bool newMessage = false;
bool transmissionComplete = true; // Flag to track if previous transmission is complete

static osjob_t sendjob;

// Pin mapping
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
            if(LMIC.dataLen) {
                // data received in rx slot after tx
                Serial.print(F("Data Received: "));
                Serial.write(LMIC.frame+LMIC.dataBeg, LMIC.dataLen);
                Serial.println();
            }
            // Set flag indicating transmission is complete
            transmissionComplete = true;
            Serial.println(F("Ready for next message. Type and press Enter."));
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

void do_send(osjob_t* j) {
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
        transmissionComplete = false;
    } else {
        // Convert char array to byte array for sending
        uint8_t payload[MAX_MSG_LEN];
        int len = strlen(message);
        for (int i = 0; i < len; i++) {
            payload[i] = message[i];
        }
        
        // Prepare upstream data transmission at the next possible time
        LMIC_setTxData2(1, payload, len, 0);
        Serial.println(F("Packet queued"));
        
        // Clear the message buffer
        memset(message, 0, MAX_MSG_LEN);
        msgIndex = 0;
        newMessage = false;
        transmissionComplete = false;
    }
    // Next TX is NOT scheduled automatically - will only happen when new data is entered
}

void setup() {
    Serial.begin(38400);
    Serial.println(F("LoRaWAN Interactive Sender"));
    Serial.println(F("Type your message and press Enter to send it to TTN"));
    
    #ifdef VCC_ENABLE
    // For Pinoccio Scout boards
    pinMode(VCC_ENABLE, OUTPUT);
    digitalWrite(VCC_ENABLE, HIGH);
    delay(1000);
    #endif

    // LMIC init
    os_init();
    // Reset the MAC state. Session and pending data transfers will be discarded.
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

    // Set up the channels used by the Things Network, which corresponds
    // to the defaults of most gateways.
    LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
    //LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);
    //LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
    //LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
    //LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
    //LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
    //LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
    //LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
    //LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);

    // Disable link check validation
    LMIC_setLinkCheckMode(0);

    // Set data rate and transmit power (note: txpow seems to be ignored by the library)
    LMIC_setDrTxpow(DR_SF12,14);
    
    // We don't send anything yet - waiting for user input
    transmissionComplete = true;
    Serial.println(F("Ready for input. Type your message and press Enter."));
}

void loop() {
    // Check if there's data available in the Serial buffer
    while (Serial.available() > 0 && newMessage == false) {
        char inChar = (char)Serial.read();
        
        // Process the incoming character
        if (inChar == '\n' || inChar == '\r') {
            // End of message
            if (msgIndex > 0) {  // Only if we have actual content
                message[msgIndex] = '\0';  // Null-terminate the string
                newMessage = true;
                Serial.print(F("Message received: "));
                Serial.println(message);
            }
        } else if (msgIndex < MAX_MSG_LEN - 1) {
            // Add character to buffer if there's space
            message[msgIndex++] = inChar;
        }
    }
    
    // If we have a new message and previous transmission is complete, send it
    if (newMessage && transmissionComplete) {
        Serial.println(F("Sending message..."));
        do_send(&sendjob);
    }
    
    // Run LMIC loop
    os_runloop_once();
}