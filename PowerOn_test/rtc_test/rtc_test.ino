#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

void setup() {
    delay(3000);
    Serial.begin(115200);
    unsigned long start = millis();
    while (!Serial && millis() - start < 3000);

    Wire.begin();

    if (!rtc.begin()) {
        Serial.println("RTC init failed!");
        while (true);
    }

    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  // 주석 처리

    // 첫 번째 업로드 이후 경과 시간 보정
    DateTime now = rtc.now();
    rtc.adjust(DateTime(now.unixtime() + 11));

    if (rtc.lostPower()) {
        Serial.println("RTC previously lost power!");
    }

    Serial.println("RTC Initialized and Synced.");
    Serial.println("Format: YYYY-MM-DD HH:MM:SS");
    Serial.println("---------------------------");
}

void loop() {
    DateTime now = rtc.now();

    char buf[25];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
             now.year(), now.month(), now.day(),
             now.hour(), now.minute(), now.second());

    Serial.println(buf);

    delay(1000);
}