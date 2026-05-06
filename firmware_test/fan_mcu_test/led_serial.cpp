#include "led.h"

void led_init() {
    // Serial 출력으로 대체, 별도 초기화 없음
}

void led_set(uint8_t mode) {
    Serial.print("[LED] Current mode: ");
    switch (mode) {
        case 0: Serial.println("AUTO");   break;
        case 1: Serial.println("CLOSED"); break;
        case 2: Serial.println("BREEZE"); break;
        case 3: Serial.println("TURBO");  break;
        default: Serial.println("UNKNOWN"); break;
    }
}