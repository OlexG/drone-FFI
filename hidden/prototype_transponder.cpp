#include <esp_now.h>
#include <WiFi.h>
#include <Arduino.h>

#define TRANSISTOR_BASE_PIN 25

typedef struct struct_message {
    uint8_t data[40];  
} struct_message;

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void play_tone(int tonePin, int frequency, int duration)
{
    tone(tonePin, frequency, duration);
    delay(duration + 50);
}

void play_sound(const uint8_t pin)
{
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
            //Serial.print("0");
        }
        //Serial.print(keyArray[i], HEX);
        //Serial.print(" ");
    }
    //Serial.println();
}

void onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    //Serial.println("Message received");
    debug(incomingData, 40);
    flickerLED(6, TRANSISTOR_BASE_PIN, 200, false);
}


void send_id_msg(){
    struct_message responseMessage;
    memset(responseMessage.data, 1, sizeof(responseMessage.data)); 
    esp_now_send(broadcastAddress, (uint8_t *)&responseMessage, sizeof(responseMessage));

}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    //Serial.print("Send Status: ");
    //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void setup() {
    //Serial.begin(9600);
    WiFi.mode(WIFI_STA); 

    // Initialize the pin as an output
    pinMode(TRANSISTOR_BASE_PIN, OUTPUT);

    // Initially set the pin to low (off)
    digitalWrite(TRANSISTOR_BASE_PIN, LOW);

    if (esp_now_init() != ESP_OK) {
        //Serial.println("Error initializing ESP-NOW");
        return;
    }
 

    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(onDataRecv);

    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(peerInfo));
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);  
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    esp_now_add_peer(&peerInfo);
}

void loop() {
    delay(1000);
    send_id_msg();
}
