#include <WiFi.h>
#include <Arduino.h>

#define TRANSISTOR_BASE_PIN 25

void setup() {
  // Initialize the pin as an output
  pinMode(TRANSISTOR_BASE_PIN, OUTPUT);

  // Initially set the pin to low (off)
  digitalWrite(TRANSISTOR_BASE_PIN, LOW);
}

void loop() {
  // Set the transistor base to high (close the circuit)
  digitalWrite(TRANSISTOR_BASE_PIN, HIGH);
  delay(500);  // Keep the circuit closed for 5 seconds

  // Set the transistor base to low (open the circuit)
  digitalWrite(TRANSISTOR_BASE_PIN, LOW);
  delay(500);  // Keep the circuit open for 5 seconds
}
