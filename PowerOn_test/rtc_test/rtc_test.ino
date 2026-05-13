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
        Serial.println("RTC init failed");
        while (true);
    }

    if (rtc.lostPower()) {
        Serial.println("RTC lost power, setting time");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    Serial.println("RTC initialized");
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