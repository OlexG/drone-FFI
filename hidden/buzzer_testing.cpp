#include <Arduino.h>

#define BUZZER_PIN 27 

void playTone(int tonePin, int frequency, int duration) {
  tone(tonePin, frequency, duration);
  delay(duration + 50); 
}

void play_granted_code() {
  playTone(BUZZER_PIN, 500, 200); 
  playTone(BUZZER_PIN, 1000, 200); 
  playTone(BUZZER_PIN, 1500, 200);
}

void setup() {

}

void loop() {
  play_granted_code(); 
  delay(2000); 
}
