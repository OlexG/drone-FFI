#include <WiFi.h>

const char *ssid = "ESP32-Access-Point"; // Name of your network
const char *password = "yourStrongPassword"; // Your WPA2 passphrase, must be at least 8 characters long

void setup() {
  Serial.begin(9600);

  // Setting the ESP32 as an Access Point with WPA2 security
  WiFi.softAP(ssid, NULL);

  Serial.println("Access Point Started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  // Nothing to do here
}