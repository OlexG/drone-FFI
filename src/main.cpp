#include <esp_now.h>
#include <WiFi.h>
#include <Arduino.h>
#include "crypto/ed25519.h"
#include <EEPROM.h>

#define TRANSISTOR_BASE_PIN 25
#define EEPROM_SIZE 100 // Define the size of EEPROM to be used

typedef struct struct_message {
    uint8_t data[80];  
} struct_message;

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
bool IS_ON = false;
bool lightOn = false;
int ID_TIMER = 20;
int lastRecievedOnMessage = -1000;
int RESET_TIMER = 100;
int SERIAL_TIMER = 500;
int FLASH_RATE = 3;
int currentTick = 0;
TaskHandle_t inf_loop_task;

const char* public_key_str = "4266f2f64bd0cf1b054005f8726ba4ae3f8a38eef8eea1ea1ca7d1c775cbf184";
const char* signature_str = "e637ccdcc9a7fda13ff422567447c259ae6eee618648591a960ce2e2296e33eba21871e93d85b28ae289bd97b60dd729d493915be808a3517faa6226e2ec230b";
const char* timestamp_str = "de117b66";

unsigned char public_key[32];
unsigned char signature[64];
unsigned char timestamp[4];

unsigned char recieved_public_key[32];
unsigned char recieved_signature[64];
unsigned char recieved_timestamp[4];

bool verify_timestamp(const unsigned char *signature, const unsigned char *timestamp, const unsigned char *public_key) {
    int result = ed25519_verify(signature, timestamp, 4, public_key);
    return result;
}

bool verify_public_key(const unsigned char *sent_public_key) {
    bool result = true;
    for (int i = 0; i < 32; i++) {
        if (sent_public_key[i] != public_key[i]) {
            result = false;
            break;
        }
    }
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

void send_id_msg() {
    struct_message responseMessage;
    memset(responseMessage.data, 0, sizeof(responseMessage.data));
    // Bytes 2 - 65 are the signature
    memcpy(responseMessage.data + 1, signature, 64);
    // Bytes 66 - 69 are the timestamp
    memcpy(responseMessage.data + 65, timestamp, 4);
    esp_now_send(broadcastAddress, (uint8_t *)&responseMessage, sizeof(responseMessage));
}

void send_confirm_msg() {
    struct_message responseMessage;
    memset(responseMessage.data, 1, sizeof(responseMessage.data)); 
    // Bytes 2 - 65 are the signature
    memcpy(responseMessage.data + 1, signature, 64);
    // Bytes 66 - 69 are the timestamp
    memcpy(responseMessage.data + 65, timestamp, 4);
    responseMessage.data[0] = 0x01;
    esp_now_send(broadcastAddress, (uint8_t *)&responseMessage, sizeof(responseMessage));
}

void onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    // Serial.println("Message received");
    // debug(incomingData, 40);
    // Bytes 2nd - 33rd are the public key
    memcpy(recieved_public_key, incomingData + 1, 32);
    if (!verify_public_key(recieved_public_key)) {
        return;
    }
    if (incomingData[0] == 0x1c) { // The interrogator is asking for the light to be turned on
        // Turn light on
        IS_ON = true;
        send_confirm_msg();
        lastRecievedOnMessage = currentTick;
    } else if (incomingData[0] == 0x2c) { // The interrogator is asking for the light to be maintained
        IS_ON = true;
        lastRecievedOnMessage = currentTick;
    } else if (incomingData[0] == 0x3c) { // The interrogator is asking for the light to be turned off
        IS_ON = false;
    }
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    // Serial.print("Send Status: ");
    // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void inf_loop(void * pvParameters) {
    for(;;) {
        delay(20);
        int ledPin = TRANSISTOR_BASE_PIN;
        if (IS_ON) {
            if (!lightOn && currentTick % FLASH_RATE == 0) {
                digitalWrite(ledPin, HIGH);
                lightOn = true;
            } else {
                digitalWrite(ledPin, LOW);
                lightOn = false;
            }
        } else if (lightOn) {
            digitalWrite(ledPin, LOW);
            lightOn = false;
        }
    }
}

void loop() {
    delay(20);
    int ledPin = TRANSISTOR_BASE_PIN;
    if (currentTick - lastRecievedOnMessage > RESET_TIMER) {
        IS_ON = false;
        lightOn = false;
        digitalWrite(ledPin, LOW);
    }

    if (currentTick % ID_TIMER == 0) {
        send_id_msg();
    }
    currentTick++;
    if (currentTick > 2147483640) {
        currentTick = 0;
    }
    /*if (currentTick % SERIAL_TIMER == 0 && Serial.available() >= 100) {
        uint8_t buffer[100];
        // Read 100 bytes from the serial port
        int bytesRead = Serial.readBytes(buffer, 100);
        if (bytesRead < 100) {
            return;
        }

        // Write to EEPROM
        for (int i = 0; i < 100; i++) {
            EEPROM.write(i, buffer[i]);
            EEPROM.commit();
        }
        EEPROM.commit();

        // Write back the buffer to the serial port
        Serial.write(buffer, bytesRead);
    }*/
}

void setup() {
    hexStringToByteArray(public_key_str, public_key, 32);
    hexStringToByteArray(signature_str, signature, 64);
    hexStringToByteArray(timestamp_str, timestamp, 4);

    Serial.begin(9600);
    WiFi.mode(WIFI_STA); 

    // Initialize the pin as an output
    pinMode(TRANSISTOR_BASE_PIN, OUTPUT);

    // Initially set the pin to low (off)
    digitalWrite(TRANSISTOR_BASE_PIN, LOW);

    if (esp_now_init() != ESP_OK) {
        //Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Initialize EEPROM
    /*EEPROM.begin(EEPROM_SIZE);

    // Read data from EEPROM
    for (int i = 0; i < 32; i++) {
        public_key[i] = EEPROM.read(i);
    }
    for (int i = 0; i < 4; i++) {
        timestamp[i] = EEPROM.read(32 + i);
    }
    for (int i = 0; i < 64; i++) {
        signature[i] = EEPROM.read(36 + i);
    }*/

    // Debugging read values
    Serial.println("Public key:");
    debug(public_key, 32);
    Serial.println("Timestamp:");
    debug(timestamp, 4);
    Serial.println("Signature:");
    debug(signature, 64);

    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(onDataRecv);

    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(peerInfo));
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);  
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    esp_now_add_peer(&peerInfo);

    xTaskCreatePinnedToCore(
                    inf_loop,   /* Task function. */
                    "inf_loop",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &inf_loop_task,      /* Task handle to keep track of created task */
                    1);
}
