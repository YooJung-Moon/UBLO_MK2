#include <SD.h>
#include <SPI.h>

#define SD_CS_PIN D10

void setup() {
    Serial.begin(115200);
    delay(3000);

    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("SD init failed");
        return;
    }
    Serial.println("SD initialized");

    // 테스트할 폴더명들
    const char* names[] = {
        "/AB",           // 2자
        "/ABCDEFGH",     // 8자
        "/ABCDEFGHI",    // 9자 (8.3 초과)
        "/206EF1312D84", // 12자 (MAC 전체)
        "/2D84"          // 4자 (MAC 끝 4자)
    };

    for (int i = 0; i < 5; i++) {
        // 폴더 생성 시도
        bool dir_ok = SD.mkdir(names[i]);
        Serial.print("mkdir ");
        Serial.print(names[i]);
        Serial.print(" → ");
        Serial.println(dir_ok ? "OK" : "FAIL");

        if (dir_ok) {
            // 파일 생성 시도
            String filepath = String(names[i]) + "/test.csv";
            File f = SD.open(filepath, FILE_WRITE);
            Serial.print("  open ");
            Serial.print(filepath);
            Serial.print(" → ");
            if (f) {
                f.println("timestamp,co2,temp,humidity");
                f.close();
                Serial.println("OK");
            } else {
                Serial.println("FAIL");
            }
        }
    }
}

void loop() {}