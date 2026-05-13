#include "sdcard.h"
#include <SD.h>
#include <SPI.h>

#define SD_CS_PIN D10

static String filename;

void sdcard_init() {
    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("SD card init failed");
        return;
    }
    // 파일명: YYYY-MM-DD.csv
    // RTC 없이 초기화 시점에 파일명 고정
    filename = "/data.csv";

    // 헤더가 없으면 추가
    if (!SD.exists(filename)) {
        File f = SD.open(filename, FILE_WRITE);
        if (f) {
            f.println("timestamp,co2,temp,humidity,mode,fan_cmd,cover_cmd");
            f.close();
        }
    }
    Serial.println("SD card initialized");
}

void sdcard_log_raw(String timestamp, uint16_t co2, float temp, float humidity) {
    File f = SD.open(filename, FILE_APPEND);
    if (f) {
        f.print(timestamp); f.print(",");
        f.print(co2);       f.print(",");
        f.print(temp);      f.print(",");
        f.print(humidity);  f.print(",");
        f.println(",,");
        f.close();
    }
}

void sdcard_log_decision(String timestamp, uint8_t mode, uint8_t fan_cmd, uint8_t cover_cmd) {
    File f = SD.open(filename, FILE_APPEND);
    if (f) {
        f.print(timestamp); f.print(",");
        f.print(",,,");
        f.print(mode);      f.print(",");
        f.print(fan_cmd);   f.print(",");
        f.println(cover_cmd);
        f.close();
    }
}