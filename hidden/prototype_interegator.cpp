#include <esp_now.h>
#include <WiFi.h>
#include <Arduino.h>
/*
Changes
Should accept "confirmation" only within a certain time period
*/
#define BUZZER_PIN 12

#define LED1_PIN 33 // green
#define LED2_PIN 32 // red

#define BUTTON_PIN 14

typedef struct struct_message {
    uint8_t data[40];  
} struct_message;

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
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

int currentTick = 0;

/* UTIL FUNCTIONS */
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

/* END OF UTIL FUNCTIONS */
void onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    // Serial.println("Message received, means the transponder is close");
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
void send_on_msg(){
    struct_message responseMessage;
    memset(responseMessage.data, 0, sizeof(responseMessage.data)); 
    responseMessage.data[0] = 0x1c; // This represents keeping the light on
    esp_now_send(broadcastAddress, (uint8_t *)&responseMessage, sizeof(responseMessage));
}

// Message for keeping the light going
void send_maintain_msg(){
    struct_message responseMessage;
    memset(responseMessage.data, 0, sizeof(responseMessage.data)); 
    responseMessage.data[0] = 0x2c; // This represents keeping the light going
    esp_now_send(broadcastAddress, (uint8_t *)&responseMessage, sizeof(responseMessage));
}

// Message for turning off the light
void send_off_msg(){
    struct_message responseMessage;
    memset(responseMessage.data, 1, sizeof(responseMessage.data)); 
    responseMessage.data[0] = 0x3c; // This represents the off message
    esp_now_send(broadcastAddress, (uint8_t *)&responseMessage, sizeof(responseMessage));
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
    int buttonState = digitalRead(BUTTON_PIN);
    delay(20);
    /* Button logic, turn the light on when clicked and every other x ticks*/
    if(buttonState == LOW && !IS_PRESSED) {
        Serial.print("Turning on the light\n");
        send_on_msg();
        IS_PRESSED = true;
    }
    if (buttonState == LOW && currentTick % ON_MESSAGE_TIMER == 0) {
        Serial.print("Keeping the light going\n");
        send_maintain_msg();
        IS_PRESSED = true;
    }


    if (buttonState != LOW && IS_PRESSED) {
        Serial.print("Turning off the light\n");
        send_off_msg();
        IS_PRESSED = false;
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
    currentTick++;
}
