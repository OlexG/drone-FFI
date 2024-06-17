#include <esp_now.h>
#include <WiFi.h>
#include <Arduino.h>

#define BUZZER_PIN 12

#define LED1_PIN 33 // green
#define LED2_PIN 32 // red

#define BUTTON_PIN 14

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
            Serial.print("0");
        }
        Serial.print(keyArray[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

void send_id_msg(){
    struct_message responseMessage;
    memset(responseMessage.data, 1, sizeof(responseMessage.data)); 
    esp_now_send(broadcastAddress, (uint8_t *)&responseMessage, sizeof(responseMessage));

}

void onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    Serial.println("Message received");
    debug(incomingData, 40);
    int buttonState = digitalRead(BUTTON_PIN);
    if(buttonState == LOW) {
        send_id_msg();
        play_sound(BUZZER_PIN);
    }
    flickerLED(2, LED1_PIN, 100, false);
}


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("Send Status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void setup() {
    Serial.begin(9600);
    WiFi.mode(WIFI_STA); 

    pinMode(LED1_PIN, OUTPUT); // green light    
    pinMode(LED2_PIN, OUTPUT); // red light
    
    pinMode(BUTTON_PIN, INPUT_PULLUP); // button

    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    flickerLED(8, LED2_PIN, 100, true);

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
    delay(200);
    
}
