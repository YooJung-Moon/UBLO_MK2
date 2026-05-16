#include "rtc.h"
#include <RTClib.h>

#define UPLOAD_OFFSET_SEC 18

static RTC_DS3231 rtc;

void rtc_init() {
    if (!rtc.begin()) {
        Serial.println("RTC init failed");
        return;
    }

/////////////////////////
    /*
     * [1단계] 첫 번째 업로드 시 주석 해제
     *         시각을 컴파일 시점 + UPLOAD_OFFSET_SEC 으로 강제 설정
     */
    // DateTime compileTime = DateTime(F(__DATE__), F(__TIME__));
    // rtc.adjust(DateTime(compileTime.unixtime() + UPLOAD_OFFSET_SEC));

    /*
     * [2단계] 두 번째 업로드 시 주석 해제
     *         배터리 방전 시에만 시각 재설정, 리셋/전원 껐다 켜도 시각 유지
     */
    if (rtc.lostPower()) {
        DateTime compileTime = DateTime(F(__DATE__), F(__TIME__));
        rtc.adjust(DateTime(compileTime.unixtime() + UPLOAD_OFFSET_SEC));
    }

 //////////////////////////   

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