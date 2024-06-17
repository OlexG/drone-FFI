#include <WiFi.h>


#define DELTA 3
#define SUM_AMNT 50
#define GATHER_HERTZ 8

const char *ssid = "ESP32-Access-Point"; // SSID of your AP
const char *password = "yourStrongPassword"; // WPA2 passphrase

int16_t get_avg_RSSI(int sum_amnt) {
  int sum = 0;
  for(int i = 0; i < sum_amnt; i++){
    sum += WiFi.RSSI();
    //Serial.println(rssi);
  }
  sum /= sum_amnt;
  return sum;
}

void setup() {

  Serial.begin(9600);
  //Serial.println("Hello world");
  WiFi.begin(ssid, NULL, 1);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi Connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
 
 int16_t rssi_avg = get_avg_RSSI(SUM_AMNT);
 Serial.print("riss_avg = ");
 Serial.println(rssi_avg);
 delay((1000 / GATHER_HERTZ));
}
