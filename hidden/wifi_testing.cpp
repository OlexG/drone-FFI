#include <WiFi.h>
#include <Arduino.h>

#define DELTA 3
#define SUM_AMOUNT 10
#define GATHER_HERTZ 16

const uint8_t p_orange_LED = 4;

const char *ssid = "ESP32-Access-Point";     // SSID of your AP
const char *password = "yourStrongPassword"; // WPA2 passphrase

int32_t get_avg_RSSI(const int sum_amnt)
{
    int32_t sum = 0;
    for (int i = 0; i < sum_amnt; i++)
    {
        sum += WiFi.RSSI();
        // Serial.println(rssi);
    }
    return (sum / sum_amnt);
}

void setup()
{
    Serial.begin(9600);

    // Connect to the WPA2-secured Access Point
    WiFi.begin(ssid, NULL, 1);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print("."); // Print dots to indicate ongoing connection attempts
    }

    Serial.println("WiFi Connected.");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void loop()
{
    int32_t rssi = get_avg_RSSI(SUM_AMOUNT);
    Serial.print(rssi);
    // Serial.print("\n");
    delay((1000 / GATHER_HERTZ));
}