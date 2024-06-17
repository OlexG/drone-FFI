#include <esp_now.h>
#include <WiFi.h>
#include <Arduino.h>

const uint8_t ledPin = 4; 
const uint8_t buttonPin = 2; 
uint8_t buttonState = 0;  
bool virgin = true;
bool virgin2 = true;

typedef struct struct_message {
    uint8_t data[40];  
} struct_message;

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

bool isMessageFilledWith01(const struct_message& message) {
    for (int i = 0; i < sizeof(message.data); i++) {
        if (message.data[i] != 0x01) {
            return false; 
        }
    }
    Serial.print("Recived struct full of 0x01");
    return true; 
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

void onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    Serial.println("Message received");
    debug(incomingData, 40);  
    
    struct_message msg_rcv;
    memcpy(msg_rcv.data, incomingData, sizeof(msg_rcv.data));

    if(virgin2) {
        struct_message responseMessage;
        memset(responseMessage.data, 0, sizeof(responseMessage.data)); 
        esp_now_send(broadcastAddress, (uint8_t *)&responseMessage, sizeof(responseMessage)); 
        virgin2 = false;
        Serial.println("Sending first zeros");
    }
    if(isMessageFilledWith01(msg_rcv)){
        flickerLED(5, 4, 100, true);
    }
}

void send_id_msg(){
    struct_message responseMessage;
    memset(responseMessage.data, 1, sizeof(responseMessage.data)); 
    esp_now_send(broadcastAddress, (uint8_t *)&responseMessage, sizeof(responseMessage));

}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("Send Status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void setup() {
    Serial.begin(9600);
    WiFi.mode(WIFI_STA); 

    pinMode(ledPin, OUTPUT);
    pinMode(buttonPin, INPUT_PULLUP);

    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
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
    buttonState = digitalRead(buttonPin);
    if (buttonState == LOW) {
        Serial.print("Sending id. message");
        send_id_msg();
        delay(500);
    }
}
