#include <Wire.h>
#include <RTClib.h>
#include <SPI.h>
#include <SD.h>

RTC_DS3231 rtc;
const int chipSelect = 4; // SD 카드 모듈의 CS 핀 번호 (보드에 맞게 수정)

void setup() {
    delay(3000);
    Serial.begin(115200);
    
    // 1. RTC 초기화
    if (!rtc.begin()) {
        Serial.println("RTC init failed");
        while (true);
    }

    // 2. SD 카드 초기화
    if (!SD.begin(chipSelect)) {
        Serial.println("SD card init failed or not present");
        // SD 카드가 없어도 동작은 계속하도록 중단하지 않음
    } else {
        Serial.println("SD card initialized.");
    }

    // 3. Smart Sync 로직 (11초 보정 포함)
    DateTime now = rtc.now();
    DateTime compiled = DateTime(F(__DATE__), F(__TIME__));
    
    // RTC 시간이 컴파일 시간보다 과거라면 (업로드 직후 또는 배터리 방전 시)
    if (now < compiled) {
        Serial.println("RTC is outdated. Synchronizing with offset...");
        // 컴파일 시간에 업로드 지연 시간(약 11초)을 더함
        rtc.adjust(compiled + TimeSpan(0, 0, 0, 11));
    } else {
        Serial.println("RTC is already up-to-date.");
    }

    Serial.println("System Ready.");
    Serial.println("Date, Time, Data...");
}

void loop() {
    DateTime now = rtc.now();
    
    // 시간 문자열 생성
    char buf[25];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
        now.year(), now.month(), now.day(),
        now.hour(), now.minute(), now.second());

    // 시리얼 출력
    Serial.println(buf);

    // 4. SD 카드 로깅 (안전 장치 포함)
    File dataFile = SD.open("datalog.txt", FILE_WRITE);
    if (dataFile) {
        dataFile.print(buf);
        dataFile.println(", Sample Data: 123"); // 실제 데이터 입력 부분
        dataFile.close(); // 파일 닫기 (데이터 안전 저장)
    } else {
        // SD 카드가 빠졌을 때 시리얼로 경고 출력
        Serial.println("Error opening datalog.txt (SD card removed?)");
    }

    delay(1000);
}