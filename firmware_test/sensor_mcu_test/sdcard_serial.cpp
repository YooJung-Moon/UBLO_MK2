#include "sdcard.h"

void sdcard_init() {
    // Serial 출력으로 대체, 별도 초기화 없음
}

void sdcard_log_raw(String timestamp, uint16_t co2, float temp, float humidity) {
    Serial.print(timestamp);
    Serial.print(", ");
    Serial.print(co2);
    Serial.print(", ");
    Serial.print(temp);
    Serial.print(", ");
    Serial.print(humidity);
    Serial.print(", , ,");  // mode, fan_cmd, cover_cmd 빈칸
    Serial.println();
}

void sdcard_log_decision(String timestamp, uint8_t mode, uint8_t fan_cmd, uint8_t cover_cmd) {
    Serial.print(timestamp);
    Serial.print(", , , , ");  // co2, temp, humidity 빈칸
    Serial.print(mode);
    Serial.print(", ");
    Serial.print(fan_cmd);
    Serial.print(", ");
    Serial.println(cover_cmd);
}