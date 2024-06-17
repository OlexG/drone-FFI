#include <WiFi.h>
#include <Arduino.h>

// Define the built-in LED pin for your board (common for ESP32 is GPIO2)
#define BUILTIN_LED 27

void setup() {
  pinMode(BUILTIN_LED, OUTPUT); // Initialize the LED pin as an output
}

void loop() {
  digitalWrite(BUILTIN_LED, HIGH); // Turn the LED on
  delay(1000);                     // Wait for 1 second
  digitalWrite(BUILTIN_LED, LOW);  // Turn the LED off
  delay(1000);                     // Wait for 1 second
}
