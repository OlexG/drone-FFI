#include <Arduino.h>

const int buttonPin = 2; 
int buttonState = 0;   
int i = 0;
const int ledPin = 4; 
const int alertButton = 16 ; 

void setup() {
  Serial.begin(9600);

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
}


void loop() {
  buttonState = digitalRead(buttonPin);
  if (buttonState == LOW) {
   
    Serial.println(i);
    i++;
  }
  delay(250); 
  digitalWrite(ledPin, HIGH); 
  delay(1000);             
  digitalWrite(ledPin, LOW); 
  delay(1000);  
}

