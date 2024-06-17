#include <WiFi.h>
#include <Arduino.h>

const int adcPin = 34; // ADC pin to read the voltage
const float referenceVoltage = 3.3; // Reference voltage of ESP32 ADC
const int adcResolution = 4095; // 12-bit ADC resolution

void setup() {
  Serial.begin(9600);
}

void loop() {
  int adcValue = analogRead(adcPin);
  float voltage = (adcValue * referenceVoltage) / adcResolution;

  Serial.print("ADC Value: ");
  Serial.print(adcValue);
  Serial.print(" - Voltage: ");
  Serial.println(voltage);

  delay(1000); // Read the voltage every second
}
