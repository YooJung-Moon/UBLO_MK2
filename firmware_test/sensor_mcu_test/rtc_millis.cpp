#include "rtc.h"

void rtc_init() {
    // millis() 기반이라 별도 초기화 없음
}

String rtc_timestamp() {
    unsigned long ms = millis();
    unsigned long s  = ms / 1000;
    unsigned long m  = s / 60;
    unsigned long h  = m / 60;

    char buf[16];
    snprintf(buf, sizeof(buf), "%02lu:%02lu:%02lu", h % 24, m % 60, s % 60);
    return String(buf);
}