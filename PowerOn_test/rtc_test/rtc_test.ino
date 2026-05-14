#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

void setup() {
    // 시리얼 모니터 연결 대기
    delay(3000);
    Serial.begin(115200);
    unsigned long start = millis();
    while (!Serial && millis() - start < 3000);

    Wire.begin();

    // RTC 초기화 확인
    if (!rtc.begin()) {
        Serial.println("RTC init failed!");
        while (true);
    }

    /* 
     * [해결 포인트] 
     * 아래 줄의 주석을 해제하고 업로드하면 즉시 현재 PC 시간으로 동기화됩니다.
     * 한 번 동기화된 후에는 다시 이 줄을 주석 처리하여 업로드하는 것이 좋습니다.
     */

    // Serial.println("Force updating RTC time to compile time...");
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); 

    // 배터리 끊김 감지 상태 확인 (참고용)
    if (rtc.lostPower()) {
        Serial.println("RTC previously lost power!");
    }

    Serial.println("RTC Initialized and Synced.");
    Serial.println("Format: YYYY-MM-DD HH:MM:SS");
    Serial.println("---------------------------");
}

void loop() {
    DateTime now = rtc.now();
    
    // 문자열 버퍼를 이용한 깔끔한 출력
    char buf[25];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
             now.year(), now.month(), now.day(),
             now.hour(), now.minute(), now.second());
    
    Serial.println(buf);
    
    delay(1000); // 1초마다 출력
}