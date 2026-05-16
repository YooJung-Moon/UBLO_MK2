#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>
#include <esp_system.h>

#define UPLOAD_OFFSET_SEC 18

RTC_DS3231 rtc;

void setup() {
    delay(3000);
    Serial.begin(115200);
    unsigned long start = millis();
    while (!Serial && millis() - start < 3000);

    Wire.begin();

    if (!rtc.begin()) {
        Serial.println("RTC init failed");
        while (true);
    }

    esp_reset_reason_t reason = esp_reset_reason();
    Serial.print("Reset reason: ");
    switch (reason) {
        case ESP_RST_SW:      Serial.println("SW");      break;
        case ESP_RST_POWERON: Serial.println("POWERON"); break;
        default:              Serial.println("OTHER");   break;
    }

    if (reason == ESP_RST_SW) {
        DateTime compileTime = DateTime(F(__DATE__), F(__TIME__));
        rtc.adjust(DateTime(compileTime.unixtime() + UPLOAD_OFFSET_SEC));
        Serial.println("RTC synced");
    } else {
        Serial.println("RTC time kept");
    }

    Serial.println("RTC initialized");
    Serial.println("Format: YYYY-MM-DD HH:MM:SS");
    Serial.println("---------------------------");
}

void loop() {
    DateTime now = rtc.now();
    char buf[20];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
        now.year(), now.month(), now.day(),
        now.hour(), now.minute(), now.second());
    Serial.println(buf);
    delay(1000);
}