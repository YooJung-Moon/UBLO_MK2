#include "rtc.h"
#include <RTClib.h>

#define UPLOAD_OFFSET_SEC 18

static RTC_DS3231 rtc;

void rtc_init() {
    if (!rtc.begin()) {
        Serial.println("RTC init failed");
        return;
    }
    DateTime compileTime = DateTime(F(__DATE__), F(__TIME__));
    rtc.adjust(DateTime(compileTime.unixtime() + UPLOAD_OFFSET_SEC));
    Serial.println("RTC initialized");
}

String rtc_timestamp() {
    DateTime now = rtc.now();
    char buf[20];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
        now.year(), now.month(), now.day(),
        now.hour(), now.minute(), now.second());
    return String(buf);
}