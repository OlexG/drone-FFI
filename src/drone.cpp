
#include <Arduino.h>
#include <utils.h>
#include <uECC.h>
#include <WiFi.h>
#include <esp_now.h>

void debug(byte *keyArray, int keySize)
{
    for (int i = 0; i < keySize; i++)
    {
        if (keyArray[i] < 16)
        { // For proper formatting of values less than 16
            Serial.print("0");
        }
        Serial.print(keyArray[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

static int RNG(uint8_t *dest, unsigned size)
{
    // Use the least-significant bits from the ADC for an unconnected pin (or connected to a source of
    // random noise). This can take a long time to generate random data if the result of analogRead(0)
    // doesn't change very frequently.
    while (size)
    {
        uint8_t val = 0;
        for (unsigned i = 0; i < 8; ++i)
        {
            int init = analogRead(0);
            int count = 0;
            while (analogRead(0) == init)
            {
                ++count;
            }

            if (count == 0)
            {
                val = (val << 1) | (init & 0x01);
            }
            else
            {
                val = (val << 1) | (count & 0x01);
            }
        }
        *dest = val;
        ++dest;
        --size;
    }
    // NOTE: it would be a good idea to hash the resulting random data using SHA-256 or similar.
    return 1;
}

uint8_t privateKey[21];
uint8_t publicKey[40];
bool stickRecievedKey = false; // false = unpaired with drone, true = paired with drone

typedef struct struct_message
{
    uint8_t data[40]; // Array to hold 40 bytes of data
} struct_message;

struct_message keyPayload;
esp_now_peer_info_t peerInfo;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    Serial.print("Send Status: ");
    if (status == ESP_NOW_SEND_SUCCESS)
    {
        Serial.println("Success");
    }
    else
    {
        Serial.println("Fail");
    }
}

void onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    struct_message incomingMessage;
    memcpy(&incomingMessage, incomingData, sizeof(struct_message));
    Serial.print("Received message: ");
    debug(incomingMessage.data, 40);
    // Check if all 0s
    bool allZero = true;
    for (int i = 0; i < 40; i++) {
        if (incomingMessage.data[i] != 0) {
            allZero = false;
            break;
        }
    }
    if (allZero) {
        Serial.println("Received all 0s, pairing with drone");
        stickRecievedKey = true;
        return;
    }
    // Check if a random number
    /* Cryptography code would go here */
    // Send back the random number
    Serial.println("Sending random number back to drone");
    esp_now_send(peerInfo.peer_addr, (uint8_t *)&incomingMessage, sizeof(incomingMessage));
}


void setup()
{
    Serial.begin(9600);
    uECC_set_rng(&RNG);
    Serial.println("Generating key pair");
    const struct uECC_Curve_t *curve = uECC_secp160r1();
    uECC_make_key(publicKey, privateKey, curve);
    // Print out keys
    Serial.println("Public key:");
    debug(publicKey, 40);
    Serial.println("Private key:");
    debug(privateKey, 21);
    Serial.println("Setting up ESP-NOW...");

    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK)
    {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Once ESP-NOW is initialized, we will register for recv CB
    esp_now_register_recv_cb(onDataRecv);

    // Once ESP-NOW is initiated, register the peer
    memset(&peerInfo, 0, sizeof(esp_now_peer_info_t));
    memset(peerInfo.peer_addr, 0xFF, 6); // Broadcast address
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    Serial.println("Registering peer...");
    // Add the peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("Failed to add peer");
        return;
    }
}

void loop()
{
    while (stickRecievedKey == false)
    {
        delay(1000);
        // Register send callback to get the delivery status
        esp_now_register_send_cb(OnDataSent);

        // Fill myData with your data
        for (int i = 0; i < 40; i++)
        {
            keyPayload.data[i] = publicKey[i]; // Example: fill with incremental numbers
        }
        Serial.println("Attempting to send data...");
        // Send data
        esp_now_send(peerInfo.peer_addr, (uint8_t *)&keyPayload, sizeof(keyPayload));
    }
}
