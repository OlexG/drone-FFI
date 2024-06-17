#include <WiFi.h>
#include <Arduino.h>

const int pin1 = 13; // First GPIO pin
const int pin2 = 14; // Second GPIO pin

void setup() {
  // Initialize the GPIO pins as outputs

    Serial.begin(9600);

  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);

  // Set the GPIO pins high
  digitalWrite(pin1, HIGH);
  digitalWrite(pin2, HIGH);

    Serial.print("Succesfully using DACs \n");
}

void loop() {
  // Nothing needed in the loop for this task
    Serial.print("Succesfully using DACs \n");

}
