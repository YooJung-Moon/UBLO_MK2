#include "led.h"

// Arduino Nano ESP32 내장 RGB LED 핀
#define LED_RED   46
#define LED_GREEN 45
#define LED_BLUE  48

void led_init() {
    pinMode(LED_RED,   OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE,  OUTPUT);
    // 꺼진 상태로 초기화 (LOW = ON, HIGH = OFF — active low)
    digitalWrite(LED_RED,   HIGH);
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_BLUE,  HIGH);
}

void led_set(uint8_t mode) {
    switch (mode) {
        case 0: // AUTO — 초록
            digitalWrite(LED_RED,   HIGH);
            digitalWrite(LED_GREEN, LOW);
            digitalWrite(LED_BLUE,  HIGH);
            break;
        case 1: // CLOSED — 빨강
            digitalWrite(LED_RED,   LOW);
            digitalWrite(LED_GREEN, HIGH);
            digitalWrite(LED_BLUE,  HIGH);
            break;
        case 2: // BREEZE — 파랑
            digitalWrite(LED_RED,   HIGH);
            digitalWrite(LED_GREEN, HIGH);
            digitalWrite(LED_BLUE,  LOW);
            break;
        case 3: // TURBO — 흰색
            digitalWrite(LED_RED,   LOW);
            digitalWrite(LED_GREEN, LOW);
            digitalWrite(LED_BLUE,  LOW);
            break;
        default:
            digitalWrite(LED_RED,   HIGH);
            digitalWrite(LED_GREEN, HIGH);
            digitalWrite(LED_BLUE,  HIGH);
            break;
    }
}