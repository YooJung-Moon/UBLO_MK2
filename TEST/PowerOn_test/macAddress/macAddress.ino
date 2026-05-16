#include <WiFi.h>

void setup() {
    Serial.begin(115200);
    
    // 네이티브 USB 시리얼을 사용하는 보드에서 연결을 기다립니다.
    while (!Serial) {
        ; // 연결될 때까지 대기
    }

    WiFi.mode(WIFI_STA);
    Serial.println("\n--- WiFi MAC Address ---");
    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());
}

void loop() {}