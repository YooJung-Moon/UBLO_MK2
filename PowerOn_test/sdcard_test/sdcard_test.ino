#include <SD.h>
#include <SPI.h>

#define SD_CS_PIN D10

void setup() {
    delay(3000);
    Serial.begin(115200);
    unsigned long start = millis();
    while (!Serial && millis() - start < 3000);

    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("SD card init failed");
        while (true);
    }
    Serial.println("SD card initialized");

    // 파일 쓰기 테스트
    File f = SD.open("/test.csv", FILE_WRITE);
    if (f) {
        f.println("timestamp,co2,temp,humidity");
        f.println("2026-05-01 10:00:00,843,22.1,55.0");
        f.close();
        Serial.println("[SD] write OK");
    } else {
        Serial.println("[SD] write failed");
    }

    // 파일 읽기 테스트
    File r = SD.open("/test.csv");
    if (r) {
        Serial.println("[SD] read:");
        while (r.available()) {
            Serial.write(r.read());
        }
        r.close();
    } else {
        Serial.println("[SD] read failed");
    }
}

void loop() {}