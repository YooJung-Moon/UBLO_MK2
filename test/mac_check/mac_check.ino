#include <WiFi.h>

void setup() {
  delay(3000);  // 추가
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
}

void loop() {}