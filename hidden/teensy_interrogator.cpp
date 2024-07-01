#include <Arduino.h>
#include "crypto/ed25519.h"
#include <SPI.h>
#include <RH_RF95.h>

// Define pins for LoRa module
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2

// Define frequency
#define RF95_FREQ 915.0
#define BUZZER_PIN 12
#define LED1_PIN 33 // green
#define LED2_PIN 32 // red
#define BUTTON_PIN 14


// Create an instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

typedef struct struct_message {
    uint8_t data[80];  
} struct_message;


bool IS_PRESSED = false;
bool IS_CONNECTED = false;
bool IS_CONNTECTED_CONFIRMATION_LIGHT_ON = false;
// Every 100 ticks, we will "reset" everything
// This solves most issues with "multiple" devices I think
int RESET_TIMER = 100;
int lastRecievedIdMessage = -100;
int lastRecievedConfirmationMessage = -1000;
int CONFIRMATION_TIMER = 100;
int ON_MESSAGE_TIMER = 20;
int SERIAL_TIMER = 500;
const char* public_key_str = "4266f2f64bd0cf1b054005f8726ba4ae3f8a38eef8eea1ea1ca7d1c775cbf184";
const char* signature_str = "e637ccdcc9a7fda13ff422567447c259ae6eee618648591a960ce2e2296e33eba21871e93d85b28ae289bd97b60dd729d493915be808a3517faa6226e2ec230b";
const char* timestamp_str = "de117b66";
unsigned char public_key[32];
unsigned char signature[64];
unsigned char timestamp[4];
unsigned char recieved_public_key[32];
unsigned char recieved_signature[64];
unsigned char recieved_timestamp[4];

int currentTick = 0;

/* UTIL FUNCTIONS */

bool verify_timestamp(const unsigned char *n_signature, const unsigned char *n_timestamp, const unsigned char *public_key) {
    int result = ed25519_verify(n_signature, n_timestamp, 4, public_key);
    return result;
}

void hexStringToByteArray(const char* hexString, unsigned char* byteArray, size_t byteArraySize) {
    for (size_t i = 0; i < byteArraySize; i++) {
        sscanf(hexString + 2*i, "%2hhx", &byteArray[i]);
    }
}

void play_tone(int tonePin, int frequency, int duration) {
    tone(tonePin, frequency, duration);
    delay(duration + 50);
}

void play_sound(const uint8_t pin) {
    play_tone(pin, 500, 200);
    play_tone(pin, 1000, 200);
    play_tone(pin, 1500, 200);
}

void flickerLED(int numFlickers, int ledPin, int delayValue, bool endingState) {
    for (int i = 0; i < numFlickers; i++) {
        digitalWrite(ledPin, HIGH);
        delay(delayValue);
        digitalWrite(ledPin, LOW);
        delay(delayValue);
    }
    digitalWrite(ledPin, endingState ? HIGH : LOW);
}

void debug(const uint8_t *keyArray, int keySize) {
    for (int i = 0; i < keySize; i++) {
        if (keyArray[i] < 16) {
            Serial.print("0");
        }
        Serial.print(keyArray[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

/* END OF UTIL FUNCTIONS */
void on_data_recieve(uint8_t *incomingData) {
    // Serial.println("Message received, means the transponder is close");
    // Bits 2 - 64 are the signature
    // Bits 65 - 69 are the timestamp
    memcpy(recieved_signature, incomingData + 1, 64);
    memcpy(recieved_timestamp, incomingData + 65, 4);
    if (!verify_timestamp(recieved_signature, recieved_timestamp, public_key)) {
        Serial.println("Timestamp not verified");
        return;
    }

    if (incomingData[0] == 0x01) { // If the transponder sends us a confirmation message of the light being on
        if (currentTick - lastRecievedConfirmationMessage > CONFIRMATION_TIMER) {
            lastRecievedConfirmationMessage = currentTick;
            // flickerLED(2, LED1_PIN, 100, false);
            play_sound(BUZZER_PIN);
        }
    } else { // If the transponder sends us a message of just being "connected"
        // flickerLED(2, LED1_PIN, 100, false);
        IS_CONNECTED = true;
        lastRecievedIdMessage = currentTick;
    }
}

// Message for turning on the light
void send_on_msg() {
    struct_message responseMessage;
    memset(responseMessage.data, 0, sizeof(responseMessage.data)); 
    responseMessage.data[0] = 0x1c; // This represents keeping the light on
    // Include public key in the bits 2nd - 33rd
    memcpy(responseMessage.data + 1, public_key, 32);
    rf95.send(responseMessage.data, sizeof(responseMessage.data));
}

// Message for keeping the light going
void send_maintain_msg() {
    struct_message responseMessage;
    memset(responseMessage.data, 0, sizeof(responseMessage.data)); 
    responseMessage.data[0] = 0x2c; // This represents keeping the light going
    // Include public key in the bits 2nd - 33rd
    memcpy(responseMessage.data + 1, public_key, 32);
    rf95.send(responseMessage.data, sizeof(responseMessage.data));
}

// Message for turning off the light
void send_off_msg() {
    struct_message responseMessage;
    memset(responseMessage.data, 1, sizeof(responseMessage.data)); 
    responseMessage.data[0] = 0x3c; // This represents the off message
    // Include public key in the bits 2nd - 33rd
    memcpy(responseMessage.data + 1, public_key, 32);
    rf95.send(responseMessage.data, sizeof(responseMessage.data));
}


void setup() {
    hexStringToByteArray(public_key_str, public_key, 32);
    hexStringToByteArray(signature_str, signature, 64);
    hexStringToByteArray(timestamp_str, timestamp, 4);

    Serial.begin(9600);
    while (!Serial) {
        ;
    }

    Serial.println("RFM95 LoRa Interrogator");

    // Initialize LoRa module pins
    pinMode(RFM95_RST, OUTPUT);
    digitalWrite(RFM95_RST, HIGH);
    delay(10);
    digitalWrite(RFM95_RST, LOW);
    delay(10);
    digitalWrite(RFM95_RST, HIGH);
    delay(10);

    // Initialize the radio
    if (!rf95.init()) {
        Serial.println("LoRa init failed. Check your connections.");
        while (1);
    }
    Serial.println("LoRa init succeeded.");

    // Set the frequency
    if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
    }
    Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

    pinMode(LED1_PIN, OUTPUT); // green light    
    pinMode(LED2_PIN, OUTPUT); // red light
    pinMode(BUTTON_PIN, INPUT_PULLUP); // button

    flickerLED(8, LED2_PIN, 100, true);
}

void loop() {
    int buttonState = digitalRead(BUTTON_PIN);
    delay(20);

    /* Button logic, turn the light on when clicked and every other x ticks */
    if(buttonState == LOW && !IS_PRESSED) {
        Serial.println("Turning on the light");
        send_on_msg();
        IS_PRESSED = true;
    }
    if (buttonState == LOW && currentTick % ON_MESSAGE_TIMER == 0) {
        Serial.println("Keeping the light going");
        send_maintain_msg();
        IS_PRESSED = true;
    }

    if (buttonState != LOW && IS_PRESSED) {
        Serial.println("Turning off the light");
        send_off_msg();
        IS_PRESSED = false;
    }

    if (currentTick % SERIAL_TIMER == 0 && Serial.available() >= 100) {
        uint8_t buffer[100];
        // Read 100 bytes from the serial port
        int bytesRead = Serial.readBytes(buffer, 100);
        if (bytesRead < 100) {
            return;
        }
        uint8_t recieved_public_key[32];
        uint8_t recieved_timestamp[4];
        uint8_t recieved_signature[64];
        memcpy(recieved_public_key, buffer, 32);
        memcpy(recieved_timestamp, buffer + 32, 4);
        memcpy(recieved_signature, buffer + 36, 64);
        // Write back the buffer to the serial port
        Serial.write(buffer, bytesRead);
    }

    /* Connection logic */
    if (currentTick - lastRecievedIdMessage > RESET_TIMER) {
        IS_CONNTECTED_CONFIRMATION_LIGHT_ON = false;
        IS_CONNECTED = false;
        digitalWrite(LED1_PIN, LOW);
    }
    if (IS_CONNECTED && !IS_CONNTECTED_CONFIRMATION_LIGHT_ON) {
        digitalWrite(LED1_PIN, HIGH);
        IS_CONNTECTED_CONFIRMATION_LIGHT_ON = true;
    }

    if (rf95.available()) {
        // Receive the message
        uint8_t buf[80];
        uint8_t len = sizeof(buf);

        if (rf95.recv(buf, &len)) {
            on_data_recieve(buf);
        }
    }
    currentTick++;
}
