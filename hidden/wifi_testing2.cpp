#include <WiFi.h>
#include <Arduino.h>

#define DELTA 2
#define SUM_AMOUNT 10
#define GATHER_HERTZ 16

#define VOLTAGE_READIN_PIN 34

const char *ssid = "ESP32-Access-Point";     // SSID of your AP
const char *password = "yourStrongPassword"; // WPA2 passphrase

const uint8_t p_button = 2;
const uint8_t p_buzzer = 27;
const uint8_t p_green_LED = 5;
const uint8_t p_red_LED = 4;

int buttonState = 0;
int32_t passed_counter = 0;

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

void flickerLED(int numFlickers, int ledPin, int delayValue, bool endingState) {
    for (int i = 0; i < numFlickers; i++) {
        digitalWrite(ledPin, HIGH);
        delay(delayValue);
        digitalWrite(ledPin, LOW);
        delay(delayValue);
    }
    digitalWrite(ledPin, endingState ? HIGH : LOW);
}

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

void setup()
{

    Serial.begin(9600);

    pinMode(p_button, INPUT_PULLUP); // button
    pinMode(p_red_LED, OUTPUT);      // indicator LED
    pinMode(p_green_LED, OUTPUT);  
    delay(500);

    pinMode(VOLTAGE_READIN_PIN, INPUT); // ADC
    // visual indicator to display when both ESP32 are ON
    Serial.print("VOLTAGE_READ_PIN: ");
    Serial.println(analogRead(VOLTAGE_READIN_PIN));
    if (analogRead(VOLTAGE_READIN_PIN) > 2000)
    {
        flickerLED(8, p_red_LED, 100, true);
    }

    WiFi.begin(ssid, NULL, 1);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("WiFi Connected.");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void loop()
{

    int32_t rssi_avg = get_avg_RSSI(SUM_AMOUNT);
    // Serial.print("riss_avg = ");
    // Serial.println(rssi_avg);
    delay(5);
    if (Serial.available() > 0)
    {
        // Read the incoming integer
        int32_t rx_value = Serial.parseInt();

        /*Serial.print(receivedNumber);
        Serial.print(", ");
        Serial.println(rssi_avg);*/

        Serial.print("delta = ");
        Serial.println(rx_value - rssi_avg);
        delay((1000 / GATHER_HERTZ));
        buttonState = digitalRead(p_button);
        //Serial.println(digitalRead(p_button));
        Serial.print("counter = ");
        Serial.println(passed_counter);

        //Serial.println(abs(rx_value - rssi_avg));
        if (abs(rx_value - rssi_avg) > DELTA)
        {
            passed_counter = 0;
        }
        else if (buttonState == LOW)
        {
            passed_counter++;
        }
        if (passed_counter > 5)
        {
            play_sound(p_buzzer);
            flickerLED(8, p_green_LED, 100, false);
            passed_counter = 0;
        }     
    }
}
